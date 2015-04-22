#!/bin/bash
exec > /dev/null
mkdir -p build
pushd build
c++ ../main.cpp -o rakshaza -g `sdl2-config --cflags --libs`
popd
