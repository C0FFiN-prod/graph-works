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
    int getPosition();
    int getFramesLength();

private:
    int position = -1;
    GraphWidget* graphView;
    QList<QList<QString>> frames;
    QMap<QString, TextBox*> textBoxes;
    unsigned int delay = 1000;

    //commands
    void set_node_color(QString color, unsigned int index);
    void set_edge_color(QString color, unsigned int srcNodeIndex, unsigned int destNodeIndex);
    void resetAllColors();
    void drawTextAtEdge(QString text, int source, int destination);
    void removeTextFromEdge(int source, int destination);
    void drawTextAtPoint(QString text, int x, int y);
    void removeTextAtPoint(int x, int y);
    void goToFrame(int targetFrameInd);

    void removeAllText();
    void removeTypeText(QString type);
    /*TODO
     * 1. set color node/edge default SET_COLOR node 1,2 DEFAULT
     * 2.
    */
};

#endif // SEQUENCER_H
