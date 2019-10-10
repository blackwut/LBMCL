#ifndef CONSTANTS_H
#define CONSTANTS_H


#define D                           3
#define Q                           19

#ifndef DIM
#define DIM                         8
#endif

#ifndef VEL
#define VEL 5.00000000000000027756e-02f//0.05f
#endif

#ifndef VISC
#define VISC 0.0089f
#endif

#define DIM_X                       DIM
#define DIM_Y                       DIM
#define DIM_Z                       DIM

#define GRID_DIM                    (DIM_X * DIM_Y * DIM_Z)

#define CELL_INITIAL_DENSITY        1.0f
#define CELL_INITIAL_VELOCITY_X     5.00000000000000027756e-02f
#define CELL_INITIAL_VELOCITY_Y     0.0f
#define CELL_INITIAL_VELOCITY_Z     0.0f

#define VISCOSITY                   VISC//0.0089f
#define TAU                         ((3.0f * VISCOSITY) / (DELTA_T * DELTA_X * DELTA_X) + 0.5f)
#define INV_TAU                     1.89861401177140698415e+00f

#define DELTA_X                     1.0f
#define DELTA_T                     1.0f
#define C2_INV                      (DELTA_T / DELTA_X)
#define C4_INV                      (C2_INV * C2_INV)


#define IDxyz(x, y, z)              ((x) + ((y) * DIM_X) + ((z) * DIM_X * DIM_Y))
#define IDxyzw(id, w)               ((id) * Q + (w))

#define IDXYZW(x, y, z, w)          (((x) + ((y) * DIM_X) + ((z) * DIM_X * DIM_Y)) * Q + (w))

// E_X
// 0   1   0  -1   0   0   0   1  -1  -1   1   1   0  -1   0   1   0  -1   0
// E_Y
// 0   0   1   0  -1   0   0   1   1  -1  -1   0   1   0  -1   0   1   0  -1
// E_Z
// 0   0   0   0   0  -1   1   0   0   0   0  -1  -1  -1  -1   1   1   1   1

#define E0_X    (+0)
#define E0_Y    (+0)
#define E0_Z    (+0)

#define E1_X    (+1)
#define E1_Y    (+0)
#define E1_Z    (+0)

#define E2_X    (+0)
#define E2_Y    (+1)
#define E2_Z    (+0)

#define E3_X    (-1)
#define E3_Y    (+0)
#define E3_Z    (+0)

#define E4_X    (+0)
#define E4_Y    (-1)
#define E4_Z    (+0)

#define E5_X    (+0)
#define E5_Y    (+0)
#define E5_Z    (-1)

#define E6_X    (+0)
#define E6_Y    (+0)
#define E6_Z    (+1)

#define E7_X    (+1)
#define E7_Y    (+1)
#define E7_Z    (+0)

#define E8_X    (-1)
#define E8_Y    (+1)
#define E8_Z    (+0)

#define E9_X    (-1)
#define E9_Y    (-1)
#define E9_Z    (+0)

#define E10_X   (+1)
#define E10_Y   (-1)
#define E10_Z   (+0)

#define E11_X   (+1)
#define E11_Y   (+0)
#define E11_Z   (-1)

#define E12_X   (+0)
#define E12_Y   (+1)
#define E12_Z   (-1)

#define E13_X   (-1)
#define E13_Y   (+0)
#define E13_Z   (-1)

#define E14_X   (+0)
#define E14_Y   (-1)
#define E14_Z   (-1)

#define E15_X   (+1)
#define E15_Y   (+0)
#define E15_Z   (+1)

#define E16_X   (+0)
#define E16_Y   (+1)
#define E16_Z   (+1)

#define E17_X   (-1)
#define E17_Y   (+0)
#define E17_Z   (+1)

#define E18_X   (+0)
#define E18_Y   (-1)
#define E18_Z   (+1)


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
#define CORNER              (1 <<  1)
#define WALL                (1 <<  2)
#define LEFT                (1 <<  3)
#define RIGHT               (1 <<  4)
#define TOP                 (1 <<  5)
#define BOTTOM              (1 <<  6)
#define FRONT               (1 <<  7)
#define BACK                (1 <<  8)
// #define LEFT_TOP            (LEFT  | TOP)
// #define LEFT_BOTTOM         (LEFT  | BOTTOM)
// #define LEFT_BACK           (LEFT  | BACK)
// #define LEFT_FRONT          (LEFT  | FRONT)
// #define LEFT_FRONT_TOP      (LEFT  | FRONT  | TOP)
// #define LEFT_FRONT_BOTTOM   (LEFT  | FRONT  | BOTTOM)
// #define LEFT_BACK_TOP       (LEFT  | BACK   | TOP)
// #define LEFT_BACK_BOTTOM    (LEFT  | BACK   | BOTTOM)
// #define RIGHT_TOP           (RIGHT | TOP)
// #define RIGHT_BOTTOM        (RIGHT | BOTTOM)
// #define RIGHT_BACK          (RIGHT | BACK)
// #define RIGHT_FRONT         (RIGHT | FRONT)
// #define RIGHT_FRONT_TOP     (RIGHT | FRONT  | TOP)
// #define RIGHT_FRONT_BOTTOM  (RIGHT | FRONT  | BOTTOM)
// #define RIGHT_BACK_TOP      (RIGHT | BACK   | TOP)
// #define RIGHT_BACK_BOTTOM   (RIGHT | BACK   | BOTTOM)
// #define FRONT_TOP           (FRONT | TOP)
// #define FRONT_BOTTOM        (FRONT | BOTTOM)
// #define BACK_TOP            (BACK  | TOP)
// #define BACK_BOTTOM         (BACK  | BOTTOM)

#define MOVING_BOUNDARY   FRONT


// f_collide[IDxyzw(id,  0)] f0  = (OMEGA_0  * rho) * (-1.5f * (ux * ux) - 1.5f * (uy * uy) - 1.5f * (uz * uz)) + (OMEGA_0 * rho);
// f_collide[IDxyzw(id,  1)] f1  = (OMEGA_1  * rho) * (ux * (3.0f * ux + 3.0f) - 1.5f * (uy * uy) - 1.5f * (uz * uz)) + (OMEGA_1 * rho);
// f_collide[IDxyzw(id,  2)] f2  = (OMEGA_2  * rho) * (-1.5f * (ux * ux) - 1.5f * (uy * uy) + uz * (3.0f * uz + 3.0f)) + (OMEGA_2 * rho);
// f_collide[IDxyzw(id,  3)] f3  = (OMEGA_3  * rho) * (ux * (3.0f * ux - 3.0f) - 1.5f * (uy * uy) - 1.5f * (uz * uz)) + (OMEGA_3 * rho);
// f_collide[IDxyzw(id,  4)] f4  = (OMEGA_4  * rho) * (-1.5f * (ux * ux) - 1.5f * (uy * uy) + uz * (3.0f * uz - 3.0f)) + (OMEGA_4 * rho);
// f_collide[IDxyzw(id,  5)] f5  = (OMEGA_5  * rho) * (-1.5f * (ux * ux) + uy * (3.0f * uy + 3.0f) - 1.5f * (uz * uz)) + (OMEGA_5 * rho);
// f_collide[IDxyzw(id,  6)] f6  = (OMEGA_6  * rho) * (-1.5f * (ux * ux) + uy * (3.0f * uy - 3.0f) - 1.5f * (uz * uz)) + (OMEGA_6 * rho);
// f_collide[IDxyzw(id,  7)] f7  = (OMEGA_7  * rho) * (ux * (3.0f * ux + 9.0f * uz + 3.0f) - 1.5f * (uy * uy) + uz * (3.0f * uz + 3.0f)) + (OMEGA_7 * rho);
// f_collide[IDxyzw(id,  8)] f8  = (OMEGA_8  * rho) * (ux * (3.0f * ux - 9.0f * uz - 3.0f) - 1.5f * (uy * uy) + uz * (3.0f * uz + 3.0f)) + (OMEGA_8 * rho);
// f_collide[IDxyzw(id,  9)] f9  = (OMEGA_9  * rho) * (ux * (3.0f * ux + 9.0f * uz - 3.0f) - 1.5f * (uy * uy) + uz * (3.0f * uz - 3.0f)) + (OMEGA_9 * rho);
// f_collide[IDxyzw(id, 10)] f10 = (OMEGA_10 * rho) * (ux * (3.0f * ux - 9.0f * uz + 3.0f) - 1.5f * (uy * uy) + uz * (3.0f * uz - 3.0f)) + (OMEGA_10 * rho);
// f_collide[IDxyzw(id, 11)] f11 = (OMEGA_11 * rho) * (ux * (3.0f * ux + 9.0f * uy + 3.0f) + uy * (3.0f * uy + 3.0f) - 1.5f * (uz * uz)) + (OMEGA_11 * rho);
// f_collide[IDxyzw(id, 12)] f12 = (OMEGA_12 * rho) * (-1.5f * (ux * ux) + uy * (3.0f * uy + 9.0f * uz + 3.0f) + uz * (3.0f * uz + 3.0f)) + (OMEGA_12 * rho);
// f_collide[IDxyzw(id, 13)] f13 = (OMEGA_13 * rho) * (ux * (3.0f * ux - 9.0f * uy - 3.0f) + uy * (3.0f * uy + 3.0f) - 1.5f * (uz * uz)) + (OMEGA_13 * rho);
// f_collide[IDxyzw(id, 14)] f14 = (OMEGA_14 * rho) * (-1.5f * (ux * ux) + uy * (3.0f * uy - 9.0f * uz + 3.0f) + uz * (3.0f * uz - 3.0f)) + (OMEGA_14 * rho);
// f_collide[IDxyzw(id, 15)] f15 = (OMEGA_15 * rho) * (ux * (3.0f * ux - 9.0f * uy + 3.0f) + uy * (3.0f * uy - 3.0f) - 1.5f * (uz * uz)) + (OMEGA_15 * rho);
// f_collide[IDxyzw(id, 16)] f16 = (OMEGA_16 * rho) * (-1.5f * (ux * ux) + uy * (3.0f * uy - 9.0f * uz - 3.0f) + uz * (3.0f * uz + 3.0f)) + (OMEGA_16 * rho);
// f_collide[IDxyzw(id, 17)] f17 = (OMEGA_17 * rho) * (ux * (3.0f * ux + 9.0f * uy - 3.0f) + uy * (3.0f * uy - 3.0f) - 1.5f * (uz * uz)) + (OMEGA_17 * rho);
// f_collide[IDxyzw(id, 18)] f18 = (OMEGA_18 * rho) * (-1.5f * (ux * ux) + uy * (3.0f * uy + 9.0f * uz - 3.0f) + uz * (3.0f * uz - 3.0f)) + (OMEGA_18 * rho);


// f_collide[IDxyzw(id,  0)] f0  = (OMEGA_0  * rho) * (-1.5 * (ux * ux) - 1.5 * (uy * uy) - 1.5 * (uz * uz))                             + (OMEGA_0  * rho);
// f_collide[IDxyzw(id,  1)] f1  = (OMEGA_1  * rho) * (ux * (3.0f * ux + 3.0f) - 1.5 * (uy * uy) - 1.5 * (uz * uz))                      + (OMEGA_1  * rho);
// f_collide[IDxyzw(id,  2)] f2  = (OMEGA_2  * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy + 3.0f) - 1.5 * (uz * uz))                     + (OMEGA_2  * rho);
// f_collide[IDxyzw(id,  3)] f3  = (OMEGA_3  * rho) * (ux * (3.0f * ux - 3.0f) - 1.5 * (uy * uy) - 1.5 * (uz * uz))                      + (OMEGA_3  * rho);
// f_collide[IDxyzw(id,  4)] f4  = (OMEGA_4  * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy - 3.0f) - 1.5 * (uz * uz))                     + (OMEGA_4  * rho);
// f_collide[IDxyzw(id,  5)] f5  = (OMEGA_5  * rho) * (-1.5 * (ux * ux) - 1.5 * (uy * uy) + uz * (3.0f * uz - 3.0f))                     + (OMEGA_5  * rho);
// f_collide[IDxyzw(id,  6)] f6  = (OMEGA_6  * rho) * (-1.5 * (ux * ux) - 1.5 * (uy * uy) + uz * (3.0f * uz + 3.0f))                     + (OMEGA_6  * rho);
// f_collide[IDxyzw(id,  7)] f7  = (OMEGA_7  * rho) * (ux * (3.0f * ux + 9.0f * uy + 3.0f) + uy * (3.0f * uy + 3.0f) - 1.5 * (uz * uz))  + (OMEGA_7  * rho);
// f_collide[IDxyzw(id,  8)] f8  = (OMEGA_8  * rho) * (ux * (3.0f * ux - 9.0f * uy - 3.0f) + uy * (3.0f * uy + 3.0f) - 1.5 * (uz * uz))  + (OMEGA_8  * rho);
// f_collide[IDxyzw(id,  9)] f9  = (OMEGA_9  * rho) * (ux * (3.0f * ux + 9.0f * uy - 3.0f) + uy * (3.0f * uy - 3.0f) - 1.5 * (uz * uz))  + (OMEGA_9  * rho);
// f_collide[IDxyzw(id, 10)] f10 = (OMEGA_10 * rho) * (ux * (3.0f * ux - 9.0f * uy + 3.0f) + uy * (3.0f * uy - 3.0f) - 1.5 * (uz * uz))  + (OMEGA_10 * rho);
// f_collide[IDxyzw(id, 11)] f11 = (OMEGA_11 * rho) * (ux * (3.0f * ux - 9.0f * uz + 3.0f) - 1.5 * (uy * uy) + uz * (3.0f * uz - 3.0f))  + (OMEGA_11 * rho);
// f_collide[IDxyzw(id, 12)] f12 = (OMEGA_12 * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy - 9.0f * uz + 3.0f) + uz * (3.0f * uz - 3.0f)) + (OMEGA_12 * rho);
// f_collide[IDxyzw(id, 13)] f13 = (OMEGA_13 * rho) * (ux * (3.0f * ux + 9.0f * uz - 3.0f) - 1.5 * (uy * uy) + uz * (3.0f * uz - 3.0f))  + (OMEGA_13 * rho);
// f_collide[IDxyzw(id, 14)] f14 = (OMEGA_14 * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy + 9.0f * uz - 3.0f) + uz * (3.0f * uz - 3.0f)) + (OMEGA_14 * rho);
// f_collide[IDxyzw(id, 15)] f15 = (OMEGA_15 * rho) * (ux * (3.0f * ux + 9.0f * uz + 3.0f) - 1.5 * (uy * uy) + uz * (3.0f * uz + 3.0f))  + (OMEGA_15 * rho);
// f_collide[IDxyzw(id, 16)] f16 = (OMEGA_16 * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy + 9.0f * uz + 3.0f) + uz * (3.0f * uz + 3.0f)) + (OMEGA_16 * rho);
// f_collide[IDxyzw(id, 17)] f17 = (OMEGA_17 * rho) * (ux * (3.0f * ux - 9.0f * uz - 3.0f) - 1.5 * (uy * uy) + uz * (3.0f * uz + 3.0f))  + (OMEGA_17 * rho);
// f_collide[IDxyzw(id, 18)] f18 = (OMEGA_18 * rho) * (-1.5 * (ux * ux) + uy * (3.0f * uy - 9.0f * uz - 3.0f) + uz * (3.0f * uz + 3.0f)) + (OMEGA_18 * rho);


#define UNROLL_19() \
    UNROLL_X( 0)    \
    UNROLL_X( 1)    \
    UNROLL_X( 2)    \
    UNROLL_X( 3)    \
    UNROLL_X( 4)    \
    UNROLL_X( 5)    \
    UNROLL_X( 6)    \
    UNROLL_X( 7)    \
    UNROLL_X( 8)    \
    UNROLL_X( 9)    \
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
    UNROLL_X( 1)         \
    UNROLL_X( 2)         \
    UNROLL_X( 5)         \
    UNROLL_X( 7)         \
    UNROLL_X( 8)         \
    UNROLL_X(11)         \
    UNROLL_X(12)         \
    UNROLL_X(13)         \
    UNROLL_X(14)


// Bhatnagar-Gross-Kroop approximation collision operator
inline float compute_bgk(const float f, const float f_eq)
{
    return f + INV_TAU * (f_eq - f);
}

inline int is_fluid(const int cell_type)
{
    return (cell_type == FLUID);
}

inline int is_wall(const int cell_type)
{
    return (cell_type == WALL);
}

inline int is_corner(const int cell_type)
{
    return (cell_type == CORNER);
}

inline int is_boundary(const int cell_type)
{
    return (cell_type & (LEFT | RIGHT | BOTTOM | TOP | BACK | FRONT));
}

inline int is_moving(const int cell_type)
{
    return (cell_type == MOVING_BOUNDARY);
}

inline int is_bounceback(const int cell_type)
{
    return (cell_type & ((LEFT | RIGHT | BOTTOM | TOP | BACK | FRONT) & ~MOVING_BOUNDARY));
}

#endif
