#!/bin/bash

for run in `seq 0 10`; do
    echo "run $run"
    for count in 100 1000; do
        for size in 1 10 100 500 1000 5000 10000 50000 100000 500000 1000000; do
            echo -n " $count $size "
            ./message-size -m $size -n $count -q
        done
        echo ""
    done
    echo ""
    echo ""
done

