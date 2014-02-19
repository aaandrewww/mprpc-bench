set -m

f=$1
iterations=$2
ff=`expr $f + $f + 1`

for i in $(eval echo {1..$ff}); do
    j=`expr 1233 + $i`;
    ./paxos -a -q -p $j -f $f &
done
sleep 1
time ./paxos -r -q -p 1234 -f $f -i $iterations

killall paxos