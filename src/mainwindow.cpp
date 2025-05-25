#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QTime>
#include <QDebug>
#include <QFile>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Initialize VLC instance with more verbose logging
    const char *vlc_args[] = {
        "--quiet",
        "--no-xlib",
        "--verbose=2",
        "--file-logging",
        "--logfile=vlc-log.txt"
    };
    vlcInstance = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);
    if (!vlcInstance) {
        qDebug() << "Failed to create VLC instance";
        return;
    }
    mediaPlayer = libvlc_media_player_new(vlcInstance);
    if (!mediaPlayer) {
        qDebug() << "Failed to create media player";
        return;
    }

    // Initialize timer for progress updates
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateProgressBar);

    // Connect buttons
    connect(ui->buttonPlay, &QPushButton::clicked, this, &MainWindow::playMedia);
    connect(ui->buttonPause, &QPushButton::clicked, this, &MainWindow::pauseMedia);
    connect(ui->buttonStop, &QPushButton::clicked, this, &MainWindow::stopMedia);
    connect(ui->buttonBrowse, &QPushButton::clicked, this, &MainWindow::browseFile);

    // Connect progress bar
    connect(ui->progressBar, &QSlider::sliderMoved, this, &MainWindow::onProgressBarSliderMoved);
    connect(ui->progressBar, &QSlider::sliderPressed, this, &MainWindow::onProgressBarSliderPressed);
    connect(ui->progressBar, &QSlider::sliderReleased, this, &MainWindow::onProgressBarSliderReleased);

    // Connect volume slider
    connect(ui->volumeSlider, &QSlider::sliderMoved, this, &MainWindow::onVolumeSliderMoved);

    // Set initial volume
    updateVolume(ui->volumeSlider->value());
    updateStatus("Ready");
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
    if (path.isEmpty()) {
        qDebug() << "No file path provided";
        return;
    }

    // Check if file exists
    QFile file(path);
    if (!file.exists()) {
        qDebug() << "File does not exist:" << path;
        updateStatus("Error: File not found");
        return;
    }

    // Convert path to absolute path and normalize separators
    QString absolutePath = QDir::toNativeSeparators(QFileInfo(path).absoluteFilePath());
    qDebug() << "Attempting to play:" << absolutePath;

    if (media) {
        libvlc_media_release(media);
        media = nullptr;
    }

    // Try creating media with file:// protocol
    QString fileUrl = "file:///" + absolutePath;
    qDebug() << "Trying with file URL:" << fileUrl;
    
    // Create new media using file URL
    media = libvlc_media_new_location(fileUrl.toUtf8().constData());
    if (!media) {
        qDebug() << "Failed to create media with file URL, trying direct path";
        // Try with direct path as fallback
        media = libvlc_media_new_path(absolutePath.toUtf8().constData());
    }

    if (!media) {
        qDebug() << "Failed to create media with both methods";
        updateStatus("Error: Failed to create media");
        return;
    }

    // Set media to player
    libvlc_media_player_set_media(mediaPlayer, media);
    
    // Start playback
    if (libvlc_media_player_play(mediaPlayer) == -1) {
        qDebug() << "Failed to start playback";
        updateStatus("Error: Failed to start playback");
        return;
    }

    isPlaying = true;
    updateTimer->start(1000);
    updateStatus("Playing");
}

void MainWindow::pauseMedia()
{
    if (!mediaPlayer) return;
    
    if (isPlaying) {
        libvlc_media_player_pause(mediaPlayer);
        updateTimer->stop();
        updateStatus("Paused");
    } else {
        libvlc_media_player_play(mediaPlayer);
        updateTimer->start(1000);
        updateStatus("Playing");
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
    updateStatus("Stopped");
}

void MainWindow::updateProgressBar()
{
    if (!mediaPlayer || isSeeking) return;

    libvlc_time_t length = libvlc_media_player_get_length(mediaPlayer);
    libvlc_time_t time = libvlc_media_player_get_time(mediaPlayer);

    if (length > 0) {
        int position = (time * 100) / length;
        ui->progressBar->setValue(position);
        
        // Update status with current time
        QString timeStr = formatTime(time) + " / " + formatTime(length);
        ui->statusLabel->setText(timeStr);
    }
}

void MainWindow::onProgressBarSliderPressed()
{
    isSeeking = true;
    updateTimer->stop();
}

void MainWindow::onProgressBarSliderReleased()
{
    isSeeking = false;
    if (isPlaying) {
        updateTimer->start(1000);
    }
    onProgressBarSliderMoved(ui->progressBar->value());
}

void MainWindow::onProgressBarSliderMoved(int position)
{
    if (!mediaPlayer || !media) return;

    libvlc_time_t length = libvlc_media_player_get_length(mediaPlayer);
    libvlc_time_t newTime = (position * length) / 100;
    libvlc_media_player_set_time(mediaPlayer, newTime, false);
    
    // Update time display even while seeking
    QString timeStr = formatTime(newTime) + " / " + formatTime(length);
    ui->statusLabel->setText(timeStr);
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

void MainWindow::browseFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Audio File"), "",
        tr("Audio Files (*.mp3 *.wav *.ogg *.flac *.oga);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        ui->lineEditPath->setText(fileName);
    }
}

void MainWindow::updateStatus(const QString &status)
{
    ui->statusLabel->setText(status);
}

QString MainWindow::formatTime(libvlc_time_t time)
{
    int seconds = time / 1000;
    int minutes = seconds / 60;
    seconds = seconds % 60;
    return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
}
