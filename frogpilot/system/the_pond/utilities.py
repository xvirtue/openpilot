#!/usr/bin/env python3
import base64
import json
import os
import re
import secrets
import subprocess
import time
import uuid

from datetime import datetime
from pathlib import Path
from PIL import Image
from pydub import AudioSegment
from typing import List
from werkzeug.utils import secure_filename

from openpilot.common.conversions import Conversions as CV
from openpilot.system.loggerd.config import get_available_bytes, get_used_bytes
from openpilot.system.loggerd.deleter import PRESERVE_ATTR_NAME, PRESERVE_ATTR_VALUE
from openpilot.system.loggerd.uploader import listdir_by_creation
from openpilot.tools.lib.route import SegmentName

from openpilot.frogpilot.common.frogpilot_variables import THEME_SAVE_PATH, params, params_tracking

LOG_CANDIDATES = [
  "qlog",
  "qlog.bz2",
  "rlog",
  "rlog.bz2",
  "raw_log.bz2",
]

SEGMENT_RE = re.compile(r"^[0-9a-fA-F]{8}--[0-9a-fA-F]{10}--\d+$")

TARGET_LOUDNESS = -15.0

XOR_KEY = "s8#pL3*Xj!aZ@dWq"

def covert_audio(input_file):
  sound = AudioSegment.from_file(input_file)
  sound = sound.set_frame_rate(48000)
  sound = sound.set_channels(1)

  output_filename = os.path.splitext(input_file)[0] + ".wav"
  sound.export(output_filename, format="wav", parameters=["-acodec", "pcm_s16le"])

  if input_file != output_filename:
    os.remove(input_file)

def create_theme(form_data, files, temporary=False):
  theme_name = form_data.get("themeName")
  if not theme_name:
    return None, "Theme name is required."

  sane_theme_name = secure_filename(theme_name.replace(" ", "_"))

  if temporary:
    base_path = Path(f"/tmp/{sane_theme_name}_{secrets.token_hex(8)}")
  else:
    base_path = THEME_SAVE_PATH / "theme_packs"

  theme_path = base_path / sane_theme_name
  theme_path.mkdir(parents=True, exist_ok=True)

  (theme_path / "user_created").touch()

  save_checklist_str = form_data.get("saveChecklist")
  save_checklist = json.loads(save_checklist_str) if save_checklist_str else {
    "colors": True, "distance_icons": True, "icons": True, "sounds": True, "turn_signals": True
  }

  if save_checklist.get("colors"):
    (theme_path / "colors").mkdir(exist_ok=True)
    colors_str = form_data.get("colors")
    if colors_str:
      color_data = json.loads(colors_str)
      with open(theme_path / "colors" / "colors.json", "w") as f:
        json.dump(color_data, f, indent=2)

  if save_checklist.get("turn_signals"):
    (theme_path / "signals").mkdir(exist_ok=True)
    turn_signal_length = form_data.get("turnSignalLength")
    turn_signal_style = form_data.get("turnSignalStyle", "Traditional")
    if turn_signal_length:
      time_file_name = f"{turn_signal_style.lower()}_{turn_signal_length}"
      (theme_path / "signals" / time_file_name).touch()

    turn_signal_type = form_data.get("turnSignalType", "Single Image").lower()
    if turn_signal_type == "sequential":
      for f in (theme_path / "signals").glob("turn_signal.*"):
        f.unlink()
      sequential_keys = [k for k in files.keys() if k.startswith("turn_signal_")]
      if sequential_keys:
        for f in (theme_path / "signals").glob("turn_signal_*"):
          f.unlink()
        sequential_keys.sort(key=lambda name: int(name.split('_')[-1]))
        for key in sequential_keys:
          file = files[key]
          if file.filename:
            ext = Path(file.filename).suffix
            save_path = theme_path / "signals" / f"{key}{ext}"
            file.save(save_path)
    else:
      for f in (theme_path / "signals").glob("turn_signal_*"):
        f.unlink()

  if save_checklist.get("icons"):
    (theme_path / "icons").mkdir(exist_ok=True)
    if "steeringWheel" in files and files['steeringWheel'].filename:
      wheel_file = files['steeringWheel']
      ext = Path(wheel_file.filename).suffix
      steering_wheel_save_path = THEME_SAVE_PATH / "steering_wheels"
      wheel_save_path = steering_wheel_save_path / f"{sane_theme_name}{ext}"
      steering_wheel_save_path.mkdir(exist_ok=True)
      for f in steering_wheel_save_path.glob(f"{sane_theme_name}.*"):
        f.unlink()
      wheel_file.save(wheel_save_path)
      img = Image.open(wheel_save_path)
      img = img.resize((250, 250), Image.Resampling.LANCZOS)
      img.save(wheel_save_path)

    file_map = {
      "settingsButton": (theme_path / "icons", "button_settings", (169, 104)),
      "homeButton": (theme_path / "icons", "button_home", (250, 250)),
    }

    for field, (dest_path, base_name, resize) in file_map.items():
      if field in files:
        file = files[field]
        if file.filename:
          for existing_file in dest_path.glob(f"{base_name}.*"):
            if existing_file.is_file():
              existing_file.unlink()
          ext = Path(file.filename).suffix
          save_path = dest_path / f"{base_name}{ext}"
          file.save(save_path)
          if resize:
            img = Image.open(save_path)
            img = img.resize(resize, Image.Resampling.LANCZOS)
            img.save(save_path)

  if save_checklist.get("distance_icons"):
    (theme_path / "distance_icons").mkdir(exist_ok=True)
    distance_icon_files = ["traffic", "aggressive", "standard", "relaxed"]
    for icon_name in distance_icon_files:
      file_key = f"distanceIcons_{icon_name}"
      if file_key in files:
        file = files[file_key]
        if file.filename:
          ext = Path(file.filename).suffix
          save_path = theme_path / "distance_icons" / f"{icon_name.lower()}{ext}"
          file.save(save_path)

  if save_checklist.get("sounds"):
    (theme_path / "sounds").mkdir(exist_ok=True)
    sound_files = ["engage", "disengage", "prompt_repeat", "startup"]
    for sound_key in sound_files:
      if sound_key in files:
        file = files[sound_key]
        if file.filename:
          ext = Path(file.filename).suffix
          temp_sound_path = theme_path / "sounds" / f"{sound_key}{ext}"
          file.save(temp_sound_path)
          covert_audio(str(temp_sound_path))

  if save_checklist.get("turn_signals"):
    file_map_signals = {
        "turnSignal": (theme_path / "signals", "turn_signal", None),
        "turnSignalBlindspot": (theme_path / "signals", "turn_signal_blindspot", None),
    }

    for field, (dest_path, base_name, resize) in file_map_signals.items():
      if field == "turnSignal" and turn_signal_type == "sequential":
        continue
      if field in files:
        file = files[field]
        if file.filename:
          for existing_file in dest_path.glob(f"{base_name}.*"):
            if existing_file.is_file():
              existing_file.unlink()
          ext = Path(file.filename).suffix
          save_path = dest_path / f"{base_name}{ext}"
          file.save(save_path)
          if resize:
            img = Image.open(save_path)
            img = img.resize(resize, Image.Resampling.LANCZOS)
            img.save(save_path)

  return theme_path, None

def decode_parameters(encoded_string):
  obfuscated_data = base64.b64decode(encoded_string.encode("utf-8")).decode("utf-8")
  decrypted_data = xor_encrypt_decrypt(obfuscated_data, XOR_KEY)
  return json.loads(decrypted_data)

def encode_parameters(params_dict):
  serialized_data = json.dumps(params_dict)
  obfuscated_data = xor_encrypt_decrypt(serialized_data, XOR_KEY)
  encoded_data = base64.b64encode(obfuscated_data.encode("utf-8")).decode("utf-8")
  return encoded_data

def ffmpeg_mp4_wrap_process_builder(filename):
  is_raw_hevc = filename.rsplit(".", 1)[-1] == "hevc"

  command = [
    "ffmpeg",
    "-hide_banner",
    "-loglevel", "error",
    "-probesize", "1M",
    "-analyzeduration", "1M",
    *(["-f", "hevc"] if is_raw_hevc else []),
    "-i", filename,
    "-c", "copy",
    "-map", "0",
    *(["-vtag", "hvc1"] if is_raw_hevc else []),
    "-f", "mp4",
    "-movflags", "empty_moov",
    "-"
  ]

  return subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

def format_git_date(raw_date: str):
  date_object = datetime.strptime(raw_date.split()[1], "%Y-%m-%d")

  day = date_object.day
  suffix = "th" if 11 <= day <= 13 else {1: "st", 2: "nd", 3: "rd"}.get(day % 10, "th")

  return date_object.strftime(f"%B {day}{suffix}, %Y")

def get_all_segment_names(footage_path):
  entries = listdir_by_creation(footage_path)

  segment_names = []
  for entry in entries:
    if not SEGMENT_RE.fullmatch(entry):
      continue

    segment_names.append(segment_to_segment_name(footage_path, entry))
  return segment_names

def get_available_cameras(segment_path):
  segment_path = Path(segment_path)
  return [
    name for name, file in {
      "driver": "dcamera.hevc",
      "forward": "fcamera.hevc",
      "wide": "ecamera.hevc"
    }.items() if (segment_path / file).exists()
  ]

def get_disk_usage():
  free = get_available_bytes()
  used = get_used_bytes()
  total = used + free

  def to_gb(b):
    return f"{b // (2**30)} GB"

  return [{
    "free": to_gb(free),
    "size": to_gb(total),
    "used": to_gb(used),
    "usedPercentage": f"{(used / total) * 100:.2f}%"
  }]

def get_drive_stats():
  stats = json.loads(params.get("ApiCache_DriveStats", encoding="utf-8") or "{}")

  is_metric = params.get_bool("IsMetric")
  conversion = 1 if is_metric else CV.KPH_TO_MPH
  unit = "kilometers" if is_metric else "miles"

  def process(timeframe):
    data = stats.get(timeframe, {})
    return {
      "distance": data.get("distance", 0) * conversion,
      "drives": data.get("routes", 0),
      "hours": data.get("minutes", 0) / 60,
      "unit": unit
    }

  stats["all"] = process("all")
  stats["week"] = process("week")
  stats["frogpilot"] = {
    "distance": params_tracking.get_int("FrogPilotKilometers") * conversion,
    "hours": params_tracking.get_int("FrogPilotMinutes") / 60,
    "drives": params_tracking.get_int("FrogPilotDrives"),
    "unit": unit
  }

  return stats

def get_repo_owner(git_normalized_origin):
  parts = git_normalized_origin.split("/")
  return parts[1] if len(parts) >= 2 else "unknown"

def get_route_start_time(log_file_path):
  if not os.path.exists(log_file_path):
    return None

  creation_time = os.path.getctime(log_file_path)
  return datetime.fromtimestamp(creation_time)

def get_routes_names(footage_path):
  segments = get_all_segment_names(footage_path)
  route_times = {segment.route_name.time_str for segment in segments}
  return sorted(route_times, reverse=True)

def get_segments_in_route(route_time_str, footage_path):
  return [
    f"{segment.time_str}--{segment.segment_num}"
    for segment in get_all_segment_names(footage_path)
    if segment.time_str == route_time_str
  ]

def get_video_duration(input_path):
  try:
    result = subprocess.run([
      "ffprobe", "-v", "error", "-show_entries", "format=duration",
      "-of", "default=noprint_wrappers=1:nokey=1", str(input_path)
    ], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=True)
    return float(result.stdout)
  except (ValueError, subprocess.CalledProcessError):
    return 60

def has_preserve_attr(path: str):
  return PRESERVE_ATTR_NAME in os.listxattr(path) and os.getxattr(path, PRESERVE_ATTR_NAME) == PRESERVE_ATTR_VALUE

def list_file(path):
  return sorted(os.listdir(path), reverse=True)

def process_route(footage_path, route_name):
  segment_path = f"{footage_path}{route_name}--0"
  qcamera_path = f"{segment_path}/qcamera.ts"
  rlog_path = f"{segment_path}/rlog"

  png_output_path = os.path.join(segment_path, "preview.png")
  if not os.path.exists(png_output_path):
    video_to_png(qcamera_path, png_output_path)

  custom_name = None
  if os.path.isdir(segment_path):
    for item in os.listdir(segment_path):
      if not item.endswith((".hevc", ".ts", ".png", ".gif")) and item not in LOG_CANDIDATES:
        custom_name = item
        break

  route_timestamp_str = custom_name
  if not custom_name:
    route_timestamp_dt = get_route_start_time(rlog_path)
    route_timestamp_str = route_timestamp_dt.isoformat() if route_timestamp_dt else None

  return {
    "name": route_name,
    "gif": f"/thumbnails/{route_name}--0/preview.gif",
    "png": f"/thumbnails/{route_name}--0/preview.png",
    "timestamp": route_timestamp_str,
    "is_preserved": has_preserve_attr(segment_path)
  }

def process_route_gif(footage_path, route_name):
  segment_path = f"{footage_path}{route_name}--0"
  qcamera_path = f"{segment_path}/qcamera.ts"
  gif_output_path = os.path.join(segment_path, "preview.gif")

  if not os.path.exists(gif_output_path):
    video_to_gif(qcamera_path, gif_output_path)

def process_screen_recording(mp4):
  stem = mp4.with_suffix("")
  png_path = stem.with_suffix(".png")

  if not png_path.exists():
    video_to_png(mp4, png_path)

  is_custom_name = False
  try:
    datetime.strptime(stem.name, "%B_%d_%Y-%I-%M%p")
  except ValueError:
    is_custom_name = True

  return {
    "filename": mp4.name,
    "gif": f"/screen_recordings/{stem.with_suffix('.gif').name}",
    "png": f"/screen_recordings/{png_path.name}",
    "timestamp": datetime.fromtimestamp(mp4.stat().st_mtime).isoformat(),
    "is_custom_name": is_custom_name
  }

def process_screen_recording_gif(mp4):
  stem = mp4.with_suffix("")
  gif_path = stem.with_suffix(".gif")
  if not gif_path.exists():
    video_to_gif(mp4, gif_path)

def run_ffmpeg(args):
  process = subprocess.Popen(["ffmpeg", "-hide_banner", "-loglevel", "error"] + args, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
  stdout, stderr = process.communicate()
  return stdout

def segment_to_segment_name(data_dir, segment):
  full_path = os.path.join(data_dir, f"FakeDongleID1337|{segment}")
  return SegmentName(full_path)

def video_to_gif(input_path, output_path):
  output_path = Path(output_path)
  sped_up_path = output_path.with_suffix(f".{uuid.uuid4()}.spedup.mp4")

  run_ffmpeg(["-i", str(input_path), "-an", "-vf", "setpts=PTS/35", str(sped_up_path)])
  run_ffmpeg(["-i", str(sped_up_path), "-loop", "0", str(output_path)])

  if os.path.exists(sped_up_path):
    os.remove(sped_up_path)

def video_to_png(input_path, output_path):
  run_ffmpeg([
    "-ss", str(get_video_duration(input_path) / 2),
    "-i", str(input_path),
    "-frames:v", "1",
    str(output_path)
  ])

def xor_encrypt_decrypt(data, key):
  return "".join(chr(ord(c) ^ ord(key[i % len(key)])) for i, c in enumerate(data))
