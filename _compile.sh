#!/bin/bash

aarch64-linux-gnu-gcc test_binary.c -o test_binary -static

./test_binary