import sys
import pyvista
import numpy


def print_matrix3D(f, mat, dim):
    for x in range(dim[0]):
        for y in range(dim[1]):
            for z in range(dim[2]):
                f.write("%e " % mat[x][y][z])
            f.write("\n")
        f.write("\n")
    f.write("\n")


n = int(sys.argv[1])

for i in range(0, n):
    s = pyvista.read('/Volumes/RamDisk/ldc.0.%02d.vti' % i)
    rho = s['rho']
    v = s['v']
    rho_dim = s.dimensions
    v_dim = s.dimensions
    v_dim[2] *= 3
    rho_reshape = numpy.reshape(rho, rho_dim)
    v_reshape = numpy.reshape(v, v_dim)

    with open('/Volumes/RamDisk/ldc.0.%02d.txt' % i, 'w') as f:
        print_matrix3D(f, rho_reshape, rho_dim)
        print_matrix3D(f, v_reshape, v_dim)
        f.close()
