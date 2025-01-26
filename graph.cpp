#include "graph.h"
#include "qbitarray.h"

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

Matrix2D Graph::makeEnabledMatrix(const Matrix2D &matrix)
{
    Matrix2D newMatrix(matrix);
    for (unsigned int i = 0; i < amount; ++i) {
        for (unsigned int j = 0; j < amount; ++j) {
            bool enabled = nodes[i]->isEnabled() && nodes[j]->isEnabled();
            newMatrix[i][j] = enabled ? newMatrix[i][j] : 0;
        }
    }
    return newMatrix;
}

const Matrix2D Graph::getMatrixAdjacent(bool full)
{
    if (full)
        return adjacent;
    return makeEnabledMatrix(adjacent);
}

const Matrix2D Graph::getMatrixFlow(bool full)
{
    if (full)
        return flow;
    return makeEnabledMatrix(flow);
}

const Matrix2D Graph::getMatrixBandwidth(bool full)
{
    if (full)
        return bandwidth;
    return makeEnabledMatrix(bandwidth);
}

const QList<QVariantList> Graph::getListEdges(bool full)
{
    QList<QVariantList> edgeList(full ? edges.size() : 0);
    unsigned int i = 0;
    for (auto [key, edge] : edges.asKeyValueRange()) {
        QVariantList tuple{key.first->getIndex(),
                           key.second->getIndex(),
                           edge->getWeight(),
                           edge->getBandwidth(),
                           edge->getFlow()};
        if (full
            || (!disabledEdges.contains(edge) && enablingMask[tuple[0].toUInt()]
                && enablingMask[tuple[1].toUInt()])) {
            if (full)
                edgeList[i++] = tuple;
            else
                edgeList.append(tuple);
        }
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
                edges[key] = new Edge(nodes[i],
                                      nodes[j],
                                      1,
                                      qMax(getEdgeType(i, j, bandwidth),
                                           getEdgeType(i, j, adjacent)));
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
                edge->setEdgeType(qMax(getEdgeType(i, j, bandwidth), getEdgeType(i, j, adjacent)));
            }
            if((i!=j)&&!adjacent[j][i]&&matrix[j][i]){
                nodes[j]->connectToNode(nodes[i]);
                auto key = qMakePair(nodes[j],nodes[i]);
                if(bandwidth[j][i] < matrix[j][i])
                    bandwidth[j][i] = matrix[j][i];
                edges[key] = new Edge(nodes[j],
                                      nodes[i],
                                      1,
                                      qMax(getEdgeType(j, i, bandwidth),
                                           getEdgeType(j, i, adjacent)));
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
                edge->setEdgeType(qMax(getEdgeType(j, i, bandwidth), getEdgeType(j, i, adjacent)));
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
                edges[key] = new Edge(nodes[i],
                                      nodes[j],
                                      1,
                                      qMax(getEdgeType(i, j, matrix), getEdgeType(i, j, adjacent)));
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
                edge->setEdgeType(qMax(getEdgeType(i, j, matrix), getEdgeType(i, j, adjacent)));
            }
            if((i!=j)&&adjacent[j][i]&&!matrix[j][i]){
                removeEdge(j, i);
            }else if((i!=j)&&!adjacent[j][i]&&matrix[j][i]){
                nodes[j]->connectToNode(nodes[i]);
                auto key = qMakePair(nodes[j],nodes[i]);
                if(flow[j][i] > matrix[j][i])
                    flow[j][i] = matrix[j][i];
                edges[key] = new Edge(nodes[j],
                                      nodes[i],
                                      1,
                                      qMax(getEdgeType(j, i, matrix), getEdgeType(j, i, adjacent)));
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
                edge->setEdgeType(qMax(getEdgeType(j, i, matrix), getEdgeType(j, i, adjacent)));
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

Edge *Graph::getEdge(unsigned int u, unsigned int v)
{
    QPair<Node *, Node *> key
        = QPair<Node *, Node *>(this->graphView->getNodes()->value(u, nullptr),
                                this->graphView->getNodes()->value(v, nullptr));
    if (!key.first || !key.second) {
        throw std::runtime_error("Source/dest node not found");
    }
    return graphView->getEdges()->value(key, nullptr);
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
        if (graphView->scene()->items().contains(edges[key])) {
            graphView->scene()->removeItem(edges[key]);
        }
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
        enablingMask.resize(amount - 1);
        for (unsigned i = 0; i < amount - 1; i++) {
            adjacent[i].removeAt(index);
            flow[i].removeAt(index);
            bandwidth[i].removeAt(index);
        }
    }

    for (unsigned int i = index + 1; i < amount; i++) {
        Node *toChange = nodes[i];
        toChange->setIndex(i - 1);
        nodes.insert(i - 1, toChange);
        enablingMask[i - 1] = toChange->isEnabled();
    }
    if (index != amount - 1)
        nodes.take(amount - 1);
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
    if (enablingMask.size() < amount + 1)
        enablingMask.resize(amount + 1);
    enablingMask[i] = true;
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
void Graph::toggleNode(Node *node, bool enabled)
{
    if (!enabled && (src == node || dst == node))
        throw std::runtime_error(
            QString("Not allowed to disable source or sink.\nNode with index %1")
                .arg(node->getIndex())
                .toStdString());
    unsavedChanges = unsavedChanges || node->isEnabled() != enabled;
    node->toggle(enabled);
    enablingMask[node->getIndex()] = enabled;
}
void Graph::toggleNode(unsigned int index, bool enabled)
{
    if (!nodes.contains(index))
        throw std::runtime_error(QString("No such node with index %1").arg(index).toStdString());
    toggleNode(nodes[index], enabled);
}

void Graph::toggleEdge(unsigned int u, unsigned int v, bool enabled)
{
    if (!nodes.contains(u))
        throw std::runtime_error(QString("Cannot find node with index %1").arg(u).toStdString());
    if (!nodes.contains(v))
        throw std::runtime_error(QString("Cannot find node with index %1").arg(v).toStdString());
    if (!edges.contains({nodes[u], nodes[v]}))
        throw std::runtime_error(
            QString("Cannot find edge between %1 and ").arg(u).arg(v).toStdString());
    toggleEdge(edges[{nodes[u], nodes[v]}], enabled);
}

void Graph::toggleEdges(const QList<Edge *> &edgeList, bool enabled)
{
    for (auto edge : edgeList)
        toggleEdge(edge, enabled);
}

bool Graph::isEdgeEnabled(Edge *edge)
{
    return !disabledEdges.contains(edge);
}
bool Graph::isNodeEnabled(Node *node)
{
    return node->isEnabled();
}

void Graph::toggleEdge(Edge *edge, bool enabled)
{
    bool isBiDirectionalSame = edge->getEdgeType() == EdgeType::BiDirectionalSame;
    Edge *reverseEdge = isBiDirectionalSame ? edges[{edge->destNode(), edge->sourceNode()}]
                                            : nullptr;
    unsavedChanges = unsavedChanges || edge->isEnabled() != enabled;
    if (!enabled) {
        disabledEdges.insert(edge);
        if (isBiDirectionalSame) {
            disabledEdges.insert(reverseEdge);
        }

    } else {
        disabledEdges.remove(edge);
        if (isBiDirectionalSame) {
            disabledEdges.remove(reverseEdge);
        }
    }
    edge->toggle(enabled);
    if (isBiDirectionalSame) {
        reverseEdge->toggle(enabled);
    }
}

void Graph::toggleNodes(const QBitArray &mask)
{
    if (mask.size() > enablingMask.size())
        enablingMask.resize(mask.size());
    for (unsigned int i = 0; i < mask.size(); ++i) {
        if (nodes.contains(i)) {
            nodes[i]->toggle(mask[i]);
            enablingMask[i] = mask[i];
        }
    }
}

QBitArray Graph::getEnablingMask()
{
    return this->enablingMask;
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
        Node *node = nodes[sourceIndex];
        unsavedChanges = unsavedChanges || src != node;
        toggleNode(node, true);
        src = node;
        if (src == dst)
            dst = nullptr;
        src->setDefaultColor(NodeColors::SourceColor);
        src->resetColor();
    }
    for (auto &i : nodes) {
        if (i != dst && i != src) {
            i->setDefaultColor(i->isEnabled() ? NodeColors::DefaultColor
                                              : NodeColors::DisabledColor);
            i->resetColor();
            i->update(i->boundingRect());
        }
    }
}

void Graph::setDestIndex(unsigned int destIndex)
{
    if (nodes.contains(destIndex)) {
        Node *node = nodes[destIndex];
        unsavedChanges = unsavedChanges || dst != node;
        toggleNode(node, true);
        dst = node;
        if (src == dst)
            src = nullptr;
        dst->setDefaultColor(NodeColors::DestColor);
        dst->resetColor();
    }
    for (auto &i : nodes) {
        if (i != dst && i != src) {
            i->setDefaultColor(i->isEnabled() ? NodeColors::DefaultColor
                                              : NodeColors::DisabledColor);
            i->resetColor();
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

