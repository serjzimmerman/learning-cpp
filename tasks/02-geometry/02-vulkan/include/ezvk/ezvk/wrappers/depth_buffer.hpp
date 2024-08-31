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

#include "ezvk/utils/algorithm.hpp"
#include "image.hpp"
#include "memory.hpp"

namespace ezvk {

inline std::vector<vk::Format>
find_depth_format(const vk::raii::PhysicalDevice &p_device) {
  auto predicate = [&p_device](const auto &candidate) -> bool {
    auto props = p_device.getFormatProperties(candidate);
    return (props.optimalTilingFeatures &
            vk::FormatFeatureFlagBits::eDepthStencilAttachment) ==
           vk::FormatFeatureFlagBits::eDepthStencilAttachment;
  };

  std::array<vk::Format, 3> candidates = {vk::Format::eD32Sfloat,
                                          vk::Format::eD32SfloatS8Uint,
                                          vk::Format::eD24UnormS8Uint};

  return utils::find_all_that_satisfy(candidates.begin(), candidates.end(),
                                      predicate);
}

inline auto create_depth_attachment(vk::Format depth_format) {
  return vk::AttachmentDescription{
      .flags = {},
      .format = depth_format,
      .samples = vk::SampleCountFlagBits::e1,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eDontCare,
      .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
      .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
      .initialLayout = vk::ImageLayout::eUndefined,
      .finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal};
}

struct depth_buffer final {
public:
  ezvk::image m_image;
  ezvk::image_view m_image_view;

private:
  vk::Format m_depth_format;

public:
  depth_buffer() = default;

  depth_buffer(const vk::raii::PhysicalDevice &p_device,
               const vk::raii::Device &l_device, vk::Format depth_format,
               const vk::Extent2D &extent2d)
      : m_depth_format{depth_format} {
    const auto extent3d = vk::Extent3D{
        .width = extent2d.width, .height = extent2d.height, .depth = 1};

    // clang-format off
    m_image = {p_device, l_device, extent3d, depth_format, vk::ImageTiling::eOptimal,
               vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal};
    // clang-format on

    m_image_view = ezvk::image_view{l_device, m_image(), depth_format,
                                    vk::ImageAspectFlagBits::eDepth};
  }

  auto depth_format() const { return m_depth_format; }
};

} // namespace ezvk
