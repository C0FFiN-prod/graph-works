#include "node.h"


Node::Node():number(0), x(0), y(0){}

Node::Node(unsigned int i):number(i), x(0), y(0){}

Node::Node(unsigned int i, int x, int y):number(i), x(x), y(y){}

unsigned int Node::getNumber()
{
    return number;
}

void Node::setNumber(unsigned int i)
{
    number = i;
}

void Node::moveTo(int x, int y)
{
    this->x=x;
    this->y=y;
}

void Node::move(int dx, int dy)
{
    this->x+=dx;
    this->y+=dy;
}

void Node::connectToNode(Node *node)
{
    this->children.insert(node);
    node->parents.insert(this);
}

void Node::disconnectFromNode(Node *node)
{
    if(children.find(node)!=children.end()){
        this->children.erase(node);
        node->parents.erase(this);
    }
}
