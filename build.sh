#!/bin/bash
set -e

currDir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
srcDir=${currDir}/src
cd ${srcDir}

mkdir -p build
cd build
cmake ..
cmake --build .
cp ${currDir}/wav/sine_test.wav ${srcDir}/build/bin

cd ${srcDir}/build/bin
./caller &
./answerrer

sleep 10000
