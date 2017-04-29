#!/bin/bash
make clean
make
./Application testcases/singlefailure.conf
