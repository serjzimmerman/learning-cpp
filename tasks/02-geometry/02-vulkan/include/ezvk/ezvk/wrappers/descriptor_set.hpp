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

#include <algorithm>
#include <numeric>
#include <tuple>
#include <vector>

#include "memory.hpp"

namespace ezvk {

inline vk::raii::DescriptorPool
create_descriptor_pool(const vk::raii::Device &l_device,
                       std::span<const vk::DescriptorPoolSize> pool_sizes) {
  const auto accum_func = [](uint32_t sum, vk::DescriptorPoolSize const &dps) {
    return sum + dps.descriptorCount;
  };
  const uint32_t max_sets =
      std::accumulate(pool_sizes.begin(), pool_sizes.end(), 0, accum_func);

  const auto descriptor_info = vk::DescriptorPoolCreateInfo{
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets = max_sets,
      .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
      .pPoolSizes = pool_sizes.data()};

  return vk::raii::DescriptorPool(l_device, descriptor_info);
}

struct descriptor_set {
  vk::raii::DescriptorSetLayout m_layout = nullptr;
  vk::raii::DescriptorSet m_descriptor_set = nullptr;

private:
  struct buffer_description {
    vk::DescriptorType type;
    vk::DeviceSize size;

    const vk::raii::Buffer &buf;
    const vk::raii::BufferView *buf_view = nullptr;
  };

public:
  struct binding_description {
    vk::DescriptorType type;
    uint32_t count;
    vk::ShaderStageFlags flags;
  };

  descriptor_set() = default;

  descriptor_set(const vk::raii::Device &l_device,
                 const ezvk::device_buffers &uniform_buffers,
                 const vk::raii::DescriptorPool &pool,
                 std::span<const binding_description> bindings) {
    m_layout = create_decriptor_set_layout(l_device, bindings);

    const auto set_alloc_info =
        vk::DescriptorSetAllocateInfo{.descriptorPool = *pool,
                                      .descriptorSetCount = 1,
                                      .pSetLayouts = &(*m_layout)};

    m_descriptor_set =
        std::move((vk::raii::DescriptorSets{l_device, set_alloc_info}).front());

    std::vector<buffer_description> buffer_data;
    buffer_data.reserve(uniform_buffers.size());

    for (unsigned i = 0; i < uniform_buffers.size(); ++i) {
      const auto temp =
          buffer_description{vk::DescriptorType::eUniformBuffer, VK_WHOLE_SIZE,
                             uniform_buffers[i].buffer()};
      buffer_data.push_back(temp);
    }

    update(l_device, buffer_data);
  }

  void update(const vk::raii::Device &l_device,
              const std::vector<buffer_description> &buffer_data,
              uint32_t binding_offset = 0) {
    std::vector<vk::DescriptorBufferInfo> buffer_infos;
    buffer_infos.reserve(buffer_data.size());

    std::vector<vk::WriteDescriptorSet> write_descriptor_sets;
    write_descriptor_sets.reserve(buffer_data.size());
    auto dst_binding = binding_offset;

    for (const auto &bd : buffer_data) {
      buffer_infos.push_back(
          {.buffer = *bd.buf, .offset = 0, .range = bd.size});

      vk::BufferView buffer_view;
      if (bd.buf_view)
        buffer_view = **bd.buf_view;

      // clang-format off
      const auto set = vk::WriteDescriptorSet{.dstSet = *m_descriptor_set,
          .dstBinding = dst_binding++, .dstArrayElement = 0,
          .descriptorCount = 1, .descriptorType = bd.type,
          .pBufferInfo = &buffer_infos.back(),
          .pTexelBufferView = (bd.buf_view ? &buffer_view : nullptr)};
      // clang-format on

      write_descriptor_sets.push_back(set);
    }

    l_device.updateDescriptorSets(write_descriptor_sets, nullptr);
  }

private:
  static vk::raii::DescriptorSetLayout create_decriptor_set_layout(
      const vk::raii::Device &l_device,
      std::span<const binding_description> binding_data) {
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    bindings.reserve(binding_data.size());

    std::transform(binding_data.begin(), binding_data.end(),
                   std::back_inserter(bindings),
                   [i = uint32_t{0}](auto &elem) mutable {
                     return vk::DescriptorSetLayoutBinding{
                         i++, elem.type, elem.count, elem.flags};
                   });

    const auto descriptor_set_info = vk::DescriptorSetLayoutCreateInfo{
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data()};

    return vk::raii::DescriptorSetLayout{l_device, descriptor_set_info};
  }
};

} // namespace ezvk
