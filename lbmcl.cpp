#include <cstdio>
#include <cmath>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "CLUtil.hpp"

#define LATTICE_DIM (8)
#define LATTICE_SIZE ((LATTICE_DIM) * (LATTICE_DIM) * (LATTICE_DIM))


const size_t f_size = LATTICE_SIZE * 19 * sizeof(float);
const size_t rho_size = LATTICE_SIZE * sizeof(float);
const size_t type_size = LATTICE_SIZE * sizeof(int);
const size_t u_size = LATTICE_SIZE * 3 * sizeof(float);

float rhoData[LATTICE_SIZE] = {-1.0f};
float velData[LATTICE_SIZE * 3] = {-1.0f};
int mapData[LATTICE_SIZE] = {-1};


size_t timestamp = 0;
double totalTime = 0.0f;



static void storeData()
{
    std::ofstream vtk;

    std::stringstream filenameBuilder;
    filenameBuilder << "/Volumes/RamDisk/lbmcl." << std::setw(2) << std::setfill('0') << timestamp << ".vti";

    vtk.open(filenameBuilder.str());

    vtk << "<?xml version=\"1.0\"?>\n" 
        << "<VTKFile type=\"ImageData\" version=\"0.1\" byte_order=\"LittleEndian\">\n" 
        << "  <ImageData WholeExtent=\"0 " << (LATTICE_DIM - 3) << " 0 " << (LATTICE_DIM - 3) << " 0 " << (LATTICE_DIM - 3) << "\" Origin=\"0 0 0\" Spacing=\"1 1 1\">\n"
        << "    <Piece Extent=\"0 " << (LATTICE_DIM - 3) << " 0 " << (LATTICE_DIM - 3) << " 0 " << (LATTICE_DIM - 3) << "\">\n"
        << "      <PointData Scalars=\"scalars\">\n"
        << "        <DataArray type=\"Float32\" Name=\"rho\" NumberOfComponents=\"1\" format=\"ascii\">\n";

    for (size_t z = 1; z < LATTICE_DIM - 1; ++z) {
        for (size_t y = 1; y < LATTICE_DIM - 1; ++y) {
            for (size_t x = 1; x < LATTICE_DIM - 1; ++x) {
                const float val = rhoData[x + (y * LATTICE_DIM) + (z * LATTICE_DIM * LATTICE_DIM)];
                vtk << std::scientific << (fabs(val) < 1e-6 ? 0.0f : val) << " ";
                // vtk << std::scientific << val << " ";
            }
            vtk << "\n";
        }
        vtk << "\n";
    }
    
    vtk << "        </DataArray>\n"
        << "        <DataArray type=\"Float32\" Name=\"v\" NumberOfComponents=\"3\" format=\"ascii\">\n";

    for (size_t z = 1; z < (LATTICE_DIM - 1); ++z) {
        for (size_t y = 1; y < (LATTICE_DIM - 1); ++y) {
            for (size_t x = 1; x < (LATTICE_DIM - 1); ++x) {
                const float val0 = velData[(x + (y * LATTICE_DIM) + (z * LATTICE_DIM * LATTICE_DIM)) * 3 + 0];
                const float val1 = velData[(x + (y * LATTICE_DIM) + (z * LATTICE_DIM * LATTICE_DIM)) * 3 + 1];
                const float val2 = velData[(x + (y * LATTICE_DIM) + (z * LATTICE_DIM * LATTICE_DIM)) * 3 + 2];
                vtk << std::scientific << (fabs(val0) < 1e-6 ? 0.0f : val0) << " "
                    << std::scientific << (fabs(val1) < 1e-6 ? 0.0f : val1) << " "
                    << std::scientific << (fabs(val2) < 1e-6 ? 0.0f : val2) << " ";
                // vtk << std::scientific << val0 << " "
                //     << std::scientific << val1 << " "
                //     << std::scientific << val2 << " ";
            }
            vtk << "\n";
        }
        vtk << "\n";
    }
    
    vtk << "        </DataArray>\n"
        << "      </PointData>\n"
        << "    </Piece>\n"
        << "  </ImageData>\n"
        << "</VTKFile>\n";
    vtk.close();
}


static void dumpAndStoreData(const cl::CommandQueue & queue,
                             const cl::Kernel & aggregateData,
                             cl::Buffer & rho,
                             cl::Buffer & u)
{
    cl::Event event_aggregateData;
    cl::Event event_read_rho;
    cl::Event event_read_u;

    cl::NDRange gws = cl::NDRange(LATTICE_DIM, LATTICE_DIM, LATTICE_DIM);
    try {
        CLUCheckError(
            queue.enqueueNDRangeKernel(aggregateData, cl::NullRange, gws, cl::NullRange, NULL, &event_aggregateData),
            "aggregateData",
            true
        );
        totalTime += CLUEventPrintStats(" aggregateData", event_aggregateData);


        CLUCheckError(
            queue.enqueueReadBuffer(rho, CL_TRUE, 0, rho_size, rhoData, NULL, &event_read_rho),
            "readRho",
            true
        );
        totalTime += CLUEventPrintStats("       readRho", event_read_rho);


        CLUCheckError(
            queue.enqueueReadBuffer(u, CL_TRUE, 0, u_size, velData, NULL, &event_read_u),
            "readU",
            true
        );
        totalTime += CLUEventPrintStats("         readU", event_read_rho);

        storeData();
    } catch (cl::Error err) {
        CLUErrorPrint(err, true);
    }
}


static void processData(const cl::CommandQueue & queue,
                        const cl::Kernel & updateBoundary,
                        const cl::Kernel & collision,
                        const cl::Kernel & streaming)
{
    cl::NDRange gws = cl::NDRange(LATTICE_DIM, LATTICE_DIM, LATTICE_DIM);

    try {
        cl::Event event_updateBoundary;
        cl::Event event_collision;
        cl::Event event_streaming;

        CLUCheckError(
            queue.enqueueNDRangeKernel(updateBoundary, cl::NullRange, gws, cl::NullRange, NULL, &event_updateBoundary),
            "updateBoundary",
            true
        );
        totalTime += CLUEventPrintStats("updateBoundary", event_updateBoundary);

        CLUCheckError(
            queue.enqueueNDRangeKernel(collision, cl::NullRange, gws, cl::NullRange, NULL, &event_collision),
            "collision",
            true
        );
        totalTime += CLUEventPrintStats("     collision", event_collision);

        CLUCheckError(
            queue.enqueueNDRangeKernel(streaming, cl::NullRange, gws, cl::NullRange, NULL, &event_streaming),
            "streaming",
            true
        );
        totalTime += CLUEventPrintStats("     streaming", event_streaming);

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
    // optionsBuilder << "-cl-single-precision-constant ";
    optionsBuilder << "-cl-fast-relaxed-math ";
    // optionsBuilder << "-cl-denorms-are-zero ";
    optionsBuilder << "-DDIM=" << LATTICE_DIM << " ";
    std::cout << "Kernels options: " << optionsBuilder.str() << std::endl;

    CLUBuildProgram(program, context, device, "kernels.cl", optionsBuilder.str());

    cl::Kernel             initLBM(program, "init");
    cl::Kernel           streaming(program, "streaming");
    cl::Kernel      streaming_swap(program, "streaming");
    cl::Kernel           collision(program, "collision");
    cl::Kernel      collision_swap(program, "collision");
    cl::Kernel      updateBoundary(program, "boundaryConditions");
    cl::Kernel updateBoundary_swap(program, "boundaryConditions");
    cl::Kernel       aggregateData(program, "aggregateData");
    cl::Kernel  aggregateData_swap(program, "aggregateData");


    cl_int err;

    cl::Buffer f_stream = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, f_size, NULL, &err);
    CLUCheckError(err, "cl::Buffer(f_stream)", true);

    cl::Buffer f_collide = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, f_size, NULL, &err);
    CLUCheckError(err, "cl::Buffer(f_collide))", true);

    cl::Buffer rho = cl::Buffer(context, CL_MEM_READ_WRITE, rho_size, NULL, &err);
    CLUCheckError(err, "cl::Buffer(rho)", true);

    cl::Buffer type = cl::Buffer(context, CL_MEM_READ_WRITE, type_size, NULL, &err);
    CLUCheckError(err, "cl::Buffer(type)", true);

    cl::Buffer u = cl::Buffer(context, CL_MEM_READ_WRITE, u_size, NULL, &err);
    CLUCheckError(err, "cl::Buffer(u)", true);


    try {
        initLBM.setArg(0, f_stream);
        initLBM.setArg(1, f_collide);
        initLBM.setArg(2, type);


        // 0
        updateBoundary.setArg(0, f_collide);
        updateBoundary.setArg(1, type);
        // 1
        updateBoundary_swap.setArg(0, f_stream);
        updateBoundary_swap.setArg(1, type);


        // 0
        collision.setArg(0, f_collide);
        collision.setArg(1, type);
        // 1
        collision_swap.setArg(0, f_stream);
        collision_swap.setArg(1, type);

        // 0
        streaming.setArg(0, f_stream);
        streaming.setArg(1, f_collide);
        streaming.setArg(2, type);
        // 1
        streaming_swap.setArg(0, f_collide);
        streaming_swap.setArg(1, f_stream);
        streaming_swap.setArg(2, type);

        // 0
        aggregateData.setArg(0, rho);
        aggregateData.setArg(1, u);
        aggregateData.setArg(2, f_collide);
        aggregateData.setArg(3, type);
        // 1
        aggregateData_swap.setArg(0, rho);
        aggregateData_swap.setArg(1, u);
        aggregateData_swap.setArg(2, f_stream);
        aggregateData_swap.setArg(3, type);


        // initLBM
        cl::Event event_initLBM;
        // cl::NDRange lws = cl::NDRange(1, 1, 1);
        cl::NDRange gws = cl::NDRange(LATTICE_DIM, LATTICE_DIM, LATTICE_DIM);
        CLUCheckError(
            queue.enqueueNDRangeKernel(initLBM, cl::NullRange, gws, cl::NullRange, NULL, &event_initLBM),
            "initLBM",
            true
        );
        totalTime += CLUEventPrintStats("initLBM", event_initLBM);

        dumpAndStoreData(queue, aggregateData, rho, u);
        timestamp++;

    } catch (cl::Error err) {
        CLUErrorPrint(err, true);
    }

    std::cout << "init completed!" << std::endl;

    while (timestamp <= iterations) {

        if (timestamp % 2 == 1) {
            processData(queue,
                        updateBoundary,
                        collision,
                        streaming);
            dumpAndStoreData(queue, aggregateData, rho, u);
        } else {
            processData(queue,
                        updateBoundary_swap,
                        collision_swap,
                        streaming_swap);
            dumpAndStoreData(queue, aggregateData_swap, rho, u);
        }
        timestamp++;
    }

    std::cout << "Total time: "
              << std::fixed << std::setw(8) << std::setprecision(4)
              << totalTime << " ms"
              << std::endl;

    return 0;
}
