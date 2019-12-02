#!/bin/bash

BENCHMARK_DIR="./benchmarks"
LOG="$BENCHMARK_DIR/stats.csv"
BENCHMARK="$BENCHMARK_DIR/benchmark.csv"


PRECISION=single
PLATFORM=0
DEVICE=2

VISCOSITY=0.0089
VELOCITY=0.05
ITERATIONS=50
EVERY=0

# Exit if an error occurs
set -e

_dim=(
    8
    16
    32
    64
    128
    256
)

_lws=(
    "8,8,1"
    "8,8,4"
    "8,8,8"
    "16,8,8"
    "16,16,1"
    "16,16,4"
    "32,32,1"
)

_stride=(
    1
    8
    16
    32
    64
    128
    256
)

if [ -e $LOG ]; then
    rm $LOG
fi

if [ -e $BENCHMARK ]; then
    rm $BENCHMARK
fi

make clean
make


for d in "${_dim[@]}"; do
    for l in "${_lws[@]}"; do
        if ((l <= d)); then
            for s in "${_stride[@]}"; do
                for k in `seq 1 11`; do
                    if [ "$PRECISION" = "single" ]; then
                        ./lbmcl -P $PLATFORM -D $DEVICE -d $d -n $VISCOSITY -u $VELOCITY -i $ITERATIONS -e $EVERY -w $l -s $s -o 2>> $LOG
                    else
                        ./lbmcl -P $PLATFORM -D $DEVICE -d $d -n $VISCOSITY -u $VELOCITY -i $ITERATIONS -e $EVERY -w $l -s $s -o -F 2>> $LOG
                    fi
                done
            done
        fi
    done
done

# Benchmark for SoA where STRIDE = DIM
for d in "${_dim[@]}"; do
    for l in "${_lws[@]}"; do
        if ((l <= d)); then
            for k in `seq 1 11`; do
                if [ "$PRECISION" = "single" ]; then
                    ./lbmcl -P $PLATFORM -D $DEVICE -d $d -n $VISCOSITY -u $VELOCITY -i $ITERATIONS -e $EVERY -w $l -s $(($d * $d * $d)) -o 2>> $LOG
                else
                    ./lbmcl -P $PLATFORM -D $DEVICE -d $d -n $VISCOSITY -u $VELOCITY -i $ITERATIONS -e $EVERY -w $l -s $(($d * $d * $d)) -o -F 2>> $LOG
                fi
            done
        fi
    done
done

DISCRIMINATOR='$9'

cat $LOG | grep -v 'beignet-opencl-icd:\|(If you' | awk -F\; '{

    i = $1";"$2";"$3";"$6";"$7;

    found = 0
    for (n in names) {
        if (i == n) found = 1
    }

    if (!found) {
        min_val[i] = '"$DISCRIMINATOR"'
        max_val[i] = '"$DISCRIMINATOR"'
        min_total_time[i]    =  $9
        min_kernels_time[i]  = $10
        min_total_mlups[i]   = $11
        min_kernels_mlups[i] = $12
        max_total_time[i]    =  $9
        max_kernels_time[i]  = $10
        max_total_mlups[i]   = $11
        max_kernels_mlups[i] = $12
    }

    count[i]+=1
    names[i]=i
    total_time[i]  +=$9
    kernels_time[i]+=$10
    total_mlups[i] +=$11
    kernel_mlups[i]+=$12

    min_update = min_val[i] > '"$DISCRIMINATOR"'
    max_update = max_val[i] < '"$DISCRIMINATOR"'

    if (min_update) {
        min_total_time[i]    =  $9
        min_kernels_time[i]  = $10
        min_total_mlups[i]   = $11
        min_kernels_mlups[i] = $12
        min_val[i]           = '"$DISCRIMINATOR"'
    }

    if (max_update) {
        max_total_time[i]    =  $9
        max_kernels_time[i]  = $10
        max_total_mlups[i]   = $11
        max_kernels_mlups[i] = $12
        max_val[i]           = '"$DISCRIMINATOR"'
    }
}
END {

    for(i in names) {
        N = count[i] - 2

        total_time[i]   -= min_total_time[i]
        kernels_time[i] -= min_kernels_time[i]
        total_mlups[i]  -= min_total_mlups[i]
        kernel_mlups[i] -= min_kernels_mlups[i]

        total_time[i]   -= max_total_time[i]
        kernels_time[i] -= max_kernels_time[i]
        total_mlups[i]  -= max_total_mlups[i]
        kernel_mlups[i] -= max_kernels_mlups[i]

        print i";"total_time[i]/N";"kernels_time[i]/N";"total_mlups[i]/N";"kernel_mlups[i]/N
    }
}' > "$BENCHMARK"
