#include "graph.h"

Graph::Graph():
    amount(0),
    adjacent(Matrix2D(10, QList<double>(10, 0))),
    flow(Matrix2D(10, QList<double>(10, 0))),
    bandwidth(Matrix2D(10, QList<double>(10, 0)))
    {};
Graph::Graph(unsigned int size):
    amount(size),
    adjacent(Matrix2D(size, QList<double>(size, 0))),
    flow(Matrix2D(size, QList<double>(size, 0))),
    bandwidth(Matrix2D(size, QList<double>(size, 0)))
    {};

Graph::Graph(Matrix2D matrix):
    amount(matrix.size()),
    adjacent(matrix),
    flow(Matrix2D(amount, QList<double>(amount, 0))),
    bandwidth(Matrix2D(amount, QList<double>(amount, 0)))
{
    unsigned int i, j;
    for( i = 0; i != amount; i++){
        for(j = i; j != amount; j++){
            if(nodes.size() < j){
                nodes.insert(j, qMove(Node(j, 0, 0)));
            }
            if (matrix[i][j]){
                nodes[i].connectAdd(&nodes[j]);
                edges.insert(qMakePair(i, j), qMove(Edge(matrix[i][j], 0, 0)));
            }
            if (matrix[j][i]){
                nodes[j].connectAdd(&nodes[i]);
                edges.insert(qMakePair(j, i), qMove(Edge(matrix[j][i], 0, 0)));
            }
        }
    }
}

