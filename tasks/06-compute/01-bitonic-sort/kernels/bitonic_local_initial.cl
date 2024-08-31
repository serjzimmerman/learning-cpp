/* clang-format off
 * Simplest possible bitonic sort using only global memory. Note: SEGMENT_SIZE should be a power of 2
 * (obviously)
 *
 *  @kernel    ( {"name" : "bitonic_local_initial_kernel", "entry" : "local_initial"} )
 *  @signature ( ["cl::Buffer", "unsigned", "unsigned", "unsigned"] )
 *  @macros    ( [{"type" : "std::string", "name": "TYPE"}, {"type" : "unsigned", "name": "SEGMENT_SIZE"}] )
 * clang-format on
 */

#define SORT2(a, b)                                                            \
  if (a > b) {                                                                 \
    TYPE temp = a;                                                             \
    a = b;                                                                     \
    b = temp;                                                                  \
  }

#define HALF_SEGMENT_SIZE (SEGMENT_SIZE / 2)
#define LOCAL_THREADS HALF_SEGMENT_SIZE

__kernel void local_initial(__global TYPE *buf, uint stage_start,
                            uint stage_end, uint step_offset) {
  uint gid = get_global_id(0);
  uint lid = get_local_id(0);
  uint sid = (gid / HALF_SEGMENT_SIZE);

  uint local_first_load_id = lid, local_second_load_id = SEGMENT_SIZE - lid - 1;

  uint first_data_load_id = sid * SEGMENT_SIZE + lid;
  uint second_data_load_id = sid * SEGMENT_SIZE + SEGMENT_SIZE - 1 - lid;

  __local TYPE segment[SEGMENT_SIZE];
  segment[local_first_load_id] = buf[first_data_load_id];
  segment[local_second_load_id] = buf[second_data_load_id];
  barrier(CLK_LOCAL_MEM_FENCE);

  for (uint stage = stage_start; stage < stage_end; ++stage) {
    for (int step = stage - step_offset; step >= 0; --step) {
      const uint half_length = 1 << step, part_length = half_length * 2;
      const uint part_index = lid >> step;

      const uint i = lid - part_index * half_length;
      uint j;

      if (stage == step) { // The first step in a stage
        j = part_length - i - 1;
      } else {
        j = i + half_length;
      }

      const uint offset = part_index * part_length;
      const uint first_index = offset + i, second_index = offset + j;

      SORT2(segment[first_index], segment[second_index]);
      barrier(CLK_LOCAL_MEM_FENCE);
    }
  }

  buf[first_data_load_id] = segment[local_first_load_id];
  buf[second_data_load_id] = segment[local_second_load_id];
}
