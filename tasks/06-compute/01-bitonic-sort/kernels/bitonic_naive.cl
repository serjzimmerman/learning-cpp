/* clang-format off
 * Simplest possible bitonic sort using only global memory
 *
 *  @kernel    ( {"name" : "bitonic_naive_kernel", "entry" : "naive_bitonic"} )
 *  @signature ( ["cl::Buffer", "unsigned", "unsigned"] )
 *  @macros    ( [{"type" : "std::string", "name": "TYPE"}] )
 * clang-format on
 */

#define SORT2(a, b)                                                            \
  if (a > b) {                                                                 \
    TYPE temp = a;                                                             \
    a = b;                                                                     \
    b = temp;                                                                  \
  }

__kernel void naive_bitonic(__global TYPE *buf, uint stage, uint step) {
  uint gid = get_global_id(0);

  const uint half_length = 1 << step, part_length = half_length * 2;
  const uint part_index = gid >> step;

  const uint i = gid - part_index * half_length;
  uint j;

  if (stage == step) { // The first step in a stage
    j = part_length - i - 1;
  } else {
    j = i + half_length;
  }

  const uint offset = part_index * part_length;
  const uint first_index = offset + i, second_index = offset + j;

  SORT2(buf[first_index], buf[second_index]);
}
