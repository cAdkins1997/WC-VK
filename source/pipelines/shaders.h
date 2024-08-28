
#ifndef SHADERS_H
#define SHADERS_H

#include "../device/device.h"

#include <fstream>

namespace shaders {

    bool load_shader_module(VkDevice device, const char* filePath, VkShaderModule* outShaderModule);

}

#endif //SHADERS_H
