#include "graph.h"

Graph::Graph():
    amount(0),
    adjacent(Matrix2D(10, QList<double>(10, 0))),
    flow(Matrix2D(10, QList<double>(10, 0)))
    {
    graphView = new GraphWidget(&edges, &nodes);
}

Graph::Graph(unsigned int size):
    amount(size),
    adjacent(Matrix2D(size, QList<double>(size, 0))),
    flow(Matrix2D(size, QList<double>(size, 0)))
    {
    graphView = new GraphWidget(&edges, &nodes);
}

Graph::Graph(Matrix2D matrix):
    amount(matrix.size()),
    adjacent(matrix),
    flow(Matrix2D(amount, QList<double>(amount, 0)))
{

    graphView = new GraphWidget(&edges, &nodes);
    //create nodes accoring to matrix size
    for(unsigned int i = 0; i!= amount; i++){ //adding all nodes
        //Node *node1 = new Node(i, graphView);
        nodes[i] = new Node(i, graphView);
    }


    //create edges with edge type definition
    for(unsigned int i = 0; i!=amount; i++){
        for(unsigned int j = i; j!=amount; j++){
            if(i!=j){
                Edge *edge1 = nullptr,  *edge2 = nullptr;
                if(matrix[i][j]==matrix[j][i] && matrix[i][j]!=0 &&i<j){
                    edge1 = new Edge(nodes[i], nodes[j], matrix[i][j], EdgeType::BiDirectionalSame );

                }else if(matrix[i][j]!=0 && matrix[j][i]!=0 && matrix[i][j]!=matrix[j][i]){
                    edge1 = new Edge(nodes[i], nodes[j], matrix[i][j], EdgeType::BiDirectionalDiff );
                    edge2 = new Edge(nodes[j], nodes[i], matrix[j][i], EdgeType::BiDirectionalDiff );

                }else if(matrix[i][j]!=0 && matrix[j][j]==0){
                    edge1 = new Edge(nodes[i], nodes[j], matrix[i][j], EdgeType::SingleDirection );

                }else if(matrix[j][i]!=0 && matrix[i][j]==0){
                    edge2 = new Edge(nodes[j], nodes[i], matrix[j][i], EdgeType::SingleDirection );

                }
                if(edge1){
                    edges[qMakePair(nodes[i], nodes[j])] = edge1;

                }
                if(edge2){
                    edges[qMakePair(nodes[j], nodes[i])] = edge2;

                }
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
                    edges.remove(qMakePair(nodes[i],nodes[j]));
                    edges.remove(qMakePair(nodes[j],nodes[i]));
                }
            }
        }
        for(j = i; j != amount; j++){
            if(adjacent[i][j]&&!matrix[i][j])
                edges.remove(qMakePair(nodes[i],nodes[j]));
            else if(!adjacent[i][j]&&matrix[i][j])
                edges[qMakePair(nodes[i],nodes[j])] = new Edge(nodes[i],nodes[j], matrix[i][j], fmin(matrix[i][j], adjacent[i][j]), getEdgeType(i,j));
            else{
                edges[qMakePair(nodes[i],nodes[j])]->setWeight(matrix[i][j]);
                edges[qMakePair(nodes[i],nodes[j])]->setFlow(fmin(matrix[i][j], adjacent[i][j]));
            }
            if(adjacent[j][i]&&!matrix[j][i])
                edges.remove(qMakePair(nodes[i],nodes[j]));
            else if(!adjacent[j][i]&&matrix[j][i])
                edges[qMakePair(nodes[j],nodes[i])] =new Edge(nodes[j],nodes[i], matrix[j][i], fmin(matrix[j][i], adjacent[j][i]), getEdgeType(j,i));
            else{
                edges[qMakePair(nodes[j],nodes[i])]->setWeight(matrix[j][i]);
                edges[qMakePair(nodes[j],nodes[i])]->setFlow(fmin(matrix[j][i], adjacent[j][i]));
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
                //nodes.remove(i);
            }
        }else{
            for(i = oldAmount; i != amount; i++){
                //nodes.insert(i, qMove(Node(i, graph)));
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
                    edges.remove(qMakePair(nodes[i],nodes[j]));
                    edges.remove(qMakePair(nodes[j],nodes[i]));
                }
            }
        }
        for(j = i; j != amount; j++){
            if(!adjacent[i][j]&&matrix[i][j]){
                edges[qMakePair(nodes[i],nodes[j])] = new Edge(nodes[i],nodes[j], matrix[i][j], matrix[i][j], getEdgeType(i,j));
                //edges.insert(qMakePair(i,j), qMove(EdgeStruct(matrix[i][j], matrix[i][j])));
                //nodes[i].connectToNode(&nodes[j]);
            }
            else {
                edges[qMakePair(nodes[i],nodes[j])]->setWeight(fmax(adjacent[i][j],matrix[i][j]));
                edges[qMakePair(nodes[i],nodes[j])]->setFlow(matrix[i][j]);
            }
            if(adjacent[j][i]&&!matrix[j][i]){
                edges.remove(qMakePair(nodes[i],nodes[j]));
                //nodes[j].disconnectFromNode(&nodes[i]);
            }
            else if(!adjacent[j][i]&&matrix[j][i]){
                edges[qMakePair(nodes[j],nodes[i])] = new Edge(nodes[j],nodes[i], matrix[i][j], matrix[j][i], getEdgeType(j,i));
                //nodes[j].connectToNode(&nodes[i]);
            }
            else {
                edges[qMakePair(nodes[j],nodes[i])]->setWeight(fmax(adjacent[j][i],matrix[j][i]));
                edges[qMakePair(nodes[i],nodes[j])]->setFlow(matrix[i][j]);
            }
            flow[i][j] = matrix[i][j];
            flow[j][i] = matrix[j][i];
            adjacent[i][j] = fmax(adjacent[i][j],matrix[i][j]);
            adjacent[j][i] = fmax(adjacent[j][i],matrix[j][i]);
        }
    }
}



void Graph::setEdgeFlow(unsigned int u, unsigned int v, double flow)
{
    auto key = qMakePair(nodes[u],nodes[v]);
    if(!edges.contains(key))
        throw std::runtime_error("No such edge");
    edges[key]->setFlow(flow);
}

EdgeType Graph::getEdgeType(int i, int j)
{
    if(adjacent[i][j]==adjacent[j][i] && adjacent[i][j]!=0 &&i<j){
        return EdgeType::BiDirectionalSame;

    }else if(adjacent[i][j]!=0 && adjacent[j][i]!=0 && adjacent[i][j]!=adjacent[j][i]){
        return EdgeType::BiDirectionalDiff;

    }else if((adjacent[i][j]!=0 && adjacent[j][j]==0)||(adjacent[j][i]!=0 && adjacent[i][j]==0)){
        return EdgeType::SingleDirection;

    }
    return Error;
}

void Graph::removeEdge(unsigned int u, unsigned int v)
{
    auto key = qMakePair(nodes[u],nodes[v]);
    if(edges.contains(key)){
        edges.remove(key);
        //nodes[u].disconnectFromNode(&nodes[v]);
        //nodes[v].disconnectFromNode(&nodes[u]);
    }
    adjacent[u][v] = 0;
    flow[u][v] = 0;
}

// void Graph::addNode(unsigned int i, int x, int y)
// {
//     if(nodes.contains(i))
//         throw std::runtime_error("Node already exists");
//     //nodes.insert(i, qMove(Node(i, graph)));
//     short adjacentIsLess = adjacent.capacity() < amount+1;
//     short flowIsLess = flow.capacity() < amount+1;
//     if (adjacentIsLess||flowIsLess){
//         if(adjacentIsLess)
//             adjacent.resize(amount+1);
//         if(flowIsLess)
//             flow.resize(amount+1);
//         for(unsigned int j = 0; j != amount+1; j++){
//             if(adjacentIsLess)
//                 adjacent[j].resize(amount+1);
//             if(flowIsLess)
//                 flow[j].resize(amount+1);

//         }
//     }
// }

