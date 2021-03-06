#include <iostream>

#include "common.h"
#include "lbm_options.hpp"
#include "lbmcl.hpp"

template <typename T>
void performSimulation(const lbm_options & opts)
{
    LBMCL<T> lbmcl(opts.dim,
                   opts.viscosity,
                   opts.velocity,
                   opts.iterations,
                   opts.every,
                   opts.vtk_path,
                   opts.lwx,
                   opts.lwy,
                   opts.lwz,
                   opts.stride,
                   opts.optimize,
                   opts.dump_path,
                   opts.dump_map,
                   opts.dump_f);

    lbmcl.setupSimulation(opts.platformID, opts.deviceID);
    lbmcl.printConfiguration();
    lbmcl.performSimulation();
    lbmcl.waitCompletion();

    std::cout << "   Total time: " << lbmcl.totalTimeMS()   << " ms"    << std::endl;
    std::cout << " Kernels time: " << lbmcl.kernelsTimeMS() << " ms"    << std::endl;
    std::cout << "  Total MLUPS: " << lbmcl.MLUPS()         << " MLUPS" << std::endl;
    std::cout << "Kernels MLUPS: " << lbmcl.kernelsMLUPS()  << " MLUPS" << std::endl;

    std::cerr << lbmcl.statistics(';');
}

int main(int argc, char * argv[])
{
    lbm_options opts;
    opts.process_args(argc, argv);
    // opts.print_values();

    if (opts.use_double) {
        performSimulation<double>(opts);
    } else {
        performSimulation<float>(opts);
    }

    return 0;
}
