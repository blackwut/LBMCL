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

    cl::Kernel initialize_kernel;
    cl::Kernel compute_kernel;
    cl::Kernel compute_swap_kernel;

    cl::Buffer f_stream;
    cl::Buffer f_collide;
    cl::Buffer rho;
    cl::Buffer u;
    cl::Buffer map;

    int * map_values = nullptr;
    T * f_values = nullptr;
    T * rho_values = nullptr;
    T * u_values = nullptr;

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
                    const T val_x = u_values[IDux(id)];
                    const T val_y = u_values[IDuy(id)];
                    const T val_z = u_values[IDuz(id)];
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


    void processData(const cl::Kernel & compute)
    {
        cl::Event compute_evt;

        CLUCheckErrorExit(
            queue.enqueueNDRangeKernel(compute, cl::NullRange, gws, lws, nullptr, &compute_evt),
            "compute"
        );
        events.emplace_back("compute", compute_evt);
    }


public:
    LBMCL(size_t dim,
          T viscosity,
          T velocity,
          size_t iterations,
          size_t every,
          std::string vtk_path = "",
          size_t lws = 32,
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
          lws(lws, 1, 1),
          gws(dim, dim, dim),
          stride(stride),
          optimize(optimize),
          dump_path(dump_path),
          dump_map(dump_map),
          dump_f(dump_f)
    {
        dump_data = (every != 0);
    }

    // Create all objects needed to perform the simulation, excluding host
    // buffers.
    // To initialize the simulation see initialize() function.
    void setupDevice(int platformID, int deviceID, bool print_options = false)
    {
        CLUSelectPlatform(platform, platformID);
        CLUSelectDevice(device, platform, deviceID);
        CLUCreateContext(context, device);
        CLUCreateQueue(queue, context, device);

        std::stringstream optionsBuilder;
        optionsBuilder << "-Werror ";
        optionsBuilder << "-I. ";
        optionsBuilder << "-DDIM=" << dim << " ";
        optionsBuilder << "-DLWS=" << lws[0] << " ";
        optionsBuilder << "-DSTRIDE=" << stride << " ";
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

        if (print_options) {
           std::cout << "Kernels options: " << optionsBuilder.str() << std::endl;
        }

        CLUBuildProgram(program, context, device, "kernels.cl", optionsBuilder.str());

        cl_int err;

        // Kernels
        initialize_kernel = cl::Kernel(program, "initialize", &err);
        CLUCheckErrorExit(err, "cl::Kernel(initialize)");

        compute_kernel = cl::Kernel(program, "compute", &err);
        CLUCheckErrorExit(err, "cl::Kernel(compute)");

        compute_swap_kernel = cl::Kernel(program, "compute", &err);
        CLUCheckErrorExit(err, "cl::Kernel(compute_swap)");

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

        try {
            // Set arguments to initialize kernel
            initialize_kernel.setArg(0, f_stream);
            initialize_kernel.setArg(1, f_collide);
            initialize_kernel.setArg(2, rho);
            initialize_kernel.setArg(3, u);
            initialize_kernel.setArg(4, map);
            // Set arguments to compute kernel
            compute_kernel.setArg(0, f_collide);
            compute_kernel.setArg(1, rho);
            compute_kernel.setArg(2, u);
            compute_kernel.setArg(3, map);
            compute_kernel.setArg(4, f_stream);
            // Set arguments to compute kernel
            compute_swap_kernel.setArg(0, f_stream);
            compute_swap_kernel.setArg(1, rho);
            compute_swap_kernel.setArg(2, u);
            compute_swap_kernel.setArg(3, map);
            compute_swap_kernel.setArg(4, f_collide);
        } catch (cl::Error err) {
            CLUErrorPrintExit(err);
        }
    }

    // Initialize host and device buffers
    void initialize()
    {
        // Call initialize kernel to setup the simulation
        cl::Event init_evt;
        CLUCheckErrorExit(
            queue.enqueueNDRangeKernel(initialize_kernel, cl::NullRange, gws, lws, nullptr, &init_evt),
            "initialize"
        );
        events.emplace_back("initialize", init_evt);

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
        if (dump_map) storeMap();
        if (dump_data) storeData(0);
        if (dump_f) storeF(f_collide, 0);

        for (size_t iteration = 1; iteration <= iterations; ++iteration) {
            const bool is_swap = (iteration % 2 == 0);

            processData((is_swap ? compute_swap_kernel : compute_kernel));

            if (dump_data && (iteration != 0) && (iteration % every == 0)) {
                storeData(iteration);
            }

            if (dump_f) {
                storeF((is_swap ? f_stream : f_collide), iteration);
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
        for (std::pair<std::string, cl::Event> & p : events) {
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

        for (std::pair<std::string, cl::Event> & p : events) {
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


    std::string statistics(char separator)
    {
        const std::string prec = (std::is_same<T, float>::value ? "single" : "double");
        const std::string dev_name = device.getInfo<CL_DEVICE_NAME>();

        std::stringstream stat;
        stat << dev_name.c_str() << separator
             << prec             << separator
             << dim              << separator
             << iterations       << separator
             << every            << separator
             << lws[0]           << separator
             << stride           << separator
             << optimize         << separator
             << totalTimeMS()    << separator
             << kernelsTimeMS()  << separator
             << MLUPS()          << separator
             << kernelsMLUPS()   << "\n";
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
