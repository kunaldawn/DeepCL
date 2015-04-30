cdef extern from "Layer.h":
    cdef cppclass Layer:
        void forward()
        void backward( float learningRate )
        bool needsBackProp()
        bool getBiased()
        int getOutputCubeSize()
        int getOutputPlanes()
        int getOutputImageSize()
        float * getOutput()
        int getOutputSize()
        int getPersistSize()
        void persistToArray(float *array)
        void unpersistFromArray(const float *array)
        string asString()
        string getClassName()


