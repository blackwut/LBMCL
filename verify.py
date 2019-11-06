import argparse
import pyvista
import numpy
from sklearn.metrics import mean_squared_error


description = ('Calculate the Mean Squared Error (MSE) and '
               'the Max Absolute Error (MAE) '
               'of density (rho) and velocity (u) '
               'from two LBM simulation datasets (target and prediction).')

parser = argparse.ArgumentParser(description=description)
parser.add_argument('-i', '--iterations', type=int, required=True,
                    help='number of iterations in VTI dataset')
parser.add_argument('-e', '--every', type=int, required=True,
                    help='step number between two iterations')
parser.add_argument('-t', '--target_path', type=str, required=True,
                    help='path of target VTI files (format ldc.0.*.vti)')
parser.add_argument('-p', '--prediction_path', type=str, required=True,
                    help='path of prediction VTI files (format lbmcl.*.vti)')


args = parser.parse_args()
iterations = args.iterations
every = args.every
target_path = args.target_path
prediction_path = args.prediction_path

fill = '0'
width = int(numpy.log10(iterations)) + 1 if iterations > 0 else 1

print('{0:^{w}}  {1:^13}  {2:^13}  {3:^13}  {4:^13}'
      .format('#it', 'MSE_RHO', 'MSE_U', 'MAE_RHO', 'MAE_U', w=width))

for i in range(0, iterations + 1, every):
    target_name = '{p}/ldc.0.{i:{f}{w}}.vti'.format(p=target_path,
                                                    i=i,
                                                    f=fill,
                                                    w=width)
    prediction_name = '{p}/lbmcl.{i:{f}{w}}.vti'.format(p=prediction_path,
                                                        i=i,
                                                        f=fill,
                                                        w=width)
    target_data = pyvista.read(target_name)
    prediction_data = pyvista.read(prediction_name)

    target_rho = numpy.nan_to_num(target_data.point_arrays['rho'])
    prediction_rho = numpy.nan_to_num(prediction_data.point_arrays['rho'])
    target_v = numpy.nan_to_num(target_data.point_arrays['v'])
    prediction_v = numpy.nan_to_num(prediction_data.point_arrays['v'])

    mse_rho = mean_squared_error(target_rho, prediction_rho)
    mse_v = mean_squared_error(target_v, prediction_v)

    mae_rho = numpy.max(numpy.abs(target_rho - prediction_rho))
    mae_v = numpy.max(numpy.abs(target_v - prediction_v))
    print('{i:{f}{w}}:  {mse_rho:e}   {mse_v:e}   {rho_max:e}   {u_max:e}'
          .format(i=i, mse_rho=mse_rho, mse_v=mse_v,
                  rho_max=mae_rho, u_max=mae_v,
                  f='', w=width))
