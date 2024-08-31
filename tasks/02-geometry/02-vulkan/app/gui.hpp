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

#include "unified_includes/glm_inlcude.hpp"
#include "unified_includes/vulkan_hpp_include.hpp"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"

#include "config.hpp"
#include "misc/ubo.hpp"
#include "misc/utility.hpp"
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

#include <string>

namespace triangles::gui {

class imgui_resources {
  vk::raii::DescriptorPool m_descriptor_pool = nullptr;

private:
  bool m_initialized = false;

  static void imgui_check_vk_error(VkResult res) {
    vk::Result hpp_result = vk::Result{res};
    const auto error_message = vk::to_string(hpp_result);
    vk::resultCheck(hpp_result, error_message.c_str());
  }

public:
  imgui_resources() = default;

  imgui_resources(const applicaton_platform &plat,
                  const vk::raii::Device &l_device,
                  const ezvk::device_queue &graphics,
                  const ezvk::swapchain &swapchain, ezvk::upload_context &ctx,
                  ezvk::render_pass &rpass);

  imgui_resources(const imgui_resources &rhs) = delete;
  imgui_resources &operator=(const imgui_resources &rhs) = delete;

  void swap(imgui_resources &rhs) {
    std::swap(m_descriptor_pool, rhs.m_descriptor_pool);
    std::swap(m_initialized, rhs.m_initialized);
  }

  imgui_resources(imgui_resources &&rhs) { swap(rhs); }

  imgui_resources &operator=(imgui_resources &&rhs) {
    swap(rhs);
    return *this;
  }

  ~imgui_resources() {
    if (!m_initialized)
      return;
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }

public:
  void fill_command_buffer(vk::raii::CommandBuffer &cmd) {
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *cmd);
  }

  void update_after_resize(const ezvk::swapchain &swapchain) {
    ImGui_ImplVulkan_SetMinImageCount(swapchain.min_image_count());
  }

  static void new_frame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }

  static void render_frame() { ImGui::Render(); }
};

struct app_gui {
  using array_color4 = std::array<float, 4>;
  struct parameters_type {
    float linear_velocity_reg = 500.0f;
    float angular_velocity_reg = 30.0f;
    float linear_velocity_mod = 5000.0f;

    float render_distance = 30000.0f;
    float fov = 90.0f;

    float light_dir_yaw = 0.0f, light_dir_pitch = 0.0f;
    float ambient_strength = 0.1f;

    glm::vec4 light_dir;

    array_color4 light_color = hex_to_rgba(0xffffffff);
    array_color4 clear_color = hex_to_rgba(0x181818ff);

    std::array<array_color4, c_color_count> colors = {
        hex_to_rgba(0x89c4e1ff), hex_to_rgba(0xff4c29ff),
        hex_to_rgba(0x2f363aff), hex_to_rgba(0x338568ff)};

    bool draw_broad_phase = false, draw_bbox = false;
  };

  static parameters_type params;
  static void draw();
};

} // namespace triangles::gui
