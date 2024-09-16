
#include "application.h"
#include <chrono>

namespace wcvk {
    Application::Application() {
        run();
    }

    Application::~Application() {

    }

    void Application::run() {
        init_descriptors();

        vk::PipelineLayoutCreateInfo pipelineLayoutCI;
        pipelineLayoutCI.pSetLayouts = &drawImagePipeline.descriptorLayout;
        pipelineLayoutCI.setLayoutCount = 1;

        assert(device.device.createPipelineLayout(&pipelineLayoutCI, nullptr, &drawImagePipeline.pipelineLayout) == vk::Result::eSuccess);

        Shader compShader = device.create_shader("../shaders/test.comp.spv");
        vk::PipelineShaderStageCreateInfo stageInfo;
        stageInfo.stage = vk::ShaderStageFlagBits::eCompute;
        stageInfo.module = compShader.module;
        stageInfo.pName = "main";

        vk::ComputePipelineCreateInfo computePipelineCI;
        computePipelineCI.layout = drawImagePipeline.pipelineLayout;
        computePipelineCI.stage = stageInfo;

        drawImagePipeline.pipeline = device.device.createComputePipeline(nullptr, computePipelineCI).value;

        while (!glfwWindowShouldClose(device.window)) {
            draw();
            glfwPollEvents();
        }
    }

    void Application::draw() {
        device.wait_on_work();

        FrameData& currentFrame = device.get_current_frame();
        uint32_t swapchainImageIndex = device.get_swapchain_image_index();
        VkImage& currentSwapchainImage = device.swapchainImages[swapchainImageIndex];
        VkImage& drawImage = device.drawImage.image;

        ComputeContext computeContext(currentFrame.commandBuffer);
        computeContext.begin();
        computeContext.set_pipeline(drawImagePipeline);
        computeContext.image_barrier(drawImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);

        computeContext.dispatch(std::ceil(device.width / 16.0), std::ceil(device.height / 16.0), 1);

        computeContext.image_barrier(drawImage, vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal);
        computeContext.image_barrier(currentSwapchainImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        vk::Extent2D extent;
        extent.height = device.height;
        extent.width = device.width;
        computeContext.copy_image(drawImage, currentSwapchainImage, extent, extent);
        computeContext.image_barrier(currentSwapchainImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR);

        computeContext.end();
        device.submit_compute_work(computeContext);
        device.present();
    }

    void Application::init_descriptors() {

        eastl::vector<DescriptorAllocator::PoolSizeRatio> sizes {
            { vk::DescriptorType::eStorageBuffer, 3 },
            { vk::DescriptorType::eUniformBuffer, 3 },
            { vk::DescriptorType::eCombinedImageSampler, 4 }
        };

        descriptorAllocator.init(device.device, 10, sizes);
        DescriptorLayoutBuilder builder;
        builder.add_binding(0, vk::DescriptorType::eStorageImage);
        drawImagePipeline.descriptorLayout = builder.build(device.device, vk::ShaderStageFlagBits::eCompute);

        drawImagePipeline.set = descriptorAllocator.allocate(device.device, drawImagePipeline.descriptorLayout);

        DescriptorWriter writer;
        writer.write_image(0, device.drawImage.imageView, VK_NULL_HANDLE, vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage);
        writer.update_set(device.device, drawImagePipeline.set);
    }
}
