// Copyright Hugh Perkins 2014, 2015 hughperkins at gmail
//
// This Source Code Form is subject to the terms of the Mozilla Public License, 
// v. 2.0. If a copy of the MPL was not distributed with this file, You can 
// obtain one at http://mozilla.org/MPL/2.0/.

#include "BackpropWeightsFsword73_BatchSize.h"
#include "util/StatefulTimer.h"
#include "util/stringhelper.h"

using namespace std;

#undef STATIC
#define STATIC 

#undef VIRTUAL
#define VIRTUAL 

VIRTUAL BackpropWeightsFsword73_BatchSize::~BackpropWeightsFsword73_BatchSize() {
//    cout << "~backpropgradWeights2naive: deleting kernel" << endl;
    delete kernel;
}
VIRTUAL void BackpropWeightsFsword73_BatchSize::calcGradWeights(int batchSize, CLWrapper *gradOutputWrapper, CLWrapper *imagesWrapper, CLWrapper *gradWeightsWrapper, CLWrapper *gradBiasWrapper) {
    StatefulTimer::instance()->timeCheck("BackpropWeightsFsword73_BatchSize  start");
//     cout << "BackpropWeightsFsword73_BatchSize::calcGradWeights start ...." << endl;
    const float learningMultiplier = learningRateToMultiplier(batchSize);

    kernel
       ->in(batchSize)
       ->in(learningMultiplier)
        ->in(imagesWrapper)
       ->in(gradOutputWrapper)
       ->inout(gradWeightsWrapper);
    if(dim.biased) {
        kernel->inout(gradBiasWrapper);
    }

    int workgroupSize = 64;
    int numWorkgroups = dim.numFilters * dim.inputPlanes * square(dim.filterSize);
    int globalSize = workgroupSize * numWorkgroups;

    kernel->run_1d(globalSize, workgroupSize);

    cl->finish();
//     cout << "... BackpropWeightsFsword73_BatchSize ::calcGradWeights done" << endl;	

    StatefulTimer::instance()->timeCheck("BackpropWeightsFsword73_BatchSize  end");

	//cout <<" dim.numFilters " << dim.numFilters << " dim.inputPlanes " << dim.inputPlanes  <<  " dim.filterSize " << dim.filterSize << " dim.outputSize "<< dim.outputSize << endl;
}
BackpropWeightsFsword73_BatchSize::BackpropWeightsFsword73_BatchSize(EasyCL *cl, LayerDimensions dim) :
        BackpropWeights(cl, dim)
            {
    std::string options = dim.buildOptionsString();

    // [[[cog
    // import stringify
    // stringify.write_kernel2("kernel", "cl/fsword73_backpropweights_fast_batchSize.cl", "test_kernel", 'options')
    // ]]]
    // generated using cog, from cl/fsword73_backpropweights_fast_batchSize.cl:
    const char * kernelSource =  
    "// Copyright // FSWORD73 AT HOTMAIL DOT COM\n"
    "//\n"
    "// This Source Code Form is subject to the terms of the Mozilla Public License,\n"
    "// v. 2.0. If a copy of the MPL was not distributed with this file, You can\n"
    "// obtain one at http://mozilla.org/MPL/2.0/.\n"
    "\n"
    "// expected defines:\n"
    "// BIASED (or not)\n"
    "\n"
    "// Original workgroup by  hughperkins at gmail in deepCL\n"
    "// globalId: [outPlane][inputPlane][filterRow][filterCol]\n"
    "// per-thread iteration: [n][outputRow][outputCol]\n"
    "//27     int workgroupsize = std::max(32, square(dim.filterSize) ); // no point in wasting cores...\n"
    "//28     int numWorkgroups = dim.inputPlanes * dim.numFilters;\n"
    "//29     int globalSize = workgroupsize * numWorkgroups;\n"
    "//30     globalSize = (( globalSize + workgroupsize - 1) / workgroupsize) * workgroupsize;\n"
    "\n"
    "//#define 	gNumFilters 	16\n"
    "//#define 	gInputPlanes	8\n"
    "//#define   gFilterSize   14\n"
    "//#define   gFilterSizeSquared (gFilterSize*gFilterSize)\n"
    "//#define   gOutputSize     14\n"
    "//#define   gInputSize      14\n"
    "//#define   gMargin         0\n"
    "\n"
    "#define   FIXED_WORKGROUPSIZE 64\n"
    "#define   FIXED_WORKGROUPSIZE_SHIFT 6\n"
    "\n"
    "void  calc_backprop_floats_batchSize(\n"
    "        global const float *gradOutput,\n"
    "				global const float *images,\n"
    "				float *thiswchange,\n"
    "				#ifdef BIASED\n"
    "				float*  thisbiaschange,\n"
    "				#endif\n"
    "				const int batchSize,\n"
    "				int globalIdOutput,\n"
    "				int localId\n"
    " )\n"
    " {\n"
    "			  *thiswchange = 0;\n"
    "\n"
    "				int IntraFilterOffset =  globalIdOutput % gFilterSizeSquared;\n"
    "				int filterRow = IntraFilterOffset / gFilterSize;\n"
    "				int filterCol = IntraFilterOffset % gFilterSize;\n"
    "\n"
    "				int filter2Id = globalIdOutput / gFilterSizeSquared;\n"
    "				int outPlane = filter2Id / gInputPlanes;\n"
    "				int upstreamPlane = filter2Id % gInputPlanes;\n"
    "\n"
    "			  int iterations = (batchSize + FIXED_WORKGROUPSIZE -1) >> FIXED_WORKGROUPSIZE_SHIFT;\n"
    "\n"
    "	     for (int i = 0; i < iterations; i++) {\n"
    "				int n = i * FIXED_WORKGROUPSIZE + localId;\n"
    "				if( n >= batchSize)\n"
    "					break;\n"
    "\n"
    "				int upstreamRow = 0 - gMargin + filterRow;\n"
    "\n"
    "				for (int outRow = 0; outRow < gOutputSize; outRow++,upstreamRow++) {\n"
    "\n"
    "					bool proceed0 = upstreamRow >= 0 && upstreamRow < gInputSize;\n"
    "					if(	proceed0 == false)\n"
    "					{\n"
    "						 continue;\n"
    "					}\n"
    "\n"
    "\n"
    "					int resultIndex = (( n * gNumFilters\n"
    "										+ outPlane) * gOutputSize\n"
    "										+ outRow) * gOutputSize\n"
    "					          + 0;  //OutCol start from 0\n"
    "\n"
    "					int upstreamCol =  0 - gMargin + filterCol;\n"
    "					int upstreamDataIndex= (( n * gInputPlanes\n"
    "													 + upstreamPlane) * gInputSize\n"
    "													 + upstreamRow) * gInputSize\n"
    "													 + upstreamCol;\n"
    "\n"
    "					for (int outCol = 0; outCol < gOutputSize; outCol++,	upstreamCol++, upstreamDataIndex++, resultIndex++){\n"
    "						//int upstreamCol = outCol - gMargin + filterCol;\n"
    "						//bool proceed = upstreamRow >= 0 && upstreamCol >= 0 && upstreamRow < gInputSize\n"
    "						//		&& upstreamCol < gInputSize;\n"
    "\n"
    "						bool proceed  = upstreamCol >=0 && upstreamCol < gInputSize ;\n"
    "\n"
    "						if (proceed) {\n"
    "							//int resultIndex = (( n * gNumFilters\n"
    "							//					+ outPlane) * gOutputSize\n"
    "							//					+ outRow) * gOutputSize\n"
    "							//					+ outCol;\n"
    "							float error = gradOutput[resultIndex];\n"
    "							//int upstreamDataIndex = (( n * gInputPlanes\n"
    "							//								 + upstreamPlane) * gInputSize\n"
    "							//								 + upstreamRow) * gInputSize\n"
    "							//								 + upstreamCol;\n"
    "\n"
    "							float upstreamResult = images[upstreamDataIndex];\n"
    "							float thisimagethiswchange = upstreamResult * error;\n"
    "							*thiswchange += thisimagethiswchange;\n"
    "							#ifdef BIASED\n"
    "							*thisbiaschange += error;\n"
    "							#endif\n"
    "						}\n"
    "					}\n"
    "				}\n"
    "			}\n"
    "\n"
    " }\n"
    "\n"
    " void  calc_backprop_floats_batchSize_v2(\n"
    "        global const float *gradOutput,\n"
    "				global const float *images,\n"
    "				float *thiswchange,\n"
    "				#ifdef BIASED\n"
    "				float*  thisbiaschange,\n"
    "				#endif\n"
    "				const int batchSize,\n"
    "				int globalIdOutput,\n"
    "				int localId\n"
    " )\n"
    " {\n"
    "			  *thiswchange = 0;\n"
    "\n"
    "				int IntraFilterOffset =  globalIdOutput % gFilterSizeSquared;\n"
    "				int filterRow = IntraFilterOffset / gFilterSize;\n"
    "				int filterCol = IntraFilterOffset % gFilterSize;\n"
    "\n"
    "				int filter2Id = globalIdOutput / gFilterSizeSquared;\n"
    "				int outPlane = filter2Id / gInputPlanes;\n"
    "				int upstreamPlane = filter2Id % gInputPlanes;\n"
    "\n"
    "			  int iterations = (gOutputSize * gOutputSize * batchSize + FIXED_WORKGROUPSIZE -1) >> FIXED_WORKGROUPSIZE_SHIFT;\n"
    "\n"
    "	      for(int i = 0; i < iterations; i++)\n"
    "				{\n"
    "						int index  = i*FIXED_WORKGROUPSIZE + localId;\n"
    "					  int n = index / (gOutputSize * gOutputSize);\n"
    "					  int offsetofOutput = index % (gOutputSize * gOutputSize);\n"
    "\n"
    "						if( offsetofOutput < (gOutputSize * gOutputSize) && n < batchSize)\n"
    "						{\n"
    "								int outRow = offsetofOutput / gOutputSize;\n"
    "								int upstreamRow = outRow - gMargin + filterRow;\n"
    "								int outCol = offsetofOutput % gOutputSize;\n"
    "								int upstreamCol = outCol - gMargin + filterCol;\n"
    "								bool proceed = upstreamRow >= 0 && upstreamCol >= 0 && upstreamRow < gInputSize\n"
    "										 && upstreamCol < gInputSize;\n"
    "\n"
    "								if (proceed) {\n"
    "											int resultIndex = (( n * gNumFilters\n"
    "																+ outPlane) * gOutputSize\n"
    "																+ outRow) * gOutputSize\n"
    "																+ outCol;\n"
    "											float error = gradOutput[resultIndex];\n"
    "											int upstreamDataIndex = (( n * gInputPlanes\n"
    "																			 + upstreamPlane) * gInputSize\n"
    "																			 + upstreamRow) * gInputSize\n"
    "																			 + upstreamCol;\n"
    "											float upstreamResult = images[upstreamDataIndex];\n"
    "											float thisimagethiswchange = upstreamResult * error;\n"
    "											*thiswchange += thisimagethiswchange;\n"
    "			#ifdef BIASED\n"
    "											*thisbiaschange += error;\n"
    "			#endif\n"
    "									}\n"
    "						}\n"
    "				}	//for loop\n"
    "\n"
    " }\n"
    "\n"
    "\n"
    "  void  calc_backprop_floats_batchSize_v3(\n"
    "        global const float *gradOutput,\n"
    "				global const float *images,\n"
    "				float *thiswchange,\n"
    "				#ifdef BIASED\n"
    "				float*  thisbiaschange,\n"
    "				#endif\n"
    "				const int batchSize,\n"
    "				int globalIdOutput,\n"
    "				int localId\n"
    " )\n"
    " {\n"
    "			  *thiswchange = 0;\n"
    "\n"
    "\n"
    "				int IntraFilterOffset =  globalIdOutput % gFilterSizeSquared;\n"
    "				int filterRow = IntraFilterOffset / gFilterSize;\n"
    "				int filterCol = IntraFilterOffset % gFilterSize;\n"
    "\n"
    "				int filter2Id = globalIdOutput / gFilterSizeSquared;\n"
    "				int outPlane = filter2Id / gInputPlanes;\n"
    "				int upstreamPlane = filter2Id % gInputPlanes;\n"
    "\n"
    "			  int iterations = (gOutputSize * gOutputSize  + FIXED_WORKGROUPSIZE -1) >> FIXED_WORKGROUPSIZE_SHIFT;\n"
    "\n"
    "	      for(int i = 0; i < iterations; i++)\n"
    "				{\n"
    "						int index = (i*FIXED_WORKGROUPSIZE + localId) ;\n"
    "					  int offsetofOutput = index % (gOutputSize * gOutputSize);\n"
    "\n"
    "						if( index < (gOutputSize * gOutputSize))\n"
    "						{\n"
    "								int outRow = offsetofOutput / gOutputSize;\n"
    "								int upstreamRow = outRow - gMargin + filterRow;\n"
    "								int outCol = offsetofOutput % gOutputSize;\n"
    "								int upstreamCol = outCol - gMargin + filterCol;\n"
    "								bool proceed = upstreamRow >= 0 && upstreamCol >= 0 && upstreamRow < gInputSize\n"
    "										 && upstreamCol < gInputSize;\n"
    "\n"
    "								int resultIndex = (( 0 * gNumFilters\n"
    "																	+ outPlane) * gOutputSize\n"
    "																	+ outRow) * gOutputSize\n"
    "																	+ outCol;\n"
    "								int upstreamDataIndex = (( 0 * gInputPlanes\n"
    "																	 + upstreamPlane) * gInputSize\n"
    "																	 + upstreamRow) * gInputSize\n"
    "																	 + upstreamCol;\n"
    "								if (proceed) {\n"
    "											for(int n =0; n < batchSize; n+=8){\n"
    "													float error = gradOutput[resultIndex];\n"
    "													float upstreamResult = images[upstreamDataIndex];\n"
    "													float thisimagethiswchange = upstreamResult * error;\n"
    "													*thiswchange += thisimagethiswchange;\n"
    "					#ifdef BIASED\n"
    "													*thisbiaschange += error;\n"
    "					#endif\n"
    "												   resultIndex 			+= gNumFilters  *  gOutputSize * gOutputSize;\n"
    "													 upstreamDataIndex += gInputPlanes *  gInputSize  * gInputSize;\n"
    "\n"
    "													//2nd batchID\n"
    "													error = gradOutput[resultIndex];\n"
    "													upstreamResult = images[upstreamDataIndex];\n"
    " 												  thisimagethiswchange = upstreamResult * error;\n"
    "													*thiswchange += thisimagethiswchange;\n"
    "					#ifdef BIASED\n"
    "													*thisbiaschange += error;\n"
    "					#endif\n"
    "												   resultIndex 			+= gNumFilters  *  gOutputSize * gOutputSize;\n"
    "													 upstreamDataIndex += gInputPlanes *  gInputSize  * gInputSize;\n"
    "\n"
    "\n"
    "													//3rd batchID\n"
    "													error = gradOutput[resultIndex];\n"
    "													upstreamResult = images[upstreamDataIndex];\n"
    " 												  thisimagethiswchange = upstreamResult * error;\n"
    "													*thiswchange += thisimagethiswchange;\n"
    "					#ifdef BIASED\n"
    "													*thisbiaschange += error;\n"
    "					#endif\n"
    "												   resultIndex 			+= gNumFilters  *  gOutputSize * gOutputSize;\n"
    "													 upstreamDataIndex += gInputPlanes *  gInputSize  * gInputSize;\n"
    "\n"
    "\n"
    "													//4th batchID\n"
    "													error = gradOutput[resultIndex];\n"
    "													upstreamResult = images[upstreamDataIndex];\n"
    " 												  thisimagethiswchange = upstreamResult * error;\n"
    "													*thiswchange += thisimagethiswchange;\n"
    "					#ifdef BIASED\n"
    "													*thisbiaschange += error;\n"
    "					#endif\n"
    "												   resultIndex 			+= gNumFilters  *  gOutputSize * gOutputSize;\n"
    "													 upstreamDataIndex += gInputPlanes *  gInputSize  * gInputSize;\n"
    "\n"
    "													//5th batchID\n"
    "													error = gradOutput[resultIndex];\n"
    "													upstreamResult = images[upstreamDataIndex];\n"
    " 												  thisimagethiswchange = upstreamResult * error;\n"
    "													*thiswchange += thisimagethiswchange;\n"
    "					#ifdef BIASED\n"
    "													*thisbiaschange += error;\n"
    "					#endif\n"
    "												   resultIndex 			+= gNumFilters  *  gOutputSize * gOutputSize;\n"
    "													 upstreamDataIndex += gInputPlanes *  gInputSize  * gInputSize;\n"
    "\n"
    "\n"
    "													//6th batchID\n"
    "													error = gradOutput[resultIndex];\n"
    "													upstreamResult = images[upstreamDataIndex];\n"
    " 												  thisimagethiswchange = upstreamResult * error;\n"
    "													*thiswchange += thisimagethiswchange;\n"
    "					#ifdef BIASED\n"
    "													*thisbiaschange += error;\n"
    "					#endif\n"
    "												   resultIndex 			+= gNumFilters  *  gOutputSize * gOutputSize;\n"
    "													 upstreamDataIndex += gInputPlanes *  gInputSize  * gInputSize;\n"
    "\n"
    "													//7th batchID\n"
    "													error = gradOutput[resultIndex];\n"
    "													upstreamResult = images[upstreamDataIndex];\n"
    " 												  thisimagethiswchange = upstreamResult * error;\n"
    "													*thiswchange += thisimagethiswchange;\n"
    "					#ifdef BIASED\n"
    "													*thisbiaschange += error;\n"
    "					#endif\n"
    "												   resultIndex 			+= gNumFilters  *  gOutputSize * gOutputSize;\n"
    "													 upstreamDataIndex += gInputPlanes *  gInputSize  * gInputSize;\n"
    "\n"
    "\n"
    "													//8th batchID\n"
    "													error = gradOutput[resultIndex];\n"
    "													upstreamResult = images[upstreamDataIndex];\n"
    " 												  thisimagethiswchange = upstreamResult * error;\n"
    "													*thiswchange += thisimagethiswchange;\n"
    "					#ifdef BIASED\n"
    "													*thisbiaschange += error;\n"
    "					#endif\n"
    "												   resultIndex 			+= gNumFilters  *  gOutputSize * gOutputSize;\n"
    "													 upstreamDataIndex += gInputPlanes *  gInputSize  * gInputSize;\n"
    "\n"
    "											}//for n\n"
    "									}	//if proceed\n"
    "						}\n"
    "				}	//for loop\n"
    "\n"
    " }\n"
    "\n"
    " void Reduction_of_Weights(\n"
    "				float* thiswchange,\n"
    "				__local  float* sdata,\n"
    "				#ifdef BIASED\n"
    "				 float*  thisbiaschange,\n"
    "				#endif\n"
    "				int localId)\n"
    " {\n"
    "\n"
    "	//store into local\n"
    "	sdata[localId] = *thiswchange;\n"
    "	barrier(CLK_LOCAL_MEM_FENCE);\n"
    "	for(unsigned int s = FIXED_WORKGROUPSIZE >>1; s > 0; s >>= 1)\n"
    "	{\n"
    "		if(localId < s)\n"
    "		{\n"
    "			sdata[localId] += sdata[localId + s];\n"
    "		}\n"
    "		barrier(CLK_LOCAL_MEM_FENCE);\n"
    "	}\n"
    "\n"
    "	if(localId == 0)\n"
    "	{\n"
    "		*thiswchange = 	sdata[0];\n"
    "\n"
    "	}\n"
    "	barrier(CLK_LOCAL_MEM_FENCE);\n"
    "#ifdef BIASED\n"
    "	sdata[localId] = *thisbiaschange;\n"
    "	barrier(CLK_LOCAL_MEM_FENCE);\n"
    "	for(unsigned int s = FIXED_WORKGROUPSIZE >>1; s > 0; s >>= 1)\n"
    "	{\n"
    "		if(localId < s)\n"
    "		{\n"
    "			sdata[localId] += sdata[localId + s];\n"
    "		}\n"
    "		barrier(CLK_LOCAL_MEM_FENCE);\n"
    "	}\n"
    "\n"
    "	if(localId == 0)\n"
    "	{\n"
    "		*thisbiaschange = 	sdata[0];\n"
    "	}\n"
    "	barrier(CLK_LOCAL_MEM_FENCE);\n"
    "#endif\n"
    "}\n"
    "\n"
    "\n"
    "inline void AtomicAdd(volatile __global float *source, const float operand) {\n"
    "    union {\n"
    "        unsigned int intVal;\n"
    "        float floatVal;\n"
    "    } newVal;\n"
    "    union {\n"
    "        unsigned int intVal;\n"
    "        float floatVal;\n"
    "    } prevVal;\n"
    "    do {\n"
    "        prevVal.floatVal = *source;\n"
    "        newVal.floatVal = prevVal.floatVal + operand;\n"
    "    } while (atomic_cmpxchg((volatile __global unsigned int *)source,\n"
    "                             prevVal.intVal, newVal.intVal)\n"
    "                             != prevVal.intVal);\n"
    "}\n"
    "\n"
    "//\n"
    "//   Global ID :  [outPlane][inputPlane][filterRow][filterCol] *  BatchSize * 64\n"
    "//   Per thread = ( gOutputSize * gOutputSize)/64\n"
    "//   workgroupsize =  64 treads == 1 image\n"
    "//   numWorkgroups  = [outPlane][inputPlane][filterRow][filterCol] *  BatchSize\n"
    "//   int globalSize = workgroupsize * numWorkgroups;\n"
    "//   Assuming   [outPlane][inputPlane][filterRow][filterCol] is cleared or preset\n"
    "//   Each workgroupsize call Atomic_Add\n"
    "//	  One pass to compete it.\n"
    "//    Stage1:  caclaute each pixels\n"
    "//    Stage2:  Rediction whole workgroupsize into 1\n"
    "//    Local LDS = 64;\n"
    "\n"
    "#if 1\n"
    "void __kernel test_kernel(\n"
    "						const int batchSize,\n"
    "						const float learningRateMultiplier,\n"
    "						__global float* images,\n"
    "//						__global const float* filter,\n"
    "						__global const float* gradOutput,\n"
    "						__global float* gradWeights\n"
    "             #ifdef BIASED\n"
    "                     ,__global float *gradBiasWeights\n"
    "             #endif\n"
    " )\n"
    "\n"
    "#else\n"
    "void __kernel backprop_floats_fast(\n"
    "        __global const float *gradOutput,\n"
    "				__global const float *images,\n"
    "        __global float *gradWeights,\n"
    "        #ifdef BIASED\n"
    "             __global float *gradBiasWeights,\n"
    "        #endif\n"
    "				const float learningRateMultiplier,\n"
    "				const int batchSize,\n"
    "			  const int gOutputSize,\n"
    " )\n"
    " #endif\n"
    " {\n"
    "\n"
    "//	 const float learningRateMultiplier = 0.0001f;\n"
    "	 int globalIdOutput = get_group_id(0);\n"
    "	 int localId  = get_local_id(0);\n"
    "	  float thiswchange = 0;\n"
    "	  __local float sdata[FIXED_WORKGROUPSIZE];\n"
    "#ifdef BIASED\n"
    "	 float thisbiaschange  = 0;\n"
    "#endif\n"
    "\n"
    "	 //It does not include any Invalid Threads since the FIXED_WORKGROUPSIZE is 64)\n"
    "   // if (globalId >= gNumFilters * gInputPlanes * gFilterSize * gFilterSize *  batchSize * FIXED_WORKGROUPSIZE) {\n"
    "   //     //Do nothing\n"
    "   // }\n"
    "	 if( (gOutputSize * gOutputSize) >= FIXED_WORKGROUPSIZE)\n"
    "	 {\n"
    "		 calc_backprop_floats_batchSize_v3(\n"
    "        gradOutput,\n"
    "				images,\n"
    "				&thiswchange,\n"
    "				#ifdef BIASED\n"
    "				&thisbiaschange,\n"
    "				#endif\n"
    "				batchSize,\n"
    "				globalIdOutput,\n"
    "				localId);\n"
    "	 }\n"
    "	 else\n"
    "	 {\n"
    "	 		 calc_backprop_floats_batchSize_v2(\n"
    "        gradOutput,\n"
    "				images,\n"
    "				&thiswchange,\n"
    "				#ifdef BIASED\n"
    "				&thisbiaschange,\n"
    "				#endif\n"
    "				batchSize,\n"
    "				globalIdOutput,\n"
    "				localId);\n"
    "	 }\n"
    "\n"
    "\n"
    "	//aggregate Data to Thread0\n"
    "	Reduction_of_Weights(\n"
    "				&thiswchange,\n"
    "				sdata,\n"
    "#ifdef BIASED\n"
    "				&thisbiaschange,\n"
    "#endif\n"
    "				localId\n"
    "	);\n"
    "\n"
    "	if(localId == 0)\n"
    "	{\n"
    "		gradWeights[ globalIdOutput ] = thiswchange;\n"
    "\n"
    "#ifdef BIASED\n"
    "		int filter2Id = globalIdOutput / gFilterSizeSquared;\n"
    "		int outPlane = filter2Id / gInputPlanes;\n"
    "		gradBiasWeights[outPlane] = thisbiaschange;\n"
    "#endif\n"
    "	}\n"
    "\n"
    "}\n"
    "\n"
    "//CNNBench command line\n"
    "//-dim 1 -lx 64 -ly 1 -gx 2359296 -gy 1 -f 3 -c1 128 -c2 128 -i 1 -x 4096 -y 4096\n"
    "	//    Input 	128x128, 4 Planes, BatchSize 128\n"
    "  //    output  128x128, 8 planes,  BatchSize 128\n"
    "  //    Filter Size = 3x3\n"
    "	//   numWorkgroups  = [outPlane][inputPlane][filterRow][filterCol] *  BatchSize\n"
    "  //   int globalSize = workgroupsize * numWorkgroups;\n"
    "	//    globalSize = 64 * [8][4][3][3] *  128   = 2359296\n"
    "  //\n"
    "	//    	    Input 	= 128x128x4x128 batch = 4096 * 2048\n"
    "  //			    output  = 128x128x8x128 batch = 4096 * 4096\n"
    "	//          force localthread_x =64,  globalthread_y =1\n"
    "	//          FilterSize =3,\n"
    "	//         -constant1 = batchSize 128\n"
    "	//         -constant2 = gOutputSize 128\n"
    "";
    kernel = cl->buildKernelFromString(kernelSource, "test_kernel", options, "cl/fsword73_backpropweights_fast_batchSize.cl");
    // [[[end]]]
}

