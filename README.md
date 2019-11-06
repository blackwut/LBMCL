# LBMCL
Lattice Boltzmann Method 3D (LBM D3Q19) computing Lid Driven Cavity Problem (LDC) written in OpenCL 1.2 with C++ bindings.

## Dependencies
- A compiler compatible with C++11 
- OpenCL 1.2
- Python 3 (numpy, scipy, scikit-learn, vtk, pyvista)

## Python Setup
To verify data produced, Python 3 and some packages are required. It is recommended to use a Python Virtualenv.
```bash
# Install virtualenv
pip3 install virtualenv

# Create an environment
cd /PATH_TO_LBMCL/LBMCL
virtualenv ./project

# Activate the environment
source ./project/bin/activate

# Install all requirements
pip install -r requirements.txt

# Deactivate the environment if needed
deactivate
```

## Makefile
There are some user defined parameters that can be modified:
```bash
DIM        ?= 8      # dimension of the cube
PRECISION	 ?= SINGLE # floating point precision: SINGLE or DOUBLE
ITERATIONS ?= 10     # number of iterations
EVERY		   ?= 1      # number of iterations to skip between to output
LWS			   ?= 8      # work group size to use for kernel execution
STRIDE		 ?= 8      # stride used in the CSoA memory layout
```

The makefile provides some targets to compile and test the simulation:
```bash
# Compile
make

# Run a simple simulation testing all the option flags
make test

# Run 10 iterations of a 8x8x8 simulation with 0.0089 viscosity and 0.05 velocity, then verify data
make test8

# Run 10 iterations of a 32x32x32 simulation with 0.0089 viscosity and 0.05 velocity, then verify data
make test32
```

## Usage
```bash
./lbmcl --help
-P  --platform            Use the specified platform                     
-D  --device              Use the specified device                       
-d  --dim                 Set the lattice cube dimension                 
-n  --viscosity           Set the fluid viscosity                        
-u  --velocity            Set the x velocity of the moving wall          
-i  --iterations          Specify the number of iterations               
-e  --every               Save simulation results every N iterations     
-w  --work_group_size     Specify the work group size of kernel launch   
-s  --stride              Specify the stride used in CSoA memory layout  
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