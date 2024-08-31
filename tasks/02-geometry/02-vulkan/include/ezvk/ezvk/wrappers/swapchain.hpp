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

#include "queues.hpp"
#include "unified_includes/vulkan_hpp_include.hpp"

#include <algorithm>
#include <cstddef>

namespace ezvk {

class swapchain {
  vk::raii::SwapchainKHR m_handle = nullptr;

  vk::SurfaceFormatKHR m_format;
  vk::Extent2D m_extent;

  std::vector<vk::Image> m_images;
  std::vector<vk::raii::ImageView> m_image_views;

  uint32_t m_min_image_count;

private:
  static vk::SurfaceFormatKHR choose_swapchain_surface_format(
      const std::vector<vk::SurfaceFormatKHR> &formats) {
    const auto format_it =
        std::find_if(formats.begin(), formats.end(), [](auto &format) {
          return (format.format == vk::Format::eB8G8R8A8Unorm &&
                  format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear);
        });
    if (format_it != formats.end())
      return *format_it;
    return formats.front();
  }

  static vk::PresentModeKHR choose_swapchain_present_mode(
      const std::vector<vk::PresentModeKHR> &present_modes) {
    const auto present_mode_it = std::find_if(
        present_modes.begin(), present_modes.end(),
        [](auto &mode) { return mode == vk::PresentModeKHR::eMailbox; });
    if (present_mode_it != present_modes.end())
      return *present_mode_it;
    return vk::PresentModeKHR::eFifo;
  }

  static vk::Extent2D
  choose_swapchain_extent(const vk::Extent2D &p_extent,
                          const vk::SurfaceCapabilitiesKHR &p_cap) {
    // In some systems UINT32_MAX is a flag to say that the size has not been
    // specified
    if (p_cap.currentExtent.width != UINT32_MAX)
      return p_cap.currentExtent;
    const auto extent = vk::Extent2D{
        std::min(p_cap.maxImageExtent.width,
                 std::max(p_cap.minImageExtent.width, p_extent.width)),
        std::min(p_cap.maxImageExtent.height,
                 std::max(p_cap.minImageExtent.height, p_extent.height))};
    return extent;
  }

  static constexpr auto c_subrange_info =
      vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eColor,
                                .baseMipLevel = 0,
                                .levelCount = 1,
                                .baseArrayLayer = 0,
                                .layerCount = 1};

public:
  swapchain() = default;

  swapchain(const vk::raii::PhysicalDevice &p_device,
            const vk::raii::Device &l_device,
            const vk::raii::SurfaceKHR &surface, const vk::Extent2D &extent,
            i_graphics_present_queues *queues,
            const vk::SwapchainKHR &old_swapchain = {}) {
    const auto capabilities = p_device.getSurfaceCapabilitiesKHR(*surface);
    const auto present_mode = choose_swapchain_present_mode(
        p_device.getSurfacePresentModesKHR(*surface));

    m_extent = choose_swapchain_extent(extent, capabilities);
    m_format = choose_swapchain_surface_format(
        p_device.getSurfaceFormatsKHR(*surface));

    m_min_image_count =
        std::max(capabilities.maxImageCount, capabilities.minImageCount + 1);

    auto create_info = vk::SwapchainCreateInfoKHR{
        .surface = *surface,
        .minImageCount = m_min_image_count,
        .imageFormat = m_format.format,
        .imageColorSpace = m_format.colorSpace,
        .imageExtent = m_extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = old_swapchain};

    const auto qfis = std::array{queues->present().family_index(),
                                 queues->graphics().family_index()};

    if (queues->graphics().family_index() != queues->present().family_index()) {
      create_info.imageSharingMode = vk::SharingMode::eConcurrent;
      create_info.queueFamilyIndexCount = qfis.size();
      create_info.pQueueFamilyIndices = qfis.data();
    }

    else {
      create_info.imageSharingMode = vk::SharingMode::eExclusive;
    }

    m_handle = l_device.createSwapchainKHR(create_info);
    m_images = (*l_device).getSwapchainImagesKHR(*m_handle);

    auto image_view_create_info =
        vk::ImageViewCreateInfo{.viewType = vk::ImageViewType::e2D,
                                .format = create_info.imageFormat,
                                .components = {},
                                .subresourceRange = c_subrange_info};

    for (auto &image : m_images) {
      image_view_create_info.image = image;
      m_image_views.push_back(l_device.createImageView(image_view_create_info));
    }
  }

  auto &operator()() & { return m_handle; }
  const auto &operator()() const & { return m_handle; }

  auto &images() const & { return m_images; }
  auto &image_views() const & { return m_image_views; }

  vk::SurfaceFormatKHR format() const { return m_format; }
  vk::Extent2D extent() const { return m_extent; }

  auto min_image_count() const { return m_min_image_count; }
};

} // namespace ezvk
