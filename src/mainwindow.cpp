#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Initialize VLC instance
    const char *vlc_args[] = {
        "--quiet", "--no-xlib"
    };
    vlcInstance = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);
    mediaPlayer = libvlc_media_player_new(vlcInstance);

    // Connect buttons
    connect(ui->buttonPlay, &QPushButton::clicked, this, &MainWindow::playMedia);
    connect(ui->buttonPause, &QPushButton::clicked, this, &MainWindow::pauseMedia);
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
}

void MainWindow::pauseMedia()
{
    libvlc_media_player_pause(mediaPlayer);
}
