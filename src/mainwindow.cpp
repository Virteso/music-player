#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Initialize VLC instance
    const char *vlc_args[] = {"--quiet", "--no-xlib"};
    vlcInstance = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);
    mediaPlayer = libvlc_media_player_new(vlcInstance);

    // Initialize timer for progress updates
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateProgressBar);

    // Connect buttons
    connect(ui->buttonPlay, &QPushButton::clicked, this, &MainWindow::playMedia);
    connect(ui->buttonPause, &QPushButton::clicked, this, &MainWindow::pauseMedia);
    connect(ui->buttonStop, &QPushButton::clicked, this, &MainWindow::stopMedia);

    // Connect progress bar
    connect(ui->progressBar, &QSlider::sliderMoved, this, &MainWindow::onProgressBarSliderMoved);

    // Connect volume slider
    connect(ui->volumeSlider, &QSlider::sliderMoved, this, &MainWindow::onVolumeSliderMoved);

    // Set initial volume
    updateVolume(ui->volumeSlider->value());
}

MainWindow::~MainWindow()
{
    if (mediaPlayer)
        libvlc_media_player_release(mediaPlayer);
    if (vlcInstance)
        libvlc_release(vlcInstance);
    delete ui;
}

void MainWindow::playMedia()
{
    QString path = ui->lineEditPath->text();
    if (path.isEmpty())
        return;

    if (media)
        libvlc_media_release(media);

    media = libvlc_media_new_path(path.toStdString().c_str());
    libvlc_media_player_set_media(mediaPlayer, media);
    libvlc_media_player_play(mediaPlayer);
    isPlaying = true;
    updateTimer->start(100); // Update progress every 100ms
}

void MainWindow::pauseMedia()
{
    if (!mediaPlayer) return;
    
    if (isPlaying) {
        libvlc_media_player_pause(mediaPlayer);
        updateTimer->stop();
    } else {
        libvlc_media_player_play(mediaPlayer);
        updateTimer->start(100);
    }
    isPlaying = !isPlaying;
}

void MainWindow::stopMedia()
{
    if (!mediaPlayer) return;
    
    libvlc_media_player_stop_async(mediaPlayer);
    isPlaying = false;
    updateTimer->stop();
    ui->progressBar->setValue(0);
}

void MainWindow::updateProgressBar()
{
    if (!mediaPlayer) return;

    libvlc_time_t length = libvlc_media_player_get_length(mediaPlayer);
    libvlc_time_t time = libvlc_media_player_get_time(mediaPlayer);

    if (length > 0) {
        int position = (time * 100) / length;
        ui->progressBar->setValue(position);
    }
}

void MainWindow::onProgressBarSliderMoved(int position)
{
    if (!mediaPlayer || !media) return;

    libvlc_time_t length = libvlc_media_player_get_length(mediaPlayer);
    libvlc_time_t newTime = (position * length) / 100;
    libvlc_media_player_set_time(mediaPlayer, newTime, false);
}

void MainWindow::updateVolume(int volume)
{
    if (!mediaPlayer) return;
    libvlc_audio_set_volume(mediaPlayer, volume);
}

void MainWindow::onVolumeSliderMoved(int position)
{
    updateVolume(position);
}
