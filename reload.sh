#!/usr/bin/bash

make

rmmod hello_world
insmod hello_world.ko

