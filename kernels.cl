#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable

#include "constants.h"


inline int is_boundary(int cell_type)
{
    return (cell_type & (LEFT | RIGHT | TOP | BOTTOM | FRONT | BACK));
}

__kernel
void init(__global float * f_stream, __global float * f_collide, __global int * type)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int z = get_global_id(2);
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

    // if (cell_type == LEFT_BACK_BOTTOM || cell_type == RIGHT_BACK_BOTTOM || cell_type == LEFT_BACK_TOP || cell_type == RIGHT_BACK_TOP)
    //     cell_type = WALL;

    if (cell_type == NONE) cell_type = FLUID;
    type[id] = cell_type;

    const float rho = CELL_INITIAL_DENSITY;
    const float ux = (cell_type == FRONT ? CELL_INITIAL_VELOCITY : 0.0f);
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

    // if (id == 0) {
    //     printf("*** TYPE ***\n");
    //     for (int z = 0; z < DIM_Z; ++z) {
    //         for (int y = 0; y < DIM_Y; ++y) {
    //             for (int x = 0; x < DIM_Z; ++x) {
    //                 const int _t = type[IDxyz(x, y, z)];
    //                 int val = 1;
                    
    //                 if (_t == WALL) val = 4;
    //                 if (_t == FRONT) val = 2;
    //                 if (_t & (LEFT | RIGHT | TOP | BOTTOM | BACK)) val = 3;
    //                 printf("%d ", val);
    //             }
    //             printf("\n");
    //         }
    //         printf("\n");
    //     }
    // }
}

__kernel
void boundaryConditions(__global float * f_collide, __global const int * type)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int z = get_global_id(2);
    const int id = IDxyz(x, y, z);
    const int cell_type = type[id];

    if (!is_boundary(cell_type)) return;

    if (cell_type == FRONT) {
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

#undef UNROLL_X
#define UNROLL_X(i)                                                 \
        {                                                           \
            const float temp = f_collide[IDxyzw(id, S_##i)];        \
            f_collide[IDxyzw(id, S_##i)] = f_collide[IDxyzw(id, i)];\
            f_collide[IDxyzw(id, i)] = temp;                        \
        }
        UNROLL_HALF_19();
    }
}


// Bhatnagar-Gross-Kroop approximation collision operator
inline float compute_bgk(const float f, const float f_eq)
{
    // return f - (f - f_eq) * INV_TAU;
    return f + (f_eq - f) * INV_TAU;
}


__kernel
void collision(__global float * f_collide, __global const int * type)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int z = get_global_id(2);
    const int id = IDxyz(x, y, z);
    const int cell_type = type[id];

    if (cell_type == FRONT || cell_type == FLUID) {

        float rho = 0.0f;
        float ux = 0.0f;
        float uy = 0.0f;
        float uz = 0.0f;

#undef  UNROLL_X
#define UNROLL_X(i)                                      \
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

        if (cell_type == FRONT) {
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
}


// push-out vs pull-in (thesis "Thesis_for_web_new.pdf" page 87 - "REAL-TIME SIMULATION OF INDOOR AIR FLOW USING THE LATTICE BOLTZMANN METHOD ON GRAPHICS PROCESSING UNIT")

__kernel
void streaming(__global float * f_stream, __global const float * f_collide, __global const int * type)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int z = get_global_id(2);
    const int id = IDxyz(x, y, z);

    const int cell_type = type[id];

    if (cell_type == WALL) return;

#undef  UNROLL_X
#define UNROLL_X(i)                                                       \
    {                                                                     \
        const int index = IDxyz(x - E##i##_X, y + E##i##_Y, z - E##i##_Z);\
        f_stream[IDxyzw(id, i)] = f_collide[IDxyzw(index, i)];            \
    }
    UNROLL_19();
}


__kernel
void aggregateData(__global float * rho, __global float * u, __global const float * f_collide, __global const int * map)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int z = get_global_id(2);
    const int id = IDxyz(x, y, z);
    const int cell_type = map[id];

    float ux = 0.0f;
    float uy = 0.0f;
    float uz = 0.0f;

    const float  f0 = f_collide[IDxyzw(id,  0)];
    const float  f1 = f_collide[IDxyzw(id,  1)];
    const float  f2 = f_collide[IDxyzw(id,  2)];
    const float  f3 = f_collide[IDxyzw(id,  3)];
    const float  f4 = f_collide[IDxyzw(id,  4)];
    const float  f5 = f_collide[IDxyzw(id,  5)];
    const float  f6 = f_collide[IDxyzw(id,  6)];
    const float  f7 = f_collide[IDxyzw(id,  7)];
    const float  f8 = f_collide[IDxyzw(id,  8)];
    const float  f9 = f_collide[IDxyzw(id,  9)];
    const float f10 = f_collide[IDxyzw(id, 10)];
    const float f11 = f_collide[IDxyzw(id, 11)];
    const float f12 = f_collide[IDxyzw(id, 12)];
    const float f13 = f_collide[IDxyzw(id, 13)];
    const float f14 = f_collide[IDxyzw(id, 14)];
    const float f15 = f_collide[IDxyzw(id, 15)];
    const float f16 = f_collide[IDxyzw(id, 16)];
    const float f17 = f_collide[IDxyzw(id, 17)];
    const float f18 = f_collide[IDxyzw(id, 18)];

    const float rho_new = f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10 + f11 + f12 + f13 + f14 + f15 + f16 + f17 + f18;

#undef  UNROLL_X
#define UNROLL_X(i)                              \
    {                                            \
        ux += E##i##_X * f##i;                   \
        uy += E##i##_Y * f##i;                   \
        uz += E##i##_Z * f##i;                   \
    }
    UNROLL_19();

    if (cell_type == FLUID || cell_type == FRONT) {
        rho[id] = rho_new;
        u[id * 3 + 0] = ux / rho_new;
        u[id * 3 + 1] = uy / rho_new;
        u[id * 3 + 2] = uz / rho_new;
    } else {
        rho[id] = NAN;
        u[id * 3 + 0] = NAN;
        u[id * 3 + 1] = NAN;
        u[id * 3 + 2] = NAN;
    }
}
