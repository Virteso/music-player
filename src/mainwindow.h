#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QListWidget>
#include <vlc/vlc.h>
#include <vlc/libvlc_events.h>
#include "playlist.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    Ui::MainWindow *ui;

    libvlc_instance_t *vlcInstance = nullptr;
    libvlc_media_player_t *mediaPlayer = nullptr;
    libvlc_media_t *media = nullptr;
    QTimer *updateTimer = nullptr;
    bool isPlaying = false;
    bool isSeeking = false;
    Playlist playlist;
    QStringList filters;

    void updateProgressBar() const;
    void updateVolume(int volume) const;
    void updateStatus(const QString &status) const;
    static QString formatTime(libvlc_time_t time);
    void loadFolder(const QString& folderPath);
    void playCurrentTrack();

private slots:
    void playMedia();
    void pauseMedia();
    void shuffle();
    void loop();
    void onProgressBarSliderMoved(int position) const;
    void onProgressBarSliderPressed();
    void onProgressBarSliderReleased();
    void onVolumeSliderMoved(int position) const;
    void browseFile();
    void browseFolder();
    void onPlaylistItemDoubleClicked(int row);
    void onMediaEnd();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
};

#endif // MAINWINDOW_H
