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

#include <spdlog/spdlog.h>

#include "ezvk/utils/utility.hpp"
#include "unified_includes/vulkan_hpp_include.hpp"
#include "wrappers/instance.hpp"

#include <cstddef>

#include <functional>
#include <iostream>
#include <sstream>
#include <string>

namespace ezvk {

inline std::string
assemble_debug_message(vk::DebugUtilsMessageTypeFlagsEXT message_types,
                       const vk::DebugUtilsMessengerCallbackDataEXT &data) {
  std::stringstream ss;

  std::span<const vk::DebugUtilsLabelEXT> queues = {data.pQueueLabels,
                                                    data.queueLabelCount},
                                          cmdbufs = {data.pCmdBufLabels,
                                                     data.cmdBufLabelCount};
  std::span<const vk::DebugUtilsObjectNameInfoEXT> objects = {data.pObjects,
                                                              data.objectCount};

  ss << "Message [id_name = <" << data.pMessageIdName
     << ">, id_num = " << data.messageIdNumber
     << ", types = " << vk::to_string(message_types)
     << "]: " << utils::trim_leading_trailing_spaces(data.pMessage) << "\n";

  if (!queues.empty())
    ss << " -- Associated Queues: --\n";
  for (uint32_t i = 0; const auto &v : queues) {
    ss << "[" << i++ << "]. name = <" << v.pLabelName << ">\n";
  }

  if (!cmdbufs.empty())
    ss << " -- Associated Command Buffers: --\n";
  for (uint32_t i = 0; const auto &v : cmdbufs) {
    ss << "[" << i++ << "]. name = <" << v.pLabelName << ">\n";
  }

  if (!objects.empty())
    ss << " -- Associated Vulkan Objects: --\n";
  for (uint32_t i = 0; const auto &v : objects) {
    ss << "[" << i++ << "]. type = <" << vk::to_string(v.objectType)
       << ">, handle = " << v.objectHandle;
    if (v.pObjectName)
      ss << ", name = <" << v.pObjectName << ">";
    ss << "\n";
  }

  return ss.str();
};

inline bool default_debug_callback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
    vk::DebugUtilsMessageTypeFlagsEXT message_types,
    const vk::DebugUtilsMessengerCallbackDataEXT &callback_data) {
  using msg_sev = vk::DebugUtilsMessageSeverityFlagBitsEXT;

  auto msg_str = assemble_debug_message(message_types, callback_data);
  if (message_severity == msg_sev::eInfo) {
    spdlog::info(msg_str);
  } else if (message_severity == msg_sev::eWarning) {
    spdlog::warn(msg_str);
  } else if (message_severity == msg_sev::eVerbose) {
    spdlog::debug(msg_str);
  } else if (message_severity == msg_sev::eError) {
    spdlog::error(msg_str);
  } else {
    assert(0);
  }

  return false;
};

using msg_sev = vk::DebugUtilsMessageSeverityFlagBitsEXT;
using msg_type = vk::DebugUtilsMessageTypeFlagBitsEXT;

static constexpr auto default_severity_flags =
    msg_sev::eVerbose | msg_sev::eWarning | msg_sev::eError | msg_sev::eInfo;
static constexpr auto default_type_flags =
    msg_type::eGeneral | msg_type::eValidation | msg_type::ePerformance;

class debug_messenger {
public:
  using callback_type = bool(vk::DebugUtilsMessageSeverityFlagBitsEXT,
                             vk::DebugUtilsMessageTypeFlagsEXT,
                             const vk::DebugUtilsMessengerCallbackDataEXT &);

private:
  vk::raii::DebugUtilsMessengerEXT m_messenger = nullptr;
  std::unique_ptr<std::function<callback_type>>
      m_callback; // Here unqiue ptr is needed so that if the object is moved
                  // from, then user_data pointer does not change.

private:
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                 VkDebugUtilsMessageTypeFlagsEXT message_types,
                 const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                 void *user_data) {
    auto *ptr = static_cast<std::function<callback_type> *>(user_data);

    // NOTE[Sergei]: I'm not sure if callback_data ptr can be nullptr. Look here
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/PFN_vkDebugUtilsMessengerCallbackEXT.html
    assert(callback_data);
    auto severity =
        static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(message_severity);
    auto types = static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(message_types);
    auto data =
        *reinterpret_cast<const vk::DebugUtilsMessengerCallbackDataEXT *>(
            callback_data);
    auto result = (*ptr)(severity, types, data);

    return (result ? VK_TRUE : VK_FALSE);
  }

public:
  debug_messenger() = default;

  debug_messenger(
      const vk::raii::Instance &instance,
      std::function<callback_type> callback = default_debug_callback,
      vk::DebugUtilsMessageSeverityFlagsEXT severity_flags =
          default_severity_flags,
      vk::DebugUtilsMessageTypeFlagsEXT type_flags = default_type_flags)
      : m_callback{std::make_unique<std::function<callback_type>>(callback)} {
    vk::DebugUtilsMessengerCreateInfoEXT create_info = {
        .messageSeverity = severity_flags,
        .messageType = type_flags,
        .pfnUserCallback = debug_callback,
        .pUserData = m_callback.get()};

    m_messenger = instance.createDebugUtilsMessengerEXT(create_info);
  }

  const auto &operator()() const & { return m_messenger; }
};

}; // namespace ezvk
