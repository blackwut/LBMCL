#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <ctime>
#include <chrono>


#include "common.h"
#include "CLUtil.hpp"
#include "ArgsUtil.hpp"
#include "StoreUtil.hpp"

static std::chrono::high_resolution_clock::time_point startPoint;
static std::chrono::high_resolution_clock::time_point endPoint;

inline void startTimer() {
    startPoint = std::chrono::high_resolution_clock::now();
}

inline void stopTimer() {
    endPoint = std::chrono::high_resolution_clock::now();
}

inline long getTimerMS() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(endPoint - startPoint).count();
}

inline long getTimerUS() {
    return std::chrono::duration_cast<std::chrono::microseconds>(endPoint - startPoint).count();
}

inline void printTimer() {
    const long msec = getTimerMS();
    const long usec = getTimerUS();
    std::cout << "Compute time: " << msec << " ms " << usec << " usec " << std::endl;
}


double totalTime = 0.0;
lbm_options opts;
std::vector< std::pair<cl::Event, std::string> > events;


static void dump_map(const cl::CommandQueue & queue,
                     const cl::Buffer & map)
{
    try {
        int * map_val = new int[opts.map_dim()];
        cl::Event read_evt;
        CLUCheckError(
            queue.enqueueReadBuffer(map, CL_TRUE, 0, opts.map_size(), map_val, nullptr, &read_evt),
            "dump_map",
            true
        );
        events.emplace_back(read_evt, "dump_map"); //totalTime += CLUEventPrintStats("read_map", read_evt);

        store_map(opts.dump_path, map_val, opts.dim);
        delete [] map_val;
    } catch (cl::Error err) {
        CLUErrorPrint(err, true);
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
        CLUCheckError(
            queue.enqueueReadBuffer(f, CL_TRUE, 0, opts.f_size(), f_val, nullptr, &read_evt),
            "dump_f",
            true
        );
        events.emplace_back(read_evt, "dump_f"); //totalTime += CLUEventPrintStats("read_f", read_evt);
    } catch (cl::Error err) {
        CLUErrorPrint(err, true);
    }
    store_f(opts.dump_path, f_val, opts.dim, iteration, iterations);
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
        CLUCheckError(
            queue.enqueueReadBuffer(rho, CL_TRUE, 0, opts.rho_size(), rho_val, nullptr, &read_rho_evt),
            "readRho",
            true
        );
        events.emplace_back(read_rho_evt, "readRho"); // totalTime += CLUEventPrintStats("readRho", read_rho_evt);


        CLUCheckError(
            queue.enqueueReadBuffer(u, CL_TRUE, 0, opts.u_size(), u_val, nullptr, &read_u_evt),
            "readU",
            true
        );
        events.emplace_back(read_u_evt, "readU"); // totalTime += CLUEventPrintStats("readU", read_u_evt);
    } catch (cl::Error err) {
        CLUErrorPrint(err, true);
    }
    store_vtk(opts.vtk_path, rho_val, u_val, opts.dim, iteration, opts.iterations);
}


static void processData(const cl::CommandQueue & queue,
                        const cl::Kernel & collideAndStream)
{
    cl::NDRange lws = cl::NDRange(opts.dim, 1, 1);
    cl::NDRange gws = cl::NDRange(opts.dim, opts.dim, opts.dim);
    try {
        cl::Event collideAndStream_evt;

        CLUCheckError(
            queue.enqueueNDRangeKernel(collideAndStream, cl::NullRange, gws, lws, nullptr, &collideAndStream_evt),
            "collideAndStream",
            true
        );
        events.emplace_back(collideAndStream_evt, "collideAndStream"); //totalTime += CLUEventPrintStats("collideAndStream", collideAndStream_evt);

    } catch (cl::Error err) {
        CLUErrorPrint(err, true);
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

    // OpenCL init
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
    optionsBuilder << "-cl-fast-relaxed-math ";
    optionsBuilder << "-DDIM="       << opts.dim << " ";
    optionsBuilder << "-DVISCOSITY=" << opts.viscosity << " ";
    optionsBuilder << "-DVELOCITY="  << opts.velocity << " ";
#ifdef FP_DOUBLE
    optionsBuilder << "-DFP_DOUBLE ";
#else
    optionsBuilder << "-DFP_SINGLE ";
    optionsBuilder << "-cl-single-precision-constant ";
#endif

    std::cout << "Kernels options: " << optionsBuilder.str() << std::endl;

    CLUBuildProgram(program, context, device, "kernels.cl", optionsBuilder.str());

    cl::Kernel               initLBM(program, "init");
    cl::Kernel      collideAndStream(program, "collideAndStream");
    cl::Kernel collideAndStream_swap(program, "collideAndStream");

    cl_int err;

    cl::Buffer f_stream = cl::Buffer(context, (opts.dump_f ? CL_MEM_READ_WRITE : CL_MEM_HOST_NO_ACCESS), opts.f_size(), nullptr, &err);
    CLUCheckError(err, "cl::Buffer(f_stream)", true);

    cl::Buffer f_collide = cl::Buffer(context, (opts.dump_f ? CL_MEM_READ_WRITE : CL_MEM_HOST_NO_ACCESS), opts.f_size(), nullptr, &err);
    CLUCheckError(err, "cl::Buffer(f_collide))", true);

    cl::Buffer rho = cl::Buffer(context, CL_MEM_READ_WRITE, opts.rho_size(), nullptr, &err);
    CLUCheckError(err, "cl::Buffer(rho)", true);

    cl::Buffer u = cl::Buffer(context, CL_MEM_READ_WRITE, opts.u_size(), nullptr, &err);
    CLUCheckError(err, "cl::Buffer(u)", true);

    cl::Buffer map = cl::Buffer(context, (opts.dump_map ? CL_MEM_READ_WRITE : CL_MEM_HOST_NO_ACCESS), opts.map_size(), nullptr, &err);
    CLUCheckError(err, "cl::Buffer(map)", true);


    startTimer();
    try {
        initLBM.setArg(0, f_stream);
        initLBM.setArg(1, f_collide);
        initLBM.setArg(2, rho);
        initLBM.setArg(3, u);
        initLBM.setArg(4, map);

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


        // initLBM
        cl::Event init_evt;
        cl::NDRange gws = cl::NDRange(opts.dim, opts.dim, opts.dim);
        CLUCheckError(
            queue.enqueueNDRangeKernel(initLBM, cl::NullRange, gws, cl::NullRange, nullptr, &init_evt),
            "initLBM",
            true
        );
        //totalTime += CLUEventPrintStats("         initLBM", init_evt);
        events.emplace_back(init_evt, "init");

        if (opts.dump_map)  dump_map(queue, map);
        if (opts.store_vtk) dump_data(queue, rho, u, rho_val, u_val, iteration);

    } catch (cl::Error err) {
        CLUErrorPrint(err, true);
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

    

    cl::Event start_evt = events.at(1).first;
    cl::Event end_evt = events.back().first;

    end_evt.wait();
    stopTimer();
    queue.finish();

    for (std::pair<cl::Event, std::string> & p : events) {
        std::cout << std::fixed << std::setw(16)
                  << p.second << ": "
                  << CLUEventsGetTime(p.first, p.first)
                  << std::endl;
    }

    printTimer();

    totalTime = CLUEventsGetTime(start_evt, end_evt);

    std::cout << " Total time:"
              << std::fixed << std::setw(12) << std::setprecision(5)
              << totalTime << " ms"
              << std::endl;

    double mlups = ((opts.dim - 2) * (opts.dim - 2) * (opts.dim - 2) * opts.iterations) / (totalTime * 1000);
    std::cout << "Performance:"
              << std::fixed << std::setw(12) << std::setprecision(5)
              << mlups << " MLUPS"
              << std::endl;


    if (rho_val != nullptr) delete[] rho_val;
    if (u_val != nullptr)   delete[] u_val;
    if (f_val != nullptr)   delete[] f_val;

    return 0;
}
