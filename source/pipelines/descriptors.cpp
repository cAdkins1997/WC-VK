
#pragma once
#include "descriptors.h"

void DescriptorLayoutBuilder::add_binding(uint32_t binding, vk::DescriptorType type) {
    vk::DescriptorSetLayoutBinding newBind{};
    newBind.binding = binding;
    newBind.descriptorCount = 1;
    newBind.descriptorType = type;
    bindings.push_back(newBind);
}

void DescriptorLayoutBuilder::clear() {
    bindings.clear();
}

vk::DescriptorSetLayout DescriptorLayoutBuilder::build(vk::Device device, vk::Flags<vk::ShaderStageFlagBits> stages, void *next,
    vk::DescriptorSetLayoutCreateFlags flags) {

    for (auto& binding : bindings) {
        binding.stageFlags |= stages;
    }

    vk::DescriptorSetLayoutCreateInfo layoutCI;
    layoutCI.pNext = next;
    layoutCI.pBindings = bindings.data();
    layoutCI.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutCI.flags = flags;

    vk::DescriptorSetLayout set;
    if (device.createDescriptorSetLayout(&layoutCI, nullptr, &set) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create descriptor set layout\n");
    }

    return set;
}

void DescriptorAllocator::init(const vk::Device& device, uint32_t maxSets, eastl::span<PoolSizeRatio> poolRatios) {
    ratios.clear();

    for (auto& ratio : ratios) {
        ratios.push_back(ratio);
    }

    vk::DescriptorPool newPool = create_pool(device, maxSets, poolRatios);

    setsPerPool = maxSets * 1.5f;

    readyPools.push_back(newPool);
}

void DescriptorAllocator::clear_pools(const vk::Device& device) {

    for (auto& pool : readyPools) {
        device.resetDescriptorPool(pool, {});
    }

    for (auto& pool : fullPools) {
        device.resetDescriptorPool(pool, {});
    }

    fullPools.clear();
}

void DescriptorAllocator::destroy_pools(const vk::Device& device) {
    for (auto& pool : readyPools) {
        device.destroyDescriptorPool(pool, {});
    }
    readyPools.clear();

    for (auto& pool : fullPools) {
        device.destroyDescriptorPool(pool, {});
    }

    fullPools.clear();

}

vk::DescriptorSet DescriptorAllocator::allocate(const vk::Device &device, vk::DescriptorSetLayout layout, void *pNext) {
    vk::DescriptorPool poolToUse = get_pool(device);

    vk::DescriptorSetAllocateInfo setAI(poolToUse, 1, &layout);

    vk::DescriptorSet set;
    auto result = device.allocateDescriptorSets(&setAI, &set);

    if (result == vk::Result::eErrorOutOfPoolMemory || result == vk::Result::eErrorFragmentedPool) {
        fullPools.push_back(poolToUse);
        poolToUse = get_pool(device);
        setAI.descriptorPool = poolToUse;

        if (device.allocateDescriptorSets(&setAI, &set) != vk::Result::eSuccess) {

        }

        readyPools.push_back(poolToUse);
    }

    return set;
}


vk::DescriptorPool DescriptorAllocator::get_pool(const vk::Device& device) {
    vk::DescriptorPool newPool;
    if (!readyPools.empty()) {
        newPool = readyPools.back();
        readyPools.pop_back();
    }
    else {
        newPool = create_pool(device, setsPerPool, ratios);

        setsPerPool = setsPerPool * 1.5;
        if (setsPerPool > 4092) {
            setsPerPool = 4092;
        }
    }
    return newPool;
}

vk::DescriptorPool DescriptorAllocator::create_pool(const vk::Device& device, int32_t setCount, eastl::span<PoolSizeRatio> poolSizeRatio) {

    eastl::vector<vk::DescriptorPoolSize> poolSizes;
    for (PoolSizeRatio& ratio : ratios) {
        poolSizes.push_back(vk::DescriptorPoolSize(ratio.type, static_cast<uint32_t>(ratio.ratio * setCount)));
    }

    vk::DescriptorPoolCreateInfo poolCI({}, setCount, static_cast<uint32_t>(poolSizes.size()), poolSizes.data());

    vk::DescriptorPool newPool = device.createDescriptorPool(poolCI, nullptr);
    return newPool;
}

void DescriptorWriter::write_image(int32_t binding, vk::ImageView image, vk::Sampler sampler, vk::ImageLayout layout, vk::DescriptorType type) {
    imageInfos.emplace_back(sampler, image, layout);
    vk::DescriptorImageInfo& imageInfo = imageInfos.front();

    vk::WriteDescriptorSet write(VK_NULL_HANDLE, binding, {}, 1);
    write.sType = vk::StructureType::eWriteDescriptorSet;
    write.dstBinding = binding;
    write.dstSet = VK_NULL_HANDLE;
    write.descriptorCount = 1;
    write.descriptorType = type;
    write.pImageInfo = &imageInfo;

    writes.push_back(write);
}


void DescriptorWriter::write_buffer(int32_t binding, vk::Buffer buffer, size_t size, size_t offset, vk::DescriptorType type) {
    bufferInfos.emplace_back(buffer, offset, size);
    vk::DescriptorBufferInfo& bufferInfo = bufferInfos.back();

    vk::WriteDescriptorSet write;
    write.sType = vk::StructureType::eWriteDescriptorSet;
    write.dstBinding = binding;
    write.dstSet = VK_NULL_HANDLE;
    write.descriptorCount = 1;
    write.descriptorType = type;
    write.pBufferInfo = &bufferInfo;
    writes.push_back(write);
}

void DescriptorWriter::clear() {

    imageInfos.clear();
    writes.clear();
    bufferInfos.clear();
}


void DescriptorWriter::update_set(vk::Device device, vk::DescriptorSet set) {

    for (vk::WriteDescriptorSet& write : writes) {
        write.dstSet = set;
    }

    device.updateDescriptorSets(static_cast<uint32_t>(writes.size()), &writes.front(), 0, nullptr);
}
