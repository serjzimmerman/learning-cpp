# HWT

Problem HWT on order statistic trees. This implementaion uses a red-black augmentation to keep the tree balanced.

## 1. How to build

### Linux
```sh
cmake -S ./ -B build/ DCMAKE_BUILD_TYPE=Release
# Build in release preset
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
There is a single test driver program called _queries_ and its sources are located in test/queries directory. No additional dependencies are required.

```sh
# Build and install
cd build/
make -j12 install
cd ../test/queries

# Run sample test
bin/queries < resources/handwritten1.dat # k 8 k 2 k -1 m 1 m 2 n 3
# -1 2 2

# To run all tests
./test.sh
```