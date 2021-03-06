import argparse
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


description = 'Convert rho and u 3D matrix from VTI to TXT format.'
parser = argparse.ArgumentParser(description=description)
parser.add_argument('-i', '--iterations', type=int, required=True,
                    help='number of iterations of VTI collection')
parser.add_argument('-e', '--every', type=int, required=True,
                    help='step number between two iterations')
parser.add_argument('-p', '--path', type=str, required=True,
                    help='path of VTI files')


args = parser.parse_args()
iterations = args.iterations
every = args.every
path = args.path

fill = '0'
width = int(numpy.log10(iterations)) + 1 if iterations > 0 else 1

for i in range(0, iterations + 1, every):
    name = '{p}/ldc.0.{i:{f}{w}}'.format(p=path, i=i, f=fill, w=width)
    data = pyvista.read(name + '.vti')
    rho = data['rho']
    v = data['v']
    rho_dim = data.dimensions
    v_dim = data.dimensions
    v_dim[2] *= 3
    rho_reshape = numpy.reshape(rho, rho_dim)
    v_reshape = numpy.reshape(v, v_dim)

    with open(name + '.txt', 'w') as f:
        print_matrix3D(f, rho_reshape, rho_dim)
        print_matrix3D(f, v_reshape, v_dim)
        f.close()
