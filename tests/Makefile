
test: build
	cd build && GTEST_COLOR=1 ctest --output-on-failure

test_cov: build
	make -C build os_test_coverage

test-ci: build
	cd build && GTEST_COLOR=1 ctest -V

setup:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

build: setup
	cmake --build build

.PHONY: setup build test
