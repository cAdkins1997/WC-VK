
#include "application.h"
#include <chrono>

#include "meshes/meshes.h"
#include "pipelines/graphicspipelines.h"

namespace wcvk {
    Application::Application() {
        run();
    }

    Application::~Application() {
        vk::Device& d = device.get_handle();
        d.waitIdle();
        d.destroyPipelineLayout(drawImagePipeline.pipelineLayout);
        d.destroyPipelineLayout(trianglePipeline.pipelineLayout);
        d.destroyPipeline(drawImagePipeline.pipeline);
        d.destroyPipeline(trianglePipeline.pipeline);
    }

    void Application::run() {
        drawImage = device.get_draw_image();
        drawHandle = drawImage.get_handle();
        drawImageExtent.height = drawImage.get_height();
        drawImageExtent.width = drawImage.get_width();

        init_descriptors();

        vk::PipelineLayoutCreateInfo drawImagePipelineLayoutCI;
        drawImagePipelineLayoutCI.pSetLayouts = &drawImagePipeline.descriptorLayout;
        drawImagePipelineLayoutCI.setLayoutCount = 1;

        drawImagePipeline.pipelineLayout = device.get_handle().createPipelineLayout(drawImagePipelineLayoutCI, nullptr);

        Shader compShader = device.create_shader("../shaders/test.comp.spv");
        vk::PipelineShaderStageCreateInfo stageInfo;
        stageInfo.stage = vk::ShaderStageFlagBits::eCompute;
        stageInfo.module = compShader.module;
        stageInfo.pName = "main";

        vk::ComputePipelineCreateInfo computePipelineCI;
        computePipelineCI.layout = drawImagePipeline.pipelineLayout;
        computePipelineCI.stage = stageInfo;

        drawImagePipeline.pipeline = device.get_handle().createComputePipeline(nullptr, computePipelineCI).value;


        Shader vertShader = device.create_shader("../shaders/test.vert.spv");
        Shader fragShader = device.create_shader("../shaders/test.frag.spv");

        vk::PipelineLayoutCreateInfo trianglePipelineLayoutCI;
        trianglePipelineLayoutCI.pSetLayouts = &trianglePipeline.descriptorLayout;
        trianglePipelineLayoutCI.setLayoutCount = 1;
        trianglePipeline.pipelineLayout = device.get_handle().createPipelineLayout(trianglePipelineLayoutCI, nullptr);

        PipelineBuilder pipelineBuilder;
        pipelineBuilder.pipelineLayout = trianglePipeline.pipelineLayout;
        pipelineBuilder.set_shader(vertShader.module, fragShader.module);
        pipelineBuilder.set_input_topology(vk::PrimitiveTopology::eTriangleList);
        pipelineBuilder.set_polygon_mode(vk::PolygonMode::eFill);

        pipelineBuilder.set_cull_mode(vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise);
        pipelineBuilder.set_multisampling_none();
        pipelineBuilder.disable_depthtest();
        pipelineBuilder.disable_blending();
        pipelineBuilder.set_color_attachment_format(device.drawImage.imageFormat);
        pipelineBuilder.set_depth_format(vk::Format::eUndefined);
        trianglePipeline.pipeline = pipelineBuilder.build_pipeline(device.device);

        device.get_handle().destroyShaderModule(vertShader.module);
        device.get_handle().destroyShaderModule(fragShader.module);
        device.get_handle().destroyShaderModule(compShader.module);

        while (!glfwWindowShouldClose(device.window)) {
            glfwPollEvents();
            draw();
        }
    }

    void Application::draw() {
        device.wait_on_work();

        FrameData& currentFrame = device.get_current_frame();
        VkImage& currentSwapchainImage = device.get_swapchain_image();
        if (device.resizeRequested) {
            return;
        }

        commands::ComputeContext computeContext(currentFrame.computeCommandBuffer);
        computeContext.begin();
        computeContext.bind_pipeline(drawImagePipeline);
        computeContext.image_barrier(drawHandle, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);
        computeContext.dispatch(std::ceil(device.width), std::ceil(device.height), 1);
        computeContext.end();

        commands::GraphicsContext graphicsContext(currentFrame.graphicsCommandBuffer);
        graphicsContext.begin();
        graphicsContext.bind_pipeline(trianglePipeline);

        graphicsContext.image_barrier(drawHandle, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
        vk::RenderingAttachmentInfo drawAttachment(drawImage.imageView, vk::ImageLayout::eColorAttachmentOptimal);

        graphicsContext.set_up_render_pass(drawImageExtent, &drawAttachment, nullptr);
        graphicsContext.bind_pipeline(trianglePipeline);
        graphicsContext.set_viewport(drawImageExtent, 0.0f, 1.0f);
        graphicsContext.set_scissor(drawImageExtent);
        graphicsContext.draw();

        graphicsContext.image_barrier(drawHandle, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal);
        graphicsContext.image_barrier(currentSwapchainImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        graphicsContext.copy_image(drawHandle, currentSwapchainImage, drawImageExtent, drawImageExtent);
        graphicsContext.image_barrier(currentSwapchainImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR);
        graphicsContext.end();

        device.submit_compute_work(computeContext, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eComputeShader);
        device.submit_graphics_work(graphicsContext, vk::PipelineStageFlagBits2::eComputeShader, vk::PipelineStageFlagBits2::eAllGraphics);

        device.present();
    }

    void Application::init_descriptors() {

        eastl::vector<DescriptorAllocator::PoolSizeRatio> sizes {
            { vk::DescriptorType::eStorageBuffer, 3 },
            { vk::DescriptorType::eUniformBuffer, 3 },
            { vk::DescriptorType::eCombinedImageSampler, 4 }
        };

        descriptorAllocator.init(device.device, 10, sizes);

        {
            DescriptorLayoutBuilder builder;
            builder.add_binding(0, vk::DescriptorType::eStorageImage);
            drawImagePipeline.descriptorLayout = builder.build(device.device, vk::ShaderStageFlagBits::eCompute);

            drawImagePipeline.set = descriptorAllocator.allocate(device.device, drawImagePipeline.descriptorLayout);

            DescriptorWriter writer;
            writer.write_image(0, device.drawImage.imageView, VK_NULL_HANDLE, vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage);
            writer.update_set(device.device, drawImagePipeline.set);
        }

        for (uint32_t i = 0; i < core::MAX_FRAMES_IN_FLIGHT; i++) {
            eastl::vector<DescriptorAllocator::PoolSizeRatio> frame_sizes {
                { vk::DescriptorType::eStorageImage, 3 },
                { vk::DescriptorType::eStorageBuffer, 3 },
                { vk::DescriptorType::eUniformBuffer, 3 },
                { vk::DescriptorType::eCombinedImageSampler, 4 },
            };

            device.frames[i].frameDescriptors = DescriptorAllocator{};
            device.frames[i].frameDescriptors.init(device.device, 1000, frame_sizes);
        }

        {
            DescriptorLayoutBuilder builder;
            builder.add_binding(0, vk::DescriptorType::eStorageImage);
            trianglePipeline.descriptorLayout = builder.build(device.device, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

            trianglePipeline.set = descriptorAllocator.allocate(device.device, trianglePipeline.descriptorLayout);

            DescriptorWriter writer;
            writer.write_image(0, device.drawImage.imageView, nullptr, vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage);
            writer.update_set(device.device, trianglePipeline.set);
        }
    }

    eastl::optional<eastl::vector<eastl::shared_ptr<Mesh>>> Application::load_GLTF_meshs(const char *path) {

    }
}
