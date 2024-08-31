# 03-rmq-lca

Problem 03-rmq-lca solving offline RMQ queries using Tarjan's LCA algorithm.

## 1. How to build
### Linux
```sh
cmake -S ./ -B build/ -DCMAKE_BUILD_TYPE=Release
cd build/
make -j12
```

## 2. How to run tests
```sh
# Navigate to build directory and run end-to-end tests
cd build/
ctest
```

## 3. Test driver program
```sh
# Build and install
cd build/
make -j12 install
cd ../test/rmq-queries

# Help message
bin/rmq-queries --help
# Available options:
#   -h [ --help ]         Print this help message
#   -m [ --measure ]      Print perfomance metrics
#   --hide                Hide output

# Run sample test
bin/rmq-queries < resources/provided1.dat # 5 10 12 3 14 1 3 0 3 0 1 2 4
# 3 10 1

# Run large test
bin/rmq-queries --hide -m < resources/uniform4.dat
# throttle::iterative_offline_rmq took 550.164ms to run

# To run all tests
./test.sh
```