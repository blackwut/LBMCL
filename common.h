#ifndef CONSTANTS_H
#define CONSTANTS_H

/***   User definitions   ***/

#ifndef DIM
#define DIM                 8
#endif

#ifndef VEL
#define VEL                 0.05f
#endif

#ifndef VISC
#define VISC                0.0089f
#endif


/***   LBM definitions   ***/

#define D                   3
#define Q                   19
#define Q_DIM               32

#define DIM_X               DIM
#define DIM_Y               DIM
#define DIM_Z               DIM

#define INITIAL_DENSITY     1.0f
#define INITIAL_VELOCITY_X  VEL
#define INITIAL_VELOCITY_Y  0.0f
#define INITIAL_VELOCITY_Z  0.0f

#define VISCOSITY           VISC
#define TAU                 ((3.0f * VISCOSITY) + 0.5f)
#define INV_TAU             1.89861401177140698415e+00f//(1.0f / TAU)


#define E0_X    ( 0)
#define E0_Y    ( 0)
#define E0_Z    ( 0)

#define E1_X    (+1)
#define E1_Y    ( 0)
#define E1_Z    ( 0)

#define E2_X    ( 0)
#define E2_Y    (+1)
#define E2_Z    ( 0)

#define E3_X    (-1)
#define E3_Y    ( 0)
#define E3_Z    ( 0)

#define E4_X    ( 0)
#define E4_Y    (-1)
#define E4_Z    ( 0)

#define E5_X    ( 0)
#define E5_Y    ( 0)
#define E5_Z    (-1)

#define E6_X    ( 0)
#define E6_Y    ( 0)
#define E6_Z    (+1)

#define E7_X    (+1)
#define E7_Y    (+1)
#define E7_Z    ( 0)

#define E8_X    (-1)
#define E8_Y    (+1)
#define E8_Z    ( 0)

#define E9_X    (-1)
#define E9_Y    (-1)
#define E9_Z    ( 0)

#define E10_X   (+1)
#define E10_Y   (-1)
#define E10_Z   ( 0)

#define E11_X   (+1)
#define E11_Y   ( 0)
#define E11_Z   (-1)

#define E12_X   ( 0)
#define E12_Y   (+1)
#define E12_Z   (-1)

#define E13_X   (-1)
#define E13_Y   ( 0)
#define E13_Z   (-1)

#define E14_X   ( 0)
#define E14_Y   (-1)
#define E14_Z   (-1)

#define E15_X   (+1)
#define E15_Y   ( 0)
#define E15_Z   (+1)

#define E16_X   ( 0)
#define E16_Y   (+1)
#define E16_Z   (+1)

#define E17_X   (-1)
#define E17_Y   ( 0)
#define E17_Z   (+1)

#define E18_X   ( 0)
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


#define IDxyz(x, y, z)      ((x) + ((y) * DIM_X) + ((z) * DIM_X * DIM_Y))
#define IDxyzw(id, w)       ((id) * Q_DIM + (w))

#define UX(id)              u[(id) * 3 + 0]
#define UY(id)              u[(id) * 3 + 1]
#define UZ(id)              u[(id) * 3 + 2]


#define PRIMITIVE_CAT(a, b) a ## b
#define CAT(a, b)           PRIMITIVE_CAT(a, b)
#define F                   f
#define S(i)                S_##i
#define F_S(i)              CAT(F, S(i))


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
    UNROLL_X( 1)         \
    UNROLL_X( 2)         \
    UNROLL_X( 5)         \
    UNROLL_X( 7)         \
    UNROLL_X( 8)         \
    UNROLL_X(11)         \
    UNROLL_X(12)         \
    UNROLL_X(13)         \
    UNROLL_X(14)


#define NONE                (0)
#define FLUID               (1 << 0)
#define MOVING              (1 << 1)
#define CORNER              (1 << 2)
#define WALL                (1 << 3)

#define LEFT                (1 << 4)
#define RIGHT               (1 << 5)
#define TOP                 (1 << 6)
#define BOTTOM              (1 << 7)
#define FRONT               (1 << 8)
#define BACK                (1 << 9)

// TODO: handle the input of moving boundary from host
#define MOVING_BOUNDARY             FRONT


// TODO: handle the input of moving boundary from host
inline int is_moving_init(const int cell_type)
{
    return (cell_type & MOVING_BOUNDARY);
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
    return (cell_type & MOVING);
}

inline int is_store_macro(const int cell_type)
{
    return (is_fluid(cell_type) || is_moving(cell_type));
}

inline int is_collision(const int cell_type)
{
    return (is_fluid(cell_type) || is_moving(cell_type));
}

inline int is_bounceback(const int cell_type)
{
    return (is_boundary(cell_type) && !(cell_type & MOVING));
}

// inline int is_streaming(const int cell_type)
// {
//     return (is_fluid(cell_type) || is_moving(cell_type));
// }


// Bhatnagar-Gross-Kroop approximation collision operator
inline float compute_bgk(const float f, const float f_eq)
{
    return f + INV_TAU * (f_eq - f);
}

// inline void _print_map(__global int * map)
// {
//     barrier(CLK_GLOBAL_MEM_FENCE);
//     for (int z = 0; z < DIM_Z; ++z) {
//         for (int y = 0; y < DIM_Y; ++y) {
//             for (int x = 0; x < DIM_X; ++x) {
//                 const int cell_type = map[IDxyz(x, y, z)];
//                 int t = 0;
//                 if (is_fluid(cell_type))    t = 1;
//                 if (is_wall(cell_type))     t = 4;
//                 if (is_corner(cell_type))   t = 5;
//                 if (is_boundary(cell_type)) t = 3;
//                 if (is_moving(cell_type))   t = 2;

//                 printf("%d ", t);
//             }
//             printf("\n");
//         }
//         printf("\n");
//     }
//     printf("\n");
// }

#endif
