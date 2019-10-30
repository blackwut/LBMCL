#!/bin/bash

set -e

ITERATIONS=500
EVERY=20
DIM=8

if (($1 < 8 || $1 > 256)); then
    echo "Please enter a number in the range [8, 256]"
    exit -1;
fi

case "$2" in
double)
    PRECISION=double
    ;;
single)
    TEST=single
    ;;
*)
    TEST=single
    ;;
esac


A="/Volumes/RamDisk/lbmcl"
S="/Volumes/RamDisk/sailfish"

if [ -d $A ]; then
    rm -r $A
fi

if [ -d $S ]; then
    rm -r $S
fi

LBMCL_COMMAND=("cd ~/LBMCL;" \
              "rm ./results/*;" \
              "make clean;" \
              "export PRECISION=$PRECISION" \
              "export ITERATIONS=$ITERATIONS" \
              "export EVERY=$EVERY" \
              "make test")

SAILFISH_COMMAND=("cd ~/sailfish;source activate.sh;" \
                 "rm ./results/*;" \
                 "examples/ldc_3d.py --precision=$PRECISION --max_iters=$ITERATIONS --every=$EVERY --output_format=vtk --output=results/ldc --visc=0.0089 -v --lat_nx=30 --lat_ny=30 --lat_nz=30")

mkdir $A
mkdir $S
ssh aottimo@sangiovese.isti.cnr.it -t $LBMCL_COMMAND
ssh aottimo@sangiovese.isti.cnr.it -t $SAILFISH_COMMAND
scp sangiovese.isti.cnr.it:~/LBMCL/results/lbmcl.*.vti $A
scp sangiovese.isti.cnr.it:~/sailfish/results/ldc.0.*.vti $S

python3 verify.py -i $ITERATIONS -e $EVERY -t $S -n $A

