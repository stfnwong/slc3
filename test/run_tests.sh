#!/bin/bash
# Run unit tests

TEST_DIR="./bin/test"

for t in ${TEST_DIR}/*; do
    ./$t || rc=$?
    if [[ rc -ne 0 ]] ; then
        exit $rc
    fi
done
