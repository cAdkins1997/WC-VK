
#pragma once
#include "../glmdefines.h"

#include "../vkcommon.h"
#include <vector>
#include <span>
#include <deque>

struct DescriptorLayoutBuilder {

    std::vector<vk::DescriptorSetLayoutBinding> bindings;

    void add_binding(uint32_t binding, vk::DescriptorType type);
    void clear();

    vk::DescriptorSetLayout build(vk::Device device, vk::Flags<vk::ShaderStageFlagBits> stages, void* next = nullptr, vk::DescriptorSetLayoutCreateFlags flags = {});
};

struct DescriptorAllocator {

    struct PoolSizeRatio {
        vk::DescriptorType type;
        float ratio;
    };

    void init(const vk::Device& device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
    void clear_pools(const vk::Device& device);
    void destroy_pools(const vk::Device& device);

    vk::DescriptorSet allocate(const vk::Device& device, vk::DescriptorSetLayout layout, void* pNext = nullptr);

private:
    vk::DescriptorPool get_pool(const vk::Device& device);
    vk::DescriptorPool create_pool(const vk::Device& device, int32_t setCount, std::span<PoolSizeRatio> poolSizeRatio);

    std::vector<PoolSizeRatio> ratios;
    std::vector<vk::DescriptorPool> fullPools;
    std::vector<vk::DescriptorPool> readyPools;
    uint32_t setsPerPool{};
};

struct DescriptorWriter {

    std::deque<vk::DescriptorImageInfo> imageInfos;
    std::deque<vk::DescriptorBufferInfo> bufferInfos;
    std::vector<vk::WriteDescriptorSet> writes;

    void write_image(int32_t binding, vk::ImageView image, vk::Sampler sampler, vk::ImageLayout layout, vk::DescriptorType type);
    void write_buffer(int32_t binding, vk::Buffer buffer, size_t size, size_t offset, vk::DescriptorType type);

    void clear();
    void update_set(vk::Device device, vk::DescriptorSet set);
};