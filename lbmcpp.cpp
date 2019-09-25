#include <cstdio>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>

#include "constants.h"


#define FOR_ZYX                                 \
    for (size_t z = 0; z < DIM; ++z) {          \
        for (size_t y = 0; y < DIM; ++y) {      \
            for (size_t x = 0; x < DIM; ++x) {

#define END_ZYX \
            }   \
        }       \
    }

void print_map(int * map)
{
    std::cout << "*** MAP ***" << std::endl;
    for (size_t z = 0; z < DIM; ++z) {
        for (size_t y = 0; y < DIM; ++y) {
            for (size_t x = 0; x < DIM; ++x) {
                std::cout << std::setw(4) << map[IDxyz(x, y, z)] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void print_f(float * f)
{
    std::cout << "*** f_stream ***" << std::endl;
    for (size_t z = 0; z < DIM; ++z) {
        for (size_t y = 0; y < DIM; ++y) {
            for (size_t x = 0; x < DIM; ++x) {
                size_t index = IDxyz(x, y, z);
                for (size_t q = 0; q < Q; ++q){
                    std::cout << std::scientific << f[IDxyzw(index, q)] << " ";
                    // std::cout << std::setw(5) << f[IDxyzw(index, q)] << " ";
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}


static void store_data(float * rho, float * u, const float * f, const int * map, const int timestamp)
{

FOR_ZYX
    const int id = IDxyz(x, y, z);
    float _rho = 0.0f;
    float ux = 0.0f;
    float uy = 0.0f;
    float uz = 0.0f;

#undef  UNROLL_X
#define UNROLL_X(i)                         \
    {                                       \
        const float _f = f[IDxyzw(id, i)];  \
        _rho += _f;                         \
        ux += E##i##_X * _f;                \
        uy += E##i##_Y * _f;                \
        uz += E##i##_Z * _f;                \
    }
    UNROLL_19();

    rho[id] = _rho;
    u[id * 3 + 0] = ux;
    u[id * 3 + 1] = uy;
    u[id * 3 + 2] = uz;

    const int cell_type = map[id];
    if (cell_type & (LEFT | RIGHT | TOP | BOTTOM | FRONT | BACK | WALL)) {
        if (cell_type != BOUNDARY_MOVING) {
            rho[id] = NAN;
            u[id * 3 + 0] = NAN;
            u[id * 3 + 1] = NAN;
            u[id * 3 + 2] = NAN;
        }
    }

END_ZYX

    const size_t from = 1;
    const size_t to = DIM - 1;
    const size_t extent = to - from - 1;


    std::ofstream vtk;

    std::stringstream filenameBuilder;
    filenameBuilder << "/Volumes/RamDisk/lbmcl." << std::setw(2) << std::setfill('0') << timestamp << ".vti";

    vtk.open(filenameBuilder.str());

    vtk << "<?xml version=\"1.0\"?>\n" 
        << "<VTKFile type=\"ImageData\" version=\"0.1\" byte_order=\"LittleEndian\">\n" 
        << "  <ImageData WholeExtent=\"0 " << extent << " 0 " << extent << " 0 " << extent << "\" Origin=\"0 0 0\" Spacing=\"1 1 1\">\n"
        << "    <Piece Extent=\"0 " << extent << " 0 " << extent << " 0 " << extent << "\">\n"
        << "      <PointData Scalars=\"scalars\">\n"
        << "        <DataArray type=\"Float32\" Name=\"rho\" NumberOfComponents=\"1\" format=\"ascii\">\n";

    for (size_t z = from; z < to; ++z) {
        for (size_t y = from; y < to; ++y) {
            for (size_t x = from; x < to; ++x) {
                const float val = rho[IDxyz(x, y, z)];
                vtk << std::scientific << val << " ";
            }
            vtk << "\n";
        }
        vtk << "\n";
    }
    
    vtk << "        </DataArray>\n"
        << "        <DataArray type=\"Float32\" Name=\"v\" NumberOfComponents=\"3\" format=\"ascii\">\n";

    for (size_t z = from; z < (to); ++z) {
        for (size_t y = from; y < (to); ++y) {
            for (size_t x = from; x < (to); ++x) {
                const float val0 = u[IDxyz(x, y, z) * 3 + 0];
                const float val1 = u[IDxyz(x, y, z) * 3 + 1];
                const float val2 = u[IDxyz(x, y, z) * 3 + 2];
                vtk << std::scientific << val0 << " "
                    << std::scientific << val1 << " "
                    << std::scientific << val2 << " ";
            }
            vtk << "\n";
        }
        vtk << "\n";
    }
    
    vtk << "        </DataArray>\n"
        << "      </PointData>\n"
        << "    </Piece>\n"
        << "  </ImageData>\n"
        << "</VTKFile>\n";
    vtk.close();
}


void init(float * f_stream, float * f_collide, int * map)
{
FOR_ZYX
    const int id = IDxyz(x, y, z);

    int cell_type = NONE;
    if (x == 1)           cell_type |= LEFT;
    if (x == (DIM_X - 2)) cell_type |= RIGHT;
    if (y == 1)           cell_type |= BOTTOM;
    if (y == (DIM_Y - 2)) cell_type |= TOP;
    if (z == 1)           cell_type |= BACK;
    if (z == (DIM_Z - 2)) cell_type |= FRONT;

    if (x == 0)           cell_type = WALL;
    if (x == (DIM_X - 1)) cell_type = WALL;
    if (y == 0)           cell_type = WALL;
    if (y == (DIM_Y - 1)) cell_type = WALL;
    if (z == 0)           cell_type = WALL;
    if (z == (DIM_Z - 1)) cell_type = WALL;

    if (cell_type == NONE) cell_type = FLUID;
    map[id] = cell_type;

    const float rho = CELL_INITIAL_DENSITY;
    const float ux = (cell_type == BOUNDARY_MOVING ? CELL_INITIAL_VELOCITY : 0.0f);
    const float uy = 0.0f;
    const float uz = 0.0f;

    const float u2 = (ux * ux) + (uy * uy) + (uz * uz);

#undef  UNROLL_X
#define UNROLL_X(i)                                                                                                          \
    {                                                                                                                        \
        const float eu = (ux * E##i##_X) + (uy * E##i##_Y) + (uz * E##i##_Z);                                                \
        const float f = (rho * OMEGA_##i) * (1.0f + (3.0f * C2_INV * eu) + (4.5f * C4_INV * eu * eu) - (1.5f * C2_INV * u2));\
        const int index = IDxyzw(id, i);                                                                                     \
        f_collide[index] = f;                                                                                                \
        f_stream[index] = f;                                                                                                 \
    }
    UNROLL_19();

END_ZYX
}

inline int is_boundary(const int cell_type)
{
    return (cell_type & (LEFT | RIGHT | TOP | BOTTOM | FRONT | BACK));
}

inline int is_fluid(const int cell_type)
{
    return (cell_type == FLUID);
}

inline int is_wet(const int cell_type)
{
    return (cell_type & FLUID);
}

inline int is_moving(const int cell_type)
{
    return (cell_type & BOUNDARY_MOVING);
}

void boundary_condition(float * f_collide, const int * map)
{
FOR_ZYX
    const int id = IDxyz(x, y, z);
    const int cell_type = map[id];

    if (is_fluid(cell_type)) continue;

    if (cell_type == BOUNDARY_MOVING) {
        f_collide[IDxyzw(id,  6)] = f_collide[IDxyzw(id,  S_6)];
        f_collide[IDxyzw(id, 15)] = f_collide[IDxyzw(id, S_15)];
        f_collide[IDxyzw(id, 16)] = f_collide[IDxyzw(id, S_16)];
        f_collide[IDxyzw(id, 17)] = f_collide[IDxyzw(id, S_17)];
        f_collide[IDxyzw(id, 18)] = f_collide[IDxyzw(id, S_18)];

        float rho = 0.0f;
#undef  UNROLL_X
#define UNROLL_X(i)                         \
        {                                   \
            rho += f_collide[IDxyzw(id, i)];\
        }
        UNROLL_19();

        const float ux = CELL_INITIAL_VELOCITY;
        const float uy = 0.0f;
        const float uz = 0.0f;

        const float u2 = (ux * ux) + (uy * uy) + (uz * uz);

#undef  UNROLL_X
#define UNROLL_X(i)                                                                                                                         \
        {                                                                                                                                   \
            const float eu = (ux * E##i##_X) + (uy * E##i##_Y) + (uz * E##i##_Z);                                                           \
            f_collide[IDxyzw(id, i)] = (rho * OMEGA_##i) * (1.0f + (3.0f * C2_INV * eu) + (4.5f * C4_INV * eu * eu) - (1.5f * C2_INV * u2));\
        }
        UNROLL_19();

    } else {

#undef  UNROLL_X
#define UNROLL_X(i)                                                 \
        {                                                           \
            const float temp = f_collide[IDxyzw(id, S_##i)];        \
            f_collide[IDxyzw(id, S_##i)] = f_collide[IDxyzw(id, i)];\
            f_collide[IDxyzw(id, i)] = temp;                        \
        }
        UNROLL_HALF_19();
    }
END_ZYX
}

inline float compute_bgk(const float f, const float f_eq)
{
    return f - (f - f_eq) * INV_TAU;
}

void collision(float * f_collide, const int * map)
{
FOR_ZYX
    const int id = IDxyz(x, y, z);
    const int cell_type = map[id];

    if (cell_type & (BOUNDARY_MOVING | FLUID)) {

        float rho = 0.0f;
        float ux = 0.0f;
        float uy = 0.0f;
        float uz = 0.0f;

#undef  UNROLL_X
#define UNROLL_X(i)                                  \
        {                                            \
            const float f = f_collide[IDxyzw(id, i)];\
            rho += f;                                \
            ux += E##i##_X * f;                      \
            uy += E##i##_Y * f;                      \
            uz += E##i##_Z * f;                      \
        }
        UNROLL_19();

        ux = ux / rho;
        uy = uy / rho;
        uz = uz / rho;

        if (is_moving(cell_type)) {
            ux = CELL_INITIAL_VELOCITY;
            uy = 0.0f;
            uz = 0.0f;
        }

        const float u2 = (ux * ux) + (uy * uy) + (uz * uz);

#undef  UNROLL_X
#define UNROLL_X(i)                                                                                                                 \
        {                                                                                                                           \
            const float eu = (ux * E##i##_X) + (uy * E##i##_Y) + (uz * E##i##_Z);                                                   \
            const float f_eq = (rho * OMEGA_##i) * (1.0f + (3.0f * C2_INV * eu) + (4.5f * C4_INV * eu * eu) - (1.5f * C2_INV * u2));\
            const int index = IDxyzw(id, i);                                                                                        \
            f_collide[index] = compute_bgk(f_collide[index], f_eq);                                                                 \
        }
        UNROLL_19();
    }
END_ZYX
}


void streaming(float * f_stream, const float * f_collide, const int * map)
{
FOR_ZYX
    const int id = IDxyz(x, y, z);
    const int cell_type = map[id];

    if (cell_type == WALL) continue;

#undef  UNROLL_X
#define UNROLL_X(i)                                                       \
    {                                                                     \
        const int index = IDxyz(x - E##i##_X, y + E##i##_Y, z - E##i##_Z);\
        f_stream[IDxyzw(id, i)] = f_collide[IDxyzw(index, i)];            \
    }
    UNROLL_19();
    
END_ZYX
}



int main(int argc, char * argv[])
{
    argc--;
    argv++;

    int argi = 0;
    int iterations = 10;
    int printsEvery = 1;

    if (argc > 0) iterations  = strtol(argv[argi++], NULL, 10);
    if (argc > 1) printsEvery = strtol(argv[argi++], NULL, 10);

    int timestamp = 0;
    float * rho = new float[DIM * DIM * DIM];
    float * u = new float[DIM * DIM * DIM * 3];
    float * f_stream = new float[DIM * DIM * DIM * Q];
    float * f_collide = new float[DIM * DIM * DIM * Q];
    int * map = new int[DIM * DIM * DIM];


    init(f_stream, f_collide, map);
    store_data(rho, u, f_collide, map, timestamp);
    timestamp++;


    for (size_t i = 0; i < iterations; ++i) {
        boundary_condition(f_collide, map);
        collision(f_collide, map);
        streaming(f_stream, f_collide, map);

        if (timestamp % printsEvery == 0) store_data(rho, u, f_collide, map, timestamp);
        timestamp++;

        // SWAP
        float * tmp = f_stream;
        f_stream = f_collide;
        f_collide = tmp;

    }

    delete[] f_stream;
    delete[] f_collide;
    delete[] map;

    return 0;
}
