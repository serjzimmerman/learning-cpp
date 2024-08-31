/* clang-format off
 * Simplest possible matrix multiplication algorithm without local memory
 *
 *  @kernel    ( {"name" : "matmult_naive_kernel", "entry" : "naive"} )
 *  @signature ( ["cl::Buffer", "cl::Buffer", "cl::Buffer", "int", "int", "int"] )
 *  @macros    ( [{"type" : "std::string", "name": "TYPE"}] )
 * clang-format on
 */

__kernel void naive(__global TYPE *A, __global TYPE *B, __global TYPE *C,
                    int AX, int AY, int BY) {
  int i = get_global_id(0);
  int j = get_global_id(1);

  TYPE sum = 0;
  for (int k = 0; k < AY; ++k) {
    sum += A[i * AY + k] * B[k * BY + j];
  }

  C[i * BY + j] = sum;
}
