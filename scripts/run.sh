#!/bin/bash

source ./change_dir.sh
change_dir

./dist/gdd $(pwd)/test/target.o
