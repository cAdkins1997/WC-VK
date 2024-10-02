
#include "application.h"
#include <chrono>

#include "pipelines/graphicspipelines.h"

namespace wcvk {
    Application::Application() {
        run();
    }

    Application::~Application() {
        const vk::Device& d = device.get_handle();
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
        drawImagePipelineLayoutCI.pushConstantRangeCount = 0;
        drawImagePipelineLayoutCI.pPushConstantRanges = nullptr;

        drawImagePipeline.pipelineLayout = device.get_handle().createPipelineLayout(drawImagePipelineLayoutCI, nullptr);

        Shader compShader = device.create_shader( "../shaders/triangle.comp.spv");
        vk::PipelineShaderStageCreateInfo stageInfo;
        stageInfo.stage = vk::ShaderStageFlagBits::eCompute;
        stageInfo.module = compShader.module;
        stageInfo.pName = "main";

        vk::ComputePipelineCreateInfo computePipelineCI;
        computePipelineCI.layout = drawImagePipeline.pipelineLayout;
        computePipelineCI.stage = stageInfo;

        drawImagePipeline.pipeline = device.get_handle().createComputePipeline(nullptr, computePipelineCI).value;

        Shader vertShader = device.create_shader("../shaders/meshbufferBDA.vert.spv");
        Shader fragShader = device.create_shader("../shaders/triangle.frag.spv");

        vk::PushConstantRange pcRange(vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants));

        vk::PipelineLayoutCreateInfo trianglePipelineLayoutCI;
        trianglePipelineLayoutCI.pSetLayouts = &trianglePipeline.descriptorLayout;
        trianglePipelineLayoutCI.setLayoutCount = 1;
        trianglePipelineLayoutCI.pushConstantRangeCount = 1;
        trianglePipelineLayoutCI.pPushConstantRanges = &pcRange;

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
        trianglePipeline.pipeline = pipelineBuilder.build_pipeline(device.get_handle());

        device.get_handle().destroyShaderModule(vertShader.module);
        device.get_handle().destroyShaderModule(fragShader.module);
        device.get_handle().destroyShaderModule(compShader.module);
        device.primaryDeletionQueue.push_function([&]() {
            device.get_handle().destroyPipelineLayout(trianglePipeline.pipelineLayout);
            device.get_handle().destroyPipelineLayout(drawImagePipeline.pipelineLayout);
            device.get_handle().destroyPipeline(trianglePipeline.pipeline);
            device.get_handle().destroyPipeline(drawImagePipeline.pipeline);
        });

        while (!glfwWindowShouldClose(device.window)) {
            glfwPollEvents();
            draw();
        }
    }

    void Application::draw() {
        device.wait_on_work();

        VkImage& currentSwapchainImage = device.get_swapchain_image();
        if (device.resizeRequested) {
            return;
        }

        FrameData& currentFrame = device.get_current_frame();

        commands::ComputeContext computeContext(currentFrame.commandBuffer);
        computeContext.begin();
        computeContext.bind_pipeline(drawImagePipeline);
        computeContext.image_barrier(drawHandle, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);
        computeContext.dispatch(std::ceil(device.width / 16.0f), std::ceil(device.height / 16.0f), 1);

        commands::GraphicsContext graphicsContext(currentFrame.commandBuffer);

        graphicsContext.image_barrier(drawHandle, vk::ImageLayout::eGeneral, vk::ImageLayout::eColorAttachmentOptimal);
        vk::RenderingAttachmentInfo drawAttachment(drawImage.imageView, vk::ImageLayout::eColorAttachmentOptimal);

        graphicsContext.set_up_render_pass(drawImageExtent, &drawAttachment, nullptr);
        graphicsContext.bind_pipeline(trianglePipeline);

        graphicsContext.set_viewport(drawImageExtent, 0.0f, 1.0f);
        graphicsContext.set_scissor(drawImageExtent);

        PushConstants pushConstants {glm::mat4{1.0f}, meshBuffer.deviceAddress};
        graphicsContext.set_push_constants(vk::ShaderStageFlagBits::eVertex, 0, pushConstants);
        graphicsContext.bind_index_buffer(meshBuffer.indexBuffer.buffer);

        graphicsContext.draw();

        graphicsContext.image_barrier(drawHandle, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal);
        graphicsContext.image_barrier(currentSwapchainImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        graphicsContext.copy_image(drawHandle, currentSwapchainImage, drawImageExtent, drawImageExtent);
        graphicsContext.image_barrier(currentSwapchainImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR);
        graphicsContext.end();

        device.submit_graphics_work(graphicsContext, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eAllGraphics);

        device.present();
    }

    void Application::init_descriptors() {
        std::array<Vertex, 4> vertices{};
        vertices[0].position = {0.5,-0.5, 0};
        vertices[1].position = {0.5,0.5, 0};
        vertices[2].position = {-0.5,-0.5, 0};
        vertices[3].position = {-0.5,0.5, 0};

        vertices[0].color = {0,0, 0,1};
        vertices[1].color = { 0.5,0.5,0.5 ,1};
        vertices[2].color = { 1,0, 0,1 };
        vertices[3].color = { 0,1, 0,1 };

        std::array<uint32_t, 6> indices{};
        indices[0] = 0;
        indices[1] = 1;
        indices[2] = 2;

        indices[3] = 2;
        indices[4] = 1;
        indices[5] = 3;

        vk_check(device.get_handle().resetFences(1, &device.immediateFence), "Failed to reset fences");
        commands::UploadContext uploadContext(device.get_handle(), device.immediateCommandBuffer, device.allocator);
        uploadContext.begin();
        meshBuffer = uploadContext.upload_mesh(vertices, indices);
        uploadContext.end();
        device.submit_upload_work(uploadContext);

        std::vector<DescriptorAllocator::PoolSizeRatio> sizes {
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
            std::vector<DescriptorAllocator::PoolSizeRatio> frame_sizes {
                        { vk::DescriptorType::eStorageImage, 3 },
                        { vk::DescriptorType::eStorageBuffer, 3 },
                        { vk::DescriptorType::eUniformBuffer, 3 },
                        { vk::DescriptorType::eCombinedImageSampler, 4 },
            };

            device.frames[i].frameDescriptors = DescriptorAllocator{};
            device.frames[i].frameDescriptors.init(device.device, 1000, frame_sizes);

            device.primaryDeletionQueue.push_function([&, i]() {
                device.frames[i].frameDescriptors.destroy_pools(device.get_handle());
            });
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

        device.primaryDeletionQueue.push_function([&]() {
            vmaDestroyBuffer(device.allocator, meshBuffer.vertexBuffer.buffer, meshBuffer.vertexBuffer.allocation);
            vmaDestroyBuffer(device.allocator, meshBuffer.indexBuffer.buffer, meshBuffer.indexBuffer.allocation);
        });
    }
}
