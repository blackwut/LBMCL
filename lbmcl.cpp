#include <cstdio>
#include <cmath>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "CLUtil.hpp"
#include "constants.h"

#define LATTICE_SIZE ((DIM) * (DIM) * (DIM))


const size_t f_size = LATTICE_SIZE * 19 * sizeof(float);
const size_t rho_size = LATTICE_SIZE * sizeof(float);
const size_t type_size = LATTICE_SIZE * sizeof(int);
const size_t u_size = LATTICE_SIZE * 3 * sizeof(float);

float rhoData[LATTICE_SIZE] = {-1.0f};
float velData[LATTICE_SIZE * 3] = {-1.0f};
int mapData[LATTICE_SIZE] = {-1};


char * dump_path = NULL;
int timestamp = 0;
int printsEvery = -1;
double totalTime = 0.0f;

#define VTK_PRINT 1


static void storeData()
{

    const size_t from = 1;
    const size_t to = DIM - 1;
    const size_t extent = to - from - 1;

    std::ofstream vtk;

    std::stringstream filenameBuilder;
    filenameBuilder << dump_path << "/lbmcl." << std::setw(2) << std::setfill('0') << timestamp << ".vti";

    vtk.open(filenameBuilder.str());

#if VTK_PRINT
    vtk << "<?xml version=\"1.0\"?>\n" 
        << "<VTKFile type=\"ImageData\" version=\"0.1\" byte_order=\"LittleEndian\">\n" 
        << "  <ImageData WholeExtent=\"0 " << extent << " 0 " << extent << " 0 " << extent << "\" Origin=\"0 0 0\" Spacing=\"1 1 1\">\n"
        << "    <Piece Extent=\"0 " << extent << " 0 " << extent << " 0 " << extent << "\">\n"
        << "      <PointData Scalars=\"scalars\">\n"
        << "        <DataArray type=\"Float32\" Name=\"rho\" NumberOfComponents=\"1\" format=\"ascii\">\n";
#endif

    for (size_t z = from; z < to; ++z) {
        for (size_t y = from; y < to; ++y) {
            for (size_t x = from; x < to; ++x) {
                const float val = rhoData[IDxyz(x, y, z)];
                vtk << std::scientific << val << " ";
            }
            vtk << "\n";
        }
        vtk << "\n";
    }

#if VTK_PRINT
    vtk << "        </DataArray>\n"
        << "        <DataArray type=\"Float32\" Name=\"v\" NumberOfComponents=\"3\" format=\"ascii\">\n";
#endif

    for (size_t z = from; z < (to); ++z) {
        for (size_t y = from; y < (to); ++y) {
            for (size_t x = from; x < (to); ++x) {
                const float val0 = velData[IDxyz(x, y, z) * 3 + 0];
                const float val1 = velData[IDxyz(x, y, z) * 3 + 1];
                const float val2 = velData[IDxyz(x, y, z) * 3 + 2];
                vtk << std::scientific << val0 << " "
                    << std::scientific << val1 << " "
                    << std::scientific << val2 << " ";
            }
            vtk << "\n";
        }
        vtk << "\n";
    }

#if VTK_PRINT
    vtk << "        </DataArray>\n"
        << "      </PointData>\n"
        << "    </Piece>\n"
        << "  </ImageData>\n"
        << "</VTKFile>\n";
#endif

    vtk.close();
}


static void dumpAndStoreData(const cl::CommandQueue & queue,
                             cl::Buffer & rho,
                             cl::Buffer & u)
{
    cl::Event event_read_rho;
    cl::Event event_read_u;

    try {
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

        if (dump_path != NULL) storeData();
    } catch (cl::Error err) {
        CLUErrorPrint(err, true);
    }
}


static void processData(const cl::CommandQueue & queue,
                        const cl::Kernel & computeMacro,
                        const cl::Kernel & updateBoundary,
                        const cl::Kernel & collision,
                        const cl::Kernel & streaming,
                        cl::Buffer & rho,
                        cl::Buffer & u)
{
    cl::NDRange gws = cl::NDRange(DIM, DIM, DIM);

    try {
        cl::Event event_computeMacro;
        cl::Event event_updateBoundary;
        cl::Event event_collision;
        cl::Event event_streaming;

        CLUCheckError(
            queue.enqueueNDRangeKernel(computeMacro, cl::NullRange, gws, cl::NullRange, NULL, &event_computeMacro),
            "computeMacro",
            true
        );
        totalTime += CLUEventPrintStats("  computeMacro", event_computeMacro);

        // TODO: check the condition 
        if ((timestamp - 1) % printsEvery == 0) dumpAndStoreData(queue, rho, u);

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
    // TODO: rewrite user input handling (take huge care of dump_path)
    argc--;
    argv++;

    int argi = 0;
    int platformID = -1;
    int deviceID = -1;
    int iterations = -1;

    if (argc > 0) platformID  = strtol(argv[argi++], NULL, 10);
    if (argc > 1) deviceID    = strtol(argv[argi++], NULL, 10);
    if (argc > 2) iterations  = strtol(argv[argi++], NULL, 10);
    if (argc > 3) printsEvery = strtol(argv[argi++], NULL, 10);
    if (argc > 4) dump_path   = argv[argi++];

    if (iterations < 0) iterations = 1;
    if (printsEvery < 0) printsEvery = 1;

    if (dump_path != NULL) {
        const size_t dump_path_len = strlen(dump_path);
        if (dump_path[dump_path_len - 1] == '/') dump_path[dump_path_len] = '\0';
    }

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
    optionsBuilder << "-DDIM=" << DIM << " ";
    std::cout << "Kernels options: " << optionsBuilder.str() << std::endl;

    CLUBuildProgram(program, context, device, "kernels.cl", optionsBuilder.str());

    cl::Kernel             initLBM(program, "init");
    cl::Kernel        computeMacro(program, "computeMacro");
    cl::Kernel   computeMacro_swap(program, "computeMacro");
    cl::Kernel      updateBoundary(program, "boundaryConditions");
    cl::Kernel updateBoundary_swap(program, "boundaryConditions");
    cl::Kernel           collision(program, "collision");
    cl::Kernel      collision_swap(program, "collision");
    cl::Kernel           streaming(program, "streaming");
    cl::Kernel      streaming_swap(program, "streaming");

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
        initLBM.setArg(2, rho);
        initLBM.setArg(3, u);
        initLBM.setArg(4, type);

        // 0
        computeMacro.setArg(0, f_collide);
        computeMacro.setArg(1, rho);
        computeMacro.setArg(2, u);
        computeMacro.setArg(3, type);
        // 1
        computeMacro_swap.setArg(0, f_stream);
        computeMacro_swap.setArg(1, rho);
        computeMacro_swap.setArg(2, u);
        computeMacro_swap.setArg(3, type);

        // 0
        updateBoundary.setArg(0, f_collide);
        updateBoundary.setArg(1, rho);
        updateBoundary.setArg(2, u);
        updateBoundary.setArg(3, type);
        // 1
        updateBoundary_swap.setArg(0, f_stream);
        updateBoundary_swap.setArg(1, rho);
        updateBoundary_swap.setArg(2, u);
        updateBoundary_swap.setArg(3, type);


        // 0
        collision.setArg(0, f_collide);
        collision.setArg(1, rho);
        collision.setArg(2, u);
        collision.setArg(3, type);
        // 1
        collision_swap.setArg(0, f_stream);
        collision_swap.setArg(1, rho);
        collision_swap.setArg(2, u);
        collision_swap.setArg(3, type);

        // 0
        streaming.setArg(0, f_stream);
        streaming.setArg(1, f_collide);
        streaming.setArg(2, type);
        // 1
        streaming_swap.setArg(0, f_collide);
        streaming_swap.setArg(1, f_stream);
        streaming_swap.setArg(2, type);


        // initLBM
        cl::Event event_initLBM;
        // cl::NDRange lws = cl::NDRange(1, 1, 1);
        cl::NDRange gws = cl::NDRange(DIM, DIM, DIM);
        CLUCheckError(
            queue.enqueueNDRangeKernel(initLBM, cl::NullRange, gws, cl::NullRange, NULL, &event_initLBM),
            "initLBM",
            true
        );
        totalTime += CLUEventPrintStats("initLBM", event_initLBM);

        dumpAndStoreData(queue, rho, u);
        timestamp++;

    } catch (cl::Error err) {
        CLUErrorPrint(err, true);
    }

    std::cout << "initLBM completed!" << std::endl;

    while (timestamp <= iterations) {

        if ((timestamp - 1) % 2 == 0) {
            processData(queue,
                        computeMacro,
                        updateBoundary,
                        collision,
                        streaming,
                        rho,
                        u);
        } else {
            processData(queue,
                        computeMacro_swap,
                        updateBoundary_swap,
                        collision_swap,
                        streaming_swap,
                        rho,
                        u);
        }
        timestamp++;
    }

    std::cout << "Total time: "
              << std::fixed << std::setw(8) << std::setprecision(4)
              << totalTime << " ms"
              << std::endl;

    float mlups = ((DIM - 2) * (DIM - 2) * (DIM - 2) * iterations) / (totalTime * 1000);
    std::cout << "Performance:"
              << std::fixed << std::setw(8) << std::setprecision(4)
              << mlups << " MLUPS"
              << std::endl;

    return 0;
}
