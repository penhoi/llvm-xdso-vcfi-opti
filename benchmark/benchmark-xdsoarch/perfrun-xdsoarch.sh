#!/bin/bash
#bash -x perfrun.sh 1000000

NCYCLES=$1
HEREDIR=`pwd`

# Loop the entire benchmark block 5 times
for run in {1..5}; do
    echo "=== Run $run ==="
    
    cd $HEREDIR/calculator
    /usr/bin/time -- ./main $(($NCYCLES * 100))

    cd $HEREDIR/calculator
    /usr/bin/time -- ./main $(($NCYCLES * 1000))

    cd $HEREDIR/calculator
    /usr/bin/time -- ./main $(($NCYCLES * 10000))


    cd $HEREDIR/logger
    /usr/bin/time -- ./main $(($NCYCLES * 1)) > /dev/null

    cd $HEREDIR/logger
    /usr/bin/time -- ./main $(($NCYCLES * 10)) > /dev/null


    cd $HEREDIR/imagefilter
    /usr/bin/time -- ./main $(($NCYCLES / 10))
    
    echo "=== Run $run completed ==="
    echo ""
done

cd $HEREDIR
