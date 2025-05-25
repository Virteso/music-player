#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QString>
#include <QVector>
#include <QDir>
#include <algorithm>
#include <random>
#include <chrono>

class Playlist {
public:
    Playlist();
    ~Playlist();

    void addFile(const QFileInfo& filePath);
    void removeFile(int index);
    void clear();
    QFileInfo getCurrentFile();
    QFileInfo getFileAt(int index);
    int getCurrentIndex() const;
    void setCurrentIndex(int index);
    int getFileCount() const;
    bool next();
    void previous();
    bool isEmpty() const;
    void setLoop(bool loop);
    bool getLoop() const;
    void shuffle();

private:
    QVector<QFileInfo> files;
    int currentIndex;
    bool loop{false};
    std::mt19937 rng;
};

#endif // PLAYLIST_H 