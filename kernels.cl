#include "common.h"

// The following definitions are provided at compile time
//
// FP_SINGLE or FP_DOUBLE   to set the simulation with float or double type
// DIM                      the cube dimension of the simulation
// LWS                      work_group_size
// STRIDE                   stride value used to calculate index of CSoA data layout
// VELOCITY                 the moving wall velocity
// VISCOSITY                the fluid viscosity

#ifndef DIM
#error DIM is not defined
#endif

#ifndef LWS
#error LWS is not defined
#endif

#ifndef STRIDE
#error STRIDE is not defined
#endif

#ifndef VELOCITY
#error VELOCITY is not defined
#endif

#ifndef VISCOSITY
#error VISCOSITY is not defined
#endif


#define SCRATCH_METHOD                  (1 << 0)
#define SAILFISH_METHOD                 (1 << 1)
#define SIMULATION_METHOD               SCRATCH_METHOD

#define CALCULATION_ORDER_SAILFISH      1
#define STREAMING_METHOD                SCRATCH_METHOD


#ifdef FP_DOUBLE
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#pragma OPENCL EXTENSION cl_amd_fp64 : enable
#endif

#define INITIAL_DENSITY                 1.0
#define INITIAL_VELOCITY_X              VELOCITY
#define INITIAL_VELOCITY_Y              0.0
#define INITIAL_VELOCITY_Z              0.0

#define TAU                             ((3.0 * VISCOSITY) + 0.5)
#define INV_TAU                         (1.0 / TAU) // 1.89861401177140698415

#define IDxyzq(id, q)                   (((id) / STRIDE) * Q + q) * STRIDE + ((id) & (STRIDE - 1))
#define IDXYZQ(x, y, z, q)              (((((x) + ((y) * DIM) + ((z) * DIM * DIM)) / STRIDE) * Q + q) * STRIDE + (((x) + ((y) * DIM) + ((z) * DIM * DIM)) & (STRIDE - 1)))

#define IDxyz(x, y, z)                  ((x) + ((y) * (DIM)) + ((z) * (DIM) * (DIM)))
#define UX(id)                          u[(id) * 3 + 0]
#define UY(id)                          u[(id) * 3 + 1]
#define UZ(id)                          u[(id) * 3 + 2]


// MACRO UNROLL of 19.
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

// MACRO UNROLL of 9.
// Only the half of the indices are present the other, that are the opposites,
// can be calculated with S(i) macro
#define UNROLL_HALF_19() \
    UNROLL_X( 1)         \
    UNROLL_X( 2)         \
    UNROLL_X( 5)         \
    UNROLL_X( 7)         \
    UNROLL_X( 8)         \
    UNROLL_X(11)         \
    UNROLL_X(12)         \
    UNROLL_X(13)         \
    UNROLL_X(14)


#define OMEGA_0     (1.0 /  3.0)
#define OMEGA_1     (1.0 / 18.0)
#define OMEGA_2     (1.0 / 18.0)
#define OMEGA_3     (1.0 / 18.0)
#define OMEGA_4     (1.0 / 18.0)
#define OMEGA_5     (1.0 / 18.0)
#define OMEGA_6     (1.0 / 18.0)
#define OMEGA_7     (1.0 / 36.0)
#define OMEGA_8     (1.0 / 36.0)
#define OMEGA_9     (1.0 / 36.0)
#define OMEGA_10    (1.0 / 36.0)
#define OMEGA_11    (1.0 / 36.0)
#define OMEGA_12    (1.0 / 36.0)
#define OMEGA_13    (1.0 / 36.0)
#define OMEGA_14    (1.0 / 36.0)
#define OMEGA_15    (1.0 / 36.0)
#define OMEGA_16    (1.0 / 36.0)
#define OMEGA_17    (1.0 / 36.0)
#define OMEGA_18    (1.0 / 36.0)


#define E0_X        ( 0)
#define E0_Y        ( 0)
#define E0_Z        ( 0)

#define E1_X        (+1)
#define E1_Y        ( 0)
#define E1_Z        ( 0)

#define E2_X        ( 0)
#define E2_Y        (+1)
#define E2_Z        ( 0)

#define E3_X        (-1)
#define E3_Y        ( 0)
#define E3_Z        ( 0)

#define E4_X        ( 0)
#define E4_Y        (-1)
#define E4_Z        ( 0)

#define E5_X        ( 0)
#define E5_Y        ( 0)
#define E5_Z        (-1)

#define E6_X        ( 0)
#define E6_Y        ( 0)
#define E6_Z        (+1)

#define E7_X        (+1)
#define E7_Y        (+1)
#define E7_Z        ( 0)

#define E8_X        (-1)
#define E8_Y        (+1)
#define E8_Z        ( 0)

#define E9_X        (-1)
#define E9_Y        (-1)
#define E9_Z        ( 0)

#define E10_X       (+1)
#define E10_Y       (-1)
#define E10_Z       ( 0)

#define E11_X       (+1)
#define E11_Y       ( 0)
#define E11_Z       (-1)

#define E12_X       ( 0)
#define E12_Y       (+1)
#define E12_Z       (-1)

#define E13_X       (-1)
#define E13_Y       ( 0)
#define E13_Z       (-1)

#define E14_X       ( 0)
#define E14_Y       (-1)
#define E14_Z       (-1)

#define E15_X       (+1)
#define E15_Y       ( 0)
#define E15_Z       (+1)

#define E16_X       ( 0)
#define E16_Y       (+1)
#define E16_Z       (+1)

#define E17_X       (-1)
#define E17_Y       ( 0)
#define E17_Z       (+1)

#define E18_X       ( 0)
#define E18_Y       (-1)
#define E18_Z       (+1)


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

#define PRIMITIVE_CAT(a, b) a ## b
#define CAT(a, b)           PRIMITIVE_CAT(a, b)
#define F                   f
#define S(i)                S_##i
#define F_S(i)              CAT(F, S(i))


inline int get_cell_type(const int x, const int y, const int z)
{
    int cell_type = NONE;

    if (x == 1)         cell_type |= LEFT;
    if (x == (DIM - 2)) cell_type |= RIGHT;
    if (y == 1)         cell_type |= BOTTOM;
    if (y == (DIM - 2)) cell_type |= TOP;
    if (z == 1)         cell_type |= BACK;
    if (z == (DIM - 2)) cell_type |= FRONT;

    if (x == 0)         cell_type = WALL;
    if (x == (DIM - 1)) cell_type = WALL;
    if (y == 0)         cell_type = WALL;
    if (y == (DIM - 1)) cell_type = WALL;
    if (z == 0)         cell_type = WALL;
    if (z == (DIM - 1)) cell_type = WALL;

    if (cell_type == (LEFT  | BACK | BOTTOM) ||
        cell_type == (RIGHT | BACK | BOTTOM) ||
        cell_type == (LEFT  | BACK | TOP   ) ||
        cell_type == (RIGHT | BACK | TOP   ))
    {
        cell_type = CORNER;
    }

    if (cell_type == MOVING_BOUNDARY) cell_type |= MOVING;
    if (cell_type == NONE)            cell_type = FLUID;

    return cell_type;
}


// Bhatnagar-Gross-Kroop approximation collision operator
inline real_t compute_bgk(const real_t f, const real_t f_eq)
{
    return f + INV_TAU * (f_eq - f);
}


#if (SIMULATION_METHOD == SCRATCH_METHOD)
__kernel
void initialize(__global real_t * restrict f_stream,
                __global real_t * restrict f_collide,
                __global real_t * restrict density,
                __global real_t * restrict u,
                __global int * restrict map)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int z = get_global_id(2);
    const int id = IDxyz(x, y, z);
    const int cell_type = get_cell_type(x, y, z);

    map[id] = cell_type;

    const real_t rho = INITIAL_DENSITY;
    const real_t ux  = (is_moving_init(cell_type) ? INITIAL_VELOCITY_X : 0.0);
    const real_t uy  = (is_moving_init(cell_type) ? INITIAL_VELOCITY_Y : 0.0);
    const real_t uz  = (is_moving_init(cell_type) ? INITIAL_VELOCITY_Z : 0.0);

    density[id] = (is_store_macro(cell_type) ? rho : NAN);
    UX(id)      = (is_store_macro(cell_type) ? ux : NAN);
    UY(id)      = (is_store_macro(cell_type) ? uy : NAN);
    UZ(id)      = (is_store_macro(cell_type) ? uz : NAN);


    real_t eu = 0.0;
    const real_t u2 = (ux * ux) + (uy * uy) + (uz * uz);
#undef  UNROLL_X
#define UNROLL_X(i)                                                                           \
    eu = (ux * E##i##_X) + (uy * E##i##_Y) + (uz * E##i##_Z);                                 \
    const real_t f##i = (rho * OMEGA_##i) * (1.0 + (3.0 * eu) + (4.5 * eu * eu) - (1.5 * u2));
    UNROLL_19();

#undef  UNROLL_X
#define UNROLL_X(i) f_collide[IDxyzq(id, i)] = (is_wall(cell_type) ? NAN : f##i);
    UNROLL_19();

#undef  UNROLL_X
#define UNROLL_X(i) f_stream[IDxyzq(id, i)] = (is_wall(cell_type) ? NAN : f##i);
    UNROLL_19();
}


__kernel
void compute(__global real_t * restrict f_stream,
             __global const real_t * restrict f_collide,
             __global real_t * restrict density,
             __global real_t * restrict u,
             __global const int * restrict map)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int z = get_global_id(2);
    const int id = IDxyz(x, y, z);
    const int cell_type = map[id];

#undef  UNROLL_X
#define UNROLL_X(i) real_t f##i = f_collide[IDxyzq(id, i)];
    UNROLL_19();

    if (is_moving(cell_type)) {
        f5  = F_S( 5);
        f11 = F_S(11);
        f12 = F_S(12);
        f13 = F_S(13);
        f14 = F_S(14);
    }

    /***   Compute Macro quantities (rho & u)   ***/
#if CALCULATION_ORDER_SAILFISH
    const real_t rho = f5 + f11 + f12 + f14 + f13 + f0 + f1 + f2 + f7 + f8 + f4 + f10 + f9 + f6 + f15 + f16 + f18 + f17 + f3;
#else                   
    const real_t rho = f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10 + f11 + f12 + f13 + f14 + f15 + f16 + f17 + f18;
#endif

    real_t ux = NAN;
    real_t uy = NAN;
    real_t uz = NAN;

    if (is_moving(cell_type)) {
        ux = INITIAL_VELOCITY_X;
        uy = INITIAL_VELOCITY_Y;
        uz = INITIAL_VELOCITY_Z;
    } else {

#if CALCULATION_ORDER_SAILFISH
            //fBE - fBW +  fE + fNE - fNW + fSE - fSW + fTE - fTW -  fW) / zero;
        ux = (f11 - f13 +  f1 +  f7 -  f8 + f10 -  f9 + f15 - f17 -  f3) / rho;
           //(fBN - fBS +  fN + fNE + fNW -  fS - fSE - fSW + fTN - fTS) / zero;
        uy = (f12 - f14 +  f2 +  f7 +  f8 -  f4 - f10 -  f9 + f16 - f18) / rho;
           //(-fB - fBE - fBN - fBS - fBW +  fT + fTE + fTN + fTS + fTW) / zero;
        uz = (-f5 - f11 - f12 - f14 - f13  + f6 + f15 + f16 + f18 + f17) / rho;
#else 
        ux = (( f1 +  f7 + f10 + f11 + f15) - ( f3 +  f8 +  f9 + f13 + f17)) / rho;
        uy = (( f2 +  f7 +  f8 + f12 + f16) - ( f4 +  f9 + f10 + f14 + f18)) / rho;
        uz = (( f6 + f15 + f16 + f17 + f18) - ( f5 + f11 + f12 + f13 + f14)) / rho;
#endif 
    }

    /***   Store macro quantities (rho & u)   ***/
    if (is_store_macro(cell_type)) {
        density[id] = rho;
        UX(id) = ux;
        UY(id) = uy;
        UZ(id) = uz;
    }


    /***   Boundary Conditions   ***/
    if (is_moving(cell_type)) {

        real_t eu = 0.0;
        const real_t u2 = (ux * ux) + (uy * uy) + (uz * uz);

#undef  UNROLL_X
#define UNROLL_X(i)                                                                  \
        eu = (ux * E##i##_X) + (uy * E##i##_Y) + (uz * E##i##_Z);                    \
        f##i = (rho * OMEGA_##i) * (1.0 + (3.0 * eu) + (4.5 * eu * eu) - (1.5 * u2));
        UNROLL_19();

    } else if (is_bounceback(cell_type)) {

#undef  UNROLL_X
#define UNROLL_X(i)                 \
        {                           \
            const real_t tmp = f##i;\
            f##i = F_S(i);          \
            F_S(i) = tmp;           \
        }
        UNROLL_HALF_19();
    }


    /***   Collision   ***/
    if (is_collision(cell_type)) {

        real_t eu = 0.0;
        const real_t u2 = (ux * ux) + (uy * uy) + (uz * uz);
#undef  UNROLL_X
#define UNROLL_X(i)                                                                                  \
        eu = (ux * E##i##_X) + (uy * E##i##_Y) + (uz * E##i##_Z);                                    \
        const real_t fnew##i = (rho * OMEGA_##i) * (1.0 + (3.0 * eu) + (4.5 * eu * eu) - (1.5 * u2));
        UNROLL_19();

#undef  UNROLL_X
#define UNROLL_X(i) f##i = compute_bgk(f##i, fnew##i);
        UNROLL_19();
    }

#if (STREAMING_METHOD == SCRATCH_METHOD)
    if (is_wall(cell_type)) return;
    if (is_corner(cell_type)) return;
#undef  UNROLL_X
#define UNROLL_X(i)                                                             \
    {                                                                           \
        const int nx = x + E##i##_X;                                            \
        const int ny = y + E##i##_Y;                                            \
        const int nz = z + E##i##_Z;                                            \
        if (0 <= nx && nx < DIM && 0 <= ny && ny < DIM && 0 <= nz && nz < DIM) {\
            f_stream[IDXYZQ(nx, ny, nz, i)] = f##i;                             \
        }                                                                       \
    }
    UNROLL_19();
#endif

#if (STREAMING_METHOD == SAILFISH_METHOD)
    const int lx = get_local_id(0);

    bool alive = true;
    if (is_wall(cell_type)) {
        alive = false;
    }

    bool propagation_only = false;
    if (is_corner(cell_type)) {
        propagation_only = true;
    }

    __local real_t  _f1[LWS];
    __local real_t  _f7[LWS];
    __local real_t _f10[LWS];
    __local real_t _f11[LWS];
    __local real_t _f15[LWS];
#define  _f3  _f1
#define  _f8 _f10
#define  _f9  _f7
#define _f13 _f15
#define _f17 _f11
    _f1[lx] = -1.0;
    barrier(CLK_LOCAL_MEM_FENCE);
    if (!propagation_only && alive)
    {
        // Update the 0-th direction distribution
        f_stream[IDxyzq(id, 0)] = f0;                                                   //  0  0  0
        // Propagation in directions orthogonal to the X axis (global memory)
        if (y < (DIM-1)) f_stream[IDXYZQ(   x, y+1,   z,  2)] = f2;                     //  0 +1  0
        if (y > 0      ) f_stream[IDXYZQ(   x, y-1,   z,  4)] = f4;                     //  0 -1  0
        if (z < (DIM-1)) f_stream[IDXYZQ(   x,   y, z+1,  6)] = f6;                     //  0  0 +1
        if (z > 0      ) f_stream[IDXYZQ(   x,   y, z-1,  5)] = f5;                     //  0  0 -1

        if (y < (DIM-1) && z < (DIM-1)) f_stream[IDXYZQ(   x, y+1, z+1, 16)] = f16;     //  0 +1 +1
        if (y > 0       && z < (DIM-1)) f_stream[IDXYZQ(   x, y-1, z+1, 18)] = f18;     //  0 -1 +1
        if (y < (DIM-1) && z > 0      ) f_stream[IDXYZQ(   x, y+1, z-1, 12)] = f12;     //  0 +1 -1
        if (y > 0       && z > 0      ) f_stream[IDXYZQ(   x, y-1, z-1, 14)] = f14;     //  0 -1 -1

        // E propagation in shared memory
        if (x < (DIM-1)) {
            // Note: propagation to ghost nodes is done directly in global memory as there
            // are no threads running for the ghost nodes.
            if (lx < (LWS-1) && x != (DIM-2)) {
                 _f1[lx + 1] =  f1;
                 _f7[lx + 1] =  f7;
                _f10[lx + 1] = f10;
                _f11[lx + 1] = f11;
                _f15[lx + 1] = f15;
                // E propagation in global memory (at right block boundary)
            } else {
                                 f_stream[IDXYZQ( x+1,   y,   z,  1)] =  f1;            // +1  0  0
                if (y < (DIM-1)) f_stream[IDXYZQ( x+1, y+1,   z,  7)] =  f7;            // +1 +1  0
                if (y > 0      ) f_stream[IDXYZQ( x+1, y-1,   z, 10)] = f10;            // +1 -1  0
                if (z < (DIM-1)) f_stream[IDXYZQ( x+1,   y, z+1, 15)] = f15;            // +1  0 +1
                if (z > 0      ) f_stream[IDXYZQ( x+1,   y, z-1, 11)] = f11;            // +1  0 -1
            }
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    // Save locally propagated distributions into global memory.
    // The leftmost thread is not updated in this block.
    if (lx > 0 && x < DIM && !propagation_only && alive)
    {
        if (_f1[lx] != -1.0) {
                             f_stream[IDXYZQ( x,   y,   z,  1)] =  _f1[lx];             //  0  0  0
            if (y < (DIM-1)) f_stream[IDXYZQ( x, y+1,   z,  7)] =  _f7[lx];             //  0 +1  0
            if (y > 0      ) f_stream[IDXYZQ( x, y-1,   z, 10)] = _f10[lx];             //  0 -1  0
            if (z < (DIM-1)) f_stream[IDXYZQ( x,   y, z+1, 15)] = _f15[lx];             //  0  0 +1
            if (z > 0      ) f_stream[IDXYZQ( x,   y, z-1, 11)] = _f11[lx];             //  0  0 -1
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    // Refill the propagation buffer with sentinel values.
    _f1[lx] = -1.0;
    barrier(CLK_LOCAL_MEM_FENCE);
    if (!propagation_only && alive)
    {
        // W propagation in shared memory
        // Note: propagation to ghost nodes is done directly in global memory as there
        // are no threads running for the ghost nodes.
        if ((lx > 1 || (lx > 0 && x >= LWS)) && !propagation_only) {
             _f3[lx - 1] = f3;
             _f8[lx - 1] = f8;
             _f9[lx - 1] = f9;
            _f13[lx - 1] = f13;
            _f17[lx - 1] = f17;
            // W propagation in global memory (at left block boundary)
        } else if (x > 0) {
                             f_stream[IDXYZQ( x-1,   y,   z,  3)] =  f3;                // -1  0  0
            if (y < (DIM-1)) f_stream[IDXYZQ( x-1, y+1,   z,  8)] =  f8;                // -1 +1  0
            if (y > 0      ) f_stream[IDXYZQ( x-1, y-1,   z,  9)] =  f9;                // -1 -1  0
            if (z < (DIM-1)) f_stream[IDXYZQ( x-1,   y, z+1, 17)] = f17;                // -1  0 +1
            if (z > 0      ) f_stream[IDXYZQ( x-1,   y, z-1, 13)] = f13;                // -1  0 -1
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    // The rightmost thread is not updated in this block.
    if (lx < (LWS-1) && x < (DIM-1) && !propagation_only && alive)
    {
        if (_f1[lx] != -1.0) {
                             f_stream[IDXYZQ( x,   y,   z,  3)] =  _f3[lx];             //  0  0  0
            if (y < (DIM-1)) f_stream[IDXYZQ( x, y+1,   z,  8)] =  _f8[lx];             //  0 +1  0
            if (y > 0      ) f_stream[IDXYZQ( x, y-1,   z,  9)] =  _f9[lx];             //  0 -1  0
            if (z < (DIM-1)) f_stream[IDXYZQ( x,   y, z+1, 17)] = _f17[lx];             //  0  0 +1
            if (z > 0      ) f_stream[IDXYZQ( x,   y, z-1, 13)] = _f13[lx];             //  0  0 -1
        }
    }
#endif
}

#else

__kernel
void initialize(__global real_t * restrict f_stream,
                __global real_t * restrict f_collide,
                __global real_t * restrict density,
                __global real_t * restrict u,
                __global int * restrict map)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int z = get_global_id(2);
    const int id = IDxyz(x, y, z);
    const int cell_type = get_cell_type(x, y, z);

    map[id] = cell_type;


    const real_t rho = INITIAL_DENSITY;
    const real_t ux  = (is_moving_init(cell_type) ? INITIAL_VELOCITY_X : 0.0);
    const real_t uy  = (is_moving_init(cell_type) ? INITIAL_VELOCITY_Y : 0.0);
    const real_t uz  = (is_moving_init(cell_type) ? INITIAL_VELOCITY_Z : 0.0);

    density[id] = (is_store_macro(cell_type) ? rho : NAN);
    UX(id)      = (is_store_macro(cell_type) ? ux : NAN);
    UY(id)      = (is_store_macro(cell_type) ? uy : NAN);
    UZ(id)      = (is_store_macro(cell_type) ? uz : NAN);

    const real_t f0  = (OMEGA_0  * rho) * (-1.5 * (ux * ux) - 1.5 * (uy * uy) - 1.5 * (uz * uz))                        + (OMEGA_0  * rho);
    const real_t f1  = (OMEGA_1  * rho) * (ux * (3.0 * ux + 3.0) - 1.5 * (uy * uy) - 1.5 * (uz * uz))                   + (OMEGA_1  * rho);
    const real_t f2  = (OMEGA_2  * rho) * (-1.5 * (ux * ux) + uy * (3.0 * uy + 3.0) - 1.5 * (uz * uz))                  + (OMEGA_2  * rho);
    const real_t f3  = (OMEGA_3  * rho) * (ux * (3.0 * ux - 3.0) - 1.5 * (uy * uy) - 1.5 * (uz * uz))                   + (OMEGA_3  * rho);
    const real_t f4  = (OMEGA_4  * rho) * (-1.5 * (ux * ux) + uy * (3.0 * uy - 3.0) - 1.5 * (uz * uz))                  + (OMEGA_4  * rho);
    const real_t f5  = (OMEGA_5  * rho) * (-1.5 * (ux * ux) - 1.5 * (uy * uy) + uz * (3.0 * uz - 3.0))                  + (OMEGA_5  * rho);
    const real_t f6  = (OMEGA_6  * rho) * (-1.5 * (ux * ux) - 1.5 * (uy * uy) + uz * (3.0 * uz + 3.0))                  + (OMEGA_6  * rho);
    const real_t f7  = (OMEGA_7  * rho) * (ux * (3.0 * ux + 9.0 * uy + 3.0) + uy * (3.0 * uy + 3.0) - 1.5 * (uz * uz))  + (OMEGA_7  * rho);
    const real_t f8  = (OMEGA_8  * rho) * (ux * (3.0 * ux - 9.0 * uy - 3.0) + uy * (3.0 * uy + 3.0) - 1.5 * (uz * uz))  + (OMEGA_8  * rho);
    const real_t f9  = (OMEGA_9  * rho) * (ux * (3.0 * ux + 9.0 * uy - 3.0) + uy * (3.0 * uy - 3.0) - 1.5 * (uz * uz))  + (OMEGA_9  * rho);
    const real_t f10 = (OMEGA_10 * rho) * (ux * (3.0 * ux - 9.0 * uy + 3.0) + uy * (3.0 * uy - 3.0) - 1.5 * (uz * uz))  + (OMEGA_10 * rho);
    const real_t f11 = (OMEGA_11 * rho) * (ux * (3.0 * ux - 9.0 * uz + 3.0) - 1.5 * (uy * uy) + uz * (3.0 * uz - 3.0))  + (OMEGA_11 * rho);
    const real_t f12 = (OMEGA_12 * rho) * (-1.5 * (ux * ux) + uy * (3.0 * uy - 9.0 * uz + 3.0) + uz * (3.0 * uz - 3.0)) + (OMEGA_12 * rho);
    const real_t f13 = (OMEGA_13 * rho) * (ux * (3.0 * ux + 9.0 * uz - 3.0) - 1.5 * (uy * uy) + uz * (3.0 * uz - 3.0))  + (OMEGA_13 * rho);
    const real_t f14 = (OMEGA_14 * rho) * (-1.5 * (ux * ux) + uy * (3.0 * uy + 9.0 * uz - 3.0) + uz * (3.0 * uz - 3.0)) + (OMEGA_14 * rho);
    const real_t f15 = (OMEGA_15 * rho) * (ux * (3.0 * ux + 9.0 * uz + 3.0) - 1.5 * (uy * uy) + uz * (3.0 * uz + 3.0))  + (OMEGA_15 * rho);
    const real_t f16 = (OMEGA_16 * rho) * (-1.5 * (ux * ux) + uy * (3.0 * uy + 9.0 * uz + 3.0) + uz * (3.0 * uz + 3.0)) + (OMEGA_16 * rho);
    const real_t f17 = (OMEGA_17 * rho) * (ux * (3.0 * ux - 9.0 * uz - 3.0) - 1.5 * (uy * uy) + uz * (3.0 * uz + 3.0))  + (OMEGA_17 * rho);
    const real_t f18 = (OMEGA_18 * rho) * (-1.5 * (ux * ux) + uy * (3.0 * uy - 9.0 * uz - 3.0) + uz * (3.0 * uz + 3.0)) + (OMEGA_18 * rho);

#undef  UNROLL_X
#define UNROLL_X(i) f_collide[IDxyzq(id, i)] = (is_wall(cell_type) ? NAN : f##i);
    UNROLL_19();

#undef  UNROLL_X
#define UNROLL_X(i) f_stream[IDxyzq(id, i)] = (is_wall(cell_type) ? NAN : f##i);
    UNROLL_19();
}


__kernel
void compute(__global real_t * restrict f_stream,
             __global const real_t * restrict f_collide,
             __global real_t * restrict density,
             __global real_t * restrict u,
             __global const int * restrict map)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int z = get_global_id(2);
    const int id = IDxyz(x, y, z);
    const int cell_type = map[id];

#undef  UNROLL_X
#define UNROLL_X(i) real_t f##i = f_collide[IDxyzq(id, i)];
    UNROLL_19();

    if (is_moving(cell_type)) {
        f5  = F_S( 5);
        f11 = F_S(11);
        f12 = F_S(12);
        f13 = F_S(13);
        f14 = F_S(14);
    }

    /***   Compute Macro quantities (rho & u)   ***/
    const real_t rho = f5 + f11 + f12 + f14 + f13 + f0 + f1 + f2 + f7 + f8 + f4 + f10 + f9 + f6 + f15 + f16 + f18 + f17 + f3;

    real_t ux = NAN;
    real_t uy = NAN;
    real_t uz = NAN;

    if (is_moving(cell_type)) {
        ux = INITIAL_VELOCITY_X;
        uy = INITIAL_VELOCITY_Y;
        uz = INITIAL_VELOCITY_Z;
    } else {
        ux = (f11 - f13 +  f1 +  f7 -  f8 + f10 -  f9 + f15 - f17 -  f3) / rho;
        uy = (f12 - f14 +  f2 +  f7 +  f8 -  f4 - f10 -  f9 + f16 - f18) / rho;
        uz = (-f5 - f11 - f12 - f14 - f13  + f6 + f15 + f16 + f18 + f17) / rho;
    }

    /***   Store macro quantities (rho & u)   ***/
    if (is_store_macro(cell_type)) {
        density[id] = rho;
        UX(id) = ux;
        UY(id) = uy;
        UZ(id) = uz;
    }


    /***   Boundary Conditions   ***/
    if (is_moving(cell_type)) {
        f0  = (OMEGA_0  * rho) * (-1.5 * (ux * ux) - 1.5 * (uy * uy) - 1.5 * (uz * uz))                        + (OMEGA_0  * rho);
        f1  = (OMEGA_1  * rho) * (ux * (3.0 * ux + 3.0) - 1.5 * (uy * uy) - 1.5 * (uz * uz))                   + (OMEGA_1  * rho);
        f2  = (OMEGA_2  * rho) * (-1.5 * (ux * ux) + uy * (3.0 * uy + 3.0) - 1.5 * (uz * uz))                  + (OMEGA_2  * rho);
        f3  = (OMEGA_3  * rho) * (ux * (3.0 * ux - 3.0) - 1.5 * (uy * uy) - 1.5 * (uz * uz))                   + (OMEGA_3  * rho);
        f4  = (OMEGA_4  * rho) * (-1.5 * (ux * ux) + uy * (3.0 * uy - 3.0) - 1.5 * (uz * uz))                  + (OMEGA_4  * rho);
        f5  = (OMEGA_5  * rho) * (-1.5 * (ux * ux) - 1.5 * (uy * uy) + uz * (3.0 * uz - 3.0))                  + (OMEGA_5  * rho);
        f6  = (OMEGA_6  * rho) * (-1.5 * (ux * ux) - 1.5 * (uy * uy) + uz * (3.0 * uz + 3.0))                  + (OMEGA_6  * rho);
        f7  = (OMEGA_7  * rho) * (ux * (3.0 * ux + 9.0 * uy + 3.0) + uy * (3.0 * uy + 3.0) - 1.5 * (uz * uz))  + (OMEGA_7  * rho);
        f8  = (OMEGA_8  * rho) * (ux * (3.0 * ux - 9.0 * uy - 3.0) + uy * (3.0 * uy + 3.0) - 1.5 * (uz * uz))  + (OMEGA_8  * rho);
        f9  = (OMEGA_9  * rho) * (ux * (3.0 * ux + 9.0 * uy - 3.0) + uy * (3.0 * uy - 3.0) - 1.5 * (uz * uz))  + (OMEGA_9  * rho);
        f10 = (OMEGA_10 * rho) * (ux * (3.0 * ux - 9.0 * uy + 3.0) + uy * (3.0 * uy - 3.0) - 1.5 * (uz * uz))  + (OMEGA_10 * rho);
        f11 = (OMEGA_11 * rho) * (ux * (3.0 * ux - 9.0 * uz + 3.0) - 1.5 * (uy * uy) + uz * (3.0 * uz - 3.0))  + (OMEGA_11 * rho);
        f12 = (OMEGA_12 * rho) * (-1.5 * (ux * ux) + uy * (3.0 * uy - 9.0 * uz + 3.0) + uz * (3.0 * uz - 3.0)) + (OMEGA_12 * rho);
        f13 = (OMEGA_13 * rho) * (ux * (3.0 * ux + 9.0 * uz - 3.0) - 1.5 * (uy * uy) + uz * (3.0 * uz - 3.0))  + (OMEGA_13 * rho);
        f14 = (OMEGA_14 * rho) * (-1.5 * (ux * ux) + uy * (3.0 * uy + 9.0 * uz - 3.0) + uz * (3.0 * uz - 3.0)) + (OMEGA_14 * rho);
        f15 = (OMEGA_15 * rho) * (ux * (3.0 * ux + 9.0 * uz + 3.0) - 1.5 * (uy * uy) + uz * (3.0 * uz + 3.0))  + (OMEGA_15 * rho);
        f16 = (OMEGA_16 * rho) * (-1.5 * (ux * ux) + uy * (3.0 * uy + 9.0 * uz + 3.0) + uz * (3.0 * uz + 3.0)) + (OMEGA_16 * rho);
        f17 = (OMEGA_17 * rho) * (ux * (3.0 * ux - 9.0 * uz - 3.0) - 1.5 * (uy * uy) + uz * (3.0 * uz + 3.0))  + (OMEGA_17 * rho);
        f18 = (OMEGA_18 * rho) * (-1.5 * (ux * ux) + uy * (3.0 * uy - 9.0 * uz - 3.0) + uz * (3.0 * uz + 3.0)) + (OMEGA_18 * rho);
    } else if (is_bounceback(cell_type)) {
        real_t tmp = f1;
        f1 = f3;
        f3 = tmp;

        tmp = f2;
        f2 = f4;
        f4 = tmp;

        tmp = f6;
        f6 = f5;
        f5 = tmp;

        tmp = f7;
        f7 = f9;
        f9 = tmp;

        tmp = f8;
        f8 = f10;
        f10 = tmp;
        
        tmp = f16;
        f16 = f14;
        f14 = tmp;

        tmp = f18;
        f18 = f12;
        f12 = tmp;

        tmp = f15;
        f15 = f13;
        f13 = tmp;

        tmp = f17;
        f17 = f11;
        f11 = tmp;
    }


    /***   Collision   ***/
    if (is_collision(cell_type)) {
        const real_t fnew0  = (OMEGA_0  * rho) * (-1.5 * (ux * ux) - 1.5 * (uy * uy) - 1.5 * (uz * uz))                        + (OMEGA_0  * rho);
        const real_t fnew1  = (OMEGA_1  * rho) * (ux * (3.0 * ux + 3.0) - 1.5 * (uy * uy) - 1.5 * (uz * uz))                   + (OMEGA_1  * rho);
        const real_t fnew2  = (OMEGA_2  * rho) * (-1.5 * (ux * ux) + uy * (3.0 * uy + 3.0) - 1.5 * (uz * uz))                  + (OMEGA_2  * rho);
        const real_t fnew3  = (OMEGA_3  * rho) * (ux * (3.0 * ux - 3.0) - 1.5 * (uy * uy) - 1.5 * (uz * uz))                   + (OMEGA_3  * rho);
        const real_t fnew4  = (OMEGA_4  * rho) * (-1.5 * (ux * ux) + uy * (3.0 * uy - 3.0) - 1.5 * (uz * uz))                  + (OMEGA_4  * rho);
        const real_t fnew5  = (OMEGA_5  * rho) * (-1.5 * (ux * ux) - 1.5 * (uy * uy) + uz * (3.0 * uz - 3.0))                  + (OMEGA_5  * rho);
        const real_t fnew6  = (OMEGA_6  * rho) * (-1.5 * (ux * ux) - 1.5 * (uy * uy) + uz * (3.0 * uz + 3.0))                  + (OMEGA_6  * rho);
        const real_t fnew7  = (OMEGA_7  * rho) * (ux * (3.0 * ux + 9.0 * uy + 3.0) + uy * (3.0 * uy + 3.0) - 1.5 * (uz * uz))  + (OMEGA_7  * rho);
        const real_t fnew8  = (OMEGA_8  * rho) * (ux * (3.0 * ux - 9.0 * uy - 3.0) + uy * (3.0 * uy + 3.0) - 1.5 * (uz * uz))  + (OMEGA_8  * rho);
        const real_t fnew9  = (OMEGA_9  * rho) * (ux * (3.0 * ux + 9.0 * uy - 3.0) + uy * (3.0 * uy - 3.0) - 1.5 * (uz * uz))  + (OMEGA_9  * rho);
        const real_t fnew10 = (OMEGA_10 * rho) * (ux * (3.0 * ux - 9.0 * uy + 3.0) + uy * (3.0 * uy - 3.0) - 1.5 * (uz * uz))  + (OMEGA_10 * rho);
        const real_t fnew11 = (OMEGA_11 * rho) * (ux * (3.0 * ux - 9.0 * uz + 3.0) - 1.5 * (uy * uy) + uz * (3.0 * uz - 3.0))  + (OMEGA_11 * rho);
        const real_t fnew12 = (OMEGA_12 * rho) * (-1.5 * (ux * ux) + uy * (3.0 * uy - 9.0 * uz + 3.0) + uz * (3.0 * uz - 3.0)) + (OMEGA_12 * rho);
        const real_t fnew13 = (OMEGA_13 * rho) * (ux * (3.0 * ux + 9.0 * uz - 3.0) - 1.5 * (uy * uy) + uz * (3.0 * uz - 3.0))  + (OMEGA_13 * rho);
        const real_t fnew14 = (OMEGA_14 * rho) * (-1.5 * (ux * ux) + uy * (3.0 * uy + 9.0 * uz - 3.0) + uz * (3.0 * uz - 3.0)) + (OMEGA_14 * rho);
        const real_t fnew15 = (OMEGA_15 * rho) * (ux * (3.0 * ux + 9.0 * uz + 3.0) - 1.5 * (uy * uy) + uz * (3.0 * uz + 3.0))  + (OMEGA_15 * rho);
        const real_t fnew16 = (OMEGA_16 * rho) * (-1.5 * (ux * ux) + uy * (3.0 * uy + 9.0 * uz + 3.0) + uz * (3.0 * uz + 3.0)) + (OMEGA_16 * rho);
        const real_t fnew17 = (OMEGA_17 * rho) * (ux * (3.0 * ux - 9.0 * uz - 3.0) - 1.5 * (uy * uy) + uz * (3.0 * uz + 3.0))  + (OMEGA_17 * rho);
        const real_t fnew18 = (OMEGA_18 * rho) * (-1.5 * (ux * ux) + uy * (3.0 * uy - 9.0 * uz - 3.0) + uz * (3.0 * uz + 3.0)) + (OMEGA_18 * rho);

#undef  UNROLL_X
#define UNROLL_X(i) f##i = compute_bgk(f##i, fnew##i);
        UNROLL_19();
    }

    const int lx = get_local_id(0);

    bool alive = true;
    if (is_wall(cell_type)) {
        alive = false;
    }

    bool propagation_only = false;
    if (is_corner(cell_type)) {
        propagation_only = true;
    }

    __local real_t  _f1[LWS];
    __local real_t  _f7[LWS];
    __local real_t _f10[LWS];
    __local real_t _f11[LWS];
    __local real_t _f15[LWS];
#define  _f3  _f1
#define  _f8 _f10
#define  _f9  _f7
#define _f13 _f15
#define _f17 _f11
    _f1[lx] = -1.0;
    barrier(CLK_LOCAL_MEM_FENCE);
    if (!propagation_only && alive)
    {
        // Update the 0-th direction distribution
        f_stream[IDxyzq(id, 0)] = f0;                                                   //  0  0  0
        // Propagation in directions orthogonal to the X axis (global memory)
        if (y < (DIM-1)) f_stream[IDXYZQ(   x, y+1,   z,  2)] = f2;                     //  0 +1  0
        if (y > 0      ) f_stream[IDXYZQ(   x, y-1,   z,  4)] = f4;                     //  0 -1  0
        if (z < (DIM-1)) f_stream[IDXYZQ(   x,   y, z+1,  6)] = f6;                     //  0  0 +1
        if (z > 0      ) f_stream[IDXYZQ(   x,   y, z-1,  5)] = f5;                     //  0  0 -1

        if (y < (DIM-1) && z < (DIM-1)) f_stream[IDXYZQ(   x, y+1, z+1, 16)] = f16;     //  0 +1 +1
        if (y > 0       && z < (DIM-1)) f_stream[IDXYZQ(   x, y-1, z+1, 18)] = f18;     //  0 -1 +1
        if (y < (DIM-1) && z > 0      ) f_stream[IDXYZQ(   x, y+1, z-1, 12)] = f12;     //  0 +1 -1
        if (y > 0       && z > 0      ) f_stream[IDXYZQ(   x, y-1, z-1, 14)] = f14;     //  0 -1 -1

        // E propagation in shared memory
        if (x < (DIM-1)) {
            // Note: propagation to ghost nodes is done directly in global memory as there
            // are no threads running for the ghost nodes.
            if (lx < (LWS-1) && x != (DIM-2)) {
                 _f1[lx + 1] =  f1;
                 _f7[lx + 1] =  f7;
                _f10[lx + 1] = f10;
                _f11[lx + 1] = f11;
                _f15[lx + 1] = f15;
                // E propagation in global memory (at right block boundary)
            } else {
                                 f_stream[IDXYZQ( x+1,   y,   z,  1)] =  f1;            // +1  0  0
                if (y < (DIM-1)) f_stream[IDXYZQ( x+1, y+1,   z,  7)] =  f7;            // +1 +1  0
                if (y > 0      ) f_stream[IDXYZQ( x+1, y-1,   z, 10)] = f10;            // +1 -1  0
                if (z < (DIM-1)) f_stream[IDXYZQ( x+1,   y, z+1, 15)] = f15;            // +1  0 +1
                if (z > 0      ) f_stream[IDXYZQ( x+1,   y, z-1, 11)] = f11;            // +1  0 -1
            }
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    // Save locally propagated distributions into global memory.
    // The leftmost thread is not updated in this block.
    if (lx > 0 && x < DIM && !propagation_only && alive)
    {
        if (_f1[lx] != -1.0) {
                             f_stream[IDXYZQ( x,   y,   z,  1)] =  _f1[lx];             //  0  0  0
            if (y < (DIM-1)) f_stream[IDXYZQ( x, y+1,   z,  7)] =  _f7[lx];             //  0 +1  0
            if (y > 0      ) f_stream[IDXYZQ( x, y-1,   z, 10)] = _f10[lx];             //  0 -1  0
            if (z < (DIM-1)) f_stream[IDXYZQ( x,   y, z+1, 15)] = _f15[lx];             //  0  0 +1
            if (z > 0      ) f_stream[IDXYZQ( x,   y, z-1, 11)] = _f11[lx];             //  0  0 -1
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    // Refill the propagation buffer with sentinel values.
    _f1[lx] = -1.0;
    barrier(CLK_LOCAL_MEM_FENCE);
    if (!propagation_only && alive)
    {
        // W propagation in shared memory
        // Note: propagation to ghost nodes is done directly in global memory as there
        // are no threads running for the ghost nodes.
        if ((lx > 1 || (lx > 0 && x >= LWS)) && !propagation_only) {
             _f3[lx - 1] = f3;
             _f8[lx - 1] = f8;
             _f9[lx - 1] = f9;
            _f13[lx - 1] = f13;
            _f17[lx - 1] = f17;
            // W propagation in global memory (at left block boundary)
        } else if (x > 0) {
                             f_stream[IDXYZQ( x-1,   y,   z,  3)] =  f3;                // -1  0  0
            if (y < (DIM-1)) f_stream[IDXYZQ( x-1, y+1,   z,  8)] =  f8;                // -1 +1  0
            if (y > 0      ) f_stream[IDXYZQ( x-1, y-1,   z,  9)] =  f9;                // -1 -1  0
            if (z < (DIM-1)) f_stream[IDXYZQ( x-1,   y, z+1, 17)] = f17;                // -1  0 +1
            if (z > 0      ) f_stream[IDXYZQ( x-1,   y, z-1, 13)] = f13;                // -1  0 -1
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    // The rightmost thread is not updated in this block.
    if (lx < (LWS-1) && x < (DIM-1) && !propagation_only && alive)
    {
        if (_f1[lx] != -1.0) {
                             f_stream[IDXYZQ( x,   y,   z,  3)] =  _f3[lx];             //  0  0  0
            if (y < (DIM-1)) f_stream[IDXYZQ( x, y+1,   z,  8)] =  _f8[lx];             //  0 +1  0
            if (y > 0      ) f_stream[IDXYZQ( x, y-1,   z,  9)] =  _f9[lx];             //  0 -1  0
            if (z < (DIM-1)) f_stream[IDXYZQ( x,   y, z+1, 17)] = _f17[lx];             //  0  0 +1
            if (z > 0      ) f_stream[IDXYZQ( x,   y, z-1, 13)] = _f13[lx];             //  0  0 -1
        }
    }
}
#endif
