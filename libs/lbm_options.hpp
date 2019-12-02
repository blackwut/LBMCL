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
    double viscosity;
    double velocity;
    size_t iterations;
    size_t every;
    std::string vtk_path;
    size_t lwx;
    size_t lwy;
    size_t lwz;
    size_t stride;
    bool use_double;
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
        lwx(8),
        lwy(8),
        lwz(8),
        stride(32),
        use_double(false),
        optimize(false),
        dump_path(RESULTS_FOLDER),
        dump_map(false),
        dump_f(false)
    {}

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
                     "-F  --use_double          Make use of \"double\" type                    \n"
                     "-o  --optimize            Use \"cl-fast-relaxed-math\" in OpenCL kernels \n"
                     "-v  --vtk_path            Specify where store VTI files                  \n"
                     "-p  --dump_path           Specify where store dumps                      \n"
                     "-m  --dump_map            Dump the lattice map                           \n"
                     "-f  --dump_f              Dump the lattice \"f\" for each iteration      \n"
                     "-h  --help                Show this help message and exit                \n";
        exit(1);
    }


    void process_args(int argc, char * argv[])
    {
        opterr = 0;

        const char * const short_opts = "P:D:d:n:u:i:e:v:w:s:Fop:mfh";
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
                {"use_double",      no_argument,       nullptr, 'F'},
                {"optimize",        no_argument,       nullptr, 'o'},
                {"vtk_path",        optional_argument, nullptr, 'v'},
                {"dump_path",       optional_argument, nullptr, 'p'},
                {"dump_map",        no_argument,       nullptr, 'm'},
                {"dump_f",          no_argument,       nullptr, 'f'},
                {"help",            no_argument,       nullptr, 'h'},
                {nullptr,           no_argument,       nullptr,   0}
        };

        int int_opt = -1;
        double real_opt = std::numeric_limits<double>::quiet_NaN();

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
                    sscanf(optarg, "%zu,%zu,%zu", &lwx, &lwy, &lwz);
                    // if ((int_opt = std::stoi(optarg)) < 0) {
                    //     std::cerr << "Please enter a valid number for work_group_size value" << std::endl;
                    //     exit(1);
                    // }
                    // lws = int_opt;
                    break;
                case 's':
                    if ((int_opt = std::stoi(optarg)) < 0) {
                        std::cerr << "Please enter a valid number for stride value" << std::endl;
                        exit(1);
                    }
                    stride = int_opt;
                    break;
                case 'F':
                    use_double = true;
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
    }
};
