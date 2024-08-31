/* clang-format off
 * Tiled matrix multiplication with local memory
 *
 *  @kernel    ( {"name" : "matmult_tiled_kernel", "entry" : "tiled"} )
 *  @signature ( ["cl::Buffer", "cl::Buffer", "cl::Buffer", "int", "int", "int"] )
 *  @macros    ( [{"type" : "std::string", "name": "TYPE"}, {"type": "unsigned", "name": "TILE_SIZE"}] )
 * clang-format on
 */

__kernel void tiled(__global TYPE *A, __global TYPE *B, __global TYPE *C,
                    int AX, int AY, int BY) {
  int tile_row = get_group_id(0);
  int tile_col = get_group_id(1);

  int local_row = get_local_id(0);
  int local_col = get_local_id(1);

  __local TYPE tile_A[TILE_SIZE * TILE_SIZE];
  __local TYPE tile_B[TILE_SIZE * TILE_SIZE];

  int global_row = TILE_SIZE * tile_row + local_row;
  int global_col = TILE_SIZE * tile_col + local_col;

  int tile_count = AY / TILE_SIZE;
  TYPE sum = 0;

  for (int t = 0; t < tile_count; ++t) {
    // Step 1. Here each work group thread is responsible for copying data into
    // the corresponding slot in the tile_A, tile_B
    tile_A[local_row * TILE_SIZE + local_col] =
        A[global_row * AY + t * TILE_SIZE + local_col];
    tile_B[local_row * TILE_SIZE + local_col] =
        B[BY * (t * TILE_SIZE + local_row) + global_col];

    // Barrier here to finish loading all the data before proceeding.
    barrier(CLK_LOCAL_MEM_FENCE);

    // Step 2. Calculate part of the resulting tile corresponding to this thread
    // and accumulate it in sum.
    for (int k = 0; k < TILE_SIZE; ++k) {
      sum +=
          tile_A[TILE_SIZE * local_row + k] * tile_B[k * TILE_SIZE + local_col];
    }

    // Wait for all threads to finish before reloading new tiles.
    barrier(CLK_LOCAL_MEM_FENCE);
  }

  C[global_row * BY + global_col] = sum;
}
