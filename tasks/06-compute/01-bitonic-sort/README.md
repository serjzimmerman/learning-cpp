# Having fun with GPGPU. Bitonic sort and Tiled matrix multiplication using OpenCL and C++

Experiments with OpenCL. Several implementations of bitonic sort for CPU/GPU. Perfomance measurements, comparisons with std::sort/__gnu_parallel::sort.
Matrix multiplication (naive, with tiling) and comparisons to baseline naive implemtation/Eigen/

## 1. How to build

We rely on Python for simple C++ codegen from OpenCL kernels. Make sure to have it installed. 

### Linux
```sh
git submodule init && git submodule update

cmake -S ./ -B build/ -DCMAKE_BUILD_TYPE=Release
# You can specify the type to use with the option -DTYPE (by default int is used)
cmake -S ./ -B build/ -DCMAKE_BUILD_TYPE=Release -DTYPE=float

# To enable Eigen make sure you have it installed systemwide and provide the following flag:
cmake -S ./ -B build/ -DCMAKE_BUILD_TYPE=Release -DEIGEN_MAT_MULT=ON
# Similarly, you can enable __gnu_parallel::sort. It relies on OpenMP:
cmake -S ./ -B build/ -DCMAKE_BUILD_TYPE=Release -DPAR_CPU_SORT=ON

cd build/
make -j12
```

### Windows
```sh
git submodule init && git submodule update
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## 2. Bitonic
To run bitonic sort use __bitonic__ target. 

```sh
./bitonic -h
# Avaliable options:
#  -h, --help                        Print this help message
#  -p, --print                       Print on failure
#  -s, --skip                        Skip comparing with std::sort
#  -l, --lower [=arg(=-2147483648)]  Lower bound
#  -u, --upper [=arg(=2147483647)]   Upper bound
#  -n, --num [=arg(=24)]             Length of the array to sort = 2^n
#  -k, --kernel [=arg(=naive)]       Which kernel to use: naive, cpu, local
#  --lsz [=arg(=256)]                Local memory size

# Run the best kernel with appropriate local size for your device:
./bitonic --kernel=local < ../resources/test8.dat

# Try out our random tests:
./bitonic --kernel=local --lsz=1024 --num=25 --random 
```

## 3. Matmult
To run bitonic sort use __matmult__ target. 

```sh
./matmult -h
# Avaliable options:
#  -h, --help                   Print this help message
#  -p, --print                  Print on failure
#  -e, --eigen                  Compare with Eigen matrix multiplication
#  -s, --skip                   Skip naive cpu calculation
#  -l, --lower [=arg(=-32)]     Lower bound
#  -u, --upper [=arg(=32)]      Upper bound
#  --ax [=arg(=512)]            Number of rows in matrix A
#  --ay [=arg(=512)]            Number of cols in matrix A
#  --by [=arg(=512)]            Number of cols in matrix B
#  -k, --kernel [=arg(=naive)]  Which kernel to use: naive, tiled, tiledarb
#  --lsz [=arg(=256)]           Local tile size
```