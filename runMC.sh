set -m

clients=$1
messages=`expr 10000000 / $clients`

./multi-client -l -p 1234 &
pid=$!
disown %1
for i in $(eval echo {1..$clients}); do
    ./multi-client -c -q -n $messages &
done
wait

kill $pid