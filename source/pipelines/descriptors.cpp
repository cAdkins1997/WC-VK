
#include "descriptors.h"

namespace descriptors {
    void DescriptorSetBuilder::build(VkDevice device, VkDescriptorSetLayout& dsLayout, VkDescriptorPool& dsPool, VkDescriptorSet& ds) {
        build_layout(device, dsLayout);
        build_pool(device, dsPool);

        VkDescriptorSetAllocateInfo dsAI = vkinit::ds_ai(dsPool, 1);
        dsAI.pSetLayouts = &dsLayout;
        vkAllocateDescriptorSets(device, &dsAI, &ds);
    }

    void DescriptorSetBuilder::build_layout(VkDevice device, VkDescriptorSetLayout& dsLayout) {
        for (uint32_t i = 0; i < 3; ++i) {
            bindings.at(i).binding = i;
            bindings.at(i).descriptorType = types.at(i);
            bindings.at(i).descriptorCount = MAX_BINDLESS_RES;
            bindings.at(i).stageFlags = VK_SHADER_STAGE_ALL;
            flags.at(i) = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
        }

        VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags = vkinit::ds_layout_binding_flags_CI(flags.data(), 3);
        VkDescriptorSetLayoutCreateInfo dsLayoutCI =vkinit::ds_layout_CI(3,VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT);
        dsLayoutCI.pBindings = bindings.data();
        dsLayoutCI.pNext = &bindingFlags;

        vkCreateDescriptorSetLayout(device, &dsLayoutCI, nullptr, &dsLayout);
    }

    void DescriptorSetBuilder::build_pool(VkDevice device, VkDescriptorPool& dsPool) {
        VkDescriptorPoolCreateInfo dsPoolCI = vkinit::ds_pool_CI(
            VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            MAX_BINDLESS_RES * poolSizes.size(),
            poolSizes.size(),
            poolSizes.data()
            );

        vkCreateDescriptorPool(device, &dsPoolCI, nullptr, &dsPool);
    }
}
