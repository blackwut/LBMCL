#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <limits>
#include <getopt.h>


#define RESULTS_FOLDER      "./results"


struct lbm_options {
    int platformID;
    int deviceID;
    size_t dim;
    real_t viscosity;
    real_t velocity;
    size_t iterations;
    size_t every;
    std::string vtk_path;
    bool store_vtk;
    size_t lws;
    size_t stride;
    bool optimize;
    std::string dump_path;
    bool dump_map;
    bool dump_f;

    lbm_options() :
        platformID(-1),
        deviceID(-1),
        dim(8),
        viscosity(0.0089),
        velocity(0.05),
        iterations(10),
        every(1),
        vtk_path(RESULTS_FOLDER),
        store_vtk(false),
        lws(32),
        stride(32),
        optimize(false),
        dump_path(RESULTS_FOLDER),
        dump_map(false),
        dump_f(false)
    {}


    size_t f_dim()   const { return (dim * dim * dim * Q); }
    size_t u_dim()   const { return (dim * dim * dim * D); }
    size_t rho_dim() const { return (dim * dim * dim); }
    size_t map_dim() const { return (dim * dim * dim); }

    size_t f_size()   const { return f_dim()   * sizeof(real_t); }
    size_t u_size()   const { return u_dim()   * sizeof(real_t); }
    size_t rho_size() const { return rho_dim() * sizeof(real_t); }
    size_t map_size() const { return map_dim() * sizeof(int);    }

    size_t device_memory_size_b() const
    {
        return f_size() * 2 + u_size() + rho_size() + map_size();
    }


    size_t device_memory_size_k() const
    {
        return device_memory_size_b() / (1 << 10);
    }


    size_t device_memory_size_m() const
    {
        return device_memory_size_b() / (1 << 20);
    }


    void print_help()
    {
        std::cout << "-P  --platform            Use the specified platform                     \n"
                     "-D  --device              Use the specified device                       \n"
                     "-d  --dim                 Set the lattice cube dimension                 \n"
                     "-n  --viscosity           Set the fluid viscosity                        \n"
                     "-u  --velocity            Set the x velocity of the moving wall          \n"
                     "-i  --iterations          Specify the number of iterations               \n"
                     "-e  --every               Save simulation results every N iterations     \n"
                     "-w  --work_group_size     Specify the work group size of kernel launch   \n"
                     "-s  --stride              Specify the stride used in CSoA memory layout  \n"
                     "-o  --optimize            Use \"cl-fast-relaxed-math\" in OpenCL kernels \n"
                     "-v  --vtk_path            Specify where store VTI files                  \n"
                     "-p  --dump_path           Specify where store dumps                      \n"
                     "-m  --dump_map            Dump the lattice map                           \n"
                     "-f  --dump_f              Dump the lattice \"f\" for each iteration      \n"
                     "-h  --help                Show this help message and exit                \n";
        exit(1);
    }


    bool is_power_of_two(size_t x)
    {
        return x && !(x & (x - 1));
    }


    size_t most_significant_bit(size_t n)
    {
        n |= n >>  1;
        n |= n >>  2;
        n |= n >>  4;
        n |= n >>  8;
        n |= n >> 16;
        n |= n >> 32;
        n = n + 1;
        return (n >> 1);
    }


    void process_args(int argc, char * argv[])
    {
        opterr = 0;

        const char * const short_opts = "P:D:d:n:u:i:e:v:w:s:op:mfh";
        const option long_opts[] = {
                {"platform",        required_argument, nullptr, 'P'},
                {"device",          required_argument, nullptr, 'D'},
                {"dim",             required_argument, nullptr, 'd'},
                {"viscosity",       required_argument, nullptr, 'n'},
                {"velocity",        required_argument, nullptr, 'u'},
                {"iterations",      required_argument, nullptr, 'i'},
                {"every",           optional_argument, nullptr, 'e'},
                {"work_group_size", optional_argument, nullptr, 'w'},
                {"stride",          optional_argument, nullptr, 's'},
                {"optimize",        no_argument,       nullptr, 'o'},
                {"vtk_path",        optional_argument, nullptr, 'v'},
                {"dump_path",       optional_argument, nullptr, 'p'},
                {"dump_map",        no_argument,       nullptr, 'm'},
                {"dump_f",          no_argument,       nullptr, 'f'},
                {"help",            no_argument,       nullptr, 'h'},
                {nullptr,           no_argument,       nullptr,   0}
        };

        int int_opt = -1;
        real_t real_opt = std::numeric_limits<real_t>::quiet_NaN();

        while (1) {
            const int opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

            if (opt < 0) break;

            switch (opt) {
                case 'P':
                    if ((int_opt = std::stoi(optarg)) < 0) {
                        std::cerr << "Please enter a valid platform" << std::endl;
                        exit(1);
                    }
                    platformID = int_opt;
                    break;
                case 'D':
                    if ((int_opt = std::stoi(optarg)) < 0) {
                        std::cerr << "Please enter a valid device" << std::endl;
                        exit(1);
                    }
                    deviceID = int_opt;
                    break;
                case 'd':
                    if ((int_opt = std::stoi(optarg)) < 0) {
                        std::cerr << "Please enter a valid lattice dimension" << std::endl;
                        exit(1);
                    }
                    dim = int_opt;
                    break;
                case 'n':
                    if ((real_opt = std::atof(optarg)) < 0) {
                        std::cerr << "Please enter a valid viscosity value" << std::endl;
                        exit(1);
                    }
                    viscosity = real_opt;
                    break;
                case 'u':
                    velocity = std::atof(optarg);
                    break;
                case 'i':
                    if ((int_opt = std::stoi(optarg)) < 0) {
                        std::cerr << "Please enter a valid number of iterations" << std::endl;
                        exit(1);
                    }
                    iterations = int_opt;
                    break;
                case 'e':
                    if ((int_opt = std::stoi(optarg)) < 0) {
                        std::cerr << "Please enter a valid number for save simulation results every N iterations" << std::endl;
                        exit(1);
                    }
                    every = int_opt;
                    break;
                case 'w':
                    if ((int_opt = std::stoi(optarg)) < 0) {
                        std::cerr << "Please enter a valid number for work_group_size value" << std::endl;
                        exit(1);
                    }
                    lws = int_opt;
                    break;
                case 's':
                    if ((int_opt = std::stoi(optarg)) < 0) {
                        std::cerr << "Please enter a valid number for stride value" << std::endl;
                        exit(1);
                    }
                    stride = int_opt;
                    break;
                case 'o':
                    optimize = true;
                    break;
                case 'v':
                    vtk_path = std::string(optarg);
                    if (vtk_path.empty()) {
                        vtk_path = RESULTS_FOLDER;
                        std::cout << "VTI files will be stored in:" << vtk_path << std::endl;
                    }
                    store_vtk = true;
                    break;
                case 'p':
                    dump_path  = std::string(optarg);
                    if (dump_path.empty()) {
                        dump_path = RESULTS_FOLDER;
                        std::cout << "dump files will be stored in: " << dump_path << std::endl;
                    }
                    break;
                case 'm':
                    dump_map = true;
                    break;
                case 'f':
                    dump_f = true;
                    break;
                case 'h':
                case '?':
                default:
                    print_help();
                    break;
            }
        }

        if (lws > dim) {
            lws = dim;
            std::cout << "work_group_size is set to " << dim << std::endl;
        }

        if (!is_power_of_two(stride)) {
            stride = most_significant_bit(stride);
            std::cout << "stride is rounded to the previous power of 2 that is " << stride << std::endl;
        }

    }


    void print_values()
    {
        std::cout << std::boolalpha
                  << "platformID       = " << platformID             << "\n"
                  << "deviceID         = " << deviceID               << "\n"
                  << "dim              = " << dim                    << "\n"
                  << "viscosity        = " << viscosity              << "\n"
                  << "velocity         = " << velocity               << "\n"
                  << "Device Mem. (B)  = " << device_memory_size_b() << "\n"
                  << "Device Mem. (KB) = " << device_memory_size_k() << "\n"
                  << "Device Mem. (MB) = " << device_memory_size_m() << "\n"
                  << "iterations       = " << iterations             << "\n"
                  << "work_group_size  = " << lws                    << "\n"
                  << "stride           = " << stride                 << "\n"
                  << "optimize         = " << optimize               << "\n"
                  << "every            = " << every                  << "\n"
                  << "VTK PATH         = " << vtk_path               << "\n"
                  << "STORE VTI        = " << store_vtk              << "\n"
                  << "DUMP F           = " << dump_f                 << "\n"
                  << "DUMP MAP         = " << dump_map               << "\n";
    }
};