import pyvista
import numpy
import argparse
from sklearn.metrics import mean_squared_error


description = 'Calculate MSE and MAX ERROR of rho and u from two VTI files.'
parser = argparse.ArgumentParser(description=description)
parser.add_argument('-i', '--iterations', type=int, required=True,
                    help='number of iterations of VTI collection')
parser.add_argument('-e', '--every', type=int, required=True,
                    help='step number of iterations')
parser.add_argument('-t', '--path_true', type=str, required=True,
                    help='path containing VTI files of true values (file format ldc.0.****.vti)')
parser.add_argument('-n', '--path_new', type=str, required=True,
                    help='path containing VTI files of new values (file format lbmcl.****.vti)')


args = parser.parse_args()
# s -> sailfish
# a -> lbmcl
iterations = args.iterations
every = args.every
base_s = args.path_true
base_a = args.path_new

fill = '0'
width = int(numpy.log10(iterations)) + 1

print('{0:^{w}}  {1:^13}  {2:^13}  {3:^13}  {4:^13}'
      .format('#it', 'MSE_RHO', 'MSE_U', 'MAX_ERR_RHO', 'MAX_ERR_U', w=width))

for i in range(0, iterations + 1, every):
    name_s = '{path_true}/ldc.0.{num:{f}{w}}.vti'.format(path_true=base_s,
                                                         num=i,
                                                         f=fill,
                                                         w=width)
    name_a = '{path_new}/lbmcl.{num:{f}{w}}.vti'.format(path_new=base_a,
                                                        num=i,
                                                        f=fill,
                                                        w=width)
    s = pyvista.read(name_s)
    a = pyvista.read(name_a)

    rho_s = numpy.nan_to_num(s.point_arrays['rho'])
    rho_a = numpy.nan_to_num(a.point_arrays['rho'])
    u_s = numpy.nan_to_num(s.point_arrays['v'])
    u_a = numpy.nan_to_num(a.point_arrays['v'])

    rho_mse = mean_squared_error(rho_s, rho_a)
    u_mse = mean_squared_error(u_s, u_a)

    rho_max_err = numpy.max(numpy.abs(rho_s - rho_a))
    u_max_err = numpy.max(numpy.abs(u_s - u_a))
    print('{num:{f}{w}}:  {rho_mse:e}   {u_mse:e}   {rho_max:e}   {u_max:e}'
          .format(num=i, rho_mse=rho_mse, u_mse=u_mse, rho_max=rho_max_err,
                  u_max=u_max_err, f='', w=width))
