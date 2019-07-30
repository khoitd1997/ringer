#!/bin/bash
set -e

currDir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd ${currDir}

mkdir -p build
cd build
cmake ..
cmake --build .
cp ${currDir}/wav/sine_test.wav ${currDir}/build/bin

cd ${currDir}/build/bin
./caller &
./answerrer

sleep 10000
