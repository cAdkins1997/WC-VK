
#ifndef VKCOMMON_H
#define VKCOMMON_H

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <iostream>

#ifdef NDEBUG
#define VK_CHECK_HPP(x)                         \
x                                               \
\

#else

#define VK_CHECK_HPP(x)                         \
do {                                            \
auto result = x;                                \
if (x != vk::Result::eSuccess) {                \
vk::to_string(result);                          \
abort();                                        \
}                                               \
}                                               \
while(0);
#endif


#endif //VKCOMMON_H
