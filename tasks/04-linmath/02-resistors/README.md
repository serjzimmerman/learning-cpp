# 02-resistor-network

Current calculation in networks of resistors & DC sources.

## 1. How to build
### Linux
```sh
cmake -S ./ -B build/ -DCMAKE_BUILD_TYPE=Release

cd build/
make -j12 install
```

## 2. How to run tests
```sh
# Navigate to build directory and run unit and end-to-end tests
cd build/
ctest
```

## 3. Test driver program
The main test driver is _network_. Bison/Flex is required to build the parser. Additionally, Boost is an unconditional requirement for program options and misc utilities.

```sh
# Build and install
cd build/
make -j12 install
cd ../test/determinant

# Help message
bin/network --help
# Available options:
#  -h [ --help ]         Print this help message
#  -n [ --nonverbose ]   Non-verbose output
#  -p [ --potentials ]   Print vertex potentials

# Run sample test
bin/network < resources/initial1.dat
# 1 -- 2: 0.442958 A
# 1 -- 3: 0.631499 A
# 1 -- 4: -1.07446 A
# 2 -- 3: 0.0757193 A
# 2 -- 4: 0.367239 A
# 3 -- 4: 0.707219 A

bin/network -p < resources/wheatstone_bridge1.dat
# 0 -- 1: 0.051606 A
# 1 -- 2: 0.035546 A
# 1 -- 3: 0.01606 A
# 2 -- 3: 0.0149893 A
# 2 -- 0: 0.0205567 A
# 3 -- 0: 0.0310493 A
# 3 : -0.963597 V
# 2 : -0.888651 V
# 1 : 0 V
# 0 : -5 V

```