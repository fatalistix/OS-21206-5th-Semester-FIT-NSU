#!/bin/bash

PROXY_HOST="localhost"
PROXY_PORT="8069"
URL="http://ascii.textfiles.com/"
REQUESTS=10
CURL_OPTIONS="--http1.0 --proxy1.0 ${PROXY_HOST}:${PROXY_PORT}"

make-request() {
    NUM=$1

    time=$(curl -s -w "%{time_total}\n" -o results/result-${NUM} ${CURL_OPTIONS} ${URL})
    echo "${NUM}: ${time}s"
}

rm -r results
mkdir -p results

for i in `seq ${REQUESTS}`; do
    make-request ${i}
done

echo -e "\nWaiting 10s for cache invalidation:\n"
sleep 10

for i in `seq $(( REQUESTS + 1)) $(( REQUESTS * 2))`; do
    make-request ${i}
done

echo -ne "\nComparing results: "
for i in `seq $(( REQUESTS * 2))`; do
    if ! diff results/result-1 results/result-$i -q; then
        exit 1
    fi
done

echo OK
