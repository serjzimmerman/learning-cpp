# 02-splay

Problem 02-splay on splay order statistic trees.

## 1. How to build
### Linux
```sh
cmake -S ./ -B build/ -DCMAKE_BUILD_TYPE=Release -DLINEAR=OFF
# Build in release preset with O(log(N)) algorithm. Pass -DLINEAR=ON to use linear algorithm with std::distance.
cd build/
make -j12
```

## 2. How to run tests
```sh
# Navigate to build directory and run unit and end-to-end tests
cd build/
ctest
```

## 3. Test driver program
The main test driver is _benchmark_. It's recommended to build with Boost installed to get command line options.

```sh
# Build and install
cd build/
make -j12 install
cd ../test/benchmark

# Help message
bin/benchmark --help
# Available options:
#   -h [ --help ]         Print this help message
#   -c [ --compare ]      Compare with std::set
#   -m [ --measure ]      Print perfomance metrics
#   --hide                Hide output

# Run sample test
bin/benchmark --hide --compare --measure < resources/triangle7.dat
# Outputs match with std::set
# throttle::splay_set took 32.3326ms to run
# std::set took 1118.12ms to run

# To run all tests
./test.sh
```

## 4. Measurements
Measurements were taken on a PC running Ryzen 3600 with -DCMAKE_BUILD_TYPE=Release. Files are located in [measurements](test/benchmark/measurements).