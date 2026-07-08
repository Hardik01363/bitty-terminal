#!/bin/bash
mkdir -p bin
gcc -o bin/bitty bitty.c \
    $(pkg-config --cflags freetype2 harfbuzz fontconfig) \
    -lleif -lrunara -lfreetype -lharfbuzz -lfontconfig -lglfw -lGL -lm -lX11 -lXrender
