#include "constants.h"

#define PRIMITIVE_CAT(a, b) a ## b
#define CAT(a, b)           PRIMITIVE_CAT(a, b)
#define F                   f
#define S(i)                S_##i
#define F_S(i)              CAT(F, S(i))


#define UX(id) u[(id) * 3 + 0]
#define UY(id) u[(id) * 3 + 1]
#define UZ(id) u[(id) * 3 + 2]

#define COLLIDE_SCRATCH     (1 << 0)
#define COLLIDE_SAILFISH    (1 << 1)
#define COLLIDE_METHOD      COLLIDE_SCRATCH

#define STREAMING_PULL      (1 << 2)
#define STREAMING_PUSH      (1 << 3)
#define STREAMING_METHOD    STREAMING_PUSH


inline int get_cell_type(const int x, const int y, const int z)
{
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

    if (cell_type == (LEFT  | BACK | BOTTOM) ||
        cell_type == (RIGHT | BACK | BOTTOM) ||
        cell_type == (LEFT  | BACK | TOP   ) ||
        cell_type == (RIGHT | BACK | TOP   ))
    {
        cell_type = CORNER;
    }

    if (cell_type == NONE) cell_type = FLUID;

    return cell_type;
}


__kernel
void init(__global float * f_stream, __global float * f_collide, __global float * density, __global float * u, __global int * map)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int z = get_global_id(2);
    const int id = IDxyz(x, y, z);
    const int cell_type = get_cell_type(x, y, z);

    const float rho = CELL_INITIAL_DENSITY;
    const float ux  = (cell_type & MOVING_BOUNDARY ? CELL_INITIAL_VELOCITY_X : 0.0f);
    const float uy  = (cell_type & MOVING_BOUNDARY ? CELL_INITIAL_VELOCITY_Y : 0.0f);
    const float uz  = (cell_type & MOVING_BOUNDARY ? CELL_INITIAL_VELOCITY_Z : 0.0f);

#if (COLLIDE_METHOD == COLLIDE_SCRATCH)
    float eu = 0.0f;
    const float u2 = (ux * ux) + (uy * uy) + (uz * uz);

#undef UNROLL_X
#define UNROLL_X(i)                                                                             \
    eu = (ux * E##i##_X) + (uy * E##i##_Y) + (uz * E##i##_Z);                                   \
    const float f##i = (rho * OMEGA_##i) * (1.0f + (3.0f * eu) + (4.5f * eu * eu) - (1.5f * u2));
    UNROLL_19();
#endif

#if (COLLIDE_METHOD == COLLIDE_SAILFISH)
    const float f0  = (OMEGA_0  * rho) * (-1.5 * (ux * ux) - 1.5 * (uy * uy) - 1.5 * (uz * uz))                             + (OMEGA_0  * rho);
    const float f1  = (OMEGA_1  * rho) * (ux * (3.0f * ux + 3.0f) - 1.5 * (uy * uy) - 1.5 * (uz * uz))                      + (OMEGA_1  * rho);
    const float f2  = (OMEGA_2  * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy + 3.0f) - 1.5 * (uz * uz))                     + (OMEGA_2  * rho);
    const float f3  = (OMEGA_3  * rho) * (ux * (3.0f * ux - 3.0f) - 1.5 * (uy * uy) - 1.5 * (uz * uz))                      + (OMEGA_3  * rho);
    const float f4  = (OMEGA_4  * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy - 3.0f) - 1.5 * (uz * uz))                     + (OMEGA_4  * rho);
    const float f5  = (OMEGA_5  * rho) * (-1.5 * (ux * ux) - 1.5 * (uy * uy) + uz * (3.0f * uz - 3.0f))                     + (OMEGA_5  * rho);
    const float f6  = (OMEGA_6  * rho) * (-1.5 * (ux * ux) - 1.5 * (uy * uy) + uz * (3.0f * uz + 3.0f))                     + (OMEGA_6  * rho);
    const float f7  = (OMEGA_7  * rho) * (ux * (3.0f * ux + 9.0f * uy + 3.0f) + uy * (3.0f * uy + 3.0f) - 1.5 * (uz * uz))  + (OMEGA_7  * rho);
    const float f8  = (OMEGA_8  * rho) * (ux * (3.0f * ux - 9.0f * uy - 3.0f) + uy * (3.0f * uy + 3.0f) - 1.5 * (uz * uz))  + (OMEGA_8  * rho);
    const float f9  = (OMEGA_9  * rho) * (ux * (3.0f * ux + 9.0f * uy - 3.0f) + uy * (3.0f * uy - 3.0f) - 1.5 * (uz * uz))  + (OMEGA_9  * rho);
    const float f10 = (OMEGA_10 * rho) * (ux * (3.0f * ux - 9.0f * uy + 3.0f) + uy * (3.0f * uy - 3.0f) - 1.5 * (uz * uz))  + (OMEGA_10 * rho);
    const float f11 = (OMEGA_11 * rho) * (ux * (3.0f * ux - 9.0f * uz + 3.0f) - 1.5 * (uy * uy) + uz * (3.0f * uz - 3.0f))  + (OMEGA_11 * rho);
    const float f12 = (OMEGA_12 * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy - 9.0f * uz + 3.0f) + uz * (3.0f * uz - 3.0f)) + (OMEGA_12 * rho);
    const float f13 = (OMEGA_13 * rho) * (ux * (3.0f * ux + 9.0f * uz - 3.0f) - 1.5 * (uy * uy) + uz * (3.0f * uz - 3.0f))  + (OMEGA_13 * rho);
    const float f14 = (OMEGA_14 * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy + 9.0f * uz - 3.0f) + uz * (3.0f * uz - 3.0f)) + (OMEGA_14 * rho);
    const float f15 = (OMEGA_15 * rho) * (ux * (3.0f * ux + 9.0f * uz + 3.0f) - 1.5 * (uy * uy) + uz * (3.0f * uz + 3.0f))  + (OMEGA_15 * rho);
    const float f16 = (OMEGA_16 * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy + 9.0f * uz + 3.0f) + uz * (3.0f * uz + 3.0f)) + (OMEGA_16 * rho);
    const float f17 = (OMEGA_17 * rho) * (ux * (3.0f * ux - 9.0f * uz - 3.0f) - 1.5 * (uy * uy) + uz * (3.0f * uz + 3.0f))  + (OMEGA_17 * rho);
    const float f18 = (OMEGA_18 * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy - 9.0f * uz - 3.0f) + uz * (3.0f * uz + 3.0f)) + (OMEGA_18 * rho);
#endif

#undef UNROLL_X
#define UNROLL_X(i) f_collide[IDxyzw(id, i)] = (is_wall(cell_type) ? NAN : f##i);
    UNROLL_19();

#undef UNROLL_X
#define UNROLL_X(i) f_stream[IDxyzw(id, i)] = NAN;
    UNROLL_19();

    density[id] = (is_wall(cell_type) || is_corner(cell_type) ? NAN : rho);
    UX(id) = (is_wall(cell_type) || is_corner(cell_type) ? NAN : ux);
    UY(id) = (is_wall(cell_type) || is_corner(cell_type) ? NAN : uy);
    UZ(id) = (is_wall(cell_type) || is_corner(cell_type) ? NAN : uz);

    map[id] = cell_type;
}


__kernel
void computeMacro(__global float * f_collide, __global float * density, __global float * u, __global const int * map)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int z = get_global_id(2);
    const int id = IDxyz(x, y, z);
    const int cell_type = map[id];

    if (is_wall(cell_type) || is_corner(cell_type)) return;

    float rho = 0.0f;
    float ux = 0.0f;
    float uy = 0.0f;
    float uz = 0.0f;

    if (is_moving(cell_type)) {
        // f_collide[IDxyzw(id,  6)] = f_collide[IDxyzw(id,  S_6)];
        // f_collide[IDxyzw(id, 15)] = f_collide[IDxyzw(id, S_15)];
        // f_collide[IDxyzw(id, 16)] = f_collide[IDxyzw(id, S_16)];
        // f_collide[IDxyzw(id, 17)] = f_collide[IDxyzw(id, S_17)];
        // f_collide[IDxyzw(id, 18)] = f_collide[IDxyzw(id, S_18)];
        f_collide[IDxyzw(id,  5)] = f_collide[IDxyzw(id,  S_5)];
        f_collide[IDxyzw(id, 11)] = f_collide[IDxyzw(id, S_11)];
        f_collide[IDxyzw(id, 12)] = f_collide[IDxyzw(id, S_12)];
        f_collide[IDxyzw(id, 13)] = f_collide[IDxyzw(id, S_13)];
        f_collide[IDxyzw(id, 14)] = f_collide[IDxyzw(id, S_14)];
    }

#undef UNROLL_X
#define UNROLL_X(i) const float f##i = f_collide[IDxyzw(id, i)];
    UNROLL_19();

    rho = f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10 + f11 + f12 + f13 + f14 + f15 + f16 + f17 + f18;

    if (is_moving(cell_type)) {
        rho = rho / (CELL_INITIAL_VELOCITY_Z + 1.0f);
        ux = CELL_INITIAL_VELOCITY_X;
        uy = CELL_INITIAL_VELOCITY_Y;
        uz = CELL_INITIAL_VELOCITY_Z;
    } else {
        ux = (f1     -f3             +f7 -f8 -f9 +f10 +f11      -f13      +f15      -f17     ) / rho;
        uy = (    f2     -f4         +f7 +f8 -f9 -f10      +f12      -f14      +f16      -f18) / rho;
        uz = (               -f5 +f6                  -f11 -f12 -f13 -f14 +f15 +f16 +f17 +f18) / rho;
    }

    if (is_moving(cell_type) || is_fluid(cell_type)) {
        density[id] = rho;
        UX(id) = ux;
        UY(id) = uy;
        UZ(id) = uz;
    } else {
        density[id] = NAN;
        UX(id) = NAN;
        UY(id) = NAN;
        UZ(id) = NAN;
    }
}


__kernel
void boundaryConditions(__global float * f_collide, __global const float * density, __global const float * u, __global const int * map)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int z = get_global_id(2);
    const int id = IDxyz(x, y, z);
    const int cell_type = map[id];


    if (is_fluid(cell_type) || is_wall(cell_type) || is_corner(cell_type)) return;

    if (is_moving(cell_type)) {
        const float rho = density[id];
        const float ux = UX(id);
        const float uy = UY(id);
        const float uz = UZ(id);

#if (COLLIDE_METHOD == COLLIDE_SCRATCH)
        float eu = 0.0f;
        const float u2 = (ux * ux) + (uy * uy) + (uz * uz);

#define UNROLL_X(i)                                                                                         \
        eu = (ux * E##i##_X) + (uy * E##i##_Y) + (uz * E##i##_Z);                                           \
        f_collide[IDxyzw(id, i)] = (rho * OMEGA_##i) * (1.0f + (3.0f * eu) + (4.5f * eu * eu) - (1.5f * u2));
        UNROLL_19();
#endif

#if (COLLIDE_METHOD == COLLIDE_SAILFISH)
        f_collide[IDxyzw(id,  0)] = (OMEGA_0  * rho) * (-1.5 * (ux * ux) - 1.5 * (uy * uy) - 1.5 * (uz * uz))                             + (OMEGA_0  * rho);
        f_collide[IDxyzw(id,  1)] = (OMEGA_1  * rho) * (ux * (3.0f * ux + 3.0f) - 1.5 * (uy * uy) - 1.5 * (uz * uz))                      + (OMEGA_1  * rho);
        f_collide[IDxyzw(id,  2)] = (OMEGA_2  * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy + 3.0f) - 1.5 * (uz * uz))                     + (OMEGA_2  * rho);
        f_collide[IDxyzw(id,  3)] = (OMEGA_3  * rho) * (ux * (3.0f * ux - 3.0f) - 1.5 * (uy * uy) - 1.5 * (uz * uz))                      + (OMEGA_3  * rho);
        f_collide[IDxyzw(id,  4)] = (OMEGA_4  * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy - 3.0f) - 1.5 * (uz * uz))                     + (OMEGA_4  * rho);
        f_collide[IDxyzw(id,  5)] = (OMEGA_5  * rho) * (-1.5 * (ux * ux) - 1.5 * (uy * uy) + uz * (3.0f * uz - 3.0f))                     + (OMEGA_5  * rho);
        f_collide[IDxyzw(id,  6)] = (OMEGA_6  * rho) * (-1.5 * (ux * ux) - 1.5 * (uy * uy) + uz * (3.0f * uz + 3.0f))                     + (OMEGA_6  * rho);
        f_collide[IDxyzw(id,  7)] = (OMEGA_7  * rho) * (ux * (3.0f * ux + 9.0f * uy + 3.0f) + uy * (3.0f * uy + 3.0f) - 1.5 * (uz * uz))  + (OMEGA_7  * rho);
        f_collide[IDxyzw(id,  8)] = (OMEGA_8  * rho) * (ux * (3.0f * ux - 9.0f * uy - 3.0f) + uy * (3.0f * uy + 3.0f) - 1.5 * (uz * uz))  + (OMEGA_8  * rho);
        f_collide[IDxyzw(id,  9)] = (OMEGA_9  * rho) * (ux * (3.0f * ux + 9.0f * uy - 3.0f) + uy * (3.0f * uy - 3.0f) - 1.5 * (uz * uz))  + (OMEGA_9  * rho);
        f_collide[IDxyzw(id, 10)] = (OMEGA_10 * rho) * (ux * (3.0f * ux - 9.0f * uy + 3.0f) + uy * (3.0f * uy - 3.0f) - 1.5 * (uz * uz))  + (OMEGA_10 * rho);
        f_collide[IDxyzw(id, 11)] = (OMEGA_11 * rho) * (ux * (3.0f * ux - 9.0f * uz + 3.0f) - 1.5 * (uy * uy) + uz * (3.0f * uz - 3.0f))  + (OMEGA_11 * rho);
        f_collide[IDxyzw(id, 12)] = (OMEGA_12 * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy - 9.0f * uz + 3.0f) + uz * (3.0f * uz - 3.0f)) + (OMEGA_12 * rho);
        f_collide[IDxyzw(id, 13)] = (OMEGA_13 * rho) * (ux * (3.0f * ux + 9.0f * uz - 3.0f) - 1.5 * (uy * uy) + uz * (3.0f * uz - 3.0f))  + (OMEGA_13 * rho);
        f_collide[IDxyzw(id, 14)] = (OMEGA_14 * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy + 9.0f * uz - 3.0f) + uz * (3.0f * uz - 3.0f)) + (OMEGA_14 * rho);
        f_collide[IDxyzw(id, 15)] = (OMEGA_15 * rho) * (ux * (3.0f * ux + 9.0f * uz + 3.0f) - 1.5 * (uy * uy) + uz * (3.0f * uz + 3.0f))  + (OMEGA_15 * rho);
        f_collide[IDxyzw(id, 16)] = (OMEGA_16 * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy + 9.0f * uz + 3.0f) + uz * (3.0f * uz + 3.0f)) + (OMEGA_16 * rho);
        f_collide[IDxyzw(id, 17)] = (OMEGA_17 * rho) * (ux * (3.0f * ux - 9.0f * uz - 3.0f) - 1.5 * (uy * uy) + uz * (3.0f * uz + 3.0f))  + (OMEGA_17 * rho);
        f_collide[IDxyzw(id, 18)] = (OMEGA_18 * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy - 9.0f * uz - 3.0f) + uz * (3.0f * uz + 3.0f)) + (OMEGA_18 * rho);
#endif

    } else {

#undef UNROLL_X
#define UNROLL_X(i) const float f##i = f_collide[IDxyzw(id, i)];
        UNROLL_19();

#undef UNROLL_X
#define UNROLL_X(i) f_collide[IDxyzw(id, i)] = F_S(i);
        UNROLL_19();
    }
}


// Bhatnagar-Gross-Kroop approximation collision operator
inline float compute_bgk(const float f, const float f_eq)
{
    return f + INV_TAU * (f_eq - f);
}

__kernel
void collision(__global float * f_collide, __global const float * density, __global const float * u, __global const int * map)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int z = get_global_id(2);
    const int id = IDxyz(x, y, z);
    const int cell_type = map[id];

    if (!(is_moving(cell_type) || is_fluid(cell_type))) return;

    const float rho = density[id];
    const float ux = UX(id);
    const float uy = UY(id);
    const float uz = UZ(id);

#if (COLLIDE_METHOD == COLLIDE_SCRATCH)
    float eu = 0.0f;
    const float u2 = (ux * ux) + (uy * uy) + (uz * uz);

#define UNROLL_X(i)                                                                              \
    eu = (ux * E##i##_X) + (uy * E##i##_Y) + (uz * E##i##_Z);                                    \
    const float f##i = (rho * OMEGA_##i) * (1.0f + (3.0f * eu) + (4.5f * eu * eu) - (1.5f * u2));
    UNROLL_19();

#endif

#if (COLLIDE_METHOD == COLLIDE_SAILFISH)
    const float f0  = (OMEGA_0  * rho) * (-1.5 * (ux * ux) - 1.5 * (uy * uy) - 1.5 * (uz * uz))                             + (OMEGA_0  * rho);
    const float f1  = (OMEGA_1  * rho) * (ux * (3.0f * ux + 3.0f) - 1.5 * (uy * uy) - 1.5 * (uz * uz))                      + (OMEGA_1  * rho);
    const float f2  = (OMEGA_2  * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy + 3.0f) - 1.5 * (uz * uz))                     + (OMEGA_2  * rho);
    const float f3  = (OMEGA_3  * rho) * (ux * (3.0f * ux - 3.0f) - 1.5 * (uy * uy) - 1.5 * (uz * uz))                      + (OMEGA_3  * rho);
    const float f4  = (OMEGA_4  * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy - 3.0f) - 1.5 * (uz * uz))                     + (OMEGA_4  * rho);
    const float f5  = (OMEGA_5  * rho) * (-1.5 * (ux * ux) - 1.5 * (uy * uy) + uz * (3.0f * uz - 3.0f))                     + (OMEGA_5  * rho);
    const float f6  = (OMEGA_6  * rho) * (-1.5 * (ux * ux) - 1.5 * (uy * uy) + uz * (3.0f * uz + 3.0f))                     + (OMEGA_6  * rho);
    const float f7  = (OMEGA_7  * rho) * (ux * (3.0f * ux + 9.0f * uy + 3.0f) + uy * (3.0f * uy + 3.0f) - 1.5 * (uz * uz))  + (OMEGA_7  * rho);
    const float f8  = (OMEGA_8  * rho) * (ux * (3.0f * ux - 9.0f * uy - 3.0f) + uy * (3.0f * uy + 3.0f) - 1.5 * (uz * uz))  + (OMEGA_8  * rho);
    const float f9  = (OMEGA_9  * rho) * (ux * (3.0f * ux + 9.0f * uy - 3.0f) + uy * (3.0f * uy - 3.0f) - 1.5 * (uz * uz))  + (OMEGA_9  * rho);
    const float f10 = (OMEGA_10 * rho) * (ux * (3.0f * ux - 9.0f * uy + 3.0f) + uy * (3.0f * uy - 3.0f) - 1.5 * (uz * uz))  + (OMEGA_10 * rho);
    const float f11 = (OMEGA_11 * rho) * (ux * (3.0f * ux - 9.0f * uz + 3.0f) - 1.5 * (uy * uy) + uz * (3.0f * uz - 3.0f))  + (OMEGA_11 * rho);
    const float f12 = (OMEGA_12 * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy - 9.0f * uz + 3.0f) + uz * (3.0f * uz - 3.0f)) + (OMEGA_12 * rho);
    const float f13 = (OMEGA_13 * rho) * (ux * (3.0f * ux + 9.0f * uz - 3.0f) - 1.5 * (uy * uy) + uz * (3.0f * uz - 3.0f))  + (OMEGA_13 * rho);
    const float f14 = (OMEGA_14 * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy + 9.0f * uz - 3.0f) + uz * (3.0f * uz - 3.0f)) + (OMEGA_14 * rho);
    const float f15 = (OMEGA_15 * rho) * (ux * (3.0f * ux + 9.0f * uz + 3.0f) - 1.5 * (uy * uy) + uz * (3.0f * uz + 3.0f))  + (OMEGA_15 * rho);
    const float f16 = (OMEGA_16 * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy + 9.0f * uz + 3.0f) + uz * (3.0f * uz + 3.0f)) + (OMEGA_16 * rho);
    const float f17 = (OMEGA_17 * rho) * (ux * (3.0f * ux - 9.0f * uz - 3.0f) - 1.5 * (uy * uy) + uz * (3.0f * uz + 3.0f))  + (OMEGA_17 * rho);
    const float f18 = (OMEGA_18 * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy - 9.0f * uz - 3.0f) + uz * (3.0f * uz + 3.0f)) + (OMEGA_18 * rho);
#endif

#undef  UNROLL_X
#define UNROLL_X(i) f_collide[IDxyzw(id, i)] = compute_bgk(f_collide[IDxyzw(id,  i)], f##i);
    UNROLL_19();
}


__kernel
void streaming(__global float * f_stream, __global const float * f_collide, __global const int * map)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int z = get_global_id(2);
    const int id = IDxyz(x, y, z);
    const int cell_type = map[id];

#if (STREAMING_METHOD == STREAMING_PULL)
    if (is_wall(cell_type) || is_corner(cell_type)) return;
#undef  UNROLL_X
#define UNROLL_X(i)                                                       \
    {                                                                     \
        const int index = IDxyz(x - E##i##_X, y - E##i##_Y, z - E##i##_Z);\
        f_stream[IDxyzw(id, i)] = f_collide[IDxyzw(index, i)];            \
    }
    UNROLL_19();
#endif

#if (STREAMING_METHOD == STREAMING_PUSH)
    if (is_wall(cell_type) || is_corner(cell_type)) return;
#undef  UNROLL_X
#define UNROLL_X(i)                                                                   \
    {                                                                                 \
        const int nx = x + E##i##_X;                                                  \
        const int ny = y + E##i##_Y;                                                  \
        const int nz = z + E##i##_Z;                                                  \
        const int index = IDxyz(nx, ny, nz);                                          \
        if (0 <= nx && nx < DIM_X && 0 <= ny && ny < DIM_Y && 0 <= nz && nz < DIM_Z) {\
            f_stream[IDxyzw(index, i)] = f_collide[IDxyzw(id, i)];                    \
        }                                                                             \
    }
    UNROLL_19();
#endif
}
