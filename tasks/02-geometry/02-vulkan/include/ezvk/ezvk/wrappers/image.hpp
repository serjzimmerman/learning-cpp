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

#include "memory.hpp"

namespace ezvk {

class image final {
  vk::raii::Image m_image = nullptr;
  vk::raii::DeviceMemory m_image_memory = nullptr;

public:
  image() = default;

  image(const vk::raii::PhysicalDevice &p_device,
        const vk::raii::Device &l_device, const vk::Extent3D &extent,
        vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlags properties) {
    // clang-format off
    const auto image_info = vk::ImageCreateInfo{
        .imageType = vk::ImageType::e2D, .format = format, .extent = extent, .mipLevels = 1,
        .arrayLayers = 1, .samples = vk::SampleCountFlagBits::e1, .tiling = tiling,
        .usage = usage, .sharingMode = vk::SharingMode::eExclusive, .initialLayout = vk::ImageLayout::eUndefined,
    };
    // clang-format on

    m_image = l_device.createImage(image_info);
    const auto memory_requirements =
        (*l_device).getImageMemoryRequirements(*m_image);
    const auto phys_device_memory_props = p_device.getMemoryProperties();

    const auto alloc_info = vk::MemoryAllocateInfo{
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex =
            find_memory_type(phys_device_memory_props,
                             memory_requirements.memoryTypeBits, properties)};

    m_image_memory = l_device.allocateMemory(alloc_info);
    const auto bind_info =
        vk::BindImageMemoryInfo{.image = *m_image, .memory = *m_image_memory};
    l_device.bindImageMemory2(bind_info);
  }

  const auto &operator()() const & { return m_image; }
};

class image_view final {
  vk::raii::ImageView m_image_view = nullptr;

public:
  image_view() = default;

  image_view(const vk::raii::Device &l_device, const vk::raii::Image &image,
             const vk::Format format,
             const vk::ImageAspectFlagBits aspect_flags) {
    // clang-format off
    const auto range = vk::ImageSubresourceRange{.aspectMask = aspect_flags, .baseMipLevel = 0,
                                       .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1};

    const auto iv_create_info = vk::ImageViewCreateInfo{
        .image = *image, .viewType = vk::ImageViewType::e2D,
        .format = format, .subresourceRange = range,
    };
    // clang-format on
    m_image_view = l_device.createImageView(iv_create_info);
  }

  const auto &operator()() const & { return m_image_view; }
};
} // namespace ezvk
