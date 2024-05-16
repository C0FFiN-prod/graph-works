#include "graph.h"

Graph::Graph():
    amount(0),
    adjacent(Matrix2D(10, QList<double>(10, 0))),
    flow(Matrix2D(10, QList<double>(10, 0))),
    bandwidth(Matrix2D(10, QList<double>(10, 0)))
{
    graphView = new GraphWidget(&edges, &nodes, &flags);
    Edge::setFlagsPtr(&flags);
}

Graph::Graph(unsigned int size):
    amount(size),
    adjacent(Matrix2D(size, QList<double>(size, 0))),
    flow(Matrix2D(size, QList<double>(size, 0)))
    {
    graphView = new GraphWidget(&edges, &nodes, &flags);
    Edge::setFlagsPtr(&flags);
}

Graph::Graph(Matrix2D &matrix):
    amount(matrix.size()),
    adjacent(matrix),
    flow(Matrix2D(amount, QList<double>(amount, 0)))
{
    graphView = new GraphWidget(&edges, &nodes, &flags);
    Edge::setFlagsPtr(&flags);
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

Graph::~Graph()
{
    for(auto edge : std::as_const(edges)){
        delete edge;
    }
    for(auto node : std::as_const(nodes)){
        delete node;
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

const Matrix2D Graph::getMatrixBandwidth()
{
    return bandwidth;
}

const QList<QVariantList> Graph::getListEdges()
{
    QList<QVariantList> edgeList(edges.size());
    unsigned int i = 0;
    for(auto [key, edge] : edges.asKeyValueRange()){
        QVariantList tuple{key.first->getIndex(), key.second->getIndex(), edge->getWeight(), edge->getBandwidth(), edge->getFlow()};
        edgeList[i++] = tuple;
    }
    return edgeList;
}


void Graph::setMatrixAdjacent(Matrix2D &matrix)
{
    unsigned int i, j;
    resizeGraph(amount, matrix.size());

    for(i = 0; i != amount; i++){
        for(j = i; j != amount; j++){
            if(adjacent[i][j]&&!matrix[i][j]){
                removeEdge(i, j);
            }
            else if(!adjacent[i][j]&&matrix[i][j]){
                nodes[i]->connectToNode(nodes[j]);
                edges[qMakePair(nodes[i],nodes[j])] = new Edge(nodes[i],nodes[j], matrix[i][j], getEdgeType(i,j,matrix));

            }
            else if(adjacent[i][j]&&matrix[i][j]){
                auto edge = edges[qMakePair(nodes[i],nodes[j])];
                edge->setWeight(matrix[i][j]);
                edge->setEdgeType(getEdgeType(i, j, matrix));
            }
            if((i!=j)&&adjacent[j][i]&&!matrix[j][i]){
                removeEdge(j, i);
            }
            else if((i!=j)&&!adjacent[j][i]&&matrix[j][i]){
                nodes[j]->connectToNode(nodes[i]);
                edges[qMakePair(nodes[j],nodes[i])] = new Edge(nodes[j],nodes[i], matrix[j][i], getEdgeType(j,i,matrix));
            }
            else if ((i!=j)&&adjacent[j][i]&&matrix[j][i]){
                auto edge = edges[qMakePair(nodes[j],nodes[i])];
                edge->setWeight(matrix[j][i]);
                edge->setEdgeType(getEdgeType(j, i, matrix));
            }
            adjacent[i][j] = matrix[i][j];
            adjacent[j][i] = matrix[j][i];
        }
    }
    unsavedChanges = false;
}

void Graph::setMatrixFlow(Matrix2D &matrix)
{
    unsigned int i, j;
    resizeGraph(amount, matrix.size());
    unsavedChanges = false;
    for(i = 0; i != amount; i++){
        for(j = i; j != amount; j++){
            if(!adjacent[i][j]&&matrix[i][j]){
                nodes[i]->connectToNode(nodes[j]);
                auto key = qMakePair(nodes[i],nodes[j]);
                edges[key] = new Edge(nodes[i],nodes[j], 1, getEdgeType(i,j,matrix));
                edges[key]->setFlow(matrix[i][j]);
                adjacent[i][j] = 1;
                unsavedChanges = true;
            }
            else if(adjacent[i][j]){
                auto edge = edges[qMakePair(nodes[i],nodes[j])];
                edge->setBandwidth(fmax(matrix[i][j], edge->getBandwidth()));
                edge->setFlow(matrix[i][j]);
            }
            if((i!=j)&&!adjacent[j][i]&&matrix[j][i]){
                nodes[j]->connectToNode(nodes[i]);
                auto key = qMakePair(nodes[j],nodes[i]);
                edges[key] = new Edge(nodes[j],nodes[i], 1, getEdgeType(j,i,matrix));
                edges[key]->setFlow(matrix[j][i]);
                adjacent[j][i] = 1;
                unsavedChanges = true;
            }
            else if ((i!=j)&&adjacent[j][i]){
                auto edge = edges[qMakePair(nodes[j],nodes[i])];
                edge->setBandwidth(fmax(matrix[j][i], edge->getBandwidth()));
                edge->setFlow(matrix[j][i]);
            }
            flow[i][j] = matrix[i][j];
            flow[j][i] = matrix[j][i];
        }
    }
}

void Graph::setMatrixBandwidth(Matrix2D& matrix)
{
    unsigned int i, j;
    resizeGraph(amount, matrix.size());
    unsavedChanges = false;
    for(i = 0; i != amount; i++){
        for(j = i; j != amount; j++){
            if(!adjacent[i][j]&&matrix[i][j]){
                nodes[i]->connectToNode(nodes[j]);
                auto key = qMakePair(nodes[i],nodes[j]);
                edges[key] = new Edge(nodes[i],nodes[j], 1, getEdgeType(i,j,matrix));
                edges[key]->setBandwidth(matrix[i][j]);
                adjacent[i][j] = 1;
                unsavedChanges = true;
            }
            else if(adjacent[i][j]){
                auto edge = edges[qMakePair(nodes[j],nodes[i])];
                edge->setBandwidth(matrix[j][i]);
                edge->setFlow(fmin(matrix[j][i], edge->getFlow()));
            }
            if((i!=j)&&!adjacent[j][i]&&matrix[j][i]){
                nodes[j]->connectToNode(nodes[i]);
                auto key = qMakePair(nodes[j],nodes[i]);
                edges[key] = new Edge(nodes[j],nodes[i], 1, getEdgeType(j,i,matrix));
                edges[key]->setBandwidth(matrix[j][i]);
                adjacent[j][i] = 1;
                unsavedChanges = true;
            }
            else if ((i!=j)&&adjacent[j][i]){
                auto edge = edges[qMakePair(nodes[j],nodes[i])];
                edge->setBandwidth(matrix[j][i]);
                edge->setFlow(fmin(matrix[j][i], edge->getFlow()));
            }
            bandwidth[i][j] = matrix[i][j];
            bandwidth[j][i] = matrix[j][i];
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

EdgeType Graph::getEdgeType(int i, int j, Matrix2D& matrix)
{
    if(matrix[i][j]==matrix[j][i] && matrix[i][j]!=0 &&i<j){
        return EdgeType::BiDirectionalSame;

    }else if(matrix[i][j]!=0 && matrix[j][i]!=0 && matrix[i][j]!=matrix[j][i]){
        return EdgeType::BiDirectionalDiff;

    }else if((matrix[i][j]!=0 && matrix[j][i]==0)||(matrix[j][i]!=0 && matrix[i][j]==0)){
        return EdgeType::SingleDirection;

    }else if(i==j && matrix[i][j]!=0){
        return EdgeType::Loop;
    }
    return Error;
}

void Graph::removeEdge(unsigned int u, unsigned int v)
{
    auto key = qMakePair(nodes[u],nodes[v]);
    if(edges.contains(key)){
        graphView->scene()->removeItem(edges[key]);
        nodes[u]->disconnectFromNode(nodes[v]);
        delete edges.take(key);
    }
    if (u<amount && v<amount){
        adjacent[u][v] = 0;
        flow[u][v] = 0;
        bandwidth[u][v] = 0;
    }
}

void Graph::addNode(unsigned int i)
{
    if(nodes.contains(i))
        throw std::runtime_error("Node already exists");
    nodes[i] = new Node(i, graphView);
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

const QFlags<GraphFlags> Graph::getFlags()
{
    return flags;
}

void Graph::setFlag(GraphFlags flag)
{
    flags|=flag;
}

void Graph::setFlags(QFlags<GraphFlags> flags)
{
    this->flags = flags;
}

void Graph::unsetFlag(GraphFlags flag)
{
    this->flags &= (~flag);
}

void Graph::toggleFlag(GraphFlags flag)
{
    this->flags^=flag;
}

bool Graph::isUnsaved()
{
    return unsavedChanges;
}

void Graph::resizeGraph(unsigned int oldAmount, unsigned int newAmount)
{
    unsigned int i, j;
    bool newAmountIsLess = oldAmount > newAmount;
    bool needResize = amount != newAmount;
    //increase nodes amount
    if(needResize){
        amount = newAmount;
        adjacent.resize(newAmount);
        flow.resize(newAmount);
        bandwidth.resize(newAmount);
        for(i = 0; i != newAmount; i++){
            adjacent[i].resize(newAmount);
            bandwidth[i].resize(newAmount);
            flow[i].resize(newAmount);
        }
        if (newAmountIsLess){
            for(i = 0; i != oldAmount; i++){
                for(j = newAmount; j != oldAmount; j++){
                    removeEdge(i, j);
                    if(i!=j)
                        removeEdge(j, i);
                }
            }
        }
        if(newAmountIsLess)
            for(i = newAmount; i != oldAmount; i++){
                if(nodes.contains(i)){
                    graphView->scene()->removeItem(nodes[i]);
                    delete nodes.take(i);
                }
            }
        else
            for(i = oldAmount; i<newAmount; i++){
                nodes[i]= new Node(i, graphView);
            }

    }
}

