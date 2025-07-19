#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QWidget>

#include "system/hardware/hw.h"
#include "selfdrive/ui/qt/util.h"
#include "selfdrive/ui/qt/qt_window.h"
#include "selfdrive/ui/qt/widgets/scrollview.h"

void runRestore(QPushButton *restoreBtn) {
  QDir backupDir("/data/backups");
  backupDir.setNameFilters(QStringList() << "*.tar.zst" << "*.tar.gz");
  backupDir.setSorting(QDir::Name | QDir::Reversed);

  QStringList backupFiles = backupDir.entryList(QDir::Files);
  QString latestBackup = backupFiles.first();

  restoreBtn->setEnabled(false);
  restoreBtn->setText("Restoring...");

  QString extractDirectory = "/data/restore_temp";
  QString sourcePath = backupDir.filePath(latestBackup);
  QString targetPath = "/data/safe_staging/finalized";

  QDir().mkpath(extractDirectory);

  if (latestBackup.endsWith(".tar.zst")) {
    std::system(qPrintable("zstd -d " + sourcePath + " -o " + extractDirectory + "/backup.tar"));
    std::system(qPrintable("tar --strip-components=1 -xf " + extractDirectory + "/backup.tar -C " + extractDirectory));

    QFile::remove(extractDirectory + "/backup.tar");
  } else {
    std::system(qPrintable("tar --strip-components=1 -xzf " + sourcePath + " -C " + extractDirectory));
  }

  QDir().mkpath(targetPath);

  std::system(qPrintable("rsync -av --delete -l " + extractDirectory + "/ " + targetPath + "/"));

  QFile(targetPath + "/.overlay_consistent").open(QIODevice::WriteOnly);
  QFile("/cache/on_backup").open(QIODevice::WriteOnly);

  QDir(extractDirectory).removeRecursively();

  restoreBtn->setText("Restored!");
  util::sleep_for(2500);

  restoreBtn->setText("Rebooting...");
  util::sleep_for(2500);

  Hardware::reboot();
}

int main(int argc, char *argv[]) {
  initApp(argc, argv);
  QApplication a(argc, argv);
  QWidget window;
  setMainWindow(&window);

  QGridLayout *main_layout = new QGridLayout(&window);
  main_layout->setMargin(50);

  QLabel *label = new QLabel(argv[1]);
  label->setWordWrap(true);
  label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  ScrollView *scroll = new ScrollView(label);
  scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  main_layout->addWidget(scroll, 0, 0, Qt::AlignTop);

  // Scroll to the bottom
  QObject::connect(scroll->verticalScrollBar(), &QAbstractSlider::rangeChanged, [=]() {
    scroll->verticalScrollBar()->setValue(scroll->verticalScrollBar()->maximum());
  });

  QPushButton *btn = new QPushButton();
#ifdef __aarch64__
  btn->setText(QObject::tr("Restore"));
  QObject::connect(btn, &QPushButton::clicked, [=]() {
    runRestore(btn);
  });
#else
  btn->setText(QObject::tr("Exit"));
  QObject::connect(btn, &QPushButton::clicked, &a, &QApplication::quit);
#endif
  main_layout->addWidget(btn, 0, 0, Qt::AlignRight | Qt::AlignBottom);

  window.setStyleSheet(R"(
    * {
      outline: none;
      color: white;
      background-color: black;
      font-size: 60px;
    }
    QPushButton {
      padding: 50px;
      padding-right: 100px;
      padding-left: 100px;
      border: 2px solid white;
      border-radius: 20px;
      margin-right: 40px;
    }
  )");

  return a.exec();
}
