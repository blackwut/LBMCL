# LBMCL
Lattice Boltzmann Method 3D (LBM D3Q19) computing Lid Driven Cavity Problem (LDC) written in OpenCL 1.2 with C++ bindings.

## Dependencies
- Compiler compatible with C++11 
- OpenCL 1.2
- Python >=3.7 (to verify data)
  - numpy
  - scipy
  - scikit-learn
  - vtk
  - pyvista

## Python Setup
To verify data produced, Python >=3.7 and some packages are required. It is recommended to use a Virtual  Environment as following:
```bash
# Create an environment
cd /PATH_TO_LBMCL/LBMCL
python3 -m venv project

# Activate the environment
source ./project/bin/activate

# Install all requirements
pip install -r requirements.txt

... use lbmcl ...

# Deactivate the environment if needed
deactivate
```

## Makefile
There are some parameters that can be modified to change the behaviour of all test targets:
```makefile
PLATFORM   = 0          # OpenCL Platform ID
DEVICE     = 0          # OpenCL Device ID
DIM        = 8          # dimension of the cube
VISCOSITY  = 0.0089     # viscosity of the fluid
VELOCITY   = 0.05       # velocity of the moving wall
ITERATIONS = 10         # number of iterations
EVERY      = 1          # number of iterations to skip between two outputs
LWS        = 8          # work group size to use for kernel execution
STRIDE     = 8          # stride used in the CSoA memory layout
PRECISION  = single     # floating point precision: single or double
OPTIMIZE   = true       # use OpenCL optimization flags
DUMP_PATH  = ./results  # path to the dumping folder
DUMP_MAP   = false      # dump the map of the simulation
DUMP_F     = false      # dump the f at each simulation step
```

The makefile provides some targets to compile and test the simulation:
```bash
# Compile
make

# Run a simple testing simulation
make test

# Run a simple simulation testing all the option flags
make testall

# Run 10 iterations of a 8x8x8 simulation with 0.0089 viscosity and 0.05 velocity, then verify data
make test8

# Run 10 iterations of a 32x32x32 simulation with 0.0089 viscosity and 0.05 velocity, then verify data
make test32
```

## LBMCL Usage
```wiki
./lbmcl --help
-P  --platform            Use the specified platform
-D  --device              Use the specified device
-d  --dim                 Set the lattice cube dimension
-n  --viscosity           Set the fluid viscosity
-u  --velocity            Set the x velocity of the moving wall
-i  --iterations          Specify the number of iterations
-e  --every               Save simulation results every N iterations     
-w  --work_group_size     Specify the work group size "x,y,z"
-s  --stride              Specify the stride used in CSoA memory layout
-F  --use_double          Make use of "double" type
-o  --optimize            Use "cl-fast-relaxed-math" in OpenCL kernels
-v  --vtk_path            Specify where store VTI files
-p  --dump_path           Specify where store dumps
-m  --dump_map            Dump the lattice map
-f  --dump_f              Dump the lattice "f" for each iteration
-h  --help                Show this help message and exit
```
For example, to run 10 iteration of a 8x8x8 simulation with 0.0089 viscosity and 0.05 velocity, storing a VTK file each iteration, you can execute:
```bash
./lbmcl -P0 -D0 -d8 -v0.0089 -u0.05 -i10 -e1
```