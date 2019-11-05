#ifndef COMMON_H
#define COMMON_H

// FP_SINGLE or FP_DOUBLE to set the simulation in float or double precision
#ifdef FP_DOUBLE
typedef double real_t;
#else
typedef float real_t;
#endif

#define D                               3
#define Q                               19

#define IDxyzqDIM(id, q, dim, stride)   (((id) / (stride)) * (dim) + q) * (stride) + ((id) & ((stride) - 1))
#define IDxyzDIM(x, y, z, dim)          ((x) + ((y) * (dim)) + ((z) * (dim) * (dim)))
#define IDux(id)                        ((id) * D + 0)
#define IDuy(id)                        ((id) * D + 1)
#define IDuz(id)                        ((id) * D + 2)


#define NONE                            (0)
#define FLUID                           (1 << 0)
#define MOVING                          (1 << 1)
#define CORNER                          (1 << 2)
#define WALL                            (1 << 3)
        
#define LEFT                            (1 << 4)
#define RIGHT                           (1 << 5)
#define TOP                             (1 << 6)
#define BOTTOM                          (1 << 7)
#define FRONT                           (1 << 8)
#define BACK                            (1 << 9)

// TODO: handle the value of moving boundary from host
#define MOVING_BOUNDARY         FRONT


// TODO: handle the value of moving boundary from host
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

#endif
