#pragma once

#include "Forward.h"

class Forward3 : public Forward {
public:
    CLKernel *kernel;
    CLKernel *repeatedAdd;
//    CLKernel *activate;

    // [[[cog
    // import cog_addheaders
    // cog_addheaders.add()
    // ]]]
    // generated, using cog:
    VIRTUAL ~Forward3();
    VIRTUAL void forward( int batchSize, CLWrapper *dataWrapper, CLWrapper *weightsWrapper, CLWrapper *biasWeightsWrapper,
    CLWrapper *outputWrapper );
    Forward3( OpenCLHelper *cl, LayerDimensions dim );

    // [[[end]]]
};

