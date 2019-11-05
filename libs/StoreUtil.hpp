#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "common.h"

#define DUMP_PRECISION     6
#define VTK_PRECISION     16


static size_t number_of_digits(const size_t val)
{
    return (val > 0) ? (int)log10((double)val) + 1 : 1;
}


static void storeMap(const std::string & dump_path,
                     const int * map,
                     const size_t dim)
{
    std::stringstream filenameBuilder;
    filenameBuilder << dump_path << "/map.dump";

    std::ofstream dump;
    dump.open(filenameBuilder.str());

    dump << "# FLUID       1" << std::endl
         << "# MOVING      2" << std::endl
         << "# BOUNDARY    3" << std::endl
         << "# WALL        4" << std::endl
         << "# CORNER      5" << std::endl
         << std::endl;

    for (size_t z = 0; z < dim; ++z) {
        for (size_t y = 0; y < dim; ++y) {
            for (size_t x = 0; x < dim; ++x) {
                const size_t cell_type = map[IDxyzDIM(x, y, z, dim)];

                int val = 0;
                if (is_fluid(cell_type))    val = 1;
                if (is_moving(cell_type))   val = 2;
                if (is_boundary(cell_type)) val = 3;
                if (is_wall(cell_type))     val = 4;
                if (is_corner(cell_type))   val = 5;
                dump << val << " ";
            }
            dump << std::endl;
        }
        dump << std::endl;
    }
    dump << std::endl;

    dump.close();
}


static void storeF(const std::string & dump_path,
                   const real_t * f,
                   const size_t dim,
                   const size_t stride,
                   const size_t iteration,
                   const size_t iterations)
{

    const size_t it_digits = number_of_digits(iterations);

    std::stringstream filenameBuilder;
    filenameBuilder << dump_path << "/f_" << std::setw(it_digits) << std::setfill('0') << iteration << ".dump";

    // (xxx,yyy,zzz) 
    // 1 + D + 1 + D + 1 + D + 1 + 1
    const size_t dim_digits = number_of_digits(dim);
    const size_t coord_spaces = dim_digits * 3 + 5;

    std::ofstream dump;
    dump.open(filenameBuilder.str());

    for (size_t z = 0; z < dim; ++z) {
        for (size_t y = 0; y < dim; ++y) {
            for (size_t s = 0; s < coord_spaces; ++s) {
                dump << " ";
            }
            for (size_t q = 0; q < Q; ++q) {
                dump << std::setw(DUMP_PRECISION + 2) << q << " ";
            }
            dump << std::endl;

            for (size_t x = 0; x < dim; ++x) {
                const size_t index = IDxyzDIM(x, y, z, dim);
                dump << std::setw(dim_digits) << "(" << x << "," << y << "," << z << ") ";
                for (size_t q = 0; q < Q; ++q) {
                    dump << std::fixed << std::setw(DUMP_PRECISION + 2) << std::setprecision(DUMP_PRECISION) << f[IDxyzqDIM(index, q, Q, stride)] << " ";
                }
                dump << std::endl;
            }
            dump << std::endl;
        }
        dump << std::endl;
    }
    dump << std::endl;

    dump.close();
}


static void storeVtk(const std::string & vtk_path,
                     const real_t * rho_val,
                     const real_t * u_val,
                     const size_t dim,
                     const size_t iteration,
                     const size_t iterations)
{
    const size_t it_digits = number_of_digits(iterations);

    std::stringstream filenameBuilder;
    filenameBuilder << vtk_path << "/lbmcl." << std::setw(it_digits) << std::setfill('0') << iteration << ".vti";


    const size_t from = 1;
    const size_t to = dim - 1;
    const size_t extent = to - from - 1;
#if FP_DOUBLE
    const std::string type = "Float64";
#else
    const std::string type = "Float32";
#endif

    std::ofstream vtk;
    vtk.open(filenameBuilder.str());

    vtk << "<?xml version=\"1.0\"?>\n" 
        << "<VTKFile type=\"ImageData\" version=\"0.1\" byte_order=\"LittleEndian\" header_type=\"UInt64\">\n" 
        << "  <ImageData WholeExtent=\"0 " << extent << " 0 " << extent << " 0 " << extent << "\" Origin=\"0 0 0\" Spacing=\"1 1 1\">\n"
        << "    <Piece Extent=\"0 " << extent << " 0 " << extent << " 0 " << extent << "\">\n"
        << "      <PointData Scalars=\"rho\">\n"
        << "        <DataArray type=\"" << type << "\" Name=\"rho\" NumberOfComponents=\"1\" format=\"ascii\">\n";

    for (size_t z = from; z < to; ++z) {
        for (size_t y = from; y < to; ++y) {
            for (size_t x = from; x < to; ++x) {
                const real_t val = rho_val[IDxyzDIM(x, y, z, dim)];
                vtk << std::scientific << std::setprecision(VTK_PRECISION) << val << " ";
            }
            vtk << "\n";
        }
    }

    vtk << "        </DataArray>\n"
        << "        <DataArray type=\"" << type << "\" Name=\"v\" NumberOfComponents=\"3\" format=\"ascii\">\n";

    for (size_t z = from; z < (to); ++z) {
        for (size_t y = from; y < (to); ++y) {
            for (size_t x = from; x < (to); ++x) {
                const size_t id = IDxyzDIM(x, y, z, dim);
                const real_t val_x = u_val[IDux(id)];
                const real_t val_y = u_val[IDuy(id)];
                const real_t val_z = u_val[IDuz(id)];
                vtk << std::scientific << std::setprecision(VTK_PRECISION) << val_x << " "
                    << std::scientific << std::setprecision(VTK_PRECISION) << val_y << " "
                    << std::scientific << std::setprecision(VTK_PRECISION) << val_z << " ";
            }
            vtk << "\n";
        }
    }

    vtk << "        </DataArray>\n"
        << "      </PointData>\n"
        << "    </Piece>\n"
        << "  </ImageData>\n"
        << "</VTKFile>\n";

    vtk.close();
}
