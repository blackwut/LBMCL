#!/bin/bash

TEST=test32

A="/Volumes/RamDisk/lbmcl"
S="/Volumes/RamDisk/sailfish"

if [ -d $A ]; then
	rm -r $A
fi

if [ -d $S ]; then
	rm -r $S
fi

mkdir $A
mkdir $S
ssh aottimo@sangiovese.isti.cnr.it -t "cd ~/LBMCL;make $TEST"
ssh aottimo@sangiovese.isti.cnr.it -t "cd ~/sailfish;source activate.sh; ./$TEST.sh"
scp sangiovese.isti.cnr.it:~/LBMCL/results/* $A
scp sangiovese.isti.cnr.it:~/sailfish/results/* $S

if [ "$TEST" -eq "test" ]; then
	python3 verify.py -i10 -e1 -t $S -n $A
else
	python3 verify.py -i500 -e20 -t $S -n $A
fi

