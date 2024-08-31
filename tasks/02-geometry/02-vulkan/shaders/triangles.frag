#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define COLOR_COUNT 4

layout (std140, binding = 0) uniform buffer {
  mat4 vp;
  vec4 colors[COLOR_COUNT];
  vec4 light_color;
  vec4 light_direction;
  float ambient_strength;
} uniform_buffer;

layout (location = 0) flat in vec3 in_color;
layout (location = 1) in vec3 in_normal;

layout (location = 0) out vec4 out_color;

void main() {
  vec3 object_color = in_color;
  vec3 light_color = uniform_buffer.light_color.xyz;

  vec3 ambient = uniform_buffer.ambient_strength * light_color;
  vec3 norm = normalize(in_normal);
  vec3 light_dir = uniform_buffer.light_direction.xyz;

  float diffuse_strength = max(dot(light_dir, norm), 0.0);
  vec3 diffuse = diffuse_strength * light_color;

  vec3 result = (ambient + diffuse) * object_color;
  out_color = vec4(result, 1.0);
}