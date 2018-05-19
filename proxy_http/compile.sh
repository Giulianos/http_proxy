#!/bin/bash

mkdir build 2> /dev/null
cd build && cmake ../../
make
if [ $? -eq 0 ];
then
  echo ''
  echo 'Executable file was written to build/target/proxy_http'
fi
