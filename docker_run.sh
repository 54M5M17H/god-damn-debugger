#!/bin/bash

## This script launches a Docker container, mounting this project inside

docker run \
	-it --privileged \
	--name gdbugger \
	--mount type=bind,source="$(pwd)"/,target=/gdbugger \
	ubuntu:latest
