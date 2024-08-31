/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "unified_includes/vulkan_hpp_include.hpp"

#include "imgui.h"

#include "config.hpp"
#include "gui.hpp"
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

constexpr uint32_t default_descriptor_count = 1000;

constexpr auto imgui_pool_sizes = std::to_array<vk::DescriptorPoolSize>(
    {{vk::DescriptorType::eSampler, default_descriptor_count},
     {vk::DescriptorType::eCombinedImageSampler, default_descriptor_count},
     {vk::DescriptorType::eSampledImage, default_descriptor_count},
     {vk::DescriptorType::eStorageImage, default_descriptor_count},
     {vk::DescriptorType::eUniformTexelBuffer, default_descriptor_count},
     {vk::DescriptorType::eStorageTexelBuffer, default_descriptor_count},
     {vk::DescriptorType::eUniformBuffer, default_descriptor_count},
     {vk::DescriptorType::eStorageBuffer, default_descriptor_count},
     {vk::DescriptorType::eUniformBufferDynamic, default_descriptor_count},
     {vk::DescriptorType::eStorageBufferDynamic, default_descriptor_count},
     {vk::DescriptorType::eInputAttachment, default_descriptor_count}});

imgui_resources::imgui_resources(const applicaton_platform &plat,
                                 const vk::raii::Device &l_device,
                                 const ezvk::device_queue &graphics,
                                 const ezvk::swapchain &swapchain,
                                 ezvk::upload_context &ctx,
                                 ezvk::render_pass &rpass) {
  m_descriptor_pool = ezvk::create_descriptor_pool(l_device, imgui_pool_sizes);

  IMGUI_CHECKVERSION(); // Verify that compiled imgui binary matches the header
  ImGui::CreateContext();

  ImGui_ImplGlfw_InitForVulkan(plat.window()(), true);

  // clang-format off
  auto info = ImGui_ImplVulkan_InitInfo{
      .Instance = *plat.instance(), .PhysicalDevice = *plat.p_device(), .Device = *l_device,
      .QueueFamily = graphics.family_index(), .Queue = *graphics.queue(),
      .PipelineCache = VK_NULL_HANDLE, .DescriptorPool = *m_descriptor_pool, 
      .Subpass = 0, .MinImageCount = swapchain.min_image_count(), 
      .ImageCount = static_cast<uint32_t>(swapchain.images().size()),
      .MSAASamples = VK_SAMPLE_COUNT_1_BIT, .Allocator = nullptr,
      .CheckVkResultFn = imgui_resources::imgui_check_vk_error};
  // clang-format on

  ImGui_ImplVulkan_Init(&info, *rpass());
  ctx.immediate_submit([](vk::raii::CommandBuffer &cmd) {
    ImGui_ImplVulkan_CreateFontsTexture(*cmd);
  }); // Upload font textures to the GPU via oneshot immediate submit

  m_initialized = true;
}

void app_gui::draw() {
  static auto metrics_window_open = false;

  if (metrics_window_open)
    ImGui::ShowMetricsWindow(&metrics_window_open);

  ImGui::Begin("Triangles with Vulkan");

  if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("Move the camera:");
    ImGui::BulletText("Forwards/Backwards with W/S");
    ImGui::BulletText("Sideways to the Left/Right with A/D");
    ImGui::BulletText("Up/Down with Space/C");

    ImGui::Text("Rotate the camera:");
    ImGui::BulletText("Yaw with Left/Right Arrows");
    ImGui::BulletText("Pitch with Up/Down Arrows");
    ImGui::BulletText("Roll with Q/E");

    ImGui::Text("Press Left Shift to change between regular/fast speed");
  }

  if (ImGui::CollapsingHeader("Movement", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
    ImGui::DragFloat("Linear velocity (regular)", &params.linear_velocity_reg,
                     1.0f);
    ImGui::DragFloat("Linear velocity (mod)", &params.linear_velocity_mod,
                     10.0f);
    ImGui::DragFloat("Angular velocity", &params.angular_velocity_reg, 0.1f);
    ImGui::PopItemWidth();
  }

  if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
    ImGui::DragFloat("Rendering distance", &params.render_distance, 50.0f);
    ImGui::DragFloat("Fov", &params.fov, 0.1f, 45.0f, 175.0f, "%.3f",
                     ImGuiSliderFlags_AlwaysClamp);

    ImGui::Checkbox("Visualize broad phase", &params.draw_broad_phase);
    ImGui::Checkbox("Draw bounding boxes", &params.draw_bbox);

    ImGui::BulletText("Color configuration");

    ImGui::ColorEdit4("Regular", params.colors[config::regular_index].data());
    ImGui::ColorEdit4("Intersecting",
                      params.colors[config::intersect_index].data());
    ImGui::ColorEdit4("Wiremesh", params.colors[config::wiremesh_index].data());
    ImGui::ColorEdit4("Bounding box", params.colors[config::bbox_index].data());
    ImGui::ColorEdit4("Clear Color", params.clear_color.data());

    if (ImGui::Button("Open Metrics/Debug Window")) {
      metrics_window_open = true;
    }

    ImGui::PopItemWidth();
  }

  if (ImGui::CollapsingHeader("Color", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);

    ImGui::DragFloat("Ambient Strength", &params.ambient_strength, 0.001f, 0.0f,
                     1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

    ImGui::ColorEdit4("Light Color", params.light_color.data());

    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.35f);
    ImGui::BulletText("Light direction");
    ImGui::DragFloat("Yaw", &params.light_dir_yaw, 0.1f, 0.0f, 360.0f, "%.3f",
                     ImGuiSliderFlags_AlwaysClamp);
    ImGui::SameLine();
    ImGui::DragFloat("Pitch", &params.light_dir_pitch, 0.1f, 0.0f, 360.0f,
                     "%.3f", ImGuiSliderFlags_AlwaysClamp);

    params.light_dir = glm::eulerAngleYX(glm::radians(params.light_dir_yaw),
                                         glm::radians(params.light_dir_pitch)) *
                       glm::vec4{0, 0, 1, 0};

    const auto light_dir = params.light_dir;

    ImGui::Text("Light direction: x = %.3f, y = %.3f, z = %.3f", light_dir.x,
                light_dir.y, light_dir.z);

    ImGui::PopItemWidth();
    ImGui::PopItemWidth();
  }

  ImGui::End();
}

app_gui::parameters_type app_gui::params = {};

} // namespace triangles::gui
