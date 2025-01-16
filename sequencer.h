#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "graphwidget.h"

class Sequencer
{
public:
    Sequencer(GraphWidget* graphView);
    void setDelay(unsigned int ms);
    int addFrame(int frameIndex = -1);
    void removeFrame(int frameIndex = -1);
    void addCommand(const QString& command, int frameIndex = -1);
    void processCommand(const QString &command);
    void clear();
    void first();
    void last();
    void next();
    void prev();
    void play();
    void draw();
private:
    int position = -1;
    GraphWidget* graphView;
    QList<QList<QString>> frames;
    unsigned int delay = 1000;

    //commands
    void set_node_color(QColor color, unsigned int index);
    void set_edge_color(QColor color, unsigned int srcNodeIndex, unsigned int destNodeIndex);
    void resetAllColors();
};

#endif // SEQUENCER_H
