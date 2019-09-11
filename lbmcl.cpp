#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "CLUtil.hpp"

#define LATTICE_DIM (32)
#define LATTICE_SIZE ((LATTICE_DIM) * (LATTICE_DIM) * (LATTICE_DIM))


float rhoData[LATTICE_DIM * LATTICE_DIM * LATTICE_DIM] = {1.0f};
float velData[LATTICE_DIM * LATTICE_DIM * LATTICE_DIM * 4] = {1.0f};
size_t timestamp = 0;


static void processData(const cl::CommandQueue & queue,
                        const cl::Kernel & streamingLBM,
                        const cl::Kernel & collisionLBM,
                        const cl::Kernel & updateBoundaryLBM,
                        const cl::Kernel & aggregateDataLBM,
                        cl::Buffer & rho,
                        cl::Buffer & u,
                        int printsEvery)
{
    cl::NDRange lws = cl::NDRange(1, 1, 1);
    cl::NDRange gws = cl::NDRange(LATTICE_DIM, LATTICE_DIM, LATTICE_DIM);

    try {
        cl::Event event_streamingLBM;
        cl::Event event_collisionLBM;
        cl::Event event_updateBoundaryLBM;
        cl::Event event_aggregateDataLBM;
        cl::Event event_read_rho;
        cl::Event event_read_u;


        CLUCheckError(
            queue.enqueueNDRangeKernel(streamingLBM, cl::NullRange, gws, lws, NULL, &event_streamingLBM),
            "streamingLBM",
            true
        );

        CLUCheckError(
            queue.enqueueNDRangeKernel(collisionLBM, cl::NullRange, gws, lws, NULL, &event_collisionLBM),
            "collisionLBM",
            true
        );

        CLUCheckError(
            queue.enqueueNDRangeKernel(updateBoundaryLBM, cl::NullRange, gws, lws, NULL, &event_updateBoundaryLBM),
            "updateBoundaryLBM",
            true
        );


        if ((timestamp % printsEvery) == 0) {
            CLUCheckError(
                queue.enqueueNDRangeKernel(aggregateDataLBM, cl::NullRange, gws, lws, NULL, &event_aggregateDataLBM),
                "aggregateDataLBM",
                true
            );
            CLUEventPrintStats(" aggregateDataLBM", event_aggregateDataLBM);


            CLUCheckError(
                queue.enqueueReadBuffer(rho, CL_TRUE, 0, LATTICE_SIZE * sizeof(float), rhoData, NULL, &event_read_rho),
                "readRho",
                true
            );
            CLUEventPrintStats("          readRho", event_read_rho);
            CLUWriteBufferCubeToVTK(rhoData, LATTICE_DIM, timestamp);


            CLUCheckError(
                queue.enqueueReadBuffer(u, CL_TRUE, 0, LATTICE_SIZE * 4 * sizeof(float), velData, NULL, &event_read_u),
                "readU",
                true
            );
            CLUEventPrintStats("            readU", event_read_rho);
            CLUWriteBufferCube3DToVTK(velData, LATTICE_DIM, timestamp);
        }

        CLUEventPrintStats("     streamingLBM", event_streamingLBM);
        CLUEventPrintStats("     collisionLBM", event_collisionLBM);
        CLUEventPrintStats("updateBoundaryLBM", event_updateBoundaryLBM);


    } catch (cl::Error err) {
        CLUErrorPrint(err, true);
    }
}

int main(int argc, char * argv[])
{
    argc--;
    argv++;

    int argi = 0;
    int platformID = -1;
    int deviceID = -1;
    int iterations = -1;
    int printsEvery = -1;

    if (argc > 0) platformID  = strtol(argv[argi++], NULL, 10);
    if (argc > 1) deviceID    = strtol(argv[argi++], NULL, 10);
    if (argc > 2) iterations  = strtol(argv[argi++], NULL, 10);
    if (argc > 3) printsEvery = strtol(argv[argi++], NULL, 10);

    if (iterations < 0) iterations = 1;
    if (printsEvery < 0) printsEvery = 1;

    cl::Platform platform;
    cl::Device device;
    cl::Context context;
    cl::CommandQueue queue;
    cl::Program program;

    CLUSelectPlatform(platform, platformID);
    CLUSelectDevice(device, platform, deviceID);
    CLUCreateContext(context, device);
    CLUCreateQueue(queue, context, device);

    std::stringstream optionsBuilder;
    optionsBuilder << "-Werror ";
    optionsBuilder << "-cl-single-precision-constant ";
    optionsBuilder << "-DDIM=" << LATTICE_DIM << " ";
    std::cout << "Kernels options: " << optionsBuilder.str() << std::endl;

    CLUBuildProgram(program, context, device, "kernels.cl", optionsBuilder.str());

    cl::Kernel            initLBM(program, "init");
    cl::Kernel      streamingLBM0(program, "streaming");
    cl::Kernel      streamingLBM1(program, "streaming");
    cl::Kernel      collisionLBM0(program, "collision");
    cl::Kernel      collisionLBM1(program, "collision");
    cl::Kernel updateBoundaryLBM0(program, "updateBoundary");
    cl::Kernel updateBoundaryLBM1(program, "updateBoundary");
    cl::Kernel  aggregateDataLBM0(program, "aggregateData");
    cl::Kernel  aggregateDataLBM1(program, "aggregateData");


    cl_int err;
    size_t f_size = LATTICE_SIZE * 19 * sizeof(float);
    size_t rho_size = LATTICE_SIZE * sizeof(float);
    size_t type_size = LATTICE_SIZE * sizeof(int);
    size_t u_size = LATTICE_SIZE * 4 * sizeof(float);     // it is a float4

    cl::Buffer f_stream = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, f_size, NULL, &err);
    CLUCheckError(err, "cl::Buffer(f_stream)", true);

    cl::Buffer f_collide = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, f_size, NULL, &err);
    CLUCheckError(err, "cl::Buffer(f_collide))", true);

    cl::Buffer rho = cl::Buffer(context, CL_MEM_READ_WRITE, rho_size, NULL, &err);
    CLUCheckError(err, "cl::Buffer(rho)", true);

    cl::Buffer type = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, type_size, NULL, &err);
    CLUCheckError(err, "cl::Buffer(type)", true);

    cl::Buffer u = cl::Buffer(context, CL_MEM_READ_WRITE, u_size, NULL, &err);
    CLUCheckError(err, "cl::Buffer(u)", true);


    try {
        initLBM.setArg(0, f_stream);
        initLBM.setArg(1, f_collide);
        initLBM.setArg(2, rho);
        initLBM.setArg(3, u);
        initLBM.setArg(4, type);

        // 0
        streamingLBM0.setArg(0, f_stream);
        streamingLBM0.setArg(1, f_collide);
        // 1
        streamingLBM1.setArg(0, f_collide);
        streamingLBM1.setArg(1, f_stream);

        // SWAP

        // 0
        collisionLBM0.setArg(0, f_stream);
        // 1
        collisionLBM1.setArg(0, f_collide);

        // 0
        updateBoundaryLBM0.setArg(0, f_stream);
        updateBoundaryLBM0.setArg(1, type);
        // 1
        updateBoundaryLBM1.setArg(0, f_collide);
        updateBoundaryLBM1.setArg(1, type);

        // 0
        aggregateDataLBM0.setArg(0, rho);
        aggregateDataLBM0.setArg(1, u);
        aggregateDataLBM0.setArg(2, f_stream);
        // 1
        aggregateDataLBM1.setArg(0, rho);
        aggregateDataLBM1.setArg(1, u);
        aggregateDataLBM1.setArg(2, f_collide);

        // initLBM
        cl::Event event_initLBM;
        cl::NDRange lws(1, 1, 1);
        cl::NDRange gws(LATTICE_DIM, LATTICE_DIM, LATTICE_DIM);
        CLUCheckError(
            queue.enqueueNDRangeKernel(initLBM, cl::NullRange, gws, lws, NULL, &event_initLBM),
            "initLBM",
            true
        );
        CLUEventPrintStats("initLBM", event_initLBM);


        cl::Event event_aggregateDataLBM;
        cl::Event event_read_rho;
        cl::Event event_read_u;
        CLUCheckError(
            queue.enqueueNDRangeKernel(aggregateDataLBM1, cl::NullRange, gws, lws, NULL, &event_aggregateDataLBM),
            "aggregateDataLBM",
            true
        );
        CLUEventPrintStats(" aggregateDataLBM", event_aggregateDataLBM);


        CLUCheckError(
            queue.enqueueReadBuffer(rho, CL_TRUE, 0, LATTICE_SIZE * sizeof(float), rhoData, NULL, &event_read_rho),
            "readRho",
            true
        );
        CLUEventPrintStats("          readRho", event_read_rho);
        CLUWriteBufferCubeToVTK(rhoData, LATTICE_DIM, timestamp);


        CLUCheckError(
            queue.enqueueReadBuffer(u, CL_TRUE, 0, LATTICE_SIZE * 4 * sizeof(float), velData, NULL, &event_read_u),
            "readU",
            true
        );
        CLUEventPrintStats("            readU", event_read_rho);
        CLUWriteBufferCube3DToVTK(velData, LATTICE_DIM, timestamp);

    } catch (cl::Error err) {
        CLUErrorPrint(err, true);
    }

    std::cout << "initLBM completed!" << std::endl;

    while (timestamp <= iterations) {

        if ((timestamp & 1) == 0) {
            processData(queue,
                        streamingLBM0,
                        collisionLBM0,
                        updateBoundaryLBM0,
                        aggregateDataLBM0, rho, u,
                        printsEvery);
        } else {
            processData(queue,
                    streamingLBM1,
                    collisionLBM1,
                    updateBoundaryLBM1,
                    aggregateDataLBM1, rho, u,
                    printsEvery);
        }
        timestamp++;
    }

    return 0;
}
