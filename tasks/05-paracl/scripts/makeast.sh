#!/bin/sh

build/pclc -i $1 -a > ast.tmp
dot -Tpng ast.tmp > ast.png