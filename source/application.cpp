
#include "application.h"
#include <chrono>

#include "pipelines/graphicspipelines.h"

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

        Shader vertShader = device.create_shader("../shaders/test.frag.spv");
        Shader fragShader = device.create_shader("../shaders/test.vert.spv");

        PipelineBuilder pipelineBuilder;
        pipelineBuilder.pipelineLayout = trianglePipeline.pipelineLayout;
        pipelineBuilder.set_shader(vertShader.module, fragShader.module);
        pipelineBuilder.set_input_topology(vk::PrimitiveTopology::eTriangleList);
        pipelineBuilder.set_polygon_mode(vk::PolygonMode::eFill);
        pipelineBuilder.set_cull_mode(vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise);
        pipelineBuilder.set_multisampling_none();
        pipelineBuilder.disable_depthtest();
        pipelineBuilder.set_color_attachment_format(device.drawImage.imageFormat);
        pipelineBuilder.set_depth_format(vk::Format::eUndefined);
        trianglePipeline.pipeline = pipelineBuilder.build_pipeline(device.device);

        device.device.destroyShaderModule(compShader.module);
        device.device.destroyShaderModule(vertShader.module);
        device.device.destroyShaderModule(fragShader.module);

        while (!glfwWindowShouldClose(device.window)) {
            draw();
            glfwPollEvents();
        }
    }

    void Application::draw() {
        device.wait_on_work();

        FrameData& currentFrame = device.get_current_frame();
        VkImage& currentSwapchainImage = device.get_swapchain_image();
        VkImage& drawImage = device.get_draw_image();

        ComputeContext computeContext(currentFrame.commandBuffer);
        computeContext.begin();
        computeContext.set_pipeline(drawImagePipeline);
        computeContext.image_barrier(drawImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);

        computeContext.dispatch(std::ceil(device.width / 16.0), std::ceil(device.height / 16.0), 1);
        computeContext.end();

        GraphicsContext graphicsContext(currentFrame.commandBuffer);
        graphicsContext.set_pipeline(trianglePipeline);
        graphicsContext.image_barrier(drawImage, vk::ImageLayout::eGeneral, vk::ImageLayout::eColorAttachmentOptimal);
        graphicsContext.set_up_render_pass(device.drawImage);
        graphicsContext.set_viewport(0, 0, 0.0f, 1.0f);
        graphicsContext.set_scissor(0, 0);
        graphicsContext.draw();

        graphicsContext.image_barrier(drawImage, vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal);
        graphicsContext.image_barrier(currentSwapchainImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        graphicsContext.copy_image(drawImage, currentSwapchainImage, {device.height, device.width}, {device.height, device.width});
        graphicsContext.image_barrier(currentSwapchainImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR);

        graphicsContext.end();
        device.submit_compute_work(computeContext);
        device.submit_graphics_work(graphicsContext);
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
