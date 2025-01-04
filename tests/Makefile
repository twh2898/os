
test: build
	cd build && GTEST_COLOR=1 ctest --output-on-failure

test-ci: build
	cd build && GTEST_COLOR=1 ctest -V

setup:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

build: setup
	cmake --build build

.PHONY: setup build test
