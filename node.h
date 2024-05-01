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
    void moveTo(int x, int y);
    void move(int dx, int dy);
    void connectAdd(Node* node);
    void connectRemove(Node* node);
private:
    std::set<Node*> connects;
    unsigned int number;
    int x, y;
};

#endif // NODE_H
