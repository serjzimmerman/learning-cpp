# 01-hwc

Introductory homework on caching algorithms in C++.

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
# Navigate to build directory and run
cd build/
ctest
```

## 3. Test driver programs
Targets for test drivers are called respectfully _belady_, _lfuc_, _lfudac_ and their sources are located in test/ directory. In order to get command line options for _lfuc_ and _lfudac_ one needs to have boost::program_options installed on the system. Example for _lfuc_(_lfudac_ is identical):

```sh
# Build and install
cd build/
make -j12 install
cd ../test/lfuc/

# Help message
bin/lfuc --help
# Available options:
#   -h [ --help ]         Print this help message
#   -v [ --verbose ]      Output verbose

# Run sample input and compare with optimal strategy
bin/lfuc -v < resources/test5.dat
# LFU hits: 1720
# Maximum possible hits: 2693
```