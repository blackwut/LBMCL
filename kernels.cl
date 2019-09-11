#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable

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


// Data Layout CSoA (Collected Structures of Array)
// Example of 19 array values with (stride = 4)
// 0,0,0,0 | 1,1,1,1 | ... | 19,19,19,19 | 0,0,0,0 | 1,1,1,1 | ...

#define D   3
#define Q   19

#define DELTA_X 1.0f            // Grid cell width
#define DELTA_T 1.0f

#define C2_INV (DELTA_T / DELTA_X)
#define C4_INV (C2_INV * C2_INV)

#ifndef DIM
#define DIM      32
#endif

#define DIM_X    DIM
#define DIM_Y    DIM
#define DIM_Z    DIM

#define CELL_INITIAL_DENSITY    1.0f
#define CELL_INITIAL_VELOCITY   0.05f


#define STRIDE              32 // for performance purpose, must be equal to the blockSize.x (that must be a power of 2)// TODO: verify it
#define MOD_STRIDE(x)       ((x) & (STRIDE - 1)) // and-bitwise modulo optimization since STRIDE is a power of 2
#define IDxyz(x, y, z)      ((x) + ((y) * DIM_X) + ((z) * DIM_X * DIM_Y))
#define IDxyzw(id, w)       ((((id) / STRIDE) * Q + (w)) * STRIDE + MOD_STRIDE(id)) // Taken from "GPU Data Access on Complex Geometries for D3Q19 Lattice Boltzmann Method" equation(3)

#define VISCOSITY 0.0089f
#define TAU  ((3.0f * VISCOSITY) / (DELTA_T * DELTA_X * DELTA_X) + 0.5f) // solve [v = (2 * tau - 1)/6 * e^2 dt] for tau
#define INV_TAU (1.0f / TAU)


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

#define E10_X    1
#define E10_Y    -1
#define E10_Z    0

#define E11_X    1
#define E11_Y    0
#define E11_Z    -1

#define E12_X    0
#define E12_Y    1
#define E12_Z    -1

#define E13_X    -1
#define E13_Y    0
#define E13_Z    -1

#define E14_X    0
#define E14_Y    -1
#define E14_Z    -1

#define E15_X    1
#define E15_Y    0
#define E15_Z    1

#define E16_X    0
#define E16_Y    1
#define E16_Z    1

#define E17_X    -1
#define E17_Y    0
#define E17_Z    1

#define E18_X    0
#define E18_Y    -1
#define E18_Z    1

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


#define FLUID               (1 <<  0)
#define NO_SLIP             (1 <<  1)
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


__kernel
void init(__global float * f_stream, __global float * f_collide, __global float * rho, __global float4 * u, __global int * type)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int z = get_global_id(2);
    const int id = IDxyz(x, y, z);

    rho[id] = 0.0f;
    u[id] = (float4)(0.0f);

    int cell_type = FLUID;
    if (x == 1)           cell_type |= LEFT;
    if (x == (DIM_X - 1)) cell_type |= RIGHT;
    if (y == 1)           cell_type |= BOTTOM;
    if (y == (DIM_Y - 1)) cell_type |= TOP;
    if (z == 1)           cell_type |= BACK;
    if (z == (DIM_Z - 1)) cell_type |= FRONT;
    type[id] = cell_type;

    float rho_new = CELL_INITIAL_DENSITY;
    float4 u_new = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    if (z == DIM_Z - 1) {
        u_new = (float4)(CELL_INITIAL_VELOCITY, 0.0f, 0.0f, 0.0f);
    } 

    const float u2 = dot(u_new, u_new);

#undef  UNROLL_X
#define UNROLL_X(i)                                                                                                                \
    {                                                                                                                              \
        const float eu = dot(u_new, (float4)(E##i##_X, E##i##_Y, E##i##_Z, 0.0f));                                                 \
        const int index = IDxyzw(id, i);                                                                                           \
        f_collide[index] = rho_new * OMEGA_##i * (1.0f + (3.0f * eu * C2_INV) + (4.5f * eu * eu * C4_INV) - (1.5f * u2 * C2_INV)); \
    }
    UNROLL_19();

#undef  UNROLL_X
#define UNROLL_X(i)                     \
    {                                   \
        f_stream[IDxyzw(id, i)] = 0.0f; \
    }
    UNROLL_19();
}


// push-out vs pull-in (thesis "Thesis_for_web_new.pdf" page 87 - "REAL-TIME SIMULATION OF INDOOR AIR FLOW USING THE LATTICE BOLTZMANN METHOD ON GRAPHICS PROCESSING UNIT")

__kernel
void streaming(__global float * f_stream, __global const float * f_collide)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int z = get_global_id(2);
    const int id = IDxyz(x, y, z);

    if ((0 < x) && x < (DIM_X - 1) && (0 < y) && y < (DIM_Y - 1) && (0 < z) && z < (DIM_Z - 1)) {
#undef  UNROLL_X
#define UNROLL_X(i)                                                             \
        {                                                                       \
            const int index = IDxyz(x + E##i##_X, y + E##i##_Y, z + E##i##_Z);  \
            f_stream[IDxyzw(id, i)] = f_collide[IDxyzw(index, i)];              \
        }
        UNROLL_19();
    }
}


// Bhatnagar-Gross-Kroop approximation collision operator
inline float compute_bgk(const float f, const float f_eq)
{
    return f - (f - f_eq) * INV_TAU;
}


__kernel
void collision(__global float * f_collide)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int z = get_global_id(2);
    const int id = IDxyz(x, y, z);

    if ((0 < x) && x < (DIM_X - 1) && (0 < y) && y < (DIM_Y - 1) && (0 < z) && z < (DIM_Z - 1)) {
        float rho_new = 0.0f;
        float4 u_new = (float4)(0.0f, 0.0f, 0.0f, 0.0f);

#undef  UNROLL_X
#define UNROLL_X(i)                                                     \
        {                                                               \
            const float f = f_collide[IDxyzw(id, i)];                   \
            rho_new += f;                                               \
            u_new += (float4)(E##i##_X, E##i##_Y, E##i##_Z, 0.0f) * f;  \
        }
        UNROLL_19();

        u_new /= rho_new;

        const float u2 = dot(u_new, u_new);

#undef  UNROLL_X
#define UNROLL_X(i)                                                                                                                     \
        {                                                                                                                               \
            const float eu = dot(u_new, (float4)(E##i##_X, E##i##_Y, E##i##_Z, 0.0f));                                                  \
            const float f_eq = rho_new * OMEGA_##i * (1.0f + (3.0f * eu * C2_INV) + (4.5f * eu * eu * C4_INV) - (1.5f * u2 * C2_INV));  \
            const int index = IDxyzw(id, i);                                                                                            \
            f_collide[index] = compute_bgk(f_collide[index], f_eq);                                                                     \
        }
        UNROLL_19();
    }
}

// ZOU - HE (thesis "Thesis_for_web_new.pdf" page 66 - "REAL-TIME SIMULATION OF INDOOR AIR FLOW USING THE LATTICE BOLTZMANN METHOD ON GRAPHICS PROCESSING UNIT")
__kernel
void updateBoundary(__global float * f, __global const int * type)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int z = get_global_id(2);
    const int id = IDxyz(x, y, z);


    // Full Bounce Back
    if ((0 < x) && x < (DIM_X - 1) && (0 < y) && y < (DIM_Y - 1) && (0 < z) && z < (DIM_Z - 1)) {
        float temp;
#undef UNROLL_X
#define UNROLL_X(i)                                     \
        {                                               \
            temp = f[IDxyzw(id, S_##i)];                \
            f[IDxyzw(id, S_##i)] = f[IDxyzw(id, i)];    \
            f[IDxyzw(id, i)] = temp;                    \
        }
        UNROLL_HALF_19();
    }
}

__kernel
void aggregateData(__global float * rho, __global float4 * u, __global const float * f_collide)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    const int z = get_global_id(2);
    const int id = IDxyz(x, y, z);

    float rho_new = 0.0f;
    float4 u_new = 0.0f;
#undef  UNROLL_X
#define UNROLL_X(i)                                                 \
    {                                                               \
        const float f = f_collide[IDxyzw(id, i)];                   \
        rho_new += f;                                               \
        u_new += (float4)(E##i##_X, E##i##_Y, E##i##_Z, 0.0f) * f;  \
    }
    UNROLL_19();

    rho[id] = rho_new;
    u[id] = u_new / rho_new;
}
