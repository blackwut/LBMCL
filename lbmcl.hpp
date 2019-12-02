#pragma once

#include <string>
#include <vector>
#include <utility>
#include <sstream>
#include <type_traits>

#include "common.h"
#include "CLUtil.hpp"


#define DIGITS(val)         (((val) > 0) ? (size_t)log10((double)(val)) + 1 : 1)

#define DUMP_PRECISION      6
#define VTK_PRECISION       16


#define IDxyzqDIM(id, q, dim, stride)   (((id) / (stride)) * (dim) + q) * (stride) + ((id) & ((stride) - 1))
#define IDxyzDIM(x, y, z, dim)          ((x) + ((y) * (dim)) + ((z) * (dim) * (dim)))
#define IDuxDIM(id, dim)                (0 * dim * dim * dim + id)
#define IDuyDIM(id, dim)                (1 * dim * dim * dim + id)
#define IDuzDIM(id, dim)                (2 * dim * dim * dim + id)


template <typename T>
class LBMCL
{
    static_assert(std::is_same<T, float>::value || std::is_same<T, double>::value,
                  "Only float or double data type is valid.");
private:
    size_t dim;
    T viscosity;
    T velocity;
    size_t iterations;
    size_t every;
    std::string vtk_path;
    cl::NDRange lws;
    cl::NDRange gws;
    size_t stride;
    bool optimize;
    std::string dump_path;
    bool dump_map;
    bool dump_f;

    bool dump_data = false;

    cl::Platform platform;
    cl::Device device;
    cl::Context context;
    cl::CommandQueue queue;
    cl::Program program;

    cl::Buffer f_stream;
    cl::Buffer f_collide;
    cl::Buffer rho;
    cl::Buffer u;
    cl::Buffer map;

    int * map_values = nullptr;
    T * f_values = nullptr;
    T * rho_values = nullptr;
    T * u_values = nullptr;

    cl::Kernel initialize_kernel;
    std::vector<cl::Kernel> compute_kernels;
    std::vector< std::pair<std::string, cl::Event> > events;

    inline size_t f_dim()   const { return (dim * dim * dim * Q); }
    inline size_t u_dim()   const { return (dim * dim * dim * D); }
    inline size_t rho_dim() const { return (dim * dim * dim); }
    inline size_t map_dim() const { return (dim * dim * dim); }
    inline size_t wet_dim() const { return (dim - 2) * (dim - 2) * (dim - 2); }

    inline size_t f_size()   const { return f_dim()   * sizeof(T);  }
    inline size_t u_size()   const { return u_dim()   * sizeof(T);  }
    inline size_t rho_size() const { return rho_dim() * sizeof(T);  }
    inline size_t map_size() const { return map_dim() * sizeof(int);}

    inline size_t device_memory_size_b() const
    {
        return f_size() * 2 + u_size() + rho_size() + map_size();
    }


    inline size_t device_memory_size_k() const
    {
        return device_memory_size_b() / (1 << 10);
    }


    inline size_t device_memory_size_m() const
    {
        return device_memory_size_b() / (1 << 20);
    }


    inline bool is_power_of_two(size_t x) const
    {
        return x && !(x & (x - 1));
    }


    inline size_t log2i(size_t x) const
    {
       size_t n;
       for (n = 0; x > 1; x >>= 1, n++);
       return n;
    } 


    inline size_t previous_power_of_two(size_t x) const
    {
        x |= x >>  1;
        x |= x >>  2;
        x |= x >>  4;
        x |= x >>  8;
        x |= x >> 16;
        x |= x >> 32;
        x = x + 1;
        return (x >> 1);
    }

    std::string kernelOptionsStr()
    {
        std::stringstream optionsBuilder;
        optionsBuilder << "-Werror ";
        optionsBuilder << "-I. ";
        optionsBuilder << "-DDIM=" << dim << " ";
        optionsBuilder << "-DLWS=" << lws[0] << " ";
        optionsBuilder << "-DSTRIDE_DIV=" << log2i(stride) << " ";
        optionsBuilder << "-DSTRIDE_MOD=" << (stride - 1) << " ";
        optionsBuilder << "-DVISCOSITY=" << viscosity << " ";
        optionsBuilder << "-DVELOCITY=" << velocity << " ";


        if (std::is_same<T, float>::value) {
            optionsBuilder << "-DFP_SINGLE ";
            optionsBuilder << "-cl-single-precision-constant ";
        } else {
            optionsBuilder << "-DFP_DOUBLE ";
        }

        if (optimize) {
            optionsBuilder << "-cl-fast-relaxed-math ";
        }

        return optionsBuilder.str();
    }


    void storeMap()
    {
        // Read from Device
        cl::Event read_evt;
        CLUCheckErrorExit(
            queue.enqueueReadBuffer(map, CL_TRUE, 0, map_size(), map_values, nullptr, &read_evt),
            "read_map"
        );
        events.emplace_back("read_map", read_evt);

        // Store to file
        std::stringstream filenameBuilder;
        filenameBuilder << dump_path << "/map.dump";

        std::ofstream dump;
        dump.open(filenameBuilder.str());

        dump << "# FLUID       1" << std::endl
             << "# MOVING      2" << std::endl
             << "# BOUNDARY    3" << std::endl
             << "# WALL        4" << std::endl
             << "# CORNER      5" << std::endl
             << std::endl;

        for (size_t z = 0; z < dim; ++z) {
            for (size_t y = 0; y < dim; ++y) {
                for (size_t x = 0; x < dim; ++x) {
                    const size_t cell_type = map_values[IDxyzDIM(x, y, z, dim)];

                    int val = 0;
                    if (is_fluid(cell_type))    val = 1;
                    if (is_moving(cell_type))   val = 2;
                    if (is_boundary(cell_type)) val = 3;
                    if (is_wall(cell_type))     val = 4;
                    if (is_corner(cell_type))   val = 5;
                    dump << val << " ";
                }
                dump << std::endl;
            }
            dump << std::endl;
        }
        dump << std::endl;

        dump.close();
    }


    void storeF(const cl::Buffer & f, size_t iteration)
    {
        // Read from Device
        cl::Event read_evt;
        CLUCheckErrorExit(
            queue.enqueueReadBuffer(f, CL_TRUE, 0, f_size(), f_values, nullptr, &read_evt),
            "read_f"
        );
        events.emplace_back("read_f", read_evt);


        // Store to file
        std::stringstream filenameBuilder;
        filenameBuilder << dump_path << "/f_" << std::setw(DIGITS(iterations)) << std::setfill('0') << iteration << ".dump";

        // (xxx,yyy,zzz) 
        // 1 + D + 1 + D + 1 + D + 1 + 1
        const size_t dim_digits = DIGITS(dim);
        const size_t coord_spaces = dim_digits * 3 + 5;

        std::ofstream dump;
        dump.open(filenameBuilder.str());

        for (size_t z = 0; z < dim; ++z) {
            for (size_t y = 0; y < dim; ++y) {
                for (size_t s = 0; s < coord_spaces; ++s) {
                    dump << " ";
                }
                for (size_t q = 0; q < Q; ++q) {
                    dump << std::setw(DUMP_PRECISION + 2) << q << " ";
                }
                dump << std::endl;

                for (size_t x = 0; x < dim; ++x) {
                    const size_t index = IDxyzDIM(x, y, z, dim);
                    dump << std::setw(dim_digits) << "(" << x << "," << y << "," << z << ") ";
                    for (size_t q = 0; q < Q; ++q) {
                        dump << std::fixed 
                             << std::setw(DUMP_PRECISION + 2)
                             << std::setprecision(DUMP_PRECISION)
                             << f_values[IDxyzqDIM(index, q, Q, stride)]
                             << " ";
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


    void storeData(size_t iteration)
    {
        // Read from Device
        cl::Event read_rho_evt;
        cl::Event read_u_evt;
    
        CLUCheckErrorExit(
            queue.enqueueReadBuffer(rho, CL_TRUE, 0, rho_size(), rho_values, nullptr, &read_rho_evt),
            "read_rho"
        );
        events.emplace_back("read_rho", read_rho_evt);

        CLUCheckErrorExit(
            queue.enqueueReadBuffer(u, CL_TRUE, 0, u_size(), u_values, nullptr, &read_u_evt),
            "read_u"
        );
        events.emplace_back("read_u", read_u_evt);


        // Store to file
        std::stringstream filenameBuilder;
        filenameBuilder << vtk_path << "/lbmcl." << std::setw(DIGITS(iterations)) << std::setfill('0') << iteration << ".vti";

        const size_t from = 1;
        const size_t to = dim - 1;
        const size_t extent = to - from - 1;
        const std::string dataTypeString = (std::is_same<T, float>::value ? "Float32" : "Float64");

        std::ofstream vtk;
        vtk.open(filenameBuilder.str());

        vtk << "<?xml version=\"1.0\"?>\n" 
            << "<VTKFile type=\"ImageData\" version=\"0.1\" byte_order=\"LittleEndian\" header_type=\"UInt64\">\n" 
            << "  <ImageData WholeExtent=\"0 " << extent << " 0 " << extent << " 0 " << extent << "\" Origin=\"0 0 0\" Spacing=\"1 1 1\">\n"
            << "    <Piece Extent=\"0 " << extent << " 0 " << extent << " 0 " << extent << "\">\n"
            << "      <PointData Scalars=\"rho\">\n"
            << "        <DataArray type=\"" << dataTypeString << "\" Name=\"rho\" NumberOfComponents=\"1\" format=\"ascii\">\n";

        for (size_t z = from; z < to; ++z) {
            for (size_t y = from; y < to; ++y) {
                for (size_t x = from; x < to; ++x) {
                    const T val = rho_values[IDxyzDIM(x, y, z, dim)];
                    vtk << std::scientific << std::setprecision(VTK_PRECISION) << val << " ";
                }
                vtk << "\n";
            }
        }

        vtk << "        </DataArray>\n"
            << "        <DataArray type=\"" << dataTypeString << "\" Name=\"v\" NumberOfComponents=\"3\" format=\"ascii\">\n";

        for (size_t z = from; z < (to); ++z) {
            for (size_t y = from; y < (to); ++y) {
                for (size_t x = from; x < (to); ++x) {
                    const size_t id = IDxyzDIM(x, y, z, dim);
                    const T val_x = u_values[IDuxDIM(id, dim)];
                    const T val_y = u_values[IDuyDIM(id, dim)];
                    const T val_z = u_values[IDuzDIM(id, dim)];
                    vtk << std::scientific << std::setprecision(VTK_PRECISION) << val_x << " "
                        << std::scientific << std::setprecision(VTK_PRECISION) << val_y << " "
                        << std::scientific << std::setprecision(VTK_PRECISION) << val_z << " ";
                }
                vtk << "\n";
            }
        }

        vtk << "        </DataArray>\n"
            << "      </PointData>\n"
            << "    </Piece>\n"
            << "  </ImageData>\n"
            << "</VTKFile>\n";

        vtk.close();
    }


public:
    LBMCL(size_t dim,
          T viscosity,
          T velocity,
          size_t iterations,
          size_t every,
          std::string vtk_path = "",
          size_t lwx = 8,
          size_t lwy = 8,
          size_t lwz = 8,
          size_t stride = 32,
          bool optimize = true,
          std::string dump_path = "",
          bool dump_map = false,
          bool dump_f = false)
        : dim(dim),
          viscosity(viscosity),
          velocity(velocity),
          iterations(iterations),
          every(every),
          vtk_path(vtk_path),
          stride(stride),
          optimize(optimize),
          dump_path(dump_path),
          dump_map(dump_map),
          dump_f(dump_f)
    {
        dump_data = (every != 0);

        if (!is_power_of_two(dim)) {
            this->dim = previous_power_of_two(dim);
            std::cout << "dim is rounded to the previous power of 2: " << this->dim << std::endl;
        }

        this->gws = cl::NDRange(this->dim, this->dim, this->dim);

        // if (work_group_size > this->dim) {
        //     work_group_size = this->dim;
        //     std::cout << "work_group_size is set to " << work_group_size << std::endl;
        // }

        if (lwx == 0) lwx = 1;
        if (lwy == 0) lwy = 1;
        if (lwz == 0) lwz = 1;

        if ((lwx * lwy * lwz) > (this->dim * this->dim * this->dim)) {
            std::cerr << "Please enter a good work_group_size to run the simulation" << std::endl;
            exit(-1);
        }

        this->lws = cl::NDRange(lwx, lwy, lwz);

        if (!is_power_of_two(this->stride)) {
            this->stride = previous_power_of_two(this->stride);
            std::cout << "stride is rounded to the previous power of 2: " << this->stride << std::endl;
        }
    }


    // Create all objects needed to perform the simulation.
    void setupSimulation(int platformID, int deviceID)
    {
        CLUSelectPlatform(platform, platformID);
        CLUSelectDevice(device, platform, deviceID);
        CLUCreateContext(context, device);
        CLUCreateQueue(queue, context, device);

        CLUBuildProgram(program, context, device, "kernels.cl", kernelOptionsStr());

        cl_int err;

        // Buffers
        f_stream = cl::Buffer(context, CL_MEM_READ_WRITE | (dump_f ? 0 : CL_MEM_HOST_NO_ACCESS), f_size(), nullptr, &err);
        CLUCheckErrorExit(err, "cl::Buffer(f_stream)");

        f_collide = cl::Buffer(context, CL_MEM_READ_WRITE | (dump_f ? 0 : CL_MEM_HOST_NO_ACCESS), f_size(), nullptr, &err);
        CLUCheckErrorExit(err, "cl::Buffer(f_collide))");

        rho = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, rho_size(), nullptr, &err);
        CLUCheckErrorExit(err, "cl::Buffer(rho)");

        u = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, u_size(), nullptr, &err);
        CLUCheckErrorExit(err, "cl::Buffer(u)");

        map = cl::Buffer(context, CL_MEM_READ_WRITE | (dump_map ? 0 : CL_MEM_HOST_NO_ACCESS), map_size(), nullptr, &err);
        CLUCheckErrorExit(err, "cl::Buffer(map)");


        // Kernels
        initialize_kernel = cl::Kernel(program, "initialize", &err);
        CLUCheckErrorExit(err, "cl::Kernel(initialize)");

        // Set arguments to initialize kernel
        try {
            initialize_kernel.setArg(0, f_stream);
            initialize_kernel.setArg(1, f_collide);
            initialize_kernel.setArg(2, rho);
            initialize_kernel.setArg(3, u);
            initialize_kernel.setArg(4, map);
        } catch (cl::Error err) {
            CLUErrorPrintExit(err);
        }

        for (size_t iteration = 1; iteration <= iterations; ++iteration) {
            const int is_store_data = (dump_data && (iteration != 0) && (iteration % every == 0)) ? 1 : 0;
            const bool is_swap = (iteration % 2 == 0);

            cl::Kernel compute_kernel = cl::Kernel(program, "compute", &err);
            CLUCheckErrorExit(err, "cl::Kernel(compute)");

            // size_t wgs = compute_kernel.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(device);
            // std::cout << "CL_KERNEL_WORK_GROUP_SIZE: " << wgs << std::endl;

            // cl_ulong lms = compute_kernel.getWorkGroupInfo<CL_KERNEL_LOCAL_MEM_SIZE>(device);
            // std::cout << "CL_KERNEL_LOCAL_MEM_SIZE: " << lms << std::endl;

            // cl_ulong pms = compute_kernel.getWorkGroupInfo<CL_KERNEL_PRIVATE_MEM_SIZE>(device);
            // std::cout << "CL_KERNEL_PRIVATE_MEM_SIZE: " << pms << std::endl;

            // size_t pwgsm = compute_kernel.getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(device);
            // std::cout << "CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE: " << pwgsm << std::endl;

            // Set arguments to compute kernel
            try {
                compute_kernel.setArg(0, (is_swap ? f_collide : f_stream ));
                compute_kernel.setArg(1, (is_swap ? f_stream :  f_collide));
                compute_kernel.setArg(2, rho);
                compute_kernel.setArg(3, u);
                compute_kernel.setArg(4, map);
                compute_kernel.setArg(5, is_store_data);
            } catch (cl::Error err) {
                CLUErrorPrintExit(err);
            }

            compute_kernels.push_back(compute_kernel);
        }

        // Allocate memory for output and dumps if needed
        if (map_values == nullptr && dump_map) {
            map_values = new int[map_dim()];
        }

        if (f_values == nullptr && dump_f) {
            f_values = new T[f_dim()];
        }

        if (dump_data) {
            if (rho_values == nullptr) rho_values = new T[rho_dim()];
            if (u_values == nullptr) u_values = new T[u_dim()];
        }
    }


    // Enqueues all the kernel to perform the simulation and then returns.
    // This is a non-blocking function, so the simulation may not be completed
    // once this function returns.
    // See waitCompletion() or performSimulationAndWait() functions to await
    // the completion of the simulation.
    void performSimulation()
    {
        // Initialize the simulation
        cl::Event init_evt;
        CLUCheckErrorExit(
            queue.enqueueNDRangeKernel(initialize_kernel, cl::NullRange, gws, lws, nullptr, &init_evt),
            "initialize"
        );
        events.emplace_back("initialize", init_evt);

        // Dump data if needed
        if (dump_map) storeMap();
        if (dump_data) storeData(0);
        if (dump_f) storeF(f_collide, 0);

        for (size_t it = 1; it <= iterations; ++it) {
            cl::Event compute_evt;
            CLUCheckErrorExit(
                queue.enqueueNDRangeKernel(compute_kernels[it - 1], cl::NullRange, gws, lws, nullptr, &compute_evt),
                "compute"
            );
            events.emplace_back("compute", compute_evt);

            if (dump_data && (it != 0) && (it % every == 0)) {
                storeData(it);
            }

            if (dump_f) {
                storeF(((it % 2 == 0) ? f_stream : f_collide), it);
            }
        }
    }


    // Wait until the simulation completes.
    void waitCompletion()
    {
        try {
            queue.finish();
        } catch (cl::Error err) {
            CLUErrorPrintExit(err);
        }
    }


    // Enqueues all the kernel to perform the simulation, wait until the
    // simulation completes and then returns.
    // This is a blocking function. Once the simulation is completed it returns.
    void performSimulationAndWait()
    {
        performSimulation();
        waitCompletion();
    }


    // Awaits for the simulation completion and then return the time spent
    // (in milliseconds) by the simulation, including initialization,
    // computation, vtk files storing and dumping.
    double totalTimeMS()
    {
        waitCompletion();

        const cl::Event start_evt = events.front().second;
        const cl::Event end_evt = events.back().second;

        return CLUEventsGetTime(start_evt, end_evt);
    }


    // Awaits for the simulation completion and then return the time spent
    // (in milliseconds) by the simulation computation, including only the time
    // experienced by initialize and compute kernels.
    double kernelsTimeMS()
    {
        waitCompletion();

        double totalTime = 0.0;
        for (const std::pair<std::string, cl::Event> & p : events) {
            totalTime += CLUEventsGetTime(p.second, p.second);
        }

        return totalTime;
    }


    // Awaits for the simulation completion and then return a vector of pairs
    // containing the name of each kernel involved in the simulation and its
    // time spent in the computation.
    std::vector< std::pair<std::string, double> > kernelsTimingsMS()
    {
        waitCompletion();

        std::vector< std::pair<std::string, double> > timings;
        timings.reserve(events.size());

        for (const std::pair<std::string, cl::Event> & p : events) {
            const std::string name = p.first;
            const double time = CLUEventsGetTime(p.second, p.second);
            timings.emplace_back(name, time);
        }
        return timings;
    }


    // Awaits for the simulation completion and then return the performance in
    // Million Lattice Updates Per Second (MLUPS), taking into account the time
    // spent by both host and device for initialization, computation, vtk files
    // storing and dumping.
    //
    //   LUPS = wet_lattices * iterations / time     [lattice_updates / seconds]
    //  MLUPS = LUPS / 1e+6                  [million_lattice_updates / seconds]
    //
    double MLUPS()
    {
        return (wet_dim() * iterations) / (totalTimeMS() * 1000);
    }


    // Awaits for the simulation completion and then return the performance in
    // Million Lattice Updates Per Second (MLUPS), taking into account only the
    // time spent by the device for initialization and computation.
    //
    //   LUPS = wet_lattices * iterations / time     [lattice_updates / seconds]
    //  MLUPS = LUPS / 1e+6                  [million_lattice_updates / seconds]
    //
    double kernelsMLUPS()
    {
        return (wet_dim() * iterations) / (kernelsTimeMS() * 1000);
    }


    void printConfiguration()
    {
        const std::string prec = (std::is_same<T, float>::value ? "single" : "double");
        const std::string dev_name = device.getInfo<CL_DEVICE_NAME>();
        std::cout << std::boolalpha
                  << "kernel options   = " << kernelOptionsStr()                          << "\n"
                  << "device           = " << dev_name                                    << "\n"
                  << "dim              = " << dim                                         << "\n"
                  << "viscosity        = " << viscosity                                   << "\n"
                  << "velocity         = " << velocity                                    << "\n"
                  << "Device Mem. (B)  = " << device_memory_size_b()                      << "\n"
                  << "Device Mem. (KB) = " << device_memory_size_k()                      << "\n"
                  << "Device Mem. (MB) = " << device_memory_size_m()                      << "\n"
                  << "iterations       = " << iterations                                  << "\n"
                  << "work_group_size  = (" << lws[0] << ", " << lws[1] << ", " << lws[2] << ")\n"
                  << "stride           = " << stride                                      << "\n"
                  << "precision        = " << prec                                        << "\n"
                  << "optimize         = " << optimize                                    << "\n"
                  << "every            = " << every                                       << "\n"
                  << "VTK PATH         = " << vtk_path                                    << "\n"
                  << "DUMP F           = " << dump_f                                      << "\n"
                  << "DUMP MAP         = " << dump_map                                    << "\n";
    }


    std::string statistics(char separator)
    {
        const std::string prec = (std::is_same<T, float>::value ? "single" : "double");
        const std::string dev_name = device.getInfo<CL_DEVICE_NAME>();

        std::stringstream stat;
        stat << dev_name.c_str()                         << separator
             << prec                                     << separator
             << dim                                      << separator
             << iterations                               << separator
             << every                                    << separator
             << lws[0] << "," << lws[1] << "," << lws[2] << separator
             << stride                                   << separator
             << optimize                                 << separator
             << totalTimeMS()                            << separator
             << kernelsTimeMS()                          << separator
             << MLUPS()                                  << separator
             << kernelsMLUPS()                           << "\n";
        return stat.str();
    }


    ~LBMCL()
    {
        if (map_values != nullptr) delete[] map_values;
        if (f_values   != nullptr) delete[] f_values;
        if (rho_values != nullptr) delete[] rho_values;
        if (u_values   != nullptr) delete[] u_values;
    }
};
