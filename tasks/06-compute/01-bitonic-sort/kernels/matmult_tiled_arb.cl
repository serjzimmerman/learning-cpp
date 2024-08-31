/* clang-format off
 * Tiled matrix multiplication with local memory. Accepts arbitrary size matrices and TILE_SIZE
 *
 *  @kernel    ( {"name" : "matmult_tiled_arb_kernel", "entry" : "tiled_arbitrary"} )
 *  @signature ( ["cl::Buffer", "cl::Buffer", "cl::Buffer", "int", "int", "int", "int"] )
 *  @macros    ( [{"type" : "std::string", "name": "TYPE"}, {"type": "unsigned", "name": "TILE_SIZE"}] )
 * clang-format on
 */

__kernel void tiled_arbitrary(__global TYPE *A, __global TYPE *B,
                              __global TYPE *C, int AX, int AY, int BY,
                              int tile_count) {
  int tile_row = get_group_id(0);
  int tile_col = get_group_id(1);

  int local_row = get_local_id(0);
  int local_col = get_local_id(1);

  __local TYPE tile_A[TILE_SIZE * TILE_SIZE];
  __local TYPE tile_B[TILE_SIZE * TILE_SIZE];

  int global_row = TILE_SIZE * tile_row + local_row;
  int global_col = TILE_SIZE * tile_col + local_col;

  int row_out_of_bounds = (global_row >= AX);
  int col_out_of_bounds = (global_col >= BY);

  TYPE sum = 0;

  for (int t = 0; t < tile_count; ++t) {
    // Step 1. Here each work group thread is responsible for copying data into
    // the corresponding slot in the tile_A, tile_B
    int curr_tiled_col = t * TILE_SIZE + local_col;
    int curr_tiled_row = t * TILE_SIZE + local_row;

    tile_A[local_row * TILE_SIZE + local_col] =
        ((curr_tiled_col >= AY || row_out_of_bounds)
             ? 0
             : A[global_row * AY + curr_tiled_col]);
    tile_B[local_row * TILE_SIZE + local_col] =
        ((curr_tiled_row >= AY || col_out_of_bounds)
             ? 0
             : B[BY * curr_tiled_row + global_col]);

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

  if (row_out_of_bounds || col_out_of_bounds)
    return;
  C[global_row * BY + global_col] = sum;
}
