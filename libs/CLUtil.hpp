#pragma once

#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
#include "cl.hpp"
#else 
#include <CL/cl.hpp>
#endif

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>


static inline const char * cl_get_error_string(cl_int err) {
    switch (err) {
        case CL_SUCCESS                                  : return "CL_SUCCESS";
        case CL_DEVICE_NOT_FOUND                         : return "CL_DEVICE_NOT_FOUND";
        case CL_DEVICE_NOT_AVAILABLE                     : return "CL_DEVICE_NOT_AVAILABLE";
        case CL_COMPILER_NOT_AVAILABLE                   : return "CL_COMPILER_NOT_AVAILABLE";
        case CL_MEM_OBJECT_ALLOCATION_FAILURE            : return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
        case CL_OUT_OF_RESOURCES                         : return "CL_OUT_OF_RESOURCES";
        case CL_OUT_OF_HOST_MEMORY                       : return "CL_OUT_OF_HOST_MEMORY";
        case CL_PROFILING_INFO_NOT_AVAILABLE             : return "CL_PROFILING_INFO_NOT_AVAILABLE";
        case CL_MEM_COPY_OVERLAP                         : return "CL_MEM_COPY_OVERLAP";
        case CL_IMAGE_FORMAT_MISMATCH                    : return "CL_IMAGE_FORMAT_MISMATCH";
        case CL_IMAGE_FORMAT_NOT_SUPPORTED               : return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
        case CL_BUILD_PROGRAM_FAILURE                    : return "CL_BUILD_PROGRAM_FAILURE";
        case CL_MAP_FAILURE                              : return "CL_MAP_FAILURE";
        case CL_MISALIGNED_SUB_BUFFER_OFFSET             : return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
        case CL_COMPILE_PROGRAM_FAILURE                  : return "CL_COMPILE_PROGRAM_FAILURE";
        case CL_LINKER_NOT_AVAILABLE                     : return "CL_LINKER_NOT_AVAILABLE";
        case CL_LINK_PROGRAM_FAILURE                     : return "CL_LINK_PROGRAM_FAILURE";
        case CL_DEVICE_PARTITION_FAILED                  : return "CL_DEVICE_PARTITION_FAILED";
        case CL_KERNEL_ARG_INFO_NOT_AVAILABLE            : return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
        case CL_INVALID_VALUE                            : return "CL_INVALID_VALUE";
        case CL_INVALID_DEVICE_TYPE                      : return "CL_INVALID_DEVICE_TYPE";
        case CL_INVALID_PLATFORM                         : return "CL_INVALID_PLATFORM";
        case CL_INVALID_DEVICE                           : return "CL_INVALID_DEVICE";
        case CL_INVALID_CONTEXT                          : return "CL_INVALID_CONTEXT";
        case CL_INVALID_QUEUE_PROPERTIES                 : return "CL_INVALID_QUEUE_PROPERTIES";
        case CL_INVALID_COMMAND_QUEUE                    : return "CL_INVALID_COMMAND_QUEUE";
        case CL_INVALID_HOST_PTR                         : return "CL_INVALID_HOST_PTR";
        case CL_INVALID_MEM_OBJECT                       : return "CL_INVALID_MEM_OBJECT";
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR          : return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
        case CL_INVALID_IMAGE_SIZE                       : return "CL_INVALID_IMAGE_SIZE";
        case CL_INVALID_SAMPLER                          : return "CL_INVALID_SAMPLER";
        case CL_INVALID_BINARY                           : return "CL_INVALID_BINARY";
        case CL_INVALID_BUILD_OPTIONS                    : return "CL_INVALID_BUILD_OPTIONS";
        case CL_INVALID_PROGRAM                          : return "CL_INVALID_PROGRAM";
        case CL_INVALID_PROGRAM_EXECUTABLE               : return "CL_INVALID_PROGRAM_EXECUTABLE";
        case CL_INVALID_KERNEL_NAME                      : return "CL_INVALID_KERNEL_NAME";
        case CL_INVALID_KERNEL_DEFINITION                : return "CL_INVALID_KERNEL_DEFINITION";
        case CL_INVALID_KERNEL                           : return "CL_INVALID_KERNEL";
        case CL_INVALID_ARG_INDEX                        : return "CL_INVALID_ARG_INDEX";
        case CL_INVALID_ARG_VALUE                        : return "CL_INVALID_ARG_VALUE";
        case CL_INVALID_ARG_SIZE                         : return "CL_INVALID_ARG_SIZE";
        case CL_INVALID_KERNEL_ARGS                      : return "CL_INVALID_KERNEL_ARGS";
        case CL_INVALID_WORK_DIMENSION                   : return "CL_INVALID_WORK_DIMENSION";
        case CL_INVALID_WORK_GROUP_SIZE                  : return "CL_INVALID_WORK_GROUP_SIZE";
        case CL_INVALID_WORK_ITEM_SIZE                   : return "CL_INVALID_WORK_ITEM_SIZE";
        case CL_INVALID_GLOBAL_OFFSET                    : return "CL_INVALID_GLOBAL_OFFSET";
        case CL_INVALID_EVENT_WAIT_LIST                  : return "CL_INVALID_EVENT_WAIT_LIST";
        case CL_INVALID_EVENT                            : return "CL_INVALID_EVENT";
        case CL_INVALID_OPERATION                        : return "CL_INVALID_OPERATION";
        case CL_INVALID_GL_OBJECT                        : return "CL_INVALID_GL_OBJECT";
        case CL_INVALID_BUFFER_SIZE                      : return "CL_INVALID_BUFFER_SIZE";
        case CL_INVALID_MIP_LEVEL                        : return "CL_INVALID_MIP_LEVEL";
        case CL_INVALID_GLOBAL_WORK_SIZE                 : return "CL_INVALID_GLOBAL_WORK_SIZE";
        case CL_INVALID_PROPERTY                         : return "CL_INVALID_PROPERTY";
        case CL_INVALID_IMAGE_DESCRIPTOR                 : return "CL_INVALID_IMAGE_DESCRIPTOR";
        case CL_INVALID_COMPILER_OPTIONS                 : return "CL_INVALID_COMPILER_OPTIONS";
        case CL_INVALID_LINKER_OPTIONS                   : return "CL_INVALID_LINKER_OPTIONS";
        case CL_INVALID_DEVICE_PARTITION_COUNT           : return "CL_INVALID_DEVICE_PARTITION_COUNT";
        default                                          : return "CL_UNKNOWN_ERROR";
    }
}

static inline void CLUErrorPrint(const cl::Error & err,
                                 bool mustExit = false)
{
    const cl_int errCode = err.err();
    const char * what = err.what();
    const char * errString = cl_get_error_string(errCode);
    std::cerr << what << "(" << errCode << ") - " << errString << std::endl;

    if (mustExit) {
        exit(errCode);
    }
}

static inline void CLUCheckError(const cl_int err,
                                 const std::string & what,
                                 bool mustExit = false)
{
    if (err != CL_SUCCESS) {
        const char * errString = cl_get_error_string(err);
        std::cerr << what << "(" << err << ") - " << errString << std::endl;
        if (mustExit) {
            exit(err);
        }
    }
}

static inline void CLUSelectPlatform(cl::Platform & platform,
                                     int selected_platform = -1)
{
    std::vector<cl::Platform> platforms;

    try {
        cl::Platform::get(&platforms);
        while (selected_platform < 0 || selected_platform >= platforms.size()) {
                int platform_id = 0;
                for (cl::Platform & p : platforms) {
                    std::cout << "#" << platform_id++             << " "
                              << p.getInfo<CL_PLATFORM_NAME>()    << " "
                              << p.getInfo<CL_PLATFORM_VENDOR>()  << " "
                              << p.getInfo<CL_PLATFORM_VERSION>()
                              << std::endl;
                }
                std::cout << std::endl << "Select a platform: ";
                std::cin >> selected_platform;
                std::cout << std::endl;
        }
        platform = platforms[selected_platform];
    } catch (cl::Error err) {
        CLUErrorPrint(err);
    }
}

static inline void CLUSelectDevice(cl::Device & device,
                                   const cl::Platform & platform,
                                   int selected_device = -1)
{
    std::vector<cl::Device> devices;

    try {
        platform.getDevices(CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_CPU, &devices);

        while (selected_device < 0 || selected_device >= devices.size()) {
            int device_id = 0;
            for (cl::Device & d : devices) {
                std::cout << "#" << device_id++
                            << " [" << (d.getInfo<CL_DEVICE_TYPE>() == 2 ? "CPU" : "GPU") << "] "
                            << d.getInfo<CL_DEVICE_NAME>()
                            << std::endl
                            << "\tVendor:            " << d.getInfo<CL_DEVICE_VENDOR>()                     << std::endl
                            << "\tMax Compute Units: " << d.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>()          << std::endl
                            << "\tGlobal Memory:     " << (d.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() >> 20)    << " MB" << std::endl
                            << "\tMax Clock Freq.:   " << d.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>()        << " MHz" << std::endl
                            << "\tMax Alloc. Memory: " << (d.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() >> 20) << " MB" << std::endl
                            << "\tLocal Memory:      " << (d.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() >> 10)     << " KB" << std::endl
                            << "\tAvailable:         " << (d.getInfo<CL_DEVICE_AVAILABLE>() ? "YES" : "NO") << std::endl
                            << std::endl;
            }
            std::cout << std::endl << "Select a device: ";
            std::cin >> selected_device;
            std::cout << std::endl;
        }
        device = devices[selected_device];
    } catch (cl::Error err) {
        CLUErrorPrint(err);
    }
}

static inline void CLUCreateContext(cl::Context & context,
                                    const cl::Device & device)
{
    try {
        context = cl::Context(device);
    } catch (cl::Error err) {
        CLUErrorPrint(err);
    }
}

static inline void CLUCreateQueue(cl::CommandQueue & queue,
                                  const cl::Context & context,
                                  const cl::Device & device)
{
    try {
        queue = cl::CommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE);
    } catch (cl::Error err) {
        CLUErrorPrint(err);
    }
}

static inline void CLUBuildProgram(cl::Program & program,
                                   const cl::Context & context,
                                   const cl::Device & device,
                                   const std::string & filename,
                                   const std::string & options)
{
    std::ifstream streamSourcecode(filename);
    std::string sourcecode(std::istreambuf_iterator<char>(streamSourcecode),
                           (std::istreambuf_iterator<char>()));
    try {
        cl::Program::Sources source(1, std::make_pair(sourcecode.c_str(), sourcecode.length() + 1));
        program = cl::Program(context, source);
        program.build(options.c_str());
    } catch(cl::Error err) {
        CLUErrorPrint(err);
        if (err.err() == CL_BUILD_PROGRAM_FAILURE) {
            std::cerr << "Build log: "
                      << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device)
                      << std::endl;
        }
    }
}

static inline void CLUEventPrintStats(const std::string & name,
                                      const cl::Event & event)
{
    try {
        event.wait();
        cl_ulong start = event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        cl_ulong end   = event.getProfilingInfo<CL_PROFILING_COMMAND_END>();
        const double time = (end - start) / 1000000.0f;

        std::cout << name << ": "
                  << std::fixed << std::setw(8) << std::setprecision(4)
                  << time << " ms"
                  << std::endl;
    } catch(cl::Error err) {
        CLUErrorPrint(err);
    }
}

static inline void CLUWriteBufferCubeToVTK(const float * data,
                                           const size_t size,
                                           const size_t timestamp)
{
    std::ofstream vtk;

    std::stringstream filenameBuilder;
    filenameBuilder << "/Volumes/RamDisk/rho." << timestamp << ".vti";

    vtk.open(filenameBuilder.str());

    vtk << "<?xml version=\"1.0\"?>\n" 
        << "<!-- openLBMflow v1.0.1, www.lbmflow.com -->\n" 
        << "<VTKFile type=\"ImageData\" version=\"0.1\" byte_order=\"LittleEndian\">\n" 
        << "  <ImageData WholeExtent=\"0 " << (size - 1) << " 0 " << (size - 1) << " 0 " << (size - 1) << "\" Origin=\"1 1 1\" Spacing=\"1 1 1\">\n"
        << "  <Piece Extent=\"0 " << (size - 1) << " 0 " << (size - 1) << " 0 " << (size - 1) << "\">\n"
        << "    <PointData Scalars=\"scalars\">\n" 
        << "      <DataArray type=\"Float32\" Name=\"Density\" NumberOfComponents=\"1\" format=\"ascii\">\n";

    for (size_t z = 0; z < size; ++z) {
        for (size_t y = 0; y < size; ++y) {
            for (size_t x = 0; x < size; ++x) {
                const float val = data[x + (y * size) + (z * size * size)];
                vtk << std::scientific << (isnan(val) ? 0.0f : val) << " ";
            }
            vtk << "\n";
        }
        vtk << "\n";
    }
    
    vtk << "      </DataArray>\n"
        << "    </PointData>\n"
        << "    <CellData>\n"
        << "    </CellData>\n"
        << "  </Piece>\n"
        << "  </ImageData>\n"
        << "</VTKFile>\n";
    vtk.close();
}

static inline void CLUWriteBufferCube3DToVTK(const float * data,
                                             const size_t size,
                                             const size_t timestamp)
{
    std::ofstream vtk;

    std::stringstream filenameBuilder;
    filenameBuilder << "/Volumes/RamDisk/velocity." << timestamp << ".vti";

    vtk.open(filenameBuilder.str());

    vtk << "<?xml version=\"1.0\"?>\n" 
        << "<!-- openLBMflow v1.0.1, www.lbmflow.com -->\n" 
        << "<VTKFile type=\"ImageData\" version=\"0.1\" byte_order=\"LittleEndian\">\n" 
        << "  <ImageData WholeExtent=\"0 " << (size - 1) << " 0 " << (size - 1) << " 0 " << (size - 1) << "\" Origin=\"0 0 0\" Spacing=\"1 1 1\">\n"
        << "  <Piece Extent=\"0 " << (size - 1) << " 0 " << (size - 1) << " 0 " << (size - 1) << "\">\n"
        << "    <PointData Scalars=\"scalars\">\n" 
        << "      <DataArray type=\"Float32\" Name=\"Velocity\" NumberOfComponents=\"3\" format=\"ascii\">\n";

    for (size_t z = 0; z < size; ++z) {
        for (size_t y = 0; y < size; ++y) {
            for (size_t x = 0; x < size; ++x) {
                const float val0 = data[(x + (y * size) + (z * size * size)) * 4    ];
                const float val1 = data[(x + (y * size) + (z * size * size)) * 4 + 1];
                const float val2 = data[(x + (y * size) + (z * size * size)) * 4 + 2];
                vtk << std::scientific << (isnan(val0) ? 0.0f : val0) << " "
                    << std::scientific << (isnan(val1) ? 0.0f : val1) << " "
                    << std::scientific << (isnan(val2) ? 0.0f : val2) << " ";
            }
            vtk << "\n";
        }
        vtk << "\n";
    }
    
    vtk << "      </DataArray>\n"
        << "    </PointData>\n"
        << "    <CellData>\n"
        << "    </CellData>\n"
        << "  </Piece>\n"
        << "  </ImageData>\n"
        << "</VTKFile>\n";
    vtk.close();
}
