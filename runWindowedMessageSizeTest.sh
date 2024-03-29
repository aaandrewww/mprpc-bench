#!/bin/bash

for run in `seq 0 10`; do
    for count in 1000000 10000000 100000000 1000000000; do
        # for size in 0; do
        size=0
            for window in 10000 50000 100000 500000 1000000 10000000 100000000; do
                if [[ "$window" -le "$count" ]]; then
                    echo -e -n "$count\t$size\t$window\t"
                    ./windowed-message-size -m $size -n $count -w $window -q
                fi
            done
        # done
    done
done
