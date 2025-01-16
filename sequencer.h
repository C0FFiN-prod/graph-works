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
};

#endif // SEQUENCER_H
