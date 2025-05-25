#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <vlc/vlc.h>

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

    void updateProgressBar();
    void updateVolume(int volume);
    void updateStatus(const QString &status) const;
    static QString formatTime(libvlc_time_t time);

private slots:
    void playMedia();
    void pauseMedia();
    void stopMedia();
    void onProgressBarSliderMoved(int position);
    void onProgressBarSliderPressed();
    void onProgressBarSliderReleased();
    void onVolumeSliderMoved(int position);
    void browseFile();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
};

#endif // MAINWINDOW_H
