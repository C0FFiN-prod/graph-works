#include "sequencer.h"

typedef QList<QString> Frame;

Sequencer::Sequencer(GraphWidget *graphView) : graphView(graphView)
{}

void Sequencer::setDelay(unsigned int ms) { this->delay = ms; }

int Sequencer::addFrame(int frameIndex)
{
    qsizetype n = frames.length();
    if (n < abs(frameIndex))
        throw std::runtime_error("Frame index is out of range");
    qsizetype newFrameIndex = (frameIndex < 0 ? n : 0) + frameIndex;
    frames.insert(newFrameIndex,Frame());
    return newFrameIndex;
}

void Sequencer::removeFrame(int frameIndex)
{
    qsizetype n = frames.length();
    if (n < abs(frameIndex))
        throw std::runtime_error("Frame index is out of range");
    frames.remove((frameIndex < 0 ? n : 0) + frameIndex);
    position = qMin(position, n-1);
}

void Sequencer::addCommand(const QString &command, int frameIndex)
{
    qsizetype n = frames.length();
    if (n-1 < frameIndex || n < -frameIndex)
        throw std::runtime_error("Frame index is out of range");
    frames[(frameIndex < 0 ? n : 0) + frameIndex].append(command);
}

void Sequencer::clear()
{

}

void Sequencer::first() {
    if (frames.empty()) {
        throw std::out_of_range("Frames are empty. Cannot move to the first frame.");
    }
    position = 0;
}

void Sequencer::last() {
    if (frames.empty()) {
        throw std::out_of_range("Frames are empty. Cannot move to the last frame.");
    }
    position = frames.size() - 1;
}

void Sequencer::next() {
    if (frames.empty()) {
        throw std::out_of_range("Frames are empty. Cannot move to the next frame.");
    }
    if (position < frames.size() - 1) {
        position++;
    } else {
        position = frames.size() - 1; // Упираемся в последний кадр
    }
}

void Sequencer::prev() {
    if (frames.empty()) {
        throw std::out_of_range("Frames are empty. Cannot move to the previous frame.");
    }
    if (position > 0) {
        position--;
    } else {
        position = 0; // Упираемся в первый кадр
    }
}

void Sequencer::play()
{

}

void Sequencer::draw()
{

}
