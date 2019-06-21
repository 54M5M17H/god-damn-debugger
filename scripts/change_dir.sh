#!/bin/bash

change_dir() {
	# get us into the correct directory
	SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
	cd $SCRIPT_DIR
	cd ..
	# cd-ing here won't affect the parent shell dir
}