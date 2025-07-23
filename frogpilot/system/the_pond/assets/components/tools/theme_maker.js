import { html, reactive } from "https://esm.sh/@arrow-js/core";
import { Modal } from "/assets/components/modal.js";

const defaultColors = {
  LaneLines: { red: 23, green: 134, blue: 68, alpha: 255 },
  LeadMarker: { red: 23, green: 134, blue: 68, alpha: 255 },
  Path: { red: 23, green: 134, blue: 68, alpha: 255 },
  PathEdge: { red: 18, green: 107, blue: 54, alpha: 255 },
  Sidebar1: { red: 23, green: 134, blue: 68, alpha: 255 },
  Sidebar2: { red: 23, green: 134, blue: 68, alpha: 255 },
  Sidebar3: { red: 23, green: 134, blue: 68, alpha: 255 },
};

const COLOR_LABELS = {
  LaneLines: "Lane Lines",
  LeadMarker: "Lead Marker",
  Path: "Path",
  PathEdge: "Path Edge",
  Sidebar1: "Sidebar Top",
  Sidebar2: "Sidebar Middle",
  Sidebar3: "Sidebar Bottom",
};

const ICON_LABELS = {
  homeButton: "Home Button",
  settingsButton: "Settings Button",
  steeringWheel: "Steering Wheel",
};

const SOUND_DEFINITIONS = [
  { key: "disengage", label: "Disengage Sound" },
  { key: "engage", label: "Engage Sound" },
  { key: "prompt_repeat", label: "Prompt Sound" },
  { key: "startup", label: "Startup Sound" },
];

const fileStore = {
  images: { distanceIcons: {} },
  sounds: {},
  sequentialFiles: [],
};

const state = reactive({
  themeName: "",
  discordUsername: "",
  turnSignalStyle: "Static",
  turnSignalLength: 100,
  turnSignalType: "Single Image",
  sequentialImages: [],
  availableThemes: [],
  themeToDelete: null,
  selectorAction: null,
  isSubmitting: false,
  themeSubmitted: false,
  colors: { ...defaultColors },
  imageFileNames: {
    homeButton: "",
    settingsButton: "",
    steeringWheel: "",
    turnSignal: "",
    turnSignalBlindspot: "",
    distanceIcons: {
      traffic: "",
      aggressive: "",
      standard: "",
      relaxed: "",
    },
  },
  soundFileNames: {
    startup: "",
    prompt_repeat: "",
    engage: "",
    disengage: "",
  },
  saveChecklist: {
    colors: true,
    distance_icons: true,
    icons: true,
    sounds: true,
    turn_signals: true,
  },
  showTurnSignalHelp: false,
  showThemeSelector: false,
  showSubmitConfirmation: false,
  showDeleteConfirmation: false,
  showSequenceManager: false,
  showSaveConfirmModal: false,
});

const fetchFileAsObject = async (url, fileName) => {
  try {
    const response = await fetch(url);
    if (!response.ok) {
        console.error(`Failed to fetch ${url}: ${response.statusText}`);
        return null;
    }
    const blob = await response.blob();
    if (blob.type.includes('html')) {
        console.error(`Failed to fetch ${url}: Received HTML instead of an asset.`);
        return null;
    }
    const extension = fileName.split('.').pop() || blob.type.split('/')[1];
    return new File([blob], `${fileName}.${extension}`, { type: blob.type });
  } catch(e) {
    console.error(`Error fetching file ${url}:`, e);
    return null;
  }
};

const populateAssetNames = (themeData = {}) => {
  const { images = {}, sounds = {} } = themeData;
  const { distanceIcons = {} } = images;

  for (const key in state.imageFileNames) {
    if (key === 'distanceIcons') {
      for (const subkey in state.imageFileNames.distanceIcons) {
        state.imageFileNames.distanceIcons[subkey] = distanceIcons[subkey] || "";
      }
    } else {
      state.imageFileNames[key] = images[key] || "";
    }
  }

  for (const key in state.soundFileNames) {
    state.soundFileNames[key] = sounds[key] || "";
  }
};

const isThemeAssetEmpty = () => {
  const hasNewFiles =
    Object.values(fileStore.images).some(val => {
      if (!val) return false;
      return typeof val === 'object' && !(val instanceof File) ? Object.values(val).some(f => f) : true;
    }) ||
    Object.values(fileStore.sounds).some(file => file) ||
    fileStore.sequentialFiles.length > 0;

  if (hasNewFiles) return false;

  const hasExistingFileNames =
    Object.values(state.imageFileNames).some(val => {
      if (!val) return false;
      return typeof val === 'object' ? Object.values(val).some(name => name) : val;
    }) ||
    Object.values(state.soundFileNames).some(name => name) ||
    state.sequentialImages.length > 0;

  return !hasExistingFileNames;
};

const getDragAfterElement = (container, y) => {
  const draggableElements = [...container.querySelectorAll('.sequential-image-item:not(.dragging)')];
  return draggableElements.reduce((closest, child) => {
    const box = child.getBoundingClientRect();
    const offset = y - box.top - box.height / 2;
    if (offset < 0 && offset > closest.offset) {
      return { offset, element: child };
    }
    return closest;
  }, { offset: Number.NEGATIVE_INFINITY }).element;
};

async function loadDefaultTheme() {
  try {
    const response = await fetch("/api/themes/default");
    if (response.ok) {
      const themeData = await response.json();
      state.colors = { ...defaultColors, ...(themeData.colors || {}) };
      state.turnSignalStyle = themeData.turnSignalStyle || "Static";
      state.turnSignalType = themeData.turnSignalType || "Single Image";
      state.turnSignalLength = themeData.turnSignalLength || 100;
      state.sequentialImages = themeData.sequentialImages || [];
      populateAssetNames(themeData);

      const themeAssets = themeData.images || {};
      const themeSounds = themeData.sounds || {};
      const assetCategories = {
          homeButton: 'icons',
          settingsButton: 'icons',
          turnSignal: 'signals',
          turnSignalBlindspot: 'signals',
          steeringWheel: 'steering_wheel'
      };

      for (const key in themeAssets) {
          if (key === 'distanceIcons') {
              for (const subkey in themeAssets.distanceIcons) {
                  if (themeAssets.distanceIcons[subkey]) {
                      const fileName = themeAssets.distanceIcons[subkey];
                      const file = await fetchFileAsObject(`/api/themes/asset/default/distance_icons/${fileName}`, fileName);
                      if (file) fileStore.images.distanceIcons[subkey] = file;
                  }
              }
          } else if (assetCategories[key] && themeAssets[key]) {
              const fileName = themeAssets[key];
              const category = assetCategories[key];
              const file = await fetchFileAsObject(`/api/themes/asset/default/${category}/${fileName}`, fileName);
              if (file) fileStore.images[key] = file;
          }
      }

       for (const key in themeSounds) {
          if (themeSounds[key]) {
              const fileName = themeSounds[key];
              const file = await fetchFileAsObject(`/api/themes/asset/default/sounds/${fileName}`, fileName);
              if(file) fileStore.sounds[key] = file;
          }
      }

      if (themeData.sequentialImages && themeData.sequentialImages.length > 0) {
          fileStore.sequentialFiles = [];
          for (const imageName of themeData.sequentialImages) {
              const file = await fetchFileAsObject(`/api/themes/asset/default/signals/${imageName}`, imageName);
              if (file) fileStore.sequentialFiles.push(file);
          }
      }
    }
  } catch(e) {
      console.error("Failed to load default theme", e);
  }
}

(async () => {
  await loadDefaultTheme();
  try {
    const response = await fetch("/api/params?key=DiscordUsername");
    state.discordUsername = await response.text();
  } catch {}
})();

export function ThemeMaker() {
  let dragStartIndex;

  const handleFileUpload = (e, type, key, subkey = null) => {
    const { files } = e.target;
    if (files.length === 0) return;

    for (const file of files) {
      if (!file.type.startsWith(`${type}/`)) {
        showSnackbar(`Invalid file type! Please upload an ${type} file.`, "error");
        e.target.value = "";
        return;
      }
    }

    state.themeSubmitted = false;

    if (key === 'turnSignal' && state.turnSignalType === 'Sequential') {
      fileStore.sequentialFiles.push(...files);
      state.sequentialImages.push(...Array.from(files).map(file => file.name));
    } else {
      const file = files[0];
      const store = type === 'image' ? fileStore.images : fileStore.sounds;
      const nameState = type === 'image' ? state.imageFileNames : state.soundFileNames;

      if (subkey) {
        store[key][subkey] = file;
        nameState[key][subkey] = file ? file.name : "";
      } else {
        store[key] = file;
        nameState[key] = file ? file.name : "";
      }
    }
  };

  const handleColorChange = (e, key) => {
    const hex = e.target.value;
    state.colors[key] = {
      red: parseInt(hex.slice(1, 3), 16),
      green: parseInt(hex.slice(3, 5), 16),
      blue: parseInt(hex.slice(5, 7), 16),
      alpha: 255,
    };
    state.themeSubmitted = false;
  };

  const validateTurnSignalLength = (e) => {
    const value = parseInt(e.target.value, 10);
    const clampedValue = isNaN(value) ? 25 : Math.max(25, Math.min(1000, value));
    state.turnSignalLength = clampedValue;
    e.target.value = clampedValue;
  };

  const toggleTurnSignalType = (type) => {
    state.turnSignalType = type;
    state.themeSubmitted = false;
    state.sequentialImages = [];
    fileStore.sequentialFiles = [];
    fileStore.images.turnSignal = undefined;
    state.imageFileNames.turnSignal = "";
  };

  const moveImage = (fromIndex, toIndex) => {
    if (toIndex < 0 || toIndex >= state.sequentialImages.length) return;
    const [name] = state.sequentialImages.splice(fromIndex, 1);
    const [file] = fileStore.sequentialFiles.splice(fromIndex, 1);
    state.sequentialImages.splice(toIndex, 0, name);
    fileStore.sequentialFiles.splice(toIndex, 0, file);
  };

  const removeSequentialImage = (index) => {
    state.sequentialImages.splice(index, 1);
    fileStore.sequentialFiles.splice(index, 1);
  };

  const getFormData = () => {
    const formData = new FormData();
    formData.append("themeName", state.themeName);
    formData.append("saveChecklist", JSON.stringify(state.saveChecklist));

    if (state.saveChecklist.colors) {
      formData.append("colors", JSON.stringify(state.colors));
    }

    if (state.saveChecklist.turn_signals) {
      formData.append("turnSignalStyle", state.turnSignalStyle);
      formData.append("turnSignalType", state.turnSignalType);
      formData.append("turnSignalLength", state.turnSignalLength);

      if (state.turnSignalType === 'Sequential') {
        fileStore.sequentialFiles.forEach((file, index) => {
          formData.append(`turn_signal_${index + 1}`, file);
        });
      }
      if (fileStore.images.turnSignal) {
        formData.append('turnSignal', fileStore.images.turnSignal);
      }
      if (fileStore.images.turnSignalBlindspot) {
        formData.append('turnSignalBlindspot', fileStore.images.turnSignalBlindspot);
      }
    }

    if (state.saveChecklist.icons) {
      ['homeButton', 'settingsButton', 'steeringWheel'].forEach(key => {
        if (fileStore.images[key]) formData.append(key, fileStore.images[key]);
      });
    }

    if (state.saveChecklist.distance_icons) {
      for (const subkey in fileStore.images.distanceIcons) {
        const file = fileStore.images.distanceIcons[subkey];
        if (file) formData.append(`distanceIcons_${subkey}`, file);
      }
    }

    if (state.saveChecklist.sounds) {
      for (const key in fileStore.sounds) {
        if (fileStore.sounds[key]) formData.append(key, fileStore.sounds[key]);
      }
    }
    return formData;
  };

  const performApiAction = async (url, options, successMessage, errorMessage) => {
    try {
      const response = await fetch(url, options);
      const result = await response.json();
      showSnackbar(result.message || (response.ok ? successMessage : errorMessage), response.ok ? "success" : "error");
      return { ok: response.ok, result };
    } catch {
      showSnackbar(`An error occurred: ${errorMessage}`, "error");
      return { ok: false };
    }
  };

  const saveTheme = async () => {
    if (!state.themeName.trim()) {
      return showSnackbar("Please enter a theme name.", "error");
    }
    await performApiAction("/api/themes", { method: "POST", body: getFormData() }, "Theme saved successfully!", "Failed to save theme.");
    state.showSaveConfirmModal = false;
  };

  const downloadTheme = async () => {
    if (!state.themeName.trim()) {
      return showSnackbar("Theme Name Is Required to download.", "error");
    }
    try {
      const response = await fetch('/api/themes/download', { method: 'POST', body: getFormData() });
      if (response.ok) {
        const blob = await response.blob();
        const url = window.URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.style.display = 'none';
        a.href = url;
        a.download = `${state.themeName.replace(/ /g, "_")}.zip`;
        document.body.appendChild(a);
        a.click();
        window.URL.revokeObjectURL(url);
        a.remove();
      } else {
        const result = await response.json();
        showSnackbar(result.message || "Failed to download theme.", "error");
      }
    } catch {
      showSnackbar("An error occurred while downloading the theme.", "error");
    }
  };

  const openThemeSelector = async (action) => {
    state.selectorAction = action;
    const { ok, result } = await performApiAction("/api/themes/list", {}, "", "Could not fetch theme list.");
    if (ok) {
      state.availableThemes = result;
      state.showThemeSelector = true;
    }
  };

  const loadTheme = async (themeName) => {
    const { ok, result } = await performApiAction(`/api/themes/load/${themeName}`, {}, "Theme loaded successfully!", "Theme not found.");
    if (ok) {
      state.themeName = result.themeName;
      state.colors = { ...defaultColors, ...(result.colors || {}) };
      state.turnSignalStyle = result.turnSignalStyle || "Traditional";
      state.turnSignalType = result.turnSignalType || "Single Image";
      state.turnSignalLength = result.turnSignalLength || 100;
      state.sequentialImages = result.sequentialImages || [];
      Object.assign(fileStore, { images: { distanceIcons: {} }, sounds: {}, sequentialFiles: [] });
      populateAssetNames(result);

      const themeAssets = result.images || {};
      const themeSounds = result.sounds || {};
      const assetCategories = {
          homeButton: 'icons',
          settingsButton: 'icons',
          turnSignal: 'signals',
          turnSignalBlindspot: 'signals',
          steeringWheel: 'steering_wheel'
      };

      for (const key in themeAssets) {
          if (key === 'distanceIcons') {
              for (const subkey in themeAssets.distanceIcons) {
                  if (themeAssets.distanceIcons[subkey]) {
                      const fileName = themeAssets.distanceIcons[subkey];
                      const file = await fetchFileAsObject(`/api/themes/asset/${themeName}/distance_icons/${fileName}`, fileName);
                      if (file) fileStore.images.distanceIcons[subkey] = file;
                  }
              }
          } else if (assetCategories[key] && themeAssets[key]) {
              const fileName = themeAssets[key];
              const category = assetCategories[key];
              const file = await fetchFileAsObject(`/api/themes/asset/${themeName}/${category}/${fileName}`, fileName);
              if (file) fileStore.images[key] = file;
          }
      }

      for (const key in themeSounds) {
        if (themeSounds[key]) {
            const fileName = themeSounds[key];
            const file = await fetchFileAsObject(`/api/themes/asset/${themeName}/sounds/${fileName}`, fileName);
            if(file) fileStore.sounds[key] = file;
        }
      }

      if (result.sequentialImages && result.sequentialImages.length > 0) {
          fileStore.sequentialFiles = [];
          for (const imageName of result.sequentialImages) {
              const file = await fetchFileAsObject(`/api/themes/asset/${themeName}/signals/${imageName}`, imageName);
              if (file) fileStore.sequentialFiles.push(file);
          }
      }
    }
  };

  const deleteTheme = async () => {
    if (!state.themeToDelete) return;
    const { ok } = await performApiAction(`/api/themes/delete/${state.themeToDelete}`, { method: "DELETE" }, "Theme deleted successfully!", "Failed to delete theme.");
    if (ok && state.themeName === state.themeToDelete) {
      state.themeName = "";
    }
    state.showDeleteConfirmation = false;
    state.themeToDelete = null;
  };

  const submitTheme = async () => {
    if (!state.discordUsername.trim()) {
      return showSnackbar("Discord username is required for submission.", "error");
    }
    state.showSubmitConfirmation = false;
    if (state.isSubmitting) return;
    state.isSubmitting = true;
    const { ok } = await performApiAction("/api/themes/submit", { method: "POST", body: getFormData() }, "Theme submitted successfully!", "Failed to submit theme.");
    if (ok) {
      state.themeSubmitted = true;
    }
    state.isSubmitting = false;
  };

  const handleThemeSelect = async (themeName) => {
    state.showThemeSelector = false;
    if (!themeName) return;
    switch (state.selectorAction) {
      case 'load':
        await loadTheme(themeName);
        break;
      case 'download':
        await loadTheme(themeName);
        await downloadTheme();
        break;
      case 'delete':
        state.themeToDelete = themeName;
        state.showDeleteConfirmation = true;
        break;
    }
  };

  const confirmSave = () => {
    if (!state.themeName.trim()) {
      return showSnackbar("Please enter a theme name.", "error");
    }
    state.showSaveConfirmModal = true;
  };

  const confirmSubmit = () => {
    if (state.themeSubmitted) {
      return showSnackbar("This theme has already been submitted.", "error");
    }
    if (isThemeAssetEmpty()) {
      return showSnackbar("Cannot submit an empty theme.", "error");
    }
    state.showSubmitConfirmation = true;
  };

  return html`
      <div class="theme-maker-container">
        <div class="theme-maker-main-widget">
          <div class="theme-maker-main-title">Theme Maker</div>
          <div class="theme-name-section">
            <label for="themeName" class="theme-name-label">Theme Name</label>
            <input type="text" id="themeName" placeholder="Enter theme name..." autocomplete="off"
              value="${() => state.themeName}" @input="${(e) => state.themeName = e.target.value}" />
          </div>
          <div class="theme-maker-sub-widgets">
            <section class="theme-maker-widget">
              <div class="theme-maker-title">Colors</div>
              <div class="theme-maker-form">
                <div class="color-section">
                  ${Object.keys(COLOR_LABELS).sort().map(key => html`
                    <label class="color-label">
                      ${COLOR_LABELS[key]}
                      <input type="color"
                        value="${() => {
                          const c = state.colors[key];
                          return `#${c.red.toString(16).padStart(2, '0')}${c.green.toString(16).padStart(2, '0')}${c.blue.toString(16).padStart(2, '0')}`;
                        }}"
                        @input="${e => handleColorChange(e, key)}" />
                    </label>`)}
                </div>
              </div>
            </section>

            <section class="theme-maker-widget">
              <div class="theme-maker-title">Distance Icons</div>
              <div class="theme-maker-form">
                <div class="upload-section">
                  ${['traffic', 'aggressive', 'standard', 'relaxed'].map(key => html`
                    <label class="file-upload-label">
                      <span class="file-upload-text">${key.charAt(0).toUpperCase() + key.slice(1)}</span>
                      <span class="file-name-display">${() => state.imageFileNames.distanceIcons[key]}</span>
                      <span class="file-upload-button">Choose File</span>
                      <input type="file" class="file-upload-input" accept="image/*"
                        @change="${e => handleFileUpload(e, 'image', 'distanceIcons', key)}" />
                    </label>`)}
                </div>
              </div>
            </section>

            <section class="theme-maker-widget">
              <div class="theme-maker-title">Icons</div>
              <div class="theme-maker-form">
                <div class="upload-section">
                  ${Object.keys(ICON_LABELS).map(key => html`
                    <label class="file-upload-label">
                      <span class="file-upload-text">${ICON_LABELS[key]}</span>
                      <span class="file-name-display">${() => state.imageFileNames[key]}</span>
                      <span class="file-upload-button">Choose File</span>
                      <input type="file" class="file-upload-input" accept="image/*"
                        @change="${e => handleFileUpload(e, 'image', key)}" />
                    </label>`)}
                </div>
              </div>
            </section>

            <section class="theme-maker-widget">
              <div class="theme-maker-title">Sounds</div>
              <div class="theme-maker-form">
                <div class="upload-section">
                  ${SOUND_DEFINITIONS.map(({ key, label }) => html`
                    <label class="file-upload-label">
                      <span class="file-upload-text">${label}</span>
                      <span class="file-name-display">${() => state.soundFileNames[key]}</span>
                      <span class="file-upload-button">Choose File</span>
                      <input type="file" class="file-upload-input" accept="audio/*"
                        @change="${e => handleFileUpload(e, 'audio', key)}" />
                    </label>`)}
                </div>
              </div>
            </section>

            <section class="theme-maker-widget">
              <div class="theme-maker-title">Turn Signals</div>
              <div class="theme-maker-form">
                <div class="upload-section">
                  <div class="turn-signal-length-section">
                    <label for="turnSignalLength" class="theme-name-label turn-signal-label">
                      Turn Signal Length (25-1000ms)
                    </label>
                    <input type="text" pattern="\\d*" id="turnSignalLength"
                      value="${() => state.turnSignalLength}"
                      @input="${e => state.turnSignalLength = e.target.value}" @blur="${validateTurnSignalLength}"
                      class="turn-signal-input">
                  </div>
                  <div class="turn-signal-style-section">
                    <label class="theme-name-label turn-signal-label">
                      Turn Signal Style
                      <span class="help-icon" @click="${() => state.showTurnSignalHelp = !state.showTurnSignalHelp}">?</span>
                    </label>
                    <div class="signal-type-toggle">
                      <button class="${() => `toggle-button ${state.turnSignalStyle === 'Static' ? 'active' : ''}`}"
                        @click="${() => state.turnSignalStyle = 'Static'}">Static</button>
                      <button class="${() => `toggle-button ${state.turnSignalStyle === 'Traditional' ? 'active' : ''}`}"
                        @click="${() => state.turnSignalStyle = 'Traditional'}">Traditional</button>
                    </div>
                    ${() => state.showTurnSignalHelp && html`
                      <div class="turn-signal-help-text">
                        <p><strong>Static</strong> - The turn signal animation appears next to the current speed.</p>
                        <p><strong>Traditional</strong> - The turn signal animation moves across the bottom of the screen.</p>
                      </div>`}
                  </div>
                  ${() => state.turnSignalStyle === 'Traditional' && html`
                    <div class="turn-signal-style-section">
                      <label class="theme-name-label turn-signal-label">Turn Signal Type</label>
                      <div class="signal-type-toggle">
                        <button class="${() => `toggle-button ${state.turnSignalType === 'Sequential' ? 'active' : ''}`}"
                          @click="${() => toggleTurnSignalType('Sequential')}">Sequential</button>
                        <button class="${() => `toggle-button ${state.turnSignalType === 'Single Image' ? 'active' : ''}`}"
                          @click="${() => toggleTurnSignalType('Single Image')}">Single Image</button>
                      </div>
                    </div>`}
                  <label class="file-upload-label">
                    <span class="file-upload-text">Blind Spot Image</span>
                    <span class="file-name-display">${() => state.imageFileNames.turnSignalBlindspot}</span>
                    <span class="file-upload-button">Choose File</span>
                    <input type="file" class="file-upload-input" accept="image/*"
                      @change="${e => handleFileUpload(e, 'image', 'turnSignalBlindspot')}" />
                  </label>
                  <label class="file-upload-label">
                    <span class="file-upload-text">${() => state.turnSignalType === 'Sequential' ? 'Turn Signal Images' : 'Turn Signal Image'}</span>
                    <span class="file-name-display">
                      ${() => state.turnSignalType === 'Sequential'
                        ? `${state.sequentialImages.length} file${state.sequentialImages.length !== 1 ? 's' : ''}`
                        : state.imageFileNames.turnSignal}
                    </span>
                    <span class="file-upload-button">${() => state.turnSignalType === 'Sequential' ? 'Choose Files' : 'Choose File'}</span>
                    <input type="file" class="file-upload-input" accept="image/*" multiple="${() => state.turnSignalType === 'Sequential'}"
                      @change="${e => handleFileUpload(e, 'image', 'turnSignal')}" />
                  </label>
                  ${() => state.turnSignalType === 'Sequential' && state.sequentialImages.length > 0 && html`
                    <button class="manage-sequence-button" @click="${() => state.showSequenceManager = true}">Manage Sequence Order</button>`}
                </div>
              </div>
            </section>
          </div>
          <div class="save-button-wrapper">
            <button class="delete-button" @click="${() => openThemeSelector('delete')}">Delete Theme</button>
            <button class="download-button" @click="${() => handleThemeSelect(state.themeName).then(downloadTheme)}">Download Theme</button>
            <button class="load-button" @click="${() => openThemeSelector('load')}">Load Theme</button>
            <button class="save-button" @click="${confirmSave}">Save Theme</button>
            <button class="submit-button" @click="${confirmSubmit}">
              ${() => state.isSubmitting ? 'Submitting...' : 'Submit Theme'}
            </button>
          </div>
        </div>

        ${() => state.showSaveConfirmModal && Modal({
          title: "Select Components to Save",
          message: html`
            <div class="checklist-container">
              <label class="checklist-item">
                <input type="checkbox" ?checked="${state.saveChecklist.colors}" @change="${(e) => state.saveChecklist.colors = e.target.checked}">
                <span class="label-text">Colors</span>
                <span class="custom-checkbox"></span>
              </label>
              <label class="checklist-item">
                <input type="checkbox" ?checked="${state.saveChecklist.distance_icons}" @change="${(e) => state.saveChecklist.distance_icons = e.target.checked}">
                <span class="label-text">Distance Icons</span>
                <span class="custom-checkbox"></span>
              </label>
              <label class="checklist-item">
                <input type="checkbox" ?checked="${state.saveChecklist.icons}" @change="${(e) => state.saveChecklist.icons = e.target.checked}">
                <span class="label-text">Icons</span>
                <span class="custom-checkbox"></span>
              </label>
              <label class="checklist-item">
                <input type="checkbox" ?checked="${state.saveChecklist.sounds}" @change="${(e) => state.saveChecklist.sounds = e.target.checked}">
                <span class="label-text">Sounds</span>
                <span class="custom-checkbox"></span>
              </label>
              <label class="checklist-item">
                <input type="checkbox" ?checked="${state.saveChecklist.turn_signals}" @change="${(e) => state.saveChecklist.turn_signals = e.target.checked}">
                <span class="label-text">Turn Signals</span>
                <span class="custom-checkbox"></span>
              </label>
            </div>
          `,
          onConfirm: saveTheme,
          onCancel: () => state.showSaveConfirmModal = false,
          confirmText: "Save",
          confirmClass: "btn-primary",
        })}

        ${() => state.showThemeSelector && Modal({
          title: `Select a theme to ${state.selectorAction}`,
          message: html`
            <div class="theme-selector-list">
              ${state.availableThemes.map(themeName => html`
                <button class="theme-selector-button" @click="${() => handleThemeSelect(themeName)}">${themeName}</button>
              `)}
            </div>`,
          onCancel: () => state.showThemeSelector = false,
          cancelText: "Close"
        })}

        ${() => state.showSubmitConfirmation && Modal({
          title: "Submit Theme for Community Use",
          message: html`
            <p>You are about to submit your theme for approval to be included for everyone to use.</p>
            <p>Please enter your Discord username below so we can contact you if needed.</p>
            <input type="text" placeholder="Discord Username" class="discord-username-input" value="${() => state.discordUsername}" @input="${e => state.discordUsername = e.target.value}" />
            <div class="checklist-container">
              <p style="margin-bottom: 10px; text-align: left; font-weight: bold;">Select components to submit:</p>
              <label class="checklist-item">
                <input type="checkbox" ?checked="${state.saveChecklist.colors}" @change="${(e) => state.saveChecklist.colors = e.target.checked}">
                <span class="label-text">Colors</span>
                <span class="custom-checkbox"></span>
              </label>
              <label class="checklist-item">
                <input type="checkbox" ?checked="${state.saveChecklist.distance_icons}" @change="${(e) => state.saveChecklist.distance_icons = e.target.checked}">
                <span class="label-text">Distance Icons</span>
                <span class="custom-checkbox"></span>
              </label>
              <label class="checklist-item">
                <input type="checkbox" ?checked="${state.saveChecklist.icons}" @change="${(e) => state.saveChecklist.icons = e.target.checked}">
                <span class="label-text">Icons</span>
                <span class="custom-checkbox"></span>
              </label>
              <label class="checklist-item">
                <input type="checkbox" ?checked="${state.saveChecklist.sounds}" @change="${(e) => state.saveChecklist.sounds = e.target.checked}">
                <span class="label-text">Sounds</span>
                <span class="custom-checkbox"></span>
              </label>
              <label class="checklist-item">
                <input type="checkbox" ?checked="${state.saveChecklist.turn_signals}" @change="${(e) => state.saveChecklist.turn_signals = e.target.checked}">
                <span class="label-text">Turn Signals</span>
                <span class="custom-checkbox"></span>
              </label>
            </div>
          `,
          onConfirm: submitTheme,
          onCancel: () => state.showSubmitConfirmation = false,
          confirmText: "Submit",
          confirmClass: "btn-primary",
        })}

        ${() => state.showDeleteConfirmation && Modal({
          title: "Confirm Deletion",
          message: `Are you sure you want to delete the theme "${state.themeToDelete}"?`,
          onConfirm: deleteTheme,
          onCancel: () => { state.showDeleteConfirmation = false; state.themeToDelete = null; },
          confirmText: "Delete",
          confirmClass: "btn-danger",
        })}

        ${() => state.showSequenceManager && Modal({
          title: "Manage Turn Signal Sequence Order",
          message: html`
            <div class="sequential-images-list"
              @dragover="${e => {
                e.preventDefault();
                const draggingItem = document.querySelector('.dragging');
                if (!draggingItem) return;
                const afterElement = getDragAfterElement(e.currentTarget, e.clientY);
                e.currentTarget.insertBefore(draggingItem, afterElement);
              }}">
              ${() => state.sequentialImages.map((name, index) => html`
                <div class="sequential-image-item" draggable="true"
                  @dragstart="${e => { dragStartIndex = index; e.currentTarget.classList.add('dragging'); }}"
                  @dragend="${e => {
                    e.currentTarget.classList.remove('dragging');
                    const newIndex = Array.from(e.currentTarget.parentNode.children).indexOf(e.currentTarget);
                    moveImage(dragStartIndex, newIndex);
                  }}">
                  <span class="sequential-image-name">${name}</span>
                  <div class="sequential-image-controls"><button @click="${() => removeSequentialImage(index)}">X</button></div>
                </div>`)}
            </div>`,
          onCancel: () => state.showSequenceManager = false,
          cancelText: "Done"
        })}
      </div>`;
}
