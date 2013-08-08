#!/bin/bash
gcc -Wl,--export-dynamic -Wall -O1 -shared -fPIC autoAway.c -lXss -lX11 -o autoAway.so