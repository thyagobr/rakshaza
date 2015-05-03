#!/bin/bash
exec > /dev/null
mkdir -p build
pushd build
gcc ../main.cpp -o rakshaza -g -Wall -DRAKSHAZA_INTERNAL=1 -DRAKSHAZA_DEBUG=1 `sdl2-config --cflags --libs` -lm
popd
