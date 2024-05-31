#ifndef GRAPHENUMS_H
#define GRAPHENUMS_H
enum GraphFlags {
    ManualMode = 1,
    ShowWeights = 2,
    ShowBandwidth = 4,
    ShowFlow = 8,
    DisplayIndex = 16
};

enum EdgeType {
    SingleDirection = 0,
    BiDirectionalSame = 1,
    BiDirectionalDiff = 2,
    Loop = 3,
    Error = -1
};
enum DeleteOptions {
    Nodes = 1,
    Edges = 2,

};

#endif // GRAPHENUMS_H
