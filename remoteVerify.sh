#!/bin/bash

set -e

ITERATIONS=500
EVERY=20
DIM=8

if (($1 < 8 || $1 > 256)); then
    echo "Please enter a number in the range [8, 256]"
    exit -1;
fi

DIM=$1

case "$2" in
double)
    PRECISION=double
    ;;
single)
    PRECISION=single
    ;;
*)
    PRECISION=single
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

LBMCL_COMMAND="cd ~/LBMCL; "
LBMCL_COMMAND+="rm ./results/*; "
LBMCL_COMMAND+="make clean; "
LBMCL_COMMAND+="export DIM=$DIM; "
LBMCL_COMMAND+="export PRECISION=$(echo "$PRECISION" | tr '[:lower:]' '[:upper:]'); "
LBMCL_COMMAND+="export ITERATIONS=$ITERATIONS; "
LBMCL_COMMAND+="export EVERY=$EVERY; "
LBMCL_COMMAND+="make test;"

SAILFISH_DIM="$(($DIM - 2))"
SAILFISH_COMMAND="cd ~/sailfish;source activate.sh; "
SAILFISH_COMMAND+="rm ./results/*; "
SAILFISH_COMMAND+="examples/ldc_3d.py --precision=$PRECISION --max_iters=$ITERATIONS --every=$EVERY --output_format=vtk --output=results/ldc --visc=0.0089 -v --lat_nx=$SAILFISH_DIM --lat_ny=$SAILFISH_DIM --lat_nz=$SAILFISH_DIM;"

mkdir $A
mkdir $S
# echo "$LBMCL_COMMAND"
# echo "$SAILFISH_COMMAND"
ssh  aottimo@sangiovese.isti.cnr.it -t "$LBMCL_COMMAND"
ssh  aottimo@sangiovese.isti.cnr.it -t "$SAILFISH_COMMAND"
scp sangiovese.isti.cnr.it:~/LBMCL/results/lbmcl.*.vti $A
scp sangiovese.isti.cnr.it:~/sailfish/results/ldc.0.*.vti $S

python3 verify.py -i $ITERATIONS -e $EVERY -t $S -n $A

