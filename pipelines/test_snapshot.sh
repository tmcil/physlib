#!/bin/bash

for dir in tests/snapshot/reference/*/; do
    uuid=$(basename "$dir")
    echo "Running executor test for $uuid"
    python3 -m executor.executor_test "$uuid"

    if [ $? -ne 0 ]; then
        echo "Test failed for $uuid"
        exit 1
    fi
done

echo "All executor tests passed."