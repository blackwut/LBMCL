#!/bin/bash

# User defined
HOST_FOLDER_LBMCL="/Volumes/RamDisk/lbmcl"
SSH_CONNECTION_LBMCL="ssh aottimo@sangiovese.isti.cnr.it"
SSH_FOLDER_LBMCL="~/LBMCL"
SCP_LBMCL="scp sangiovese.isti.cnr.it:$SSH_FOLDER_LBMCL/results/lbmcl.*.vti $HOST_FOLDER_LBMCL"

HOST_FOLDER_SAILFISH="/Volumes/RamDisk/sailfish"
SSH_CONNECTION_SAILFISH="ssh aottimo@sangiovese.isti.cnr.it"
SSH_FOLDER_SAILFISH="~/sailfish"
SCP_SAILFISH="scp aottimo@sangiovese.isti.cnr.it:$SSH_FOLDER_SAILFISH/results/ldc.*.vti $HOST_FOLDER_SAILFISH"


# Exit if an error occurs
set -e


# Help message
function print_help
{
    echo "USAGE: ./remoteVerify.sh  DIM  WORK_GROUP_SIZE  STRIDE  ITERATIONS  EVERY  [PRECISION]"
}


# Input checks
if [ -z "$1" ] || (("$1" < 8 || "$1" > 256)); then
    print_help
    echo "DIM: Please enter a number in the range [8, 256]"
    exit -1;
fi
DIM="$1"

if [ -z "$2" ] || (("$2" < 0 || "$2" > 256)); then
    print_help
    echo "WORK_GROUP_SIZE: Please enter a number in the range [1, 256]"
    exit -1;
fi
LWS="$2"

if [ -z "$3" ] || (("$3" < 0 || "$3" > 256)); then
    print_help
    echo "STRIDE: Please enter a number in the range [0, 256]"
    exit -1;
fi
STRIDE="$3"

if [ -z "$4" ] || (("$4" < 0)); then
    print_help
    echo "ITERATIONS: Please enter a number greater than 0"
    exit -1;
fi
ITERATIONS="$4"

if [ -z "$5" ] || (("$5" < 0 || "$5" > "$ITERATIONS")); then
    print_help
    echo "EVERY: Please enter a number in the range [1, $ITERATIONS]"
    exit -1;
fi
EVERY="$5"

case "$6" in
double)
    PRECISION="double"
    ;;
single)
    PRECISION="single"
    ;;
*)
    PRECISION="single"
    ;;
esac


# Create HOST folders if they don't exist. Otherwise remove *.vti files inside them
if [ ! -d "$HOST_FOLDER_SAILFISH" ]; then
    mkdir "$HOST_FOLDER_SAILFISH"
else
    rm -f "$HOST_FOLDER_SAILFISH"/ldc.0.*.vti
fi

if [ ! -d "$HOST_FOLDER_LBMCL" ]; then
    mkdir "$HOST_FOLDER_LBMCL"
else
    rm -f "$HOST_FOLDER_LBMCL"/lbmcl.*.vti
fi

# Prepare commands
LBMCL_COMMAND="cd $SSH_FOLDER_LBMCL; "
LBMCL_COMMAND+="rm ./results/*.vti; "
LBMCL_COMMAND+="make clean; "
LBMCL_COMMAND+="make; "
LBMCL_COMMAND+="./lbmcl -P0 -D0 -d $DIM -n0.0089 -u0.05 -i $ITERATIONS -e $EVERY -w $LWS -s $STRIDE -o -v ./results"
if [ "$PRECISION" = "double" ]; then
    LBMCL_COMMAND+=" -F"
fi

SAILFISH_DIM="$(($DIM - 2))"
SAILFISH_COMMAND="cd ~/sailfish; "
SAILFISH_COMMAND+="source project/bin/activate; "
SAILFISH_COMMAND+="unset PYTHONPATH; "
SAILFISH_COMMAND+="export PYTHONPATH=$PWD:$PYTHONPATH; "
SAILFISH_COMMAND+="rm ./results/*.vti; "
SAILFISH_COMMAND+="examples/ldc_3d.py --precision=$PRECISION --max_iters=$ITERATIONS --every=$EVERY --perf_stats_every=$EVERY --output_format=vtk --output=results/ldc --visc=0.0089 -v --lat_nx=$SAILFISH_DIM --lat_ny=$SAILFISH_DIM --lat_nz=$SAILFISH_DIM;"


# Execute commands
$SSH_CONNECTION_LBMCL -t "$LBMCL_COMMAND"
$SSH_CONNECTION_SAILFISH -t "$SAILFISH_COMMAND"
$SCP_LBMCL
$SCP_SAILFISH


# Verify results
python3 verify.py -i "$ITERATIONS" -e "$EVERY" -t "$HOST_FOLDER_SAILFISH" -p "$HOST_FOLDER_LBMCL"
