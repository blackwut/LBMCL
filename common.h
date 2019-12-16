#ifndef COMMON_H
#define COMMON_H

#define D                               3
#define Q                               19

#define NONE                            0x00000000 //(0) 
#define FLUID                           0x00000001 //(1 << 0)
#define MOVING                          0x00000002 //(1 << 1)
#define CORNER                          0x00000004 //(1 << 2)
#define WALL                            0x00000008 //(1 << 3)

#define LEFT                            0x00000010 //(1 << 4)
#define RIGHT                           0x00000020 //(1 << 5)
#define TOP                             0x00000040 //(1 << 6)
#define BOTTOM                          0x00000080 //(1 << 7)
#define FRONT                           0x00000100 //(1 << 8)
#define BACK                            0x00000200 //(1 << 9)

#define MOVING_BOUNDARY         FRONT


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
