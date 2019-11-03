#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <iomanip>
#include <sstream>


#include "common.h"
#include "CLUtil.hpp"
#include "ArgsUtil.hpp"
#include "StoreUtil.hpp"

lbm_options opts;
std::vector< std::pair<cl::Event, std::string> > events;


static void dump_map(const cl::CommandQueue & queue,
                     const cl::Buffer & map)
{
    try {
        int * map_val = new int[opts.map_dim()];
        cl::Event read_evt;
        CLUCheckErrorExit(
            queue.enqueueReadBuffer(map, CL_TRUE, 0, opts.map_size(), map_val, nullptr, &read_evt),
            "dump_map"
        );
        events.emplace_back(read_evt, "dump_map");

        store_map(opts.dump_path, map_val, opts.dim);
        delete [] map_val;
    } catch (cl::Error err) {
        CLUErrorPrintExit(err);
    }
}

static void dump_f(const cl::CommandQueue & queue,
                   const cl::Buffer & f,
                   real_t * f_val,
                   const size_t iteration,
                   const size_t iterations)
{
    try {
        cl::Event read_evt;
        CLUCheckErrorExit(
            queue.enqueueReadBuffer(f, CL_TRUE, 0, opts.f_size(), f_val, nullptr, &read_evt),
            "dump_f"
        );
        events.emplace_back(read_evt, "dump_f");
    } catch (cl::Error err) {
        CLUErrorPrintExit(err);
    }
    store_f(opts.dump_path, f_val, opts.dim, opts.stride, iteration, iterations);
}

static void dump_data(const cl::CommandQueue & queue,
                      const cl::Buffer & rho,
                      const cl::Buffer & u,
                      real_t * rho_val,
                      real_t * u_val,
                      const size_t iteration)
{
    cl::Event read_rho_evt;
    cl::Event read_u_evt;

    try {
        CLUCheckErrorExit(
            queue.enqueueReadBuffer(rho, CL_TRUE, 0, opts.rho_size(), rho_val, nullptr, &read_rho_evt),
            "readRho"
        );
        events.emplace_back(read_rho_evt, "readRho");


        CLUCheckErrorExit(
            queue.enqueueReadBuffer(u, CL_TRUE, 0, opts.u_size(), u_val, nullptr, &read_u_evt),
            "readU"
        );
        events.emplace_back(read_u_evt, "readU");
    } catch (cl::Error err) {
        CLUErrorPrintExit(err);
    }
    store_vtk(opts.vtk_path, rho_val, u_val, opts.dim, iteration, opts.iterations);
}


static void processData(const cl::CommandQueue & queue,
                        const cl::Kernel & collideAndStream)
{
    cl::NDRange lws = cl::NDRange(opts.lws, 1, 1);
    cl::NDRange gws = cl::NDRange(opts.dim, opts.dim, opts.dim);
    try {
        cl::Event collideAndStream_evt;

        CLUCheckErrorExit(
            queue.enqueueNDRangeKernel(collideAndStream, cl::NullRange, gws, lws, nullptr, &collideAndStream_evt),
            "collideAndStream"
        );
        events.emplace_back(collideAndStream_evt, "collideAndStream");

    } catch (cl::Error err) {
        CLUErrorPrintExit(err);
    }
}


int main(int argc, char * argv[])
{
    // ARGS
    size_t iteration = 0;
    opts.process_args(argc, argv);
    opts.print_values();

    real_t * rho_val = nullptr;
    real_t * u_val = nullptr;
    real_t * f_val = nullptr;

    if (opts.store_vtk) {
        rho_val = new real_t[opts.rho_dim()];
        u_val = new real_t[opts.u_dim()];
    }

    if (opts.dump_f) {
        f_val = new real_t[opts.f_dim()];
    }

    // OpenCL initialize
    cl::Platform platform;
    cl::Device device;
    cl::Context context;
    cl::CommandQueue queue;
    cl::Program program;

    CLUSelectPlatform(platform, opts.platformID);
    CLUSelectDevice(device, platform, opts.deviceID);
    CLUCreateContext(context, device);
    CLUCreateQueue(queue, context, device);

    std::stringstream optionsBuilder;
    optionsBuilder << "-Werror ";
    optionsBuilder << "-I. ";
    optionsBuilder << "-DDIM=" << opts.dim << " ";
    optionsBuilder << "-DVISCOSITY=" << opts.viscosity << " ";
    optionsBuilder << "-DVELOCITY=" << opts.velocity << " ";
    optionsBuilder << "-DSTRIDE=" << opts.stride << " ";
#ifdef FP_DOUBLE
    optionsBuilder << "-DFP_DOUBLE ";
#else
    optionsBuilder << "-DFP_SINGLE ";
#endif
    if (opts.optimize) {
        optionsBuilder << "-cl-fast-relaxed-math ";
    }

    std::cout << "Kernels options: " << optionsBuilder.str() << std::endl;

    CLUBuildProgram(program, context, device, "kernels.cl", optionsBuilder.str());

    cl::Kernel            initialize(program, "initialize");
    cl::Kernel      collideAndStream(program, "collideAndStream");
    cl::Kernel collideAndStream_swap(program, "collideAndStream");

    cl_int err;

    cl::Buffer f_stream = cl::Buffer(context, (opts.dump_f ? CL_MEM_READ_WRITE : CL_MEM_HOST_NO_ACCESS), opts.f_size(), nullptr, &err);
    CLUCheckErrorExit(err, "cl::Buffer(f_stream)");

    cl::Buffer f_collide = cl::Buffer(context, (opts.dump_f ? CL_MEM_READ_WRITE : CL_MEM_HOST_NO_ACCESS), opts.f_size(), nullptr, &err);
    CLUCheckErrorExit(err, "cl::Buffer(f_collide))");

    cl::Buffer rho = cl::Buffer(context, CL_MEM_READ_WRITE, opts.rho_size(), nullptr, &err);
    CLUCheckErrorExit(err, "cl::Buffer(rho)");

    cl::Buffer u = cl::Buffer(context, CL_MEM_READ_WRITE, opts.u_size(), nullptr, &err);
    CLUCheckErrorExit(err, "cl::Buffer(u)");

    cl::Buffer map = cl::Buffer(context, (opts.dump_map ? CL_MEM_READ_WRITE : CL_MEM_HOST_NO_ACCESS), opts.map_size(), nullptr, &err);
    CLUCheckErrorExit(err, "cl::Buffer(map)");


    try {
        initialize.setArg(0, f_stream);
        initialize.setArg(1, f_collide);
        initialize.setArg(2, rho);
        initialize.setArg(3, u);
        initialize.setArg(4, map);

        // 1
        collideAndStream.setArg(0, f_collide);
        collideAndStream.setArg(1, rho);
        collideAndStream.setArg(2, u);
        collideAndStream.setArg(3, map);
        collideAndStream.setArg(4, f_stream);
        // 0
        collideAndStream_swap.setArg(0, f_stream);
        collideAndStream_swap.setArg(1, rho);
        collideAndStream_swap.setArg(2, u);
        collideAndStream_swap.setArg(3, map);
        collideAndStream_swap.setArg(4, f_collide);


        // initialize
        cl::Event init_evt;
        cl::NDRange lws = cl::NDRange(opts.lws, 1, 1);
        cl::NDRange gws = cl::NDRange(opts.dim, opts.dim, opts.dim);
        CLUCheckErrorExit(
            queue.enqueueNDRangeKernel(initialize, cl::NullRange, gws, lws, nullptr, &init_evt),
            "initialize"
        );
        events.emplace_back(init_evt, "initialize");

        if (opts.dump_map)  dump_map(queue, map);
        if (opts.store_vtk) dump_data(queue, rho, u, rho_val, u_val, iteration);

    } catch (cl::Error err) {
        CLUErrorPrintExit(err);
    }


    while (iteration <= opts.iterations) {

        int is_swap = (iteration & 1);

        if (opts.dump_f) {
            dump_f(queue, (is_swap ? f_stream : f_collide), f_val, iteration, opts.iterations);
        }

        processData(queue, (is_swap ? collideAndStream_swap : collideAndStream));
        iteration++;

        if (opts.store_vtk && (iteration != 0) && (iteration % opts.every == 0)) {
            dump_data(queue, rho, u, rho_val, u_val, iteration);
        }
    }

    queue.finish();

    double totalTime = 0.0;
    for (std::pair<cl::Event, std::string> & p : events) {
        const double time_evt = CLUEventsGetTime(p.first, p.first);
        totalTime += time_evt;

        std::cout << std::fixed << std::setw(16)
                  << p.second << ": "
                  << time_evt
                  << std::endl;
    }

    std::cout << " Total time: "
              << totalTime << " ms"
              << std::endl;

    double mlups = ((opts.dim - 2) * (opts.dim - 2) * (opts.dim - 2) * opts.iterations) / (totalTime * 1000);
    std::cout << "Performance: "
              << mlups << " MLUPS"
              << std::endl;

    if (rho_val != nullptr) delete[] rho_val;
    if (u_val != nullptr)   delete[] u_val;
    if (f_val != nullptr)   delete[] f_val;

    return 0;
}
