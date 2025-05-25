#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QString>
#include <QVector>
#include <QDir>

class Playlist {
public:
    Playlist();
    ~Playlist();

    void addFile(const QString& filePath);
    void removeFile(int index);
    void clear();
    QString getCurrentFile() const;
    QString getFileAt(int index) const;
    int getCurrentIndex() const;
    void setCurrentIndex(int index);
    int getFileCount() const;
    void next();
    void previous();
    bool isEmpty() const;

private:
    QVector<QString> files;
    int currentIndex;
};

#endif // PLAYLIST_H 