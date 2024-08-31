/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "app/camera.hpp"
#include "geometry/equal.hpp"

#include "ezvk/utils/algorithm.hpp"
#include "ezvk/utils/utility.hpp"

#include "config.hpp"
#include "misc/ubo.hpp"
#include "misc/utility.hpp"
#include "misc/vertex.hpp"
#include "pipeline.hpp"
#include "platform.hpp"

#include "ezvk/debug.hpp"
#include "ezvk/window.hpp"
#include "ezvk/wrappers/debugged_instance.hpp"
#include "ezvk/wrappers/depth_buffer.hpp"
#include "ezvk/wrappers/descriptor_set.hpp"
#include "ezvk/wrappers/device.hpp"
#include "ezvk/wrappers/instance.hpp"
#include "ezvk/wrappers/memory.hpp"
#include "ezvk/wrappers/queues.hpp"
#include "ezvk/wrappers/renderpass.hpp"
#include "ezvk/wrappers/shaders.hpp"
#include "ezvk/wrappers/swapchain.hpp"

#include "app/camera.hpp"
#include "app/keyboard_handler.hpp"
#include "gui.hpp"

#include "unified_includes/glfw_include.hpp"
#include "unified_includes/glm_inlcude.hpp"
#include "unified_includes/vulkan_hpp_include.hpp"

#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <span>
#include <stdexcept>

#if defined(VK_VALIDATION_LAYER) || !defined(NDEBUG)
#define USE_DEBUG_EXTENSION
#endif

namespace triangles {
struct input_data {
  std::span<const triangles::triangle_vertex_type> tr_vert;
  std::span<const triangles::wireframe_vertex_type> broad_vert, bbox_vert;
};

class application final {
private:
  static constexpr uint32_t c_max_frames_in_flight = 2; // Double buffering
  static constexpr uint32_t c_graphics_queue_index = 0;
  static constexpr uint32_t c_present_queue_index = 0;

private:
  applicaton_platform m_platform;

  // Logical device and queues needed for rendering
  ezvk::logical_device m_l_device;
  std::unique_ptr<ezvk::i_graphics_present_queues> m_graphics_present;

  vk::raii::CommandPool m_command_pool = nullptr;
  ezvk::upload_context m_oneshot_upload;
  ezvk::swapchain m_swapchain;

  vk::raii::DescriptorPool m_descriptor_pool = nullptr;

  ezvk::descriptor_set m_descriptor_set;
  ezvk::device_buffers m_uniform_buffers;

  ezvk::render_pass m_primitives_render_pass;
  ezvk::pipeline_layout m_primitives_pipeline_layout;

  ezvk::depth_buffer m_depth_buffer;

  pipeline<triangle_vertex_type> m_triangle_pipeline;
  pipeline<wireframe_vertex_type> m_wireframe_pipeline;

  ezvk::framebuffers m_framebuffers;
  std::atomic_bool m_data_loaded = false;

  struct vertex_draw_info {
    ezvk::device_buffer buf;

    std::atomic_bool loaded = false, in_staging = false;
    uint32_t count = 0, size = 0;

    ezvk::device_buffer staging_buffer;

  public:
    operator bool() const { return loaded; }
  };

  vertex_draw_info m_triangle_draw_info;
  vertex_draw_info m_wireframe_broad_draw_info;
  vertex_draw_info m_wireframe_bbox_draw_info;

  struct frame_rendering_info {
    vk::raii::Semaphore image_availible_semaphore, render_finished_semaphore;
    vk::raii::Fence in_flight_fence;
  };

  vk::raii::CommandBuffers m_primitives_command_buffers = nullptr;

  std::vector<frame_rendering_info> m_rendering_info;
  std::chrono::time_point<std::chrono::system_clock> m_prev_frame_start;

  std::size_t m_curr_frame = 0;
  utils3d::camera m_camera;

  bool m_mod_speed = false;
  bool m_first_frame = true;

private:
  gui::imgui_resources m_imgui_data;
  using gui_type = gui::app_gui;

private:
  application(applicaton_platform platform);

private:
  class singleton_helper {
    std::unique_ptr<application> m_instance;

  public:
    application &get(applicaton_platform *platform = nullptr) {
      if (platform) {
        if (m_instance.get())
          throw std::invalid_argument{
              "Application instance is already initialized"};
        m_instance =
            std::unique_ptr<application>{new application{std::move(*platform)}};
        return *m_instance.get();
      }

      if (!m_instance.get())
        throw std::invalid_argument{
            "Application instance hasn't been initialized"};
      return *m_instance.get();
    }

    void destroy() { m_instance.reset(); }
  };

public:
  static singleton_helper &instance() {
    static singleton_helper helper;
    return helper;
  }

  void loop() {
    auto current_time = std::chrono::system_clock::now();

    if (!m_first_frame) {
      physics_loop(
          std::chrono::duration<float>{current_time - m_prev_frame_start}
              .count());
    } else {
      m_first_frame = false;
    }

    m_prev_frame_start = current_time;

    gui::imgui_resources::new_frame();
    gui_type::draw();
    gui::imgui_resources::render_frame();

    // Here we update the camera parameters
    m_camera.set_far_z_clip(gui_type::params.render_distance);
    m_camera.set_fov_degrees(gui_type::params.fov);

    render_frame();
  }

  auto *window() const { return m_platform.window()(); }

  void load_input_data(const input_data &data) {
    if (m_data_loaded)
      throw std::invalid_argument{
          "For now you can't load vertex data more than once"};

    if (!data.tr_vert.empty())
      load_draw_info(data.tr_vert, m_triangle_draw_info);
    if (!data.broad_vert.empty())
      load_draw_info(data.broad_vert, m_wireframe_broad_draw_info);
    if (!data.bbox_vert.empty())
      load_draw_info(data.bbox_vert, m_wireframe_bbox_draw_info);

    m_data_loaded = true;
  }

  void shutdown() { m_l_device().waitIdle(); }

private:
  ezvk::device_buffer copy_to_staging_memory(const auto &container) {
    assert(!container.empty());

    auto staging_buffer = ezvk::device_buffer{
        m_platform.p_device(), m_l_device(),
        vk::BufferUsageFlagBits::eTransferSrc, std::span{container},
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent};

    return staging_buffer;
  }

  void copy_to_device_memory(vk::raii::CommandBuffer &cmd,
                             vertex_draw_info &info) {
    const auto size = info.size;

    info.buf = {m_platform.p_device(), m_l_device(), size,
                vk::BufferUsageFlagBits::eVertexBuffer |
                    vk::BufferUsageFlagBits::eTransferDst,
                vk::MemoryPropertyFlagBits::eDeviceLocal};

    auto &src_buffer = info.staging_buffer.buffer();
    auto &dst_buffer = info.buf.buffer();

    vk::BufferCopy copy = {0, 0, size};
    cmd.copyBuffer(*src_buffer, *dst_buffer, copy);

    const auto barrier = vk::BufferMemoryBarrier{
        .srcAccessMask = vk::AccessFlagBits::eTransferWrite,
        .dstAccessMask = vk::AccessFlagBits::eVertexAttributeRead,
        .buffer = *dst_buffer,
        .offset = 0,
        .size = info.size};

    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                        vk::PipelineStageFlagBits::eVertexInput, {}, nullptr,
                        barrier, nullptr);
  }

  void load_draw_info(const auto &vertices, vertex_draw_info &info) {
    assert(!vertices.empty());
    info.count = vertices.size();
    info.size = ezvk::utils::sizeof_container(vertices);
    info.staging_buffer = copy_to_staging_memory(vertices);
    info.in_staging = true;
  }

  void physics_loop(float delta);
  void initialize_primitives_pipeline();
  void initialize_input_hanlder();
  void initialize_imgui();
  void initialize_logical_device_queues();
  void initialize_frame_rendering_info();
  void fill_command_buffer(vk::raii::CommandBuffer &cmd, uint32_t image_index,
                           vk::Extent2D extent);
  void recreate_swap_chain();

  void render_frame();
};

} // namespace triangles
