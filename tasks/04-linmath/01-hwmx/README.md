# 01-hwmx
Calculation of the matrix determinant 
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
The main test driver is _determinant_. It's recommended to build with Boost installed to get command line options.

```sh
# Build and install
cd build/
make -j12 install
cd ../test/determinant

# Help message
bin/determinant --help
# Available options:
#   -h [ --help ]               Print this help message
#   -m [ --measure ]            Print perfomance metrics
#   -t [ --type ] arg (=double) Type for matrix element (int, long, float, 
#                               double)

# Run sample test
bin/determinant --type long < resources/medium3.dat
# 69984
bin/determinant --type double < resources/medium3.dat
# 69984.000000

```