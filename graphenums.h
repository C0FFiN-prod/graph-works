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
enum NodeColors {
    SelectionColor = 0x0078d4,
    DefaultColor = 0x000000,
    SourceColor = 0xFFD300,
    DestColor = 0x3914AF
};
enum SelectOptions { Source = 1, Destination = 3 };
#endif // GRAPHENUMS_H
