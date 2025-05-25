#include "playlist.h"

Playlist::Playlist() : currentIndex(-1)
{
    rng = std::mt19937(std::chrono::high_resolution_clock::now().time_since_epoch().count());
}

Playlist::~Playlist() = default;

void Playlist::addFile(const QFileInfo& filePath) {
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

QFileInfo Playlist::getCurrentFile() {
    if (currentIndex >= 0 && currentIndex < files.size()) {
        return files[currentIndex];
    }
    return QFileInfo();
}

QFileInfo Playlist::getFileAt(int index) {
    if (index >= 0 && index < files.size()) {
        return files[index];
    }
    return QFileInfo();
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

bool Playlist::next() {
    if (!files.isEmpty()) {
        if (loop)
        {
            currentIndex = (currentIndex + 1) % files.size();
            return true;
        }
        ++currentIndex;
        if (currentIndex >= files.size())
        {
            return false;
        }
        return true;
    }
    return false;
}

void Playlist::previous() {
    if (!files.isEmpty()) {
        currentIndex = (currentIndex - 1 + files.size()) % files.size();
    }
}

bool Playlist::isEmpty() const {
    return files.isEmpty();
}

void Playlist::setLoop(bool l)
{
    loop = l;
}

bool Playlist::getLoop() const
{
    return loop;
}

void Playlist::shuffle()
{
    std::ranges::shuffle(files, rng);
}
