#!/bin/bash

for run in `seq 0 10`; do
    for count in 1000000 10000000 50000000 100000000; do
        for size in 0 10 100 1000; do
            for window in 10 100 1000 10000 100000 1000000; do
                if [[ "$window" -le "$size" ]]; then
                    echo -e -n "$count\t$size\t$window\t"
                    ./windowed-message-size -m $size -n $count -w $window -q
                fi
            done
        done
    done
done
