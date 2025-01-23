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
    goToFrame(-1);
}

void Sequencer::last() {
    if (frames.empty()) {
        throw std::out_of_range("Frames are empty. Cannot move to the last frame.");
    }

    goToFrame(frames.size() - 1);
}

void Sequencer::next() {
    if (frames.empty()) {
        throw std::out_of_range("Frames are empty. Cannot move to the next frame.");
    }
    if (position < frames.size() - 1) {
        while ((position < frames.size() - 1) && frames[++position].empty())
            ;
        draw();
    }

}

void Sequencer::prev() {
    if (frames.empty()) {
        throw std::out_of_range("Frames are empty. Cannot move to the previous frame.");
    }
    goToFrame(--position);
}

void Sequencer::goToFrame(int targetFrameInd)
{
    for (auto tb : textBoxes.asKeyValueRange()) {
        if (tb.first.endsWith("edge")) {
            tb.second->containsCustomText = false;
        } else {
            graphView->scene()->removeItem(tb.second);
            delete tb.second;
        }
    }
    textBoxes.clear();
    resetAllColors();
    graphView->initScene();
    position = -1;
    qDebug() << "Changes cleared";
    if (!isSequenceStateless) {
        while (position < targetFrameInd) {
            position++;
            draw();
        }
    } else {
        if (targetFrameInd >= 0) {
            position = targetFrameInd;
            draw();
        }
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
        QString color = parts[3];
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
    } else if (action == "SET_TEXT" && parts.size() >= 3) {
        QString type = parts[1]; // edge или point
        QString idOrCoordinates = parts[2];
        QString text = command.section(' ', 3);
        if (type == "edge") {
            if (!idOrCoordinates.contains(',')) {
                qDebug() << "Not found SET_TEXT edge separator";
                return;
            }
            QStringList _buff = idOrCoordinates.split(',');
            if (_buff.size() != 2) {
                qDebug() << "Invalid argument count for edge id";
                return;
            }
            drawTextAtEdge(text, _buff[0].toInt(), _buff[1].toInt());
        } else if (type == "point") {
            if (!idOrCoordinates.contains(',')) {
                qDebug() << "Not found SET_TEXT point separator";
                return;
            }
            QStringList _buff = idOrCoordinates.split(',');
            drawTextAtPoint(text, _buff[0].toInt(), _buff[1].toInt());
            if (_buff.size() != 2) {
                qDebug() << "Invalid argument count for point coords";
                return;
            }

        } else {
            qDebug() << "Unknown SET_TEXT goal. Expected \"edge\" or \"point\"";
            return;
        }
        qDebug() << "Set text for" << type << idOrCoordinates << "to" << text;

    } else if (action == "RESET_COLORS") {
        resetAllColors();
        qDebug() << "Reset colors for all elements";

    } else if (action == "REMOVE_TEXT" && parts.size() == 3) {
        QString type = parts[1]; // edge или point
        QString idOrCoordinates = parts[2];

        if (type == "edge") {
            if (!idOrCoordinates.contains(',')) {
                qDebug() << "Not found SET_TEXT edge separator";
                return;
            }
            QStringList _buff = idOrCoordinates.split(',');
            if (_buff.size() != 2) {
                qDebug() << "Invalid argument count for edge id";
                return;
            }
            removeTextFromEdge(_buff[0].toInt(), _buff[1].toInt());
        } else if (type == "point") {
            if (!idOrCoordinates.contains(',')) {
                qDebug() << "Not found SET_TEXT point separator";
                return;
            }
            QStringList _buff = idOrCoordinates.split(',');
            if (_buff.size() != 2) {
                qDebug() << "Invalid argument count for point coordinates";
                return;
            }
            removeTextAtPoint(_buff[0].toInt(), _buff[1].toInt());
        } else {
            qDebug() << "Unknown REMOVE_TEXT goal. Expected \"edge\" or \"point\"";
            return;
        }
        qDebug() << "Removed text for" << type << idOrCoordinates;

    } else if (action == "REMOVE_ALL_TEXT") {
        removeAllText();
    } else if (action == "REMOVE_TYPE_TEXT" && parts.size() == 2) {
        QString type = parts[1];
        if (type != "edge" && type != "point") {
            qDebug() << "Unknown REMOVE_TYPE_TEXT goal. Expected \"edge\" or \"point\"";
            return;
        }
        removeTypeText(type);
    } else {
        qDebug() << "Unknown command:" << command;
    }
}

void Sequencer::set_node_color(QString color, unsigned int index)
{
    if (color == "DEFAULT") {
        this->graphView->getNodes()->find(index).value()->resetColor();
        return;
    }
    this->graphView->getNodes()->find(index).value()->setCurrentColor(color);
}

void Sequencer::set_edge_color(QString color, unsigned int srcNodeIndex, unsigned int destNodeIndex)
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

    Edge *currentEdge = this->graphView->getEdges()->value(key);

    if (color == "DEFAULT") {
        currentEdge->resetColor();
        if (currentEdge->getEdgeType() == EdgeType::BiDirectionalSame) {
            key = QPair<Node *, Node *>(this->graphView->getNodes()->value(destNodeIndex, nullptr),
                                        this->graphView->getNodes()->value(srcNodeIndex, nullptr));
            if (!graphView->getEdges()->contains(key))
                throw std::runtime_error(QString("No bidirectional edge found for %1,%2")
                                             .arg(srcNodeIndex)
                                             .arg(destNodeIndex)
                                             .toStdString());
            this->graphView->getEdges()->value(key)->resetColor();
        }
        return;
    }

    currentEdge->setCurrentColor(color);
    if (currentEdge->getEdgeType() == EdgeType::BiDirectionalSame) {
        key = QPair<Node *, Node *>(this->graphView->getNodes()->value(destNodeIndex, nullptr),
                                    this->graphView->getNodes()->value(srcNodeIndex, nullptr));
        if (!graphView->getEdges()->contains(key))
            throw std::runtime_error(QString("No bidirectional edge found for %1,%2")
                                         .arg(srcNodeIndex)
                                         .arg(destNodeIndex)
                                         .toStdString());
        this->graphView->getEdges()->value(key)->setCurrentColor(color);
    }
}

void Sequencer::drawTextAtEdge(QString text, int source, int destination)
{
    QPair<Node *, Node *> key_ptrs
        = QPair<Node *, Node *>(this->graphView->getNodes()->value(source, nullptr),
                                this->graphView->getNodes()->value(destination, nullptr));
    if (!key_ptrs.first || !key_ptrs.second) {
        throw std::runtime_error("Source/dest node not found");
    }
    if (!graphView->getEdges()->contains(key_ptrs))
        throw std::runtime_error(
            QString("No such edge found for %1,%2").arg(source).arg(destination).toStdString());

    QString key = QString::number(source) + ',' + QString::number(destination) + ":edge";
    if (textBoxes.contains(key)) {
        textBoxes[key]->setCustomText(text);
        return;
    }

    TextBox *currentTextBox = &(this->graphView->getEdges()->value(key_ptrs)->info);
    currentTextBox->setCustomText(text);
    textBoxes.insert(key, currentTextBox);
}

void Sequencer::removeTextFromEdge(int source, int destination)
{
    QPair<Node *, Node *> key_ptrs
        = QPair<Node *, Node *>(this->graphView->getNodes()->value(source, nullptr),
                                this->graphView->getNodes()->value(destination, nullptr));
    if (!key_ptrs.first || !key_ptrs.second) {
        throw std::runtime_error("Source/dest node not found");
    }
    if (!graphView->getEdges()->contains(key_ptrs))
        throw std::runtime_error(
            QString("No such edge found for %1,%2").arg(source).arg(destination).toStdString());

    QString key = QString::number(source) + ',' + QString::number(destination) + ":edge";
    if (textBoxes.contains(key)) {
        textBoxes[key]->containsCustomText = false;
        return;
    } else {
        throw std::runtime_error(
            QString("No such textbox found for %1,%2").arg(source).arg(destination).toStdString());
    }
}

void Sequencer::drawTextAtPoint(QString text, int x, int y)
{
    QString key = QString::number(x) + ',' + QString::number(y) + ":point";
    if (textBoxes.contains(key)) {
        textBoxes[key]->setCustomText(text);
        return;
    }

    TextBox *currentTextBox = new TextBox();
    currentTextBox->setCustomText(text);
    currentTextBox->setZValue(1);
    currentTextBox->rectBackground = true;
    currentTextBox->moveTo(QPoint(x, y));

    this->graphView->scene()->addItem(currentTextBox);
    textBoxes.insert(key, currentTextBox);
}

void Sequencer::removeTextAtPoint(int x, int y)
{
    QString key = QString::number(x) + ',' + QString::number(y) + ":point";
    if (textBoxes.contains(key)) {
        TextBox *textBoxtoDelete = textBoxes.take(key);
        graphView->scene()->removeItem(textBoxtoDelete);
        delete textBoxtoDelete;
    } else {
        throw std::runtime_error(
            QString("No such textbox found in coords %1,%2").arg(x).arg(y).toStdString());
    }
}

void Sequencer::removeAllText()
{
    for (auto tb : textBoxes.asKeyValueRange()) {
        if (tb.first.endsWith("edge")) {
            tb.second->containsCustomText = false;
        } else {
            graphView->scene()->removeItem(tb.second);
            delete tb.second;
        }
    }
    textBoxes.clear();
}

void Sequencer::removeTypeText(QString type)
{
    if (type == "edge") {
        for (auto tb : textBoxes.asKeyValueRange()) {
            if (tb.first.endsWith("edge")) {
                tb.second->containsCustomText = false;
            }
        }
    } else if (type == "point") {
        for (auto it = textBoxes.begin(); it != textBoxes.end();) {
            if (it.key().endsWith("point")) {
                graphView->scene()->removeItem(it.value());
                it = textBoxes.erase(it);
            } else {
                ++it;
            }
        }
    }
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
    for (auto it = textBoxes.constBegin(); it != textBoxes.constEnd(); ++it) {
        TextBox *textbox = it.value();
        textbox->update();
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
    for (auto tb : textBoxes.asKeyValueRange()) {
        if (tb.first.endsWith("edge")) {
            tb.second->containsCustomText = false;
        } else {
            graphView->scene()->removeItem(tb.second);
            delete tb.second;
        }
    }
    textBoxes.clear();
    resetAllColors();
    frames.clear();
    graphView->initScene();
    position = -1;
}
