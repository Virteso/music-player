#include "playlist.h"

Playlist::Playlist() : currentIndex(-1) {}

Playlist::~Playlist() = default;

void Playlist::addFile(const QString& filePath) {
    files.append(filePath);
    if (currentIndex == -1) {
        currentIndex = 0;
    }
}

void Playlist::removeFile(int index) {
    if (index >= 0 && index < files.size()) {
        files.removeAt(index);
        if (currentIndex >= files.size()) {
            currentIndex = files.size() - 1;
        }
    }
}

void Playlist::clear() {
    files.clear();
    currentIndex = -1;
}

QString Playlist::getCurrentFile() const {
    if (currentIndex >= 0 && currentIndex < files.size()) {
        return files[currentIndex];
    }
    return QString();
}

QString Playlist::getFileAt(int index) const {
    if (index >= 0 && index < files.size()) {
        return files[index];
    }
    return QString();
}

int Playlist::getCurrentIndex() const {
    return currentIndex;
}

void Playlist::setCurrentIndex(int index) {
    if (index >= 0 && index < files.size()) {
        currentIndex = index;
    }
}

int Playlist::getFileCount() const {
    return files.size();
}

void Playlist::next() {
    if (!files.isEmpty()) {
        currentIndex = (currentIndex + 1) % files.size();
    }
}

void Playlist::previous() {
    if (!files.isEmpty()) {
        currentIndex = (currentIndex - 1 + files.size()) % files.size();
    }
}

bool Playlist::isEmpty() const {
    return files.isEmpty();
} 