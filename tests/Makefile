
test: build
	make -C tests

setup:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

build: setup
	cmake --build build

.PHONY: setup build test
