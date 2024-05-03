#ifndef NODE_H
#define NODE_H

#include <set>
class Node
{
public:
    Node();;
    Node(unsigned int i);;
    Node(unsigned int i, int x, int y);;
    unsigned int getNumber();
    void setNumber(unsigned int i);
    void moveTo(int x, int y);
    void move(int dx, int dy);
    void connectToNode(Node* node);
    void disconnectFromNode(Node* node);
private:
    std::set<Node*> children;
    std::set<Node*> parents;
    unsigned int number;
    int x, y;
};

#endif // NODE_H
