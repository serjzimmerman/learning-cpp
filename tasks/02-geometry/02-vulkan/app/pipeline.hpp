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

#include <numeric>
#include <tuple>
#include <vector>

#include "ezvk/wrappers/descriptor_set.hpp"
#include "ezvk/wrappers/memory.hpp"
#include "ezvk/wrappers/shaders.hpp"

#include "misc/vertex.hpp"

namespace triangles {

template <typename t_vertex_type> class pipeline final {
  vk::raii::Pipeline m_pipeline = nullptr;
  vk::raii::ShaderModule m_vertex_shader = nullptr, m_fragment_shader = nullptr;

private:
  static auto vertex_input_state_create_info(
      const vk::VertexInputBindingDescription &binding, const auto &attr) {
    return vk::PipelineVertexInputStateCreateInfo{
        .flags = vk::PipelineVertexInputStateCreateFlags(),
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &binding,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attr.size()),
        .pVertexAttributeDescriptions = attr.data()};
  }

  static auto color_blend_state_create_info(
      const vk::PipelineColorBlendAttachmentState &attach) {
    return vk::PipelineColorBlendStateCreateInfo{
        .logicOpEnable = VK_FALSE,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &attach,
        .blendConstants = {},
    };
  }

  auto add_shader_stages(const vk::raii::Device &l_device, std::string vertex,
                         std::string frag) {
    std::vector<vk::PipelineShaderStageCreateInfo> shader_stages;

    const auto add_stage = [&shader_stages, &l_device](auto path, auto stage,
                                                       auto &shader_module) {
      shader_module = ezvk::create_module(path, l_device);
      vk::PipelineShaderStageCreateInfo info = {
          .stage = stage, .module = *shader_module, .pName = "main"};
      shader_stages.push_back(info);
    };

    add_stage(vertex, vk::ShaderStageFlagBits::eVertex, m_vertex_shader);
    add_stage(frag, vk::ShaderStageFlagBits::eFragment, m_fragment_shader);

    return shader_stages;
  }

  static constexpr auto c_color_attachments =
      vk::PipelineColorBlendAttachmentState{
          .blendEnable = VK_FALSE,
          .colorWriteMask =
              vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
              vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

  static constexpr auto c_dynamic_states =
      std::array{vk::DynamicState::eViewport, vk::DynamicState::eScissor};
  static constexpr auto c_viewport_info =
      vk::PipelineViewportStateCreateInfo{.viewportCount = 1,
                                          .pViewports = nullptr,
                                          .scissorCount = 1,
                                          .pScissors = nullptr};

  static constexpr auto c_depth_stencil_info =
      vk::PipelineDepthStencilStateCreateInfo{.depthTestEnable = VK_TRUE,
                                              .depthWriteEnable = VK_TRUE,
                                              .depthCompareOp =
                                                  vk::CompareOp::eLess,
                                              .depthBoundsTestEnable = VK_FALSE,
                                              .stencilTestEnable = VK_FALSE,
                                              .minDepthBounds = 0.0f,
                                              .maxDepthBounds = 1.0f};

  static constexpr auto c_multisampling_info =
      vk::PipelineMultisampleStateCreateInfo{.rasterizationSamples =
                                                 vk::SampleCountFlagBits::e1,
                                             .sampleShadingEnable = VK_FALSE};

public:
  pipeline() = default;

  pipeline(const vk::raii::Device &l_device, std::string vertex,
           std::string frag, const vk::raii::PipelineLayout &layout,
           const vk::raii::RenderPass &pass,
           vk::PipelineRasterizationStateCreateInfo rast_info,
           vk::PrimitiveTopology primitive_topology) {
    const auto binding_description = t_vertex_type::get_binding_description();
    const auto attribute_description =
        t_vertex_type::get_attribute_description();

    const auto vertex_input_info = vertex_input_state_create_info(
        binding_description, attribute_description);
    const auto color_blend_info =
        color_blend_state_create_info(c_color_attachments);

    const auto input_asm_info = vk::PipelineInputAssemblyStateCreateInfo{
        .flags = vk::PipelineInputAssemblyStateCreateFlags(),
        .topology = primitive_topology};

    const auto dynamic_state_info = vk::PipelineDynamicStateCreateInfo{
        .dynamicStateCount = static_cast<uint32_t>(c_dynamic_states.size()),
        .pDynamicStates = c_dynamic_states.data()};

    auto shader_stages = add_shader_stages(l_device, vertex, frag);

    // clang-format off
    const auto pipeline_info = vk::GraphicsPipelineCreateInfo{
        .stageCount = static_cast<uint32_t>(shader_stages.size()), .pStages = shader_stages.data(),
        .pVertexInputState = &vertex_input_info, .pInputAssemblyState = &input_asm_info,
        .pViewportState = &c_viewport_info, .pRasterizationState = &rast_info,
        .pMultisampleState = &c_multisampling_info, .pDepthStencilState = &c_depth_stencil_info,
        .pColorBlendState = &color_blend_info, .pDynamicState = &dynamic_state_info,
        .layout = *layout, .renderPass = *pass, .subpass = 0, .basePipelineHandle = nullptr};
    // clang-format on

    m_pipeline = l_device.createGraphicsPipeline(nullptr, pipeline_info);
  }

  const auto &operator()() const & { return m_pipeline; }
};

} // namespace triangles
