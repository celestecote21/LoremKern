#!/usr/bin/bash

make

rmmod loremKern
insmod loremKern.ko

