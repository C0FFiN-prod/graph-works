#include "graph.h"

Graph::Graph():
    amount(0),
    adjacent(Matrix2D(10, QList<double>(10, 0))),
    flow(Matrix2D(10, QList<double>(10, 0)))
    {};
Graph::Graph(unsigned int size):
    amount(size),
    adjacent(Matrix2D(size, QList<double>(size, 0))),
    flow(Matrix2D(size, QList<double>(size, 0)))
    {};

Graph::Graph(Matrix2D matrix):
    amount(matrix.size()),
    adjacent(matrix),
    flow(Matrix2D(amount, QList<double>(amount, 0)))
{
    unsigned int i, j;
    for( i = 0; i != amount; i++){
        for(j = i; j != amount; j++){
            if(nodes.size() < j){
                nodes.insert(j, qMove(Node(j, 0, 0)));
            }
            if (matrix[i][j]){
                nodes[i].connectToNode(&nodes[j]);
                edges.insert(qMakePair(i, j), qMove(Edge(matrix[i][j], 0)));
            }
            if (matrix[j][i]){
                nodes[j].connectToNode(&nodes[i]);
                edges.insert(qMakePair(j, i), qMove(Edge(matrix[j][i], 0)));
            }
        }
    }
}

const Matrix2D Graph::getMatrixAdjacent()
{
    return adjacent;
}

const Matrix2D Graph::getMatrixFlow()
{
    return flow;
}


void Graph::setMatrixAdjacent(Matrix2D matrix)
{
    unsigned int i, j, oldAmount = amount;
    short newAmountIsLess = amount > matrix.size(), needResize = this->amount != matrix.size();

    amount = matrix.size();
    if(needResize){
        adjacent.resize(amount);
        if(!newAmountIsLess){
            flow.resize(amount);
        }
    }
    for(i = 0; i != amount; i++){
        if(needResize){
            adjacent[i].resize(amount);
            if (newAmountIsLess){
                for(j = amount; j != oldAmount; j++){
                    edges.remove(qMakePair(i,j));
                    edges.remove(qMakePair(j,i));
                }
            }
        }
        for(j = i; j != amount; j++){
            if(adjacent[i][j]&&!matrix[i][j])
                edges.remove(qMakePair(i,j));
            else if(!adjacent[i][j]&&matrix[i][j])
                edges.insert(qMakePair(i,j), qMove(Edge(matrix[i][j], 0)));
            else{
                edges[qMakePair(i, j)].weight = matrix[i][j];
                edges[qMakePair(i, j)].flow = fmin(matrix[i][j], adjacent[i][j]);
            }
            if(adjacent[j][i]&&!matrix[j][i])
                edges.remove(qMakePair(i,j));
            else if(!adjacent[j][i]&&matrix[j][i])
                edges.insert(qMakePair(j,i), qMove(Edge(matrix[j][i], 0)));
            else{
                edges[qMakePair(j, i)].weight = matrix[j][i];
                edges[qMakePair(j, i)].flow = fmin(matrix[j][i], adjacent[j][i]);
            }
            adjacent[i][j] = matrix[i][j];
        }
    }
}

void Graph::setMatrixFlow(Matrix2D matrix)
{
    unsigned int i, j, oldAmount = amount;
    short newAmountIsLess = amount > matrix.size(), needResize = this->amount != matrix.size();

    amount = matrix.size();
    if(needResize){
        if(!newAmountIsLess)
            adjacent.resize(amount);
        flow.resize(amount);
        if(newAmountIsLess){
            for(i = amount; i != oldAmount; i++){
                nodes.remove(i);
            }
        }else{
            for(i = oldAmount; i != amount; i++){
                nodes.insert(i, qMove(Node(i)));
            }
        }
    }
    for(i = 0; i != amount; i++){
        if(needResize){
            if(!newAmountIsLess)
                adjacent[i].resize(amount);
            flow[i].resize(amount);
            if (newAmountIsLess){
                for(j = amount; j != oldAmount; j++){
                    edges.remove(qMakePair(i,j));
                    edges.remove(qMakePair(j,i));
                }
            }
        }
        for(j = i; j != amount; j++){
            if(!adjacent[i][j]&&matrix[i][j]){
                edges.insert(qMakePair(i,j), qMove(Edge(matrix[i][j], matrix[i][j])));
                nodes[i].connectToNode(&nodes[j]);
            }
            else {
                edges[qMakePair(i, j)].weight = fmax(adjacent[i][j],matrix[i][j]);
                edges[qMakePair(i, j)].flow = matrix[i][j];
            }
            if(adjacent[j][i]&&!matrix[j][i]){
                edges.remove(qMakePair(i,j));
                nodes[j].disconnectFromNode(&nodes[i]);
            }
            else if(!adjacent[j][i]&&matrix[j][i]){
                edges.insert(qMakePair(j,i), qMove(Edge(matrix[i][j], matrix[j][i])));
                nodes[j].connectToNode(&nodes[i]);
            }
            else {
                edges[qMakePair(j, i)].weight = fmax(adjacent[j][i],matrix[j][i]);
                edges[qMakePair(i, j)].flow = matrix[i][j];
            }
            flow[i][j] = matrix[i][j];
            flow[j][i] = matrix[j][i];
            adjacent[i][j] = fmax(adjacent[i][j],matrix[i][j]);
            adjacent[j][i] = fmax(adjacent[j][i],matrix[j][i]);
        }
    }
}

void Graph::setEdge(unsigned int u, unsigned int v, Edge &edge)
{
    auto key = qMakePair(u,v);
    if(edges.contains(key))
        addEdge(u, v, edge);
    else {
        edges[key] = edge;
    }
}

void Graph::setEdgeFlow(unsigned int u, unsigned int v, double flow)
{
    auto key = qMakePair(u,v);
    if(!edges.contains(key))
        throw std::runtime_error("No such edge");
    edges[key].flow = flow;
}

void Graph::removeEdge(unsigned int u, unsigned int v)
{
    auto key = qMakePair(u,v);
    if(edges.contains(key)){
        edges.remove(key);
        nodes[u].disconnectFromNode(&nodes[v]);
        nodes[v].disconnectFromNode(&nodes[u]);
    }
    adjacent[u][v] = 0;
    flow[u][v] = 0;
}

void Graph::addNode(unsigned int i, int x, int y)
{
    if(nodes.contains(i))
        throw std::runtime_error("Node already exists");
    nodes.insert(i, qMove(Node(i, x, y)));
    short adjacentIsLess = adjacent.capacity() < amount+1;
    short flowIsLess = flow.capacity() < amount+1;
    if (adjacentIsLess||flowIsLess){
        if(adjacentIsLess)
            adjacent.resize(amount+1);
        if(flowIsLess)
            flow.resize(amount+1);
        for(unsigned int j = 0; j != amount+1; j++){
            if(adjacentIsLess)
                adjacent[j].resize(amount+1);
            if(flowIsLess)
                flow[j].resize(amount+1);

        }
    }
}

void Graph::addEdge(unsigned int u, unsigned int v, Edge& edge)
{
    edges.insert(qMakePair(u,v), edge);
    adjacent[u][v] = edge.weight;
    flow[u][v] = edge.flow;
}

