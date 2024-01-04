#!/bin/bash

PROXY_HOST="localhost"
PROXY_PORT="8069"
URL="http://ascii.textfiles.com/"
CONCURRENCY=10
REQUESTS=100
CURL_OPTIONS="--http1.0 --proxy1.0 ${PROXY_HOST}:${PROXY_PORT}"

make-request() {
    NUM=$1

    time=$(curl -s -w "%{time_total}\n" -o results/result-${NUM} ${CURL_OPTIONS} ${URL})
    echo "${NUM}: ${time}s"
}

rm -r results
mkdir -p results

export -f make-request
export URL CURL_OPTIONS
seq ${REQUESTS} | xargs -P ${CONCURRENCY} -I {} bash -c 'make-request "$@"' _ {}

echo -ne "\nComparing results: "
for i in `seq ${REQUESTS}`; do
    if ! diff results/result-1 results/result-$i -q; then
        exit 1
    fi
done

echo OK
