
#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#include "../vkinit.h"
#include "../device/resources.h"

#include <array>
#include <memory>

static constexpr uint32_t UniformBinding = 0;
static constexpr uint32_t StorageBinding = 1;
static constexpr uint32_t TextureBinding = 2;

static constexpr int UNIFORMCOUNT = 65536;
static constexpr int STORAGECOUNT = 65536;
static constexpr int TEXTURECOUNT = 65536;

namespace descriptors {
    struct PBRParams {
        BufferHandle meshTransforms;
        BufferHandle pointLights;
        BufferHandle camera;
        uint32_t padding;
    };

    struct SkyBoxParams {
        BufferHandle camera;
        TextureHandle skybox;
        uint32_t padding1;
        uint32_t padding2;
    };

    class BindlessParams {
        struct Range {
            uint32_t offset;
            uint32_t size;
            void *data;
        };

    public:
        explicit BindlessParams(uint32_t _minAlignment) : minAlignment(_minAlignment) {}

        template<class TData>
        uint32_t add_range(TData &&data) {
            size_t dataSize = sizeof(TData);
            auto* bytes = new TData;
            *bytes = data;

            uint32_t currentOffset = lastOffset;
            ranges.push_back({currentOffset, dataSize, bytes});

            lastOffset += pad_size_to_min_alignment(dataSize, minAlignment);
            return currentOffset;
        }

        void build(VkDevice device, VmaAllocator allocator, VkDescriptorPool descriptorPool) {
            VkBufferCreateInfo bufferCI{};
            bufferCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            bufferCI.size = lastOffset;

            VmaAllocationCreateInfo allocCI{};
            VmaAllocationInfo allocInfo{};
            vmaCreateBuffer(allocator, &bufferCI, &allocCI, &buffer, &allocation, &allocInfo);

            uint8_t *data = nullptr;
            vmaMapMemory(allocator, allocation, reinterpret_cast<void**>(data));

            for (const auto& range : ranges) {
                memcpy(data + range.offset, range.data, range.size);
            }

            vmaUnmapMemory(allocator, allocation);

            auto layoutBinding = vkinit::ds_layout_binding(
                1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL
            );
            auto layoutCI = vkinit::ds_layout_CI(1, 0);
            vkCreateDescriptorSetLayout(device, &layoutCI, nullptr, &layout);

            uint32_t maxRangeSize = 0;
            for (auto& range : ranges) {
                maxRangeSize = std::max(range.size, maxRangeSize);
            }

            auto dsAI = vkinit::ds_ai(descriptorPool, 1);
            vkAllocateDescriptorSets(device, &dsAI, &descriptorSet);

            auto dsBufferInfo = vkinit::ds_buffer_info(buffer, 0, maxRangeSize);
            auto dsWrite = vkinit::write_ds(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0, descriptorSet, 1, 0);
            vkUpdateDescriptorSets(device, 1, &dsWrite, 0, nullptr);
        }

        [[nodiscard]] VkDescriptorSet get_descriptor_set() const { return descriptorSet; }
        [[nodiscard]] VkDescriptorSetLayout get_descriptor_set_layout() const { return layout; }

    private:
        uint32_t pad_size_to_min_alignment(uint32_t originalSize, uint32_t minAlignment) {
            return (originalSize + minAlignment - 1) & ~(minAlignment - 1);
        }

        uint32_t minAlignment;
        uint32_t lastOffset = 0;
        std::vector<Range> ranges;

        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

        VmaAllocation allocation = VK_NULL_HANDLE;
        VkBuffer buffer = VK_NULL_HANDLE;
    };
}

    /*struct DescriptorSetBuilder {
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
        void build_layout(VkDevice device, VkDescriptorSetLayout& dsLayout);
        void build_pool(VkDevice device, VkDescriptorPool& dsPool);
    };
}*/


#endif //DESCRIPTORS_H
