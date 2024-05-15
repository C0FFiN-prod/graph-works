#ifndef GRAPHENUMS_H
#define GRAPHENUMS_H
enum GraphFlags {
    ManualMode = 1,
    ShowWeights = 2,
    ShowBandwidth = 4,
    ShowFlow = 8
};

enum EdgeType{
    BiDirectionalSame =0,
    BiDirectionalDiff = 1,
    SingleDirection = 2,
    Loop = 3,
    Error =-1
};

#endif // GRAPHENUMS_H
