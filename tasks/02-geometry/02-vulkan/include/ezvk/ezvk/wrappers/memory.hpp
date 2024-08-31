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

#include "unified_includes/vulkan_hpp_include.hpp"

#include "ezvk/error.hpp"
#include "memory.hpp"

#include "ezvk/utils/utility.hpp"
#include "queues.hpp"

#include <cstddef>
#include <cstdlib>
#include <functional>
#include <iterator>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace ezvk {

class vk_memory_error : public ezvk::vk_error {
public:
  vk_memory_error(std::string msg) : ezvk::vk_error{msg} {}
};

inline uint32_t
find_memory_type(vk::PhysicalDeviceMemoryProperties mem_properties,
                 uint32_t type_filter, vk::MemoryPropertyFlags property_flags) {
  uint32_t i = 0;

  auto found = std::find_if(
      mem_properties.memoryTypes.begin(), mem_properties.memoryTypes.end(),
      [&i, property_flags, type_filter](auto a) {
        return (type_filter & (1 << i++)) &&
               ((a.propertyFlags & property_flags) == property_flags);
      });

  if (found == mem_properties.memoryTypes.end())
    throw ezvk::vk_memory_error{"Could not find suitable memory type"};
  return i - 1;
}

inline vk::raii::DeviceMemory
allocate_device_memory(const vk::raii::Device &l_device,
                       vk::PhysicalDeviceMemoryProperties properties,
                       vk::MemoryRequirements requirements,
                       vk::MemoryPropertyFlags property_flags) {
  uint32_t mem_type_index =
      find_memory_type(properties, requirements.memoryTypeBits, property_flags);
  vk::MemoryAllocateInfo mem_allocate_info{.allocationSize = requirements.size,
                                           .memoryTypeIndex = mem_type_index};
  return {l_device, mem_allocate_info};
}

class framebuffers final : private std::vector<vk::raii::Framebuffer> {
public:
  framebuffers() = default;

  framebuffers(const vk::raii::Device &l_device,
               const std::vector<vk::raii::ImageView> &image_views,
               const vk::Extent2D &extent,
               const vk::raii::RenderPass &render_pass) {
    auto framebuffer_info =
        vk::FramebufferCreateInfo{.renderPass = *render_pass,
                                  .attachmentCount = 1,
                                  .width = extent.width,
                                  .height = extent.height,
                                  .layers = 1};

    for (const auto &view : image_views) {
      framebuffer_info.pAttachments = std::addressof(*view);
      vector::emplace_back(l_device, framebuffer_info);
    }
  }

  framebuffers(const vk::raii::Device &l_device,
               const std::vector<vk::raii::ImageView> &image_views,
               const vk::Extent2D &extent,
               const vk::raii::RenderPass &render_pass,
               const vk::raii::ImageView &depth_image_view) {
    auto framebuffer_info =
        vk::FramebufferCreateInfo{.renderPass = *render_pass,
                                  .width = extent.width,
                                  .height = extent.height,
                                  .layers = 1};

    for (const auto &view : image_views) {
      std::array<vk::ImageView, 2> attachments = {*view, *depth_image_view};
      framebuffer_info.attachmentCount = attachments.size();
      framebuffer_info.pAttachments = attachments.data();
      vector::emplace_back(l_device, framebuffer_info);
    }
  }

  using vector::operator[];
  using vector::back;
  using vector::begin;
  using vector::cbegin;
  using vector::cend;
  using vector::end;
  using vector::front;
  using vector::size;
};

class device_buffer final {
  vk::raii::Buffer m_buffer = nullptr;
  vk::raii::DeviceMemory m_memory = nullptr;

public:
  device_buffer() = default;

  // Sure, this is not encapsulation, but rather a consistent interface
  const auto &buffer() const & { return m_buffer; }
  const auto &memory() const & { return m_memory; }

  static constexpr auto default_property_flags =
      vk::MemoryPropertyFlagBits::eHostVisible |
      vk::MemoryPropertyFlagBits::eHostCoherent;

  device_buffer(
      const vk::raii::PhysicalDevice &p_device,
      const vk::raii::Device &l_device, vk::DeviceSize size,
      vk::BufferUsageFlags usage,
      vk::MemoryPropertyFlags property_flags = default_property_flags) {
    m_buffer = l_device.createBuffer(
        vk::BufferCreateInfo{.size = size, .usage = usage});
    m_memory = {allocate_device_memory(l_device, p_device.getMemoryProperties(),
                                       m_buffer.getMemoryRequirements(),
                                       property_flags)};
    m_buffer.bindMemory(*m_memory, 0);
  }

  template <typename T>
  device_buffer(const vk::raii::PhysicalDevice &p_device,
                const vk::raii::Device &l_device,
                const vk::BufferUsageFlags usage, std::span<const T> data,
                vk::MemoryPropertyFlags property_flags = default_property_flags)
      : device_buffer{p_device, l_device, utils::sizeof_container(data), usage,
                      property_flags} {
    copy_to_device(data);
  }

  template <typename T>
  void copy_to_device(std::span<const T> view,
                      vk::DeviceSize stride = sizeof(T)) {
    assert(sizeof(T) <= stride);

    auto memory =
        static_cast<uint8_t *>(m_memory.mapMemory(0, view.size() * stride));

    if (stride == sizeof(T)) {
      std::memcpy(memory, view.data(), view.size() * sizeof(T));
    }

    else {
      for (unsigned i = 0; i < view.size(); ++i) {
        std::memcpy(memory + stride * i, &view[i], sizeof(T));
      }
    }

    m_memory.unmapMemory();
  }

  template <typename T> void copy_to_device(const T &data) {
    copy_to_device<T>(std::span<const T>{&data, 1});
  }
};

struct device_buffers final : private std::vector<device_buffer> {
  device_buffers() = default;

  device_buffers(std::size_t count, std::size_t max_size,
                 const vk::raii::PhysicalDevice &p_device,
                 const vk::raii::Device &l_device, vk::BufferUsageFlags usage,
                 vk::MemoryPropertyFlags property_flags =
                     device_buffer::default_property_flags) {
    vector::reserve(count);
    for (unsigned i = 0; i < count; i++) {
      vector::emplace_back(p_device, l_device, max_size, usage, property_flags);
    }
  }

  using vector::operator[];
  using vector::back;
  using vector::begin;
  using vector::cbegin;
  using vector::cend;
  using vector::end;
  using vector::front;
  using vector::size;
};

class upload_context {
  vk::Device m_device;
  vk::Queue m_transfer_queue;

  vk::raii::Fence m_upload_fence = nullptr;
  vk::raii::CommandBuffer m_command_buffer = nullptr;

public:
  upload_context() = default;

  upload_context(const vk::raii::Device &l_device, vk::Queue transfer_queue,
                 const vk::raii::CommandPool &pool)
      : m_device{*l_device}, m_transfer_queue{transfer_queue} {
    vk::CommandBufferAllocateInfo alloc_info = {
        .commandPool = *pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1};
    m_command_buffer =
        std::move(vk::raii::CommandBuffers{l_device, alloc_info}.front());
    m_upload_fence = l_device.createFence({});
  }

  void immediate_submit(std::function<void(vk::raii::CommandBuffer &cmd)>
                            cmd_buffer_creation_func) {
    m_command_buffer.reset();
    m_command_buffer.begin(
        {.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    cmd_buffer_creation_func(m_command_buffer);
    m_command_buffer.end();

    vk::SubmitInfo submit_info = {.commandBufferCount = 1,
                                  .pCommandBuffers = &(*m_command_buffer)};
    m_transfer_queue.submit(submit_info, *m_upload_fence);

    static_cast<void>(
        m_device.waitForFences(*m_upload_fence, true, UINT64_MAX));
    m_device.resetFences(*m_upload_fence);
  };
};

} // namespace ezvk
