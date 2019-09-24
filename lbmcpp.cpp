#include <cstdio>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>


#define DIM 8

#define DIM_X    DIM
#define DIM_Y    DIM
#define DIM_Z    DIM

#define UNROLL_19() \
    UNROLL_X(0)     \
    UNROLL_X(1)     \
    UNROLL_X(2)     \
    UNROLL_X(3)     \
    UNROLL_X(4)     \
    UNROLL_X(5)     \
    UNROLL_X(6)     \
    UNROLL_X(7)     \
    UNROLL_X(8)     \
    UNROLL_X(9)     \
    UNROLL_X(10)    \
    UNROLL_X(11)    \
    UNROLL_X(12)    \
    UNROLL_X(13)    \
    UNROLL_X(14)    \
    UNROLL_X(15)    \
    UNROLL_X(16)    \
    UNROLL_X(17)    \
    UNROLL_X(18)

#define UNROLL_HALF_19() \
    UNROLL_X(1)          \
    UNROLL_X(2)          \
    UNROLL_X(3)          \
    UNROLL_X(4)          \
    UNROLL_X(5)          \
    UNROLL_X(6)          \
    UNROLL_X(7)          \
    UNROLL_X(8)          \
    UNROLL_X(9)          \

#define D   3
#define Q   19

#define CELL_INITIAL_DENSITY    1.0f
#define CELL_INITIAL_VELOCITY   0.05f

#define DELTA_X 1.0f
#define DELTA_T 1.0f
#define C2_INV (DELTA_T / DELTA_X)
#define C4_INV (C2_INV * C2_INV)

#define IDxyz(x, y, z)      ((x) + ((y) * DIM_X) + ((z) * DIM_X * DIM_Y))
#define IDxyzw(id, w)       ((id) * Q + (w))

#define VISCOSITY   0.0089f
#define TAU         ((3.0f * VISCOSITY) / (DELTA_T * DELTA_X * DELTA_X) + 0.5f)
#define INV_TAU     (1.0f / TAU)


#define E0_X    0
#define E0_Y    0
#define E0_Z    0

#define E1_X    1
#define E1_Y    0
#define E1_Z    0

#define E2_X    0
#define E2_Y    1
#define E2_Z    0

#define E3_X    -1
#define E3_Y    0
#define E3_Z    0

#define E4_X    0
#define E4_Y    -1
#define E4_Z    0

#define E5_X    0
#define E5_Y    0
#define E5_Z    -1

#define E6_X    0
#define E6_Y    0
#define E6_Z    1

#define E7_X    1
#define E7_Y    1
#define E7_Z    0

#define E8_X    -1
#define E8_Y    1
#define E8_Z    0

#define E9_X    -1
#define E9_Y    -1
#define E9_Z    0

#define E10_X   1
#define E10_Y   -1
#define E10_Z   0

#define E11_X   1
#define E11_Y   0
#define E11_Z   -1

#define E12_X   0
#define E12_Y   1
#define E12_Z   -1

#define E13_X   -1
#define E13_Y   0
#define E13_Z   -1

#define E14_X   0
#define E14_Y   -1
#define E14_Z   -1

#define E15_X   1
#define E15_Y   0
#define E15_Z   1

#define E16_X   0
#define E16_Y   1
#define E16_Z   1

#define E17_X   -1
#define E17_Y   0
#define E17_Z   1

#define E18_X   0
#define E18_Y   -1
#define E18_Z   1

#define OMEGA_0     (1.0f /  3.0f)
#define OMEGA_1     (1.0f / 18.0f)
#define OMEGA_2     (1.0f / 18.0f)
#define OMEGA_3     (1.0f / 18.0f)
#define OMEGA_4     (1.0f / 18.0f)
#define OMEGA_5     (1.0f / 18.0f)
#define OMEGA_6     (1.0f / 18.0f)
#define OMEGA_7     (1.0f / 36.0f)
#define OMEGA_8     (1.0f / 36.0f)
#define OMEGA_9     (1.0f / 36.0f)
#define OMEGA_10    (1.0f / 36.0f)
#define OMEGA_11    (1.0f / 36.0f)
#define OMEGA_12    (1.0f / 36.0f)
#define OMEGA_13    (1.0f / 36.0f)
#define OMEGA_14    (1.0f / 36.0f)
#define OMEGA_15    (1.0f / 36.0f)
#define OMEGA_16    (1.0f / 36.0f)
#define OMEGA_17    (1.0f / 36.0f)
#define OMEGA_18    (1.0f / 36.0f)

#define S_0          0
#define S_1          3
#define S_2          4
#define S_3          1
#define S_4          2
#define S_5          6
#define S_6          5
#define S_7          9
#define S_8         10
#define S_9          7
#define S_10         8
#define S_11        17
#define S_12        18
#define S_13        15
#define S_14        16
#define S_15        13
#define S_16        14
#define S_17        11
#define S_18        12


#define NONE                (0)
#define FLUID               (1 <<  0)
#define WALL                (1 <<  1)
#define LEFT                (1 <<  2)
#define RIGHT               (1 <<  3)
#define TOP                 (1 <<  4)
#define BOTTOM              (1 <<  5)
#define FRONT               (1 <<  6)
#define BACK                (1 <<  7)
#define LEFT_TOP            (LEFT  | TOP)
#define LEFT_BOTTOM         (LEFT  | BOTTOM)
#define LEFT_BACK           (LEFT  | BACK)
#define LEFT_FRONT          (LEFT  | FRONT)
#define LEFT_FRONT_TOP      (LEFT  | FRONT  | TOP)
#define LEFT_FRONT_BOTTOM   (LEFT  | FRONT  | BOTTOM)
#define LEFT_BACK_TOP       (LEFT  | BACK   | TOP)
#define LEFT_BACK_BOTTOM    (LEFT  | BACK   | BOTTOM)
#define RIGHT_TOP           (RIGHT | TOP)
#define RIGHT_BOTTOM        (RIGHT | BOTTOM)
#define RIGHT_BACK          (RIGHT | BACK)
#define RIGHT_FRONT         (RIGHT | FRONT)
#define RIGHT_FRONT_TOP     (RIGHT | FRONT  | TOP)
#define RIGHT_FRONT_BOTTOM  (RIGHT | FRONT  | BOTTOM)
#define RIGHT_BACK_TOP      (RIGHT | BACK   | TOP)
#define RIGHT_BACK_BOTTOM   (RIGHT | BACK   | BOTTOM)
#define FRONT_TOP           (FRONT | TOP)
#define FRONT_BOTTOM        (FRONT | BOTTOM)
#define BACK_TOP            (BACK  | TOP)
#define BACK_BOTTOM         (BACK  | BOTTOM)

#define BOUNDARY_MOVING   FRONT


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


    if (cell_type == WALL) {
#undef  UNROLL_X
#define UNROLL_X(i)                         \
        {                                   \
            f_stream[IDxyzw(id, i)] = NAN;  \
            f_collide[IDxyzw(id, i)] = NAN; \
        }
        UNROLL_19();

    } else {

        const float rho = CELL_INITIAL_DENSITY;
        const float ux = (cell_type == BOUNDARY_MOVING ? CELL_INITIAL_VELOCITY : 0.0f);
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

#undef  UNROLL_X
#define UNROLL_X(i)                         \
        {                                   \
            f_stream[IDxyzw(id, i)] = NAN; \
        }
        UNROLL_19();
    }

END_ZYX
}

inline int is_boundary(const int cell_type)
{
    return (cell_type & (LEFT | RIGHT | TOP | BOTTOM | FRONT | BACK));
}

void boundary_condition(float * f_collide, const int * map)
{
FOR_ZYX
    const int id = IDxyz(x, y, z);
    const int cell_type = map[id];

    if (!is_boundary(cell_type)) continue;

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

        if (cell_type == BOUNDARY_MOVING) {
            rho = 1.0f;
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


void init_fake(float * f_stream)
{
FOR_ZYX
    const int id = IDxyz(x, y, z);
#undef  UNROLL_X
#define UNROLL_X(i)                             \
        {                                       \
            const int index = IDxyzw(id, i);    \
            f_stream[index] = id;            \
        }
        UNROLL_19();
END_ZYX
}

void streaming(float * f_stream, const float * f_collide, const int * map)
{
FOR_ZYX
    const int id = IDxyz(x, y, z);
    const int cell_type = map[id];

    if (cell_type != WALL) {
#undef  UNROLL_X
#define UNROLL_X(i)                                                           \
        {                                                                     \
            const int index = IDxyz(x - E##i##_X, y - E##i##_Y, z - E##i##_Z);\
            f_stream[IDxyzw(id, i)] = f_collide[IDxyzw(index, i)];            \
        }
        UNROLL_19();
    }
END_ZYX
}

void streaming_push(float * f_stream, const float * f_collide, const int * map)
{
FOR_ZYX
    const int id = IDxyz(x, y, z);
    const int cell_type = map[id];

    if (cell_type != WALL) {
#undef  UNROLL_X
#define UNROLL_X(i)                                                           \
        {                                                                     \
            const int index = IDxyz(x + E##i##_X, y + E##i##_Y, z + E##i##_Z);\
            f_stream[IDxyzw(index, i)] = f_collide[IDxyzw(id, i)];            \
        }
        UNROLL_19();
    }
END_ZYX
}


int main(int argc, char * argv[])
{
    argc--;
    argv++;

    int argi = 0;
    int iterations = 10;
    if (argc > 0) iterations  = strtol(argv[argi++], NULL, 10);

    int timestamp = 0;
    float * rho = new float[DIM * DIM * DIM];
    float * u = new float[DIM * DIM * DIM * 3];
    float * f_stream = new float[DIM * DIM * DIM * Q];
    float * f_collide = new float[DIM * DIM * DIM * Q];
    int * map = new int[DIM * DIM * DIM];


    init(f_stream, f_collide, map);
    store_data(rho, u, f_collide, map, timestamp++);

    // init_fake(f_stream);
    // init_fake(f_collide);
    // // print_f(f_stream);
    // streaming_push(f_stream, f_collide, map);
    // print_f(f_stream);
    // print_map(map);
    // print_f(f_collide);

    for (size_t i = 0; i < 10; ++i) {
        boundary_condition(f_collide, map);
        collision(f_collide, map);
        streaming(f_stream, f_collide, map);

        // SWAP
        float * tmp = f_stream;
        f_stream = f_collide;
        f_collide = tmp;

        store_data(rho, u, f_stream, map, timestamp++);
    }

    delete[] f_stream;
    delete[] f_collide;
    delete[] map;

    return 0;
}
