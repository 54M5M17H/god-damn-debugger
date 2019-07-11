#!/bin/bash

source $(dirname $0)/change_dir.sh

OUTPUT_TO=dist

main () {
	if ["$1" -eq ""]; then
		TO_DO=build
	else
		TO_DO=$!
	fi

	$TO_DO
}

build () {
	mkdir -p $OUTPUT_TO
	cd $OUTPUT_TO
	cmake ..
	make
}

clean () {
	rm -rf $OUTPUT_TO
}

change_dir
main $*
change_dir

source ./scripts/build_tests.sh
