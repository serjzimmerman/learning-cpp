/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "app/camera.hpp"
#include "geometry/equal.hpp"

#include "ezvk/utils/algorithm.hpp"
#include "ezvk/utils/utility.hpp"

#include "application.hpp"
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

namespace triangles {

static constexpr vk::AttachmentDescription
    primitives_renderpass_attachment_description = {
        .flags = vk::AttachmentDescriptionFlags{},
        .format = vk::Format::eB8G8R8A8Unorm,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::ePresentSrcKHR};

application::application(applicaton_platform platform)
    : m_platform{std::move(platform)} {
  initialize_logical_device_queues();

  // Create command pool and a context for submitting immediate copy operations
  // (graphics queue family implicitly supports copy operations)
  m_command_pool = ezvk::create_command_pool(
      m_l_device(), m_graphics_present->graphics().family_index(), true);
  m_oneshot_upload = ezvk::upload_context{
      m_l_device(), *m_graphics_present->graphics().queue(), m_command_pool};

  m_swapchain = {m_platform.p_device(), m_l_device(), m_platform.surface(),
                 m_platform.window().extent(), m_graphics_present.get()};

  initialize_primitives_pipeline();
  initialize_input_hanlder(); // Bind key strokes

  initialize_frame_rendering_info(); // Initialize data needed to render
                                     // primitives
  initialize_imgui();                // Initialize GUI specific objects
}

static constexpr auto c_global_descriptor_pool_sizes =
    std::to_array<vk::DescriptorPoolSize>(
        {{vk::DescriptorType::eUniformBuffer, 16}});

// We use two pipelines with the same descriptor set, so we should allocate a
// descriptor set with 2 binding points for a uniform buffer.
static constexpr auto c_descriptor_set_bindings =
    std::to_array<ezvk::descriptor_set::binding_description>(
        {{vk::DescriptorType::eUniformBuffer, 1,
          vk::ShaderStageFlagBits::eAllGraphics},
         {vk::DescriptorType::eUniformBuffer, 1,
          vk::ShaderStageFlagBits::eAllGraphics}});

static constexpr vk::PipelineRasterizationStateCreateInfo
    triangle_rasterization_state_create_info = {
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eFront,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f,
};

static constexpr vk::PipelineRasterizationStateCreateInfo
    wireframe_rasterization_state_create_info = {
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = vk::PolygonMode::eLine,
        .cullMode = vk::CullModeFlagBits::eNone,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f,
};

void application::initialize_primitives_pipeline() {
  m_descriptor_pool = ezvk::create_descriptor_pool(
      m_l_device(), c_global_descriptor_pool_sizes);

  m_uniform_buffers = {c_max_frames_in_flight, sizeof(triangles::ubo),
                       m_platform.p_device(), m_l_device(),
                       vk::BufferUsageFlagBits::eUniformBuffer};

  m_descriptor_set = {m_l_device(), m_uniform_buffers, m_descriptor_pool,
                      c_descriptor_set_bindings};

  // clang-format off
  constexpr vk::AttachmentReference 
    color_attachment_ref = {.attachment = 0, .layout = vk::ImageLayout::eColorAttachmentOptimal}, 
    depth_attachment_ref = {.attachment = 1, .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal};
  // clang-format on

  const auto subpass = vk::SubpassDescription{
      .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_attachment_ref,
      .pDepthStencilAttachment = &depth_attachment_ref};

  const auto depth_format =
      ezvk::find_depth_format(m_platform.p_device()).at(0);

  const auto attachments =
      std::array{primitives_renderpass_attachment_description,
                 ezvk::create_depth_attachment(depth_format)};

  m_primitives_render_pass =
      ezvk::render_pass{m_l_device(), subpass, attachments};
  m_depth_buffer = {m_platform.m_p_device, m_l_device(), depth_format,
                    m_swapchain.extent()};
  m_primitives_pipeline_layout = {m_l_device(), m_descriptor_set.m_layout};

  m_triangle_pipeline = {m_l_device(),
                         "shaders/triangles_vert.spv",
                         "shaders/triangles_frag.spv",
                         m_primitives_pipeline_layout(),
                         m_primitives_render_pass(),
                         triangle_rasterization_state_create_info,
                         vk::PrimitiveTopology::eTriangleList};

  m_wireframe_pipeline = {m_l_device(),
                          "shaders/wireframe_vert.spv",
                          "shaders/wireframe_frag.spv",
                          m_primitives_pipeline_layout(),
                          m_primitives_render_pass(),
                          wireframe_rasterization_state_create_info,
                          vk::PrimitiveTopology::eLineList};

  m_framebuffers = {m_l_device(), m_swapchain.image_views(),
                    m_swapchain.extent(), m_primitives_render_pass(),
                    m_depth_buffer.m_image_view()};
}

void application::initialize_input_hanlder() {
  using ezio::keyboard_handler;
  using button_state = keyboard_handler::button_state;

  auto &handler = keyboard_handler::instance();
  const auto keys = std::to_array<
      std::pair<keyboard_handler::key_index, keyboard_handler::button_state>>(
      {{GLFW_KEY_W, button_state::e_held_down},
       {GLFW_KEY_A, button_state::e_held_down},
       {GLFW_KEY_S, button_state::e_held_down},
       {GLFW_KEY_D, button_state::e_held_down},
       {GLFW_KEY_SPACE, button_state::e_held_down},
       {GLFW_KEY_C, button_state::e_held_down},
       {GLFW_KEY_Q, button_state::e_held_down},
       {GLFW_KEY_E, button_state::e_held_down},
       {GLFW_KEY_RIGHT, button_state::e_held_down},
       {GLFW_KEY_LEFT, button_state::e_held_down},
       {GLFW_KEY_UP, button_state::e_held_down},
       {GLFW_KEY_DOWN, button_state::e_held_down},
       {GLFW_KEY_LEFT_SHIFT, button_state::e_pressed}});

  handler.monitor(keys.begin(), keys.end());
  handler.bind(m_platform.window()());
}

void application::initialize_imgui() {
  m_imgui_data = {m_platform,  m_l_device(),     m_graphics_present->graphics(),
                  m_swapchain, m_oneshot_upload, m_primitives_render_pass};

  ImGui::StyleColorsDark();
}

void application::initialize_logical_device_queues() {
  const auto graphics_queue_indices =
      ezvk::find_graphics_family_indices(m_platform.p_device());
  const auto present_queue_indices = ezvk::find_present_family_indices(
      m_platform.p_device(), m_platform.surface());

  if (graphics_queue_indices.empty() || present_queue_indices.empty())
    throw ezvk::vk_error{"Platform does not support graphics"};

  std::vector<ezvk::queue_family_index_type> intersection;
  std::set_intersection(
      graphics_queue_indices.begin(), graphics_queue_indices.end(),
      present_queue_indices.begin(), present_queue_indices.end(),
      std::back_inserter(intersection));

  const auto default_priority = 1.0f;
  std::vector<vk::DeviceQueueCreateInfo> reqs;
  ezvk::queue_family_index_type chosen_graphics = 0, chosen_present = 0;

  if (intersection.empty()) {
    chosen_graphics =
        graphics_queue_indices
            .front(); // Maybe find a queue family with maximum number of queues
    chosen_present = present_queue_indices.front();
    reqs.push_back({.queueFamilyIndex = chosen_graphics,
                    .queueCount = 1,
                    .pQueuePriorities = &default_priority});
    reqs.push_back({.queueFamilyIndex = chosen_present,
                    .queueCount = 1,
                    .pQueuePriorities = &default_priority});
  }

  else {
    chosen_graphics = chosen_present = intersection.front();
    reqs.push_back({.queueFamilyIndex = chosen_graphics,
                    .queueCount = 1,
                    .pQueuePriorities = &default_priority});
  }

  const auto extensions = config::required_physical_device_extensions();
  m_l_device = {m_platform.p_device(), reqs, extensions.begin(),
                extensions.end()};

  m_graphics_present = ezvk::make_graphics_present_queues(
      m_l_device(), chosen_graphics, c_graphics_queue_index, chosen_present,
      c_present_queue_index);
}

void application::initialize_frame_rendering_info() {
  for (uint32_t i = 0; i < c_max_frames_in_flight; ++i) {
    auto primitive = frame_rendering_info{
        m_l_device().createSemaphore({}), m_l_device().createSemaphore({}),
        m_l_device().createFence(
            {.flags = vk::FenceCreateFlagBits::eSignaled})};
    m_rendering_info.push_back(std::move(primitive));
  }

  const auto alloc_info = vk::CommandBufferAllocateInfo{
      .commandPool = *m_command_pool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = c_max_frames_in_flight};

  m_primitives_command_buffers =
      vk::raii::CommandBuffers{m_l_device(), alloc_info};
}

void application::recreate_swap_chain() {
  auto extent = m_platform.window().extent();

  while (extent.width == 0 || extent.height == 0) {
    extent = m_platform.window().extent();
    glfwWaitEvents();
  }

  auto &old_swapchain = m_swapchain();

  auto new_swapchain = ezvk::swapchain{
      m_platform.p_device(),    m_l_device(),  m_platform.surface(), extent,
      m_graphics_present.get(), *old_swapchain};

  m_l_device().waitIdle();
  old_swapchain.clear(); // Destroy the old swapchain
  m_swapchain = std::move(new_swapchain);

  // Minimum number of images may have changed during swapchain recreation
  m_depth_buffer = {m_platform.m_p_device, m_l_device(),
                    m_depth_buffer.depth_format(), m_swapchain.extent()};
  m_framebuffers = {m_l_device(), m_swapchain.image_views(),
                    m_swapchain.extent(), m_primitives_render_pass(),
                    m_depth_buffer.m_image_view()};

  m_imgui_data.update_after_resize(m_swapchain);
}

void application::physics_loop(float delta) {
  auto &handler = ezio::keyboard_handler::instance();
  const auto events = handler.poll();

  if (ImGui::GetIO().WantCaptureKeyboard)
    return;
  if (events.contains(GLFW_KEY_LEFT_SHIFT))
    m_mod_speed = !m_mod_speed;

  const auto calculate_movement = [events](int plus, int minus) {
    return (1.0f * events.count(plus)) - (1.0f * events.count(minus));
  };

  const auto fwd_movement = calculate_movement(GLFW_KEY_W, GLFW_KEY_S);
  const auto side_movement = calculate_movement(GLFW_KEY_D, GLFW_KEY_A);
  const auto up_movement = calculate_movement(GLFW_KEY_SPACE, GLFW_KEY_C);

  const auto dir_movement = fwd_movement * m_camera.get_direction() +
                            side_movement * m_camera.get_sideways() +
                            up_movement * m_camera.get_up();
  const auto speed = (m_mod_speed ? gui_type::params.linear_velocity_mod
                                  : gui_type::params.linear_velocity_reg);

  if (throttle::geometry::is_definitely_greater(glm::length(dir_movement),
                                                0.0f)) {
    m_camera.translate(glm::normalize(dir_movement) * speed * delta);
  }

  const auto yaw_movement = calculate_movement(GLFW_KEY_RIGHT, GLFW_KEY_LEFT);
  const auto pitch_movement = calculate_movement(GLFW_KEY_DOWN, GLFW_KEY_UP);
  const auto roll_movement = calculate_movement(GLFW_KEY_Q, GLFW_KEY_E);

  const auto angular_per_delta_t =
      glm::radians(gui_type::params.angular_velocity_reg) * delta;

  const auto yaw_rotation =
      glm::angleAxis(yaw_movement * angular_per_delta_t, m_camera.get_up());
  const auto pitch_rotation = glm::angleAxis(
      pitch_movement * angular_per_delta_t, m_camera.get_sideways());
  const auto roll_rotation = glm::angleAxis(roll_movement * angular_per_delta_t,
                                            m_camera.get_direction());

  const auto resulting_rotation = yaw_rotation * pitch_rotation * roll_rotation;
  m_camera.rotate(resulting_rotation);
}

void application::fill_command_buffer(vk::raii::CommandBuffer &cmd,
                                      uint32_t image_index,
                                      vk::Extent2D extent) {
  cmd.reset();
  cmd.begin({.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});

  const auto submit_copy = [&](auto &info) -> void {
    if (!info.in_staging)
      return;
    copy_to_device_memory(cmd, info);
    info.in_staging = false;
    info.loaded = true;
  };

  submit_copy(m_triangle_draw_info);
  submit_copy(m_wireframe_bbox_draw_info);
  submit_copy(m_wireframe_broad_draw_info);

  std::array<vk::ClearValue, 2> clear_values;
  clear_values[0].color = gui_type::params.clear_color;
  clear_values[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

  const auto render_pass_info = vk::RenderPassBeginInfo{
      .renderPass = *m_primitives_render_pass(),
      .framebuffer = *m_framebuffers[image_index],
      .renderArea = {vk::Offset2D{0, 0}, extent},
      .clearValueCount = static_cast<uint32_t>(clear_values.size()),
      .pClearValues = clear_values.data()};

  cmd.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);

  const auto viewport = vk::Viewport{0.0f,
                                     static_cast<float>(extent.height),
                                     static_cast<float>(extent.width),
                                     -static_cast<float>(extent.height),
                                     0.0f,
                                     1.0f};

  cmd.setViewport(0, {viewport});
  cmd.setScissor(0, {{vk::Offset2D{0, 0}, extent}});

  cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                         *m_primitives_pipeline_layout(), 0,
                         {*m_descriptor_set.m_descriptor_set}, nullptr);

  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_triangle_pipeline());

  const auto submit_draw_info = [&cmd](const auto &info) -> void {
    if (!info)
      return;
    cmd.bindVertexBuffers(0, *info.buf.buffer(), {0});
    cmd.draw(info.count, 1, 0, 0);
  };

  submit_draw_info(m_triangle_draw_info);
  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_wireframe_pipeline());

  if (gui_type::params.draw_broad_phase) {
    submit_draw_info(m_wireframe_broad_draw_info);
  }

  if (gui_type::params.draw_bbox) {
    submit_draw_info(m_wireframe_bbox_draw_info);
  }

  m_imgui_data.fill_command_buffer(cmd);

  cmd.endRenderPass();
  cmd.end();
}

void application::render_frame() {
  auto &current_frame_data = m_rendering_info.at(m_curr_frame);
  static_cast<void>(m_l_device().waitForFences(
      {*current_frame_data.in_flight_fence}, VK_TRUE, UINT64_MAX));
  // Static cast to silence warning

  const auto acquire_info = vk::AcquireNextImageInfoKHR{
      .swapchain = *m_swapchain(),
      .timeout = UINT64_MAX,
      .semaphore = *current_frame_data.image_availible_semaphore,
      .fence = nullptr,
      .deviceMask = 1};

  uint32_t image_index;
  try {
    image_index = m_l_device().acquireNextImage2KHR(acquire_info).second;
  } catch (vk::OutOfDateKHRError &) {
    recreate_swap_chain();
    return;
  }

  fill_command_buffer(m_primitives_command_buffers[m_curr_frame], image_index,
                      m_swapchain.extent());

  const auto cmds = std::array{*m_primitives_command_buffers[m_curr_frame]};
  ubo uniform_buffer = {m_camera.get_vp_matrix(m_swapchain.extent().width,
                                               m_swapchain.extent().height),
                        {},
                        glm_vec_from_array(gui_type::params.light_color),
                        gui_type::params.light_dir,
                        gui_type::params.ambient_strength};

  std::transform(gui_type::params.colors.begin(), gui_type::params.colors.end(),
                 uniform_buffer.colors.begin(),
                 [](auto a) { return glm_vec_from_array(a); });

  m_uniform_buffers[m_curr_frame].copy_to_device(uniform_buffer);

  vk::PipelineStageFlags wait_stages =
      vk::PipelineStageFlagBits::eColorAttachmentOutput;

  const auto submit_info = vk::SubmitInfo{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores =
          std::addressof(*current_frame_data.image_availible_semaphore),
      .pWaitDstStageMask = std::addressof(wait_stages),
      .commandBufferCount = cmds.size(),
      .pCommandBuffers = cmds.data(),
      .signalSemaphoreCount = 1,
      .pSignalSemaphores =
          std::addressof(*current_frame_data.render_finished_semaphore)};

  m_l_device().resetFences(*current_frame_data.in_flight_fence);
  m_graphics_present->graphics().queue().submit(
      submit_info, *current_frame_data.in_flight_fence);

  vk::PresentInfoKHR present_info = {
      .waitSemaphoreCount = 1,
      .pWaitSemaphores =
          std::addressof(*current_frame_data.render_finished_semaphore),
      .swapchainCount = 1,
      .pSwapchains = std::addressof(*m_swapchain()),
      .pImageIndices = &image_index};

  vk::Result result_present;
  try {
    result_present =
        m_graphics_present->present().queue().presentKHR(present_info);
  } catch (vk::OutOfDateKHRError &) {
    result_present = vk::Result::eErrorOutOfDateKHR;
  }

  if (result_present == vk::Result::eSuboptimalKHR ||
      result_present == vk::Result::eErrorOutOfDateKHR) {
    recreate_swap_chain();
  }

  m_curr_frame = (m_curr_frame + 1) % c_max_frames_in_flight;
}

} // namespace triangles
