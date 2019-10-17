#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <getopt.h>

#include "CLUtil.hpp"
#include "constants.h"

#undef IDxyz
#define IDxyz(x, y, z) ((x) + ((y) * options.dim) + ((z) * options.dim * options.dim))


#define VTK_FORMAT      1
#define RESULTS_FOLDER "./results"



int timestamp = 0;
double totalTime = 0.0;


struct lbm_options {
    int platformID;
    int deviceID;
    int dim;
    float viscosity;
    float velocity;
    int iterations;
    int every;
    std::string vti_path;
    bool store_vti;
    std::string dump_path;
    bool dump_map;
    bool dump_f;

    lbm_options() :
        platformID(-1),
        deviceID(-1),
        dim(8),
        viscosity(0.0089f),
        velocity(0.05f),
        iterations(10),
        every(-1),
        vti_path(RESULTS_FOLDER),
        store_vti(false),
        dump_path(RESULTS_FOLDER),
        dump_map(false),
        dump_f(false)
    {}

    size_t f_dim()   const { return (dim * dim * dim * Q_DIM); }
    size_t u_dim()   const { return (dim * dim * dim * D); }
    size_t rho_dim() const { return (dim * dim * dim); }
    size_t map_dim() const { return (dim * dim * dim); }

    size_t f_size()   const { return f_dim()   * sizeof(float); }
    size_t u_size()   const { return u_dim()   * sizeof(float); }
    size_t rho_size() const { return rho_dim() * sizeof(float); }
    size_t map_size() const { return map_dim() * sizeof(int);   }

    size_t device_memory_size_b() const
    {
        return f_size() * 2 + u_size() + rho_size() + map_size();
    }

    size_t device_memory_size_k() const
    {
        return device_memory_size_b() / (float)(1 << 10);
    }

    size_t device_memory_size_m() const
    {
        return device_memory_size_b() / (1 << 20);
    }

    void print_options()
    {
        std::cout << "PlatformID       = " << platformID             << "\n"
                  << "DeviceID         = " << deviceID               << "\n"
                  << "dim              = " << dim                    << "\n"
                  << "viscosity        = " << viscosity              << "\n"
                  << "velocity         = " << velocity               << "\n"
                  << "Device Mem. (KB) = " << device_memory_size_k() << "\n"
                  << "Device Mem. (MB) = " << device_memory_size_m() << "\n"
                  << "VTI PATH         = " << vti_path               << "\n"
                  << "STORE VTI        = " << store_vti              << "\n"
                  << "DUMP F           = " << dump_f                 << "\n"
                  << "DUMP MAP         = " << dump_map               << "\n";
    }
};

lbm_options options;


static void print_help()
{
    std::cout << "-P  --platform      Use the specified platform                \n"
                 "-D  --device        Use the specified device                  \n"
                 "-d  --dim           Set the lattice dimension                 \n"
                 "-v  --viscosity     Set the fluid viscosity                   \n"
                 "-u  --velocity      Set the x velocity of the moving wall     \n"
                 "-i  --iterations    Specify the number of iterations          \n"
                 "-e  --every         Save simulation results every N iterations\n"
                 "-k  --vtk-path      Specify where store VTI files             \n"
                 "-p  --dump-path     Specify where store dumps                 \n"
                 "-m  --dump-map      Dump the lattice map                      \n"
                 "-f  --dump-f        Dump the lattice \"f\" for each iteration \n"
                 "-h  --help          Show this help message and exit           \n";
    exit(1);
}


static void process_args(int argc, char * argv[])
{
    const char * const short_opts = "P:D:d:v:u:i:e:k:p::mfh";
    const option long_opts[] = {
            {"platform",   required_argument, nullptr, 'P'},
            {"device",     required_argument, nullptr, 'D'},
            {"dim",        required_argument, nullptr, 'd'},
            {"viscosity",  required_argument, nullptr, 'v'},
            {"velocity",   required_argument, nullptr, 'u'},
            {"iterations", required_argument, nullptr, 'i'},
            {"every",      optional_argument, nullptr, 'e'},
            {"vti-path",   optional_argument, nullptr, 'k'},
            {"dump-path",  optional_argument, nullptr, 'p'},
            {"dump-map",   no_argument,       nullptr, 'm'},
            {"dump-f",     no_argument,       nullptr, 'f'},
            {"help",       no_argument,       nullptr, 'h'},
            {nullptr,      no_argument,       nullptr,   0}
    };

    while (1) {
        const int opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

        if (opt < 0) break;

        switch (opt) {
            case 'P':
                if ((options.platformID = std::stoi(optarg)) < 0) {
                    std::cerr << "Please enter a valid platform" << std::endl;
                    exit(1);
                }
                break;
            case 'D':
                if ((options.deviceID = std::stoi(optarg)) < 0) {
                    std::cerr << "Please enter a valid device" << std::endl;
                    exit(1);
                }
                break;
            case 'd':
                if ((options.dim = std::stoi(optarg)) < 0) {
                    std::cerr << "Please enter a valid lattice dimension" << std::endl;
                    exit(1);
                }
                break;
            case 'v':
                if ((options.viscosity = std::atof(optarg)) < 0) {
                    std::cerr << "Please enter a valid viscosity value" << std::endl;
                    exit(1);
                }
                break;
            case 'u':
                options.velocity = std::atof(optarg);
                break;
            case 'i':
                if ((options.iterations = std::stoi(optarg)) < 0) {
                    std::cerr << "Please enter a valid number of iterations" << std::endl;
                    exit(1);
                }
                break;
            case 'e':
                if ((options.every = std::stoi(optarg)) < 0) {
                    std::cerr << "Please enter a valid number for save simulation results every N iterations" << std::endl;
                    exit(1);
                }
                break;
            case 'k':
                printf("-k = %s\n", optarg);
                options.vti_path = std::string(optarg);
                if (options.vti_path.empty()) {
                    std::cout << "VTI files will be stored here" << std::endl;
                    options.vti_path = RESULTS_FOLDER;
                }
                options.store_vti = true;
                break;
            case 'p':
                options.dump_path  = std::string(optarg);
                if (options.dump_path.empty()) {
                    std::cout << "dump files will be stored here" << std::endl;
                    options.dump_path = RESULTS_FOLDER;
                }
                break;
            case 'm':
                options.dump_map = true;
                break;
            case 'f':
                options.dump_f = true;
                break;
            case 'h':
            case '?':
            default:
                print_help();
                break;
        }
    }
}


static void dump_map(const cl::CommandQueue & queue,
                     const cl::Buffer & map,
                     float * map_val)
{
    try {
        cl::Event event_read_map;
        CLUCheckError(
            queue.enqueueReadBuffer(map, CL_TRUE, 0, options.map_size(), map_val, NULL, &event_read_map),
            "dump_f",
            true
        );
        totalTime += CLUEventPrintStats("        read_map", event_read_map);
    } catch (cl::Error err) {
        CLUErrorPrint(err, true);
    }

    std::stringstream filenameBuilder;
    filenameBuilder << options.dump_path << "/map.dump";
    std::ofstream dump;
    dump.open(filenameBuilder.str());

    for (int z = 0; z < options.dim; ++z) {
        for (int y = 0; y < options.dim; ++y) {
            for (int x = 0; x < options.dim; ++x) {
                const int cell_type = map_val[IDxyz(x, y, z)];
                
                int val = 1;
                if (is_wall(cell_type)) val = 4;
                if (is_moving(cell_type)) val = 2;
                if (!is_moving(cell_type) && is_boundary(cell_type)) val = 3;
                dump << val << " ";
            }
            dump << std::endl;
        }
        dump << std::endl;
    }
    dump << std::endl;
    dump.close();
}


static void dump_f(const cl::CommandQueue & queue,
                   const cl::Buffer & f,
                   float * f_val)
{
    try {
        cl::Event event_read_f;
        CLUCheckError(
            queue.enqueueReadBuffer(f, CL_TRUE, 0, options.f_size(), f_val, NULL, &event_read_f),
            "dump_f",
            true
        );
        totalTime += CLUEventPrintStats("          read_f", event_read_f);
    } catch (cl::Error err) {
        CLUErrorPrint(err, true);
    }

    std::stringstream filenameBuilder;
    filenameBuilder << options.dump_path << "/f_" << std::setw(4) << std::setfill('0') << timestamp << ".dump";
    std::ofstream dump;
    dump.open(filenameBuilder.str());

    for (int z = 0; z < options.dim; ++z) {
        for (int y = 0; y < options.dim; ++y) {
            dump << "        ";
            for (int q = 0; q < Q; ++q) {
                dump << std::setw(8) << q << " ";
            }
            dump << std::endl;

            for (int x = 0; x < options.dim; ++x) {
                const int index = IDxyz(x, y, z);
                dump << "(" << x << "," << y << "," << z << ") ";
                for (int q = 0; q < Q; ++q) {
                    dump << std::fixed << std::setw(8) << std::setprecision(6) << f_val[IDxyzw(index,  q)] << " ";
                }
                dump << std::endl;
            }
            dump << std::endl;
        }
        dump << std::endl;
    }
    dump << std::endl;
    dump.close();
}


static void store_vti(const cl::CommandQueue & queue,
                      const cl::Buffer & rho,
                      const cl::Buffer & u,
                      float * rho_val,
                      float * u_val)
{
    cl::Event event_read_rho;
    cl::Event event_read_u;

    try {
        CLUCheckError(
            queue.enqueueReadBuffer(rho, CL_TRUE, 0, options.rho_size(), rho_val, NULL, &event_read_rho),
            "readRho",
            true
        );
        totalTime += CLUEventPrintStats("         readRho", event_read_rho);


        CLUCheckError(
            queue.enqueueReadBuffer(u, CL_TRUE, 0, options.u_size(), u_val, NULL, &event_read_u),
            "readU",
            true
        );
        totalTime += CLUEventPrintStats("           readU", event_read_rho);
    } catch (cl::Error err) {
        CLUErrorPrint(err, true);
    }


    const size_t from = 1;
    const size_t to = options.dim - 1;

#if VTK_FORMAT
    const size_t extent = to - from - 1;
#endif

    std::stringstream filenameBuilder;
    filenameBuilder << options.vti_path << "/lbmcl." << std::setw(4) << std::setfill('0') << timestamp << ".vti";
    std::ofstream vtk;
    vtk.open(filenameBuilder.str());

#if VTK_FORMAT
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
                const float val = rho_val[IDxyz(x, y, z)];
                vtk << std::scientific << val << " ";
            }
            vtk << "\n";
        }
        vtk << "\n";
    }

#if VTK_FORMAT
    vtk << "        </DataArray>\n"
        << "        <DataArray type=\"Float32\" Name=\"v\" NumberOfComponents=\"3\" format=\"ascii\">\n";
#endif

    for (size_t z = from; z < (to); ++z) {
        for (size_t y = from; y < (to); ++y) {
            for (size_t x = from; x < (to); ++x) {
                const float val_x = u_val[IDxyz(x, y, z) * 3 + 0];
                const float val_y = u_val[IDxyz(x, y, z) * 3 + 1];
                const float val_z = u_val[IDxyz(x, y, z) * 3 + 2];
                vtk << std::scientific << val_x << " "
                    << std::scientific << val_y << " "
                    << std::scientific << val_z << " ";
            }
            vtk << "\n";
        }
        vtk << "\n";
    }

#if VTK_FORMAT
    vtk << "        </DataArray>\n"
        << "      </PointData>\n"
        << "    </Piece>\n"
        << "  </ImageData>\n"
        << "</VTKFile>\n";
#endif

    vtk.close();
}


static void processData(const cl::CommandQueue & queue,
                        const cl::Kernel & collideAndStream,
                        const cl::Buffer & rho,
                        const cl::Buffer & u,
                        float * rho_val,
                        float * u_val,
                        const cl::Buffer & f,
                        float * f_val)
{
    cl::NDRange lws = cl::NDRange(options.dim, 1, 1);
    cl::NDRange gws = cl::NDRange(options.dim, options.dim, options.dim);
    try {
        cl::Event event_collideAndStream;

        CLUCheckError(
            queue.enqueueNDRangeKernel(collideAndStream, cl::NullRange, gws, lws, NULL, &event_collideAndStream),
            "collideAndStream",
            true
        );
        totalTime += CLUEventPrintStats("collideAndStream", event_collideAndStream);

        // TODO: check the condition 
        if (options.store_vti && ((timestamp - 1) % options.every == 0)) store_vti(queue, rho, u, rho_val, u_val);
        if (options.dump_f) dump_f(queue, f, f_val);

    } catch (cl::Error err) {
        CLUErrorPrint(err, true);
    }
}

int main(int argc, char * argv[])
{
    // ARGS
    process_args(argc, argv);
    options.print_options();

    float * rho_val = NULL;
    float * u_val = NULL;
    float * map_val = NULL;
    float * f_val = NULL;

    if (options.store_vti) {
        rho_val = new float[options.rho_dim()];
        u_val = new float[options.u_dim()];
    }

    if (options.dump_map) {
        map_val = new float[options.map_dim()];
    }

    if (options.dump_f) {
        f_val = new float[options.f_dim()];
    }

    // OpenCL init
    cl::Platform platform;
    cl::Device device;
    cl::Context context;
    cl::CommandQueue queue;
    cl::Program program;

    CLUSelectPlatform(platform, options.platformID);
    CLUSelectDevice(device, platform, options.deviceID);
    CLUCreateContext(context, device);
    CLUCreateQueue(queue, context, device);

    std::stringstream optionsBuilder;
    optionsBuilder << "-Werror ";
    optionsBuilder << "-I. ";
    optionsBuilder << "-cl-fast-relaxed-math ";
    optionsBuilder << "-cl-denorms-are-zero ";
    optionsBuilder << "-DDIM=" << options.dim << " ";
    optionsBuilder << "-DVISC=" << options.viscosity << " ";
    optionsBuilder << "-DVEL=" << options.velocity << " ";
    std::cout << "Kernels options: " << optionsBuilder.str() << std::endl;

    CLUBuildProgram(program, context, device, "kernels.cl", optionsBuilder.str());

    cl::Kernel                initLBM(program, "init");
    cl::Kernel      collideAndStream(program, "collideAndStream");
    cl::Kernel collideAndStream_swap(program, "collideAndStream");

    cl_int err;

    cl::Buffer f_stream = cl::Buffer(context, CL_MEM_READ_WRITE, options.f_size(), NULL, &err);
    CLUCheckError(err, "cl::Buffer(f_stream)", true);

    cl::Buffer f_collide = cl::Buffer(context, CL_MEM_READ_WRITE, options.f_size(), NULL, &err);
    CLUCheckError(err, "cl::Buffer(f_collide))", true);

    cl::Buffer rho = cl::Buffer(context, CL_MEM_READ_WRITE, options.rho_size(), NULL, &err);
    CLUCheckError(err, "cl::Buffer(rho)", true);

    cl::Buffer u = cl::Buffer(context, CL_MEM_READ_WRITE, options.u_size(), NULL, &err);
    CLUCheckError(err, "cl::Buffer(u)", true);

    cl::Buffer map = cl::Buffer(context, CL_MEM_READ_WRITE, options.map_size(), NULL, &err);
    CLUCheckError(err, "cl::Buffer(map)", true);


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
        cl::Event event_initLBM;
        cl::NDRange gws = cl::NDRange(options.dim, options.dim, options.dim);
        CLUCheckError(
            queue.enqueueNDRangeKernel(initLBM, cl::NullRange, gws, cl::NullRange, NULL, &event_initLBM),
            "initLBM",
            true
        );
        totalTime += CLUEventPrintStats("         initLBM", event_initLBM);

        if (options.dump_map)  dump_map(queue, map, map_val);
        if (options.dump_f)    dump_f(queue, f_collide, f_val);
        if (options.store_vti) store_vti(queue, rho, u, rho_val, u_val);
        timestamp++;

    } catch (cl::Error err) {
        CLUErrorPrint(err, true);
    }


    while (timestamp <= options.iterations) {

        if ((timestamp - 1) % 2 == 0) {
            processData(queue,
                        collideAndStream,
                        rho,
                        u,
                        rho_val,
                        u_val,
                        f_stream,
                        f_val);
        } else {
            processData(queue,
                        collideAndStream_swap,
                        rho,
                        u,
                        rho_val,
                        u_val,
                        f_collide,
                        f_val);
        }
        timestamp++;
    }

    std::cout << "Total time: "
              << std::fixed << std::setw(8) << std::setprecision(4)
              << totalTime << " ms"
              << std::endl;

    float mlups = ((options.dim - 2) * (options.dim - 2) * (options.dim - 2) * options.iterations) / (totalTime * 1000);
    std::cout << "Performance:"
              << std::fixed << std::setw(8) << std::setprecision(4)
              << mlups << " MLUPS"
              << std::endl;


    if (rho_val != NULL) delete[] rho_val;
    if (u_val != NULL)   delete[] u_val;
    if (f_val != NULL)   delete[] f_val;
    if (map_val != NULL) delete[] map_val;

    return 0;
}
