
DARK_MODE=OFF

test: build
	cd build && GTEST_COLOR=1 ctest --output-on-failure

test_cov: build
	GTEST_COLOR=1 cmake --build build --target os_test_coverage

test-ci: build
	cd build && GTEST_COLOR=1 ctest -V

setup:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DOS_TEST_COVERAGE_DARK=${DARK_MODE}

build: setup
	cmake --build build

.PHONY: setup build test
