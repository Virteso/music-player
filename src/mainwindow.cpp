#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QTime>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QFileInfo>

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
        "--logfile=vlc-log.txt",
        "--aout=winmm"  // Use Windows Multimedia audio output instead of DirectSound
    };
    vlcInstance = libvlc_new(std::size(vlc_args), vlc_args);
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
    updateTimer->start(1000);

    // Connect buttons
    connect(ui->buttonPlay, &QPushButton::clicked, this, &MainWindow::playMedia);
    connect(ui->buttonPause, &QPushButton::clicked, this, &MainWindow::pauseMedia);
    connect(ui->buttonStop, &QPushButton::clicked, this, &MainWindow::stopMedia);
    connect(ui->buttonBrowse, &QPushButton::clicked, this, &MainWindow::browseFile);
    connect(ui->buttonBrowseFolder, &QPushButton::clicked, this, &MainWindow::browseFolder);
    connect(ui->playlistWidget, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        onPlaylistItemDoubleClicked(ui->playlistWidget->row(item));
    });

    // Connect progress bar
    connect(ui->progressBar, &QSlider::sliderMoved, this, &MainWindow::onProgressBarSliderMoved);
    connect(ui->progressBar, &QSlider::sliderPressed, this, &MainWindow::onProgressBarSliderPressed);
    connect(ui->progressBar, &QSlider::sliderReleased, this, &MainWindow::onProgressBarSliderReleased);

    // Connect volume slider
    connect(ui->volumeSlider, &QSlider::sliderMoved, this, &MainWindow::onVolumeSliderMoved);

    // Set initial volume
    updateVolume(ui->volumeSlider->value());
    updateStatus("Ready");

    // Connect media end event
    libvlc_event_manager_t* eventManager = libvlc_media_player_event_manager(mediaPlayer);
    libvlc_event_attach(eventManager, libvlc_MediaPlayerStopped, [](const libvlc_event_t*, void* data) {
        QMetaObject::invokeMethod(static_cast<MainWindow*>(data), "onMediaEnd", Qt::QueuedConnection);
    }, this);
}

MainWindow::~MainWindow()
{
    if (media) {
        libvlc_media_release(media);
    }
    if (mediaPlayer)
        libvlc_media_player_release(mediaPlayer);
    if (vlcInstance)
        libvlc_release(vlcInstance);
    delete ui;
}

void MainWindow::playMedia()
{
    if (playlist.isEmpty()) {
        updateStatus("No tracks in playlist");
        return;
    }

    QString currentFile = playlist.getCurrentFile();
    qDebug() << "Attempting to play file:" << currentFile;
    
    // Stop current playback if any
    if (isPlaying) {
        libvlc_media_player_stop_async(mediaPlayer);
        updateTimer->stop();  // Stop the timer
        isPlaying = false;
    }

    if (media) {
        libvlc_media_release(media);
    }

    // Convert path to absolute path and normalize separators
    QString absolutePath = QDir::toNativeSeparators(QFileInfo(currentFile).absoluteFilePath());
    qDebug() << "Absolute path:" << absolutePath;

    // Try creating media with direct path first
    media = libvlc_media_new_path(absolutePath.toUtf8().constData());
    if (!media) {
        qDebug() << "Failed to create media with direct path, trying file URL";
        QString fileUrl = "file:///" + absolutePath;
        qDebug() << "Trying with file URL:" << fileUrl;
        media = libvlc_media_new_location(fileUrl.toUtf8().constData());
    }

    if (!media) {
        qDebug() << "Failed to create media with both methods";
        updateStatus("Error: Failed to create media");
        return;
    }

    libvlc_media_player_set_media(mediaPlayer, media);
    
    // Start playback
    if (libvlc_media_player_play(mediaPlayer) == -1) {
        qDebug() << "Failed to start playback";
        updateStatus("Error: Failed to start playback");
        return;
    }

    // Set progress bar range to milliseconds
    ui->progressBar->setRange(0, 1000);
    updateTimer->start(100);  // Start the timer
    isPlaying = true;
    updateStatus("Playing: " + QFileInfo(currentFile).fileName());
}

void MainWindow::pauseMedia()
{
    if (isPlaying) {
        libvlc_media_player_pause(mediaPlayer);
        updateTimer->stop();  // Stop the timer when paused
        updateStatus("Paused");
    } else {
        libvlc_media_player_play(mediaPlayer);
        updateTimer->start(100);  // Restart the timer when playing
        updateStatus("Playing");
    }
    isPlaying = !isPlaying;
}

void MainWindow::stopMedia()
{
    libvlc_media_player_stop_async(mediaPlayer);
    updateTimer->stop();  // Stop the timer
    isPlaying = false;
    ui->progressBar->setValue(0);
    updateStatus("Stopped");
}

void MainWindow::updateProgressBar() const
{
    if (!isSeeking && mediaPlayer) {
        libvlc_time_t length = libvlc_media_player_get_length(mediaPlayer);
        libvlc_time_t time = libvlc_media_player_get_time(mediaPlayer);

        if (length > 0) {
            // Update progress bar (0-1000 range)
            int position = static_cast<int>((time * 1000.0f) / length);
            ui->progressBar->setValue(position);

            // Update time display
            QString timeStr = formatTime(time) + " / " + formatTime(length);
            ui->statusLabel->setText(timeStr);
        }
    }
}

void MainWindow::onProgressBarSliderPressed()
{
    isSeeking = true;
}

void MainWindow::onProgressBarSliderReleased()
{
    if (mediaPlayer) {
        libvlc_time_t length = libvlc_media_player_get_length(mediaPlayer);
        libvlc_time_t newTime = (ui->progressBar->value() * length) / 1000;
        libvlc_media_player_set_time(mediaPlayer, newTime, false);
    }
    isSeeking = false;
}

void MainWindow::onProgressBarSliderMoved(int position) const
{
    if (isSeeking && mediaPlayer) {
        libvlc_time_t length = libvlc_media_player_get_length(mediaPlayer);
        libvlc_time_t newTime = (position * length) / 1000;
        QString timeStr = formatTime(newTime) + " / " + formatTime(length);
        ui->statusLabel->setText(timeStr);
    }
}

void MainWindow::updateVolume(int volume) const
{
    libvlc_audio_set_volume(mediaPlayer, volume);
}

void MainWindow::onVolumeSliderMoved(int position) const
{
    updateVolume(position);
}

void MainWindow::browseFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open Media File", "", "Media Files (*.mp3 *.wav *.ogg *.flac)");
    if (!filePath.isEmpty()) {
        ui->lineEditPath->setText(filePath);
        playlist.clear();
        playlist.addFile(filePath);
        ui->playlistWidget->clear();
        ui->playlistWidget->addItem(QFileInfo(filePath).fileName());
        playMedia(); // Automatically start playing the selected file
    }
}

void MainWindow::browseFolder()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, "Select Music Folder");
    if (!folderPath.isEmpty()) {
        loadFolder(folderPath);
    }
}

void MainWindow::loadFolder(const QString& folderPath)
{
    QDir dir(folderPath);
    QStringList filters;
    filters << "*.mp3" << "*.wav" << "*.ogg" << "*.flac";
    dir.setNameFilters(filters);

    playlist.clear();
    ui->playlistWidget->clear();

    QFileInfoList fileList = dir.entryInfoList();
    for (const QFileInfo& fileInfo : fileList) {
        playlist.addFile(fileInfo.absoluteFilePath());
        ui->playlistWidget->addItem(fileInfo.fileName());
    }

    if (!playlist.isEmpty()) {
        updateStatus(QString("Loaded %1 tracks").arg(playlist.getFileCount()));
        playMedia(); // Automatically start playing the first track
    } else {
        updateStatus("No media files found in selected folder");
    }
}

void MainWindow::onPlaylistItemDoubleClicked(int row)
{
    playlist.setCurrentIndex(row);
    playMedia();
}

void MainWindow::onMediaEnd()
{
    if (isPlaying) {  // Only proceed to next track if we're actually playing
        playlist.next();
        playMedia();
    }
}

void MainWindow::playCurrentTrack()
{
    if (!playlist.isEmpty()) {
        QString currentFile = playlist.getCurrentFile();
        if (media) {
            libvlc_media_release(media);
        }
        media = libvlc_media_new_path(currentFile.toUtf8().constData());
        libvlc_media_player_set_media(mediaPlayer, media);
        libvlc_media_player_play(mediaPlayer);
        isPlaying = true;
        updateStatus("Playing: " + QFileInfo(currentFile).fileName());
        ui->playlistWidget->setCurrentRow(playlist.getCurrentIndex());
    }
}

void MainWindow::updateStatus(const QString &status) const
{
    ui->statusLabel->setText(status);
}

QString MainWindow::formatTime(libvlc_time_t time)
{
    int seconds = time / 1000;
    int minutes = seconds / 60;
    seconds %= 60;
    return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
}
