#include "sequencer.h"

typedef QList<QString> Frame;

Sequencer::Sequencer(GraphWidget *graphView) : graphView(graphView)
{}

void Sequencer::setDelay(unsigned int ms) { this->delay = ms; }

int Sequencer::addFrame(int frameIndex)
{
    qsizetype n = frames.length();
    if (n < frameIndex || n+1 < -frameIndex)
        throw std::runtime_error("Frame index is out of range");
    qsizetype newFrameIndex = (frameIndex < 0 ? n + 1 : 0) + frameIndex;
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

void Sequencer::first() {
    if (frames.empty()) {
        throw std::out_of_range("Frames are empty. Cannot move to the first frame.");
    }
    position = 0;
    draw();
}

void Sequencer::last() {
    if (frames.empty()) {
        throw std::out_of_range("Frames are empty. Cannot move to the last frame.");
    }
    position = frames.size() - 1;
    draw();
}

void Sequencer::next() {
    if (frames.empty()) {
        throw std::out_of_range("Frames are empty. Cannot move to the next frame.");
    }
    if (position < frames.size() - 1) {
        position++;
        draw();
    }

}

void Sequencer::prev() {
    if (frames.empty()) {
        throw std::out_of_range("Frames are empty. Cannot move to the previous frame.");
    }
    if (position > 0) {
        position--;
        draw();
    }

}

void Sequencer::processCommand(const QString &command)
{
    QStringList parts = command.split(" ");
    if (parts.isEmpty())
        return;

    QString action = parts[0];

    if (action == "SET_COLOR" && parts.size() == 4) {
        QString type = parts[1]; // node или edge
        QString id = parts[2];
        QColor color = QColor(parts[3]);
        if (parts[1] == "node") {
            this->set_node_color(color, id.toInt());
        } else if (parts[1] == "edge") {
            if (!parts[2].contains(',')) {
                qDebug() << "Invalid separator in edge id";
                return;
            }
            QStringList _buff = parts[2].split(",");
            if (_buff.size() != 2) {
                qDebug() << "Invalid arguments count for edge id";
                return;
            }
            set_edge_color(color, _buff[0].toInt(), _buff[1].toInt());
        }
        qDebug() << "Set color for" << type << id << "to" << color;
    } else if (action == "SET_TEXT" && parts.size() >= 4) {
        QString type = parts[1]; // edge или point
        QString idOrCoordinates = parts[2];
        QString text = command.section(' ', 3); // Весь оставшийся текст

        qDebug() << "Set text for" << type << idOrCoordinates << "to" << text;
        // Реализация логики добавления текста
    } else if (action == "RESET_COLORS") {
        resetAllColors();
        qDebug() << "Reset colors for all elements";

    } else if (action == "REMOVE_TEXT" && parts.size() == 3) {
        QString type = parts[1]; // edge или point
        QString idOrCoordinates = parts[2];

        qDebug() << "Remove text for" << type << idOrCoordinates;
        // Реализация удаления текста
    } else {
        qDebug() << "Unknown command:" << command;
    }
}
void Sequencer::set_node_color(QColor color, unsigned int index)
{
    this->graphView->getNodes()->find(index).value()->setCurrentColor(color);
    // this->graphView->getNodes()->find(index).value()->update();
}

void Sequencer::set_edge_color(QColor color, unsigned int srcNodeIndex, unsigned int destNodeIndex)
{
    if (srcNodeIndex >= this->graphView->getNodes()->size()
        || destNodeIndex >= this->graphView->getNodes()->size()) {
        throw std::out_of_range("Index of dest/source node is out of range");
    }
    QPair<Node *, Node *> key
        = QPair<Node *, Node *>(this->graphView->getNodes()->value(srcNodeIndex, nullptr),
                                this->graphView->getNodes()->value(destNodeIndex, nullptr));
    if (!key.first || !key.second) {
        throw std::runtime_error("Source/dest node not found");
    }
    if (!graphView->getEdges()->contains(key))
        throw std::runtime_error(QString("No such edge found for %1,%2").arg(srcNodeIndex).arg(destNodeIndex).toStdString());
    this->graphView->getEdges()->value(key)->setCurrentColor(color);
}

void Sequencer::resetAllColors()
{
    for (auto &node : *this->graphView->getNodes()) {
        node->resetColor();
    }
    for (auto &edge : *this->graphView->getEdges()) {
        edge->resetColor();
    }
}

void Sequencer::play()
{

}

void Sequencer::draw()
{
    for (auto &command : frames[position]) {
        processCommand(command);
        qDebug() << "Executed commmand " << command;
    }
    graphView->initScene();
}

int Sequencer::getPosition()
{
    return this->position;
}

int Sequencer::getFramesLength()
{
    return this->frames.length();
}

void Sequencer::clear() {
    for(auto& tb : textBoxes){
        graphView->scene()->removeItem(tb);
    }
    resetAllColors();
    frames.clear();
    graphView->initScene();
    position = -1;
    qDebug() << "Sequencer cleared";
}
