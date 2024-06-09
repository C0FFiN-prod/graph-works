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
    clear();
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
    unsavedChanges = true;
    for(i = 0; i != amount; i++){
        for(j = i; j != amount; j++){
            if(adjacent[i][j]&&!matrix[i][j]){
                removeEdge(i, j);
            }
            else if(!adjacent[i][j]&&matrix[i][j]){
                nodes[i]->connectToNode(nodes[j]);
                auto key = qMakePair(nodes[i],nodes[j]);
                edges[key] = new Edge(nodes[i],nodes[j], matrix[i][j], getEdgeType(i,j,matrix));
                edges[key]->setFlow(flow[i][j]);
                edges[key]->setBandwidth(bandwidth[i][j]);
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
                auto key = qMakePair(nodes[j],nodes[i]);
                edges[key] = new Edge(nodes[j],nodes[i], matrix[j][i], getEdgeType(j,i,matrix));
                edges[key]->setFlow(flow[j][i]);
                edges[key]->setBandwidth(bandwidth[j][i]);
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
}

void Graph::setMatrixFlow(Matrix2D &matrix)
{
    unsigned int i, j;
    resizeGraph(amount, matrix.size());
    unsavedChanges = true;
    for(i = 0; i != amount; i++){
        for(j = i; j != amount; j++){
            if(!adjacent[i][j]&&matrix[i][j]){
                nodes[i]->connectToNode(nodes[j]);
                auto key = qMakePair(nodes[i],nodes[j]);
                if(bandwidth[i][j] < matrix[i][j])
                    bandwidth[i][j] = matrix[i][j];
                edges[key] = new Edge(nodes[i],nodes[j], 1, getEdgeType(i,j,bandwidth));
                edges[key]->setBandwidth(matrix[i][j]);
                edges[key]->setFlow(matrix[i][j]);
                adjacent[i][j] = 1;
            }
            else if(adjacent[i][j]){
                auto edge = edges[qMakePair(nodes[i],nodes[j])];
                if(edge->getBandwidth() < matrix[i][j]){
                    bandwidth[i][j] = matrix[i][j];
                    edge->setBandwidth(matrix[i][j]);
                }
                edge->setFlow(matrix[i][j]);
                edge->setEdgeType(getEdgeType(i, j, bandwidth));
            }
            if((i!=j)&&!adjacent[j][i]&&matrix[j][i]){
                nodes[j]->connectToNode(nodes[i]);
                auto key = qMakePair(nodes[j],nodes[i]);
                if(bandwidth[j][i] < matrix[j][i])
                    bandwidth[j][i] = matrix[j][i];
                edges[key] = new Edge(nodes[j],nodes[i], 1, getEdgeType(j,i,bandwidth));
                edges[key]->setBandwidth(matrix[j][i]);
                edges[key]->setFlow(matrix[j][i]);
                adjacent[j][i] = 1;
            }
            else if ((i!=j)&&adjacent[j][i]){
                auto edge = edges[qMakePair(nodes[j],nodes[i])];
                if(edge->getBandwidth() < matrix[j][i]){
                    bandwidth[j][i] = matrix[j][i];
                    edge->setBandwidth(matrix[j][i]);
                }
                edge->setFlow(matrix[j][i]);
                edge->setEdgeType(getEdgeType(j, i, bandwidth));
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
    unsavedChanges = true;
    for(i = 0; i != amount; i++){
        for(j = i; j != amount; j++){
            if(adjacent[i][j]&&!matrix[i][j]){
                removeEdge(i, j);
            }else if(!adjacent[i][j]&&matrix[i][j]){
                nodes[i]->connectToNode(nodes[j]);
                auto key = qMakePair(nodes[i],nodes[j]);
                if(flow[i][j] > matrix[i][j])
                    flow[i][j] = matrix[i][j];
                edges[key] = new Edge(nodes[i],nodes[j], 1, getEdgeType(i,j,matrix));
                edges[key]->setBandwidth(matrix[i][j]);
                edges[key]->setFlow(flow[i][j]);
                adjacent[i][j] = 1;
            }
            else if(adjacent[i][j]){
                auto edge = edges[qMakePair(nodes[i],nodes[j])];
                edge->setBandwidth(matrix[i][j]);
                if(edge->getFlow() > matrix[i][j]){
                    flow[i][j] = matrix[i][j];
                    edge->setFlow(matrix[i][j]);
                }
                edge->setEdgeType(getEdgeType(i, j, matrix));
            }
            if((i!=j)&&adjacent[j][i]&&!matrix[j][i]){
                removeEdge(j, i);
            }else if((i!=j)&&!adjacent[j][i]&&matrix[j][i]){
                nodes[j]->connectToNode(nodes[i]);
                auto key = qMakePair(nodes[j],nodes[i]);
                if(flow[j][i] > matrix[j][i])
                    flow[j][i] = matrix[j][i];
                edges[key] = new Edge(nodes[j],nodes[i], 1, getEdgeType(j,i,matrix));
                edges[key]->setBandwidth(matrix[j][i]);
                edges[key]->setFlow(flow[j][i]);
                adjacent[j][i] = 1;
            }
            else if ((i!=j)&&adjacent[j][i]){
                auto edge = edges[qMakePair(nodes[j],nodes[i])];
                edge->setBandwidth(matrix[j][i]);
                if(edge->getFlow() > matrix[j][i]){
                    flow[j][i] = matrix[j][i];
                    edge->setFlow(matrix[j][i]);
                }
                edge->setEdgeType(getEdgeType(i, j, matrix));
            }
            bandwidth[i][j] = matrix[i][j];
            bandwidth[j][i] = matrix[j][i];
        }
    }
}

void Graph::setEdge(unsigned int u, unsigned int v, double w)
{
    auto key = qMakePair(nodes[u], nodes[v]);
    if (edges.contains(key))
        throw std::runtime_error("Edge already exists");
    unsavedChanges = true;
    adjacent[u][v] = w;
    EdgeType bandType = getEdgeType(u, v, bandwidth);
    EdgeType adjType = getEdgeType(u, v, adjacent);
    EdgeType type = (bandType > adjType) ? (bandType) : (adjType);
    edges[key] = new Edge(key.first, key.second, w, type);
}

void Graph::setEdgeFlow(unsigned int u, unsigned int v, double f)
{
    auto key = qMakePair(nodes[u],nodes[v]);
    if(!edges.contains(key))
        throw std::runtime_error("No such edge");
    unsavedChanges = true;
    edges[key]->setFlow(f);
    if (edges[key]->getBandwidth() < f) {
        bandwidth[u][v] = f;
    }
    flow[u][v] = f;
    EdgeType bandType = getEdgeType(u, v, bandwidth);
    EdgeType adjType = getEdgeType(u, v, adjacent);
    edges[key]->setEdgeType((bandType > adjType) ? (bandType) : (adjType));

    key = qMakePair(nodes[v], nodes[u]);
    if (edges.contains(key)) {
        bandType = getEdgeType(v, u, bandwidth);
        adjType = getEdgeType(v, u, adjacent);
        edges[key]->setEdgeType((bandType > adjType) ? (bandType) : (adjType));
    }
}

void Graph::setEdgeWeight(unsigned int u, unsigned int v, double w)
{
    auto key = qMakePair(nodes[u], nodes[v]);
    if (!edges.contains(key))
        throw std::runtime_error("No such edge");
    if (!w)
        throw std::runtime_error("Weight can't be zero");
    unsavedChanges = true;
    adjacent[u][v] = w;
    edges[key]->setWeight(w);
    EdgeType bandType = getEdgeType(u, v, bandwidth);
    EdgeType adjType = getEdgeType(u, v, adjacent);
    edges[key]->setEdgeType((bandType > adjType) ? (bandType) : (adjType));

    key = qMakePair(nodes[v], nodes[u]);
    if (edges.contains(key)) {
        bandType = getEdgeType(v, u, bandwidth);
        adjType = getEdgeType(v, u, adjacent);
        edges[key]->setEdgeType((bandType > adjType) ? (bandType) : (adjType));
    }
}

void Graph::setEdgeBandwidth(unsigned int u, unsigned int v, double b)
{
    auto key = qMakePair(nodes[u], nodes[v]);
    if (!edges.contains(key))
        throw std::runtime_error("No such edge");
    unsavedChanges = true;
    edges[key]->setBandwidth(b);
    if (edges[key]->getFlow() > b) {
        flow[u][v] = b;
    }
    bandwidth[u][v] = b;
    EdgeType bandType = getEdgeType(u, v, bandwidth);
    EdgeType adjType = getEdgeType(u, v, adjacent);
    edges[key]->setEdgeType((bandType > adjType) ? (bandType) : (adjType));

    key = qMakePair(nodes[v], nodes[u]);
    if (edges.contains(key)) {
        bandType = getEdgeType(v, u, bandwidth);
        adjType = getEdgeType(v, u, adjacent);
        edges[key]->setEdgeType((bandType > adjType) ? (bandType) : (adjType));
    }
}

QString Graph::getNodeName(unsigned int u)
{
    return nodes[u]->getDisplayName();
}

EdgeType Graph::getEdgeType(int i, int j, Matrix2D& matrix)
{
    if(i==j && matrix[i][j]!=0){
        return EdgeType::Loop;
    }else if(matrix[i][j]==matrix[j][i] && matrix[i][j]!=0){
        return EdgeType::BiDirectionalSame;

    }else if(matrix[i][j]!=0 && matrix[j][i]!=0 && matrix[i][j]!=matrix[j][i]){
        return EdgeType::BiDirectionalDiff;

    }else if((matrix[i][j]!=0 && matrix[j][i]==0)||(matrix[j][i]!=0 && matrix[i][j]==0)){
        return EdgeType::SingleDirection;
    }
    return Error;
}

void Graph::removeEdge(unsigned int u, unsigned int v)
{
    auto key = qMakePair(nodes[u],nodes[v]);
    if (u < amount && v < amount) {
        adjacent[u][v] = 0;
        flow[u][v] = 0;
        bandwidth[u][v] = 0;
    }
    if(edges.contains(key)){
        graphView->scene()->removeItem(edges[key]);
        nodes[u]->disconnectFromNode(nodes[v]);
        delete edges.take(key);

        key = qMakePair(nodes[v], nodes[u]);
        if (edges.contains(key) && (edges[key]->getEdgeType() != EdgeType::Error)) {
            edges[key]->setEdgeType(EdgeType::SingleDirection);
        }
    }
}
void Graph::removeNode(unsigned int index)
{
    if (!nodes.contains(index))
        throw std::runtime_error("No such node");

    Node *toRemove = nodes[index];
    unsavedChanges = true;
    for (auto &i : toRemove->edgeList) {
        int source = i->sourceNode()->getIndex();
        int dest = i->destNode()->getIndex();
        this->removeEdge(source, dest);
        this->removeEdge(dest, source);
    }
    toRemove = nodes.take(index);
    if (index < amount && toRemove != nullptr) {
        adjacent.removeAt(index);
        flow.removeAt(index);
        bandwidth.removeAt(index);

        for (unsigned i = 0; i < amount - 1; i++) {
            adjacent[i].removeAt(index);
            flow[i].removeAt(index);
            bandwidth[i].removeAt(index);
        }
    }

    for (unsigned int i = index + 1; i < amount; i++) {
        auto toChange = nodes.take(i);
        toChange->setIndex(i - 1);
        nodes.insert(i - 1, toChange);
    }
    if ((int) index == getSourceIndex()) {
        src = nullptr;
    } else if ((int) index == getDestIndex()) {
        dst = nullptr;
    }
    graphView->scene()->removeItem(toRemove);
    amount--;
    delete toRemove;
}

void Graph::addNode(unsigned int i, const QString &name = "")
{
    if(nodes.contains(i))
        throw std::runtime_error("Node already exists");
    unsavedChanges = true;
    nodes[i] = new Node(i, graphView);
    if (!name.isEmpty())
        nodes[i]->setDisplayName(name);
    bool adjacentIsLess = adjacent.size() < amount + 1;
    bool flowIsLess = flow.size() < amount + 1;
    bool bandwidthIsLess = bandwidth.size() < amount + 1;
    if (adjacentIsLess || flowIsLess || bandwidthIsLess) {
        if(adjacentIsLess)
            adjacent.resize(amount+1);
        if(flowIsLess)
            flow.resize(amount+1);
        if (bandwidthIsLess)
            bandwidth.resize(amount + 1);
        for(unsigned int j = 0; j != amount+1; j++){
            if(adjacentIsLess)
                adjacent[j].resize(amount+1);
            if(flowIsLess)
                flow[j].resize(amount+1);
            if (bandwidthIsLess)
                bandwidth[j].resize(amount + 1);
        }
    }
    amount++;
}

void Graph::updateNodes()
{
    for (auto &node : nodes)
        node->update(node->boundingRect());
}

bool Graph::edgeExists(unsigned int u, unsigned int v)
{
    if (!nodes.contains(u))
        return false;
    if (!nodes.contains(v))
        return false;
    return edges.contains(qMakePair(nodes[u], nodes[v]));
}

const QFlags<GraphFlags> Graph::getFlags()
{
    return flags;
}

void Graph::setFlag(GraphFlags flag, bool set)
{
    if (set)
        flags |= flag;
    else
        flags &= ~flag;
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

unsigned int Graph::getAmount()
{
    return this->amount;
}

int Graph::getSourceIndex()
{
    if (src == nullptr)
        return -1;
    if (nodes.values().contains(src))
        return src->getIndex();
    return -1;
}
int Graph::getDestIndex()
{
    if (dst == nullptr)
        return -1;
    if (nodes.values().contains(dst))
        return dst->getIndex();
    return -1;
}

void Graph::setSourceIndex(unsigned int sourceIndex)
{
    if (nodes.contains(sourceIndex)) {
        src = nodes[sourceIndex];
        if (src == dst)
            dst = nullptr;
        src->setDefaultColor(NodeColors::SourceColor);
    }
    for (auto &i : nodes) {
        if (i != dst && i != src) {
            i->setDefaultColor(NodeColors::DefaultColor);
            i->update(i->boundingRect());
        }
    }
}

void Graph::setDestIndex(unsigned int destIndex)
{
    if (nodes.contains(destIndex)) {
        dst = nodes[destIndex];
        if (src == dst)
            src = nullptr;
        dst->setDefaultColor(NodeColors::DestColor);
    }
    for (auto &i : nodes) {
        if (i != dst && i != src) {
            i->setDefaultColor(NodeColors::DefaultColor);
            i->update(i->boundingRect());
        }
    }
}
bool Graph::isUnsaved()
{
    return unsavedChanges;
}

void Graph::changesSaved()
{
    unsavedChanges = false;
    graphView->initScene();
}

void Graph::clear()
{
    resizeGraph(amount, 0);
    for (auto edge : std::as_const(edges)) {
        delete edge;
    }
    for (auto node : std::as_const(nodes)) {
        delete node;
    }
    edges.clear();
    nodes.clear();
    unsavedChanges = true;
    //graphView->scene()->clear();
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

