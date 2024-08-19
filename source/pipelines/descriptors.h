
#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#include "../device.h"
#include <array>

enum class TextureHandle : uint32_t { Invalid = 0 };
enum class BufferHandle : uint32_t { Invalid = 0 };

static constexpr uint32_t UniformBinding = 0;
static constexpr uint32_t StorageBinding = 1;
static constexpr uint32_t TextureBinding = 2;

static constexpr int UNIFORMCOUNT = 65536;
static constexpr int STORAGECOUNT = 65536;
static constexpr int TEXTURECOUNT = 65536;

namespace descriptors {
    struct DescriptorSetBuilder {
        void build(VkDevice device, VkDescriptorSetLayout& dsLayout, VkDescriptorPool& dsPool, VkDescriptorSet& ds);

        std::array<VkDescriptorSetLayoutBinding, 3> bindings{};
        std::array<VkDescriptorBindingFlags, 3> flags{};
        std::array<VkDescriptorType, 3> types{
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
        };

        std::vector<VkDescriptorPoolSize> poolSizes = {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, UNIFORMCOUNT },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, STORAGECOUNT },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, TEXTURECOUNT},
        };

        static constexpr uint32_t MAX_BINDLESS_RES = 196608;

    private:
        Device deivce;

        void build_layout(VkDevice device, VkDescriptorSetLayout& dsLayout);
        void build_pool(VkDevice device, VkDescriptorPool& dsPool);
    };
}


#endif //DESCRIPTORS_H
