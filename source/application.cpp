
#include "application.h"
#include <chrono>

#include "meshes.h"
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

        depthImage = device.get_depth_image();
        depthHandle = depthImage.get_handle();

        sceneDataBuffer = device.create_buffer(
            sizeof(SceneData),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
            VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
            VMA_ALLOCATION_CREATE_MAPPED_BIT
    );

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
        pipelineBuilder.enable_depthtest(vk::True, vk::CompareOp::eGreaterOrEqual);
        pipelineBuilder.disable_blending();
        pipelineBuilder.set_color_attachment_format(device.drawImage.imageFormat);
        pipelineBuilder.set_depth_format(depthImage.get_format());
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

        drawAttachment.imageView = drawImage.imageView;
        drawAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        drawAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        drawAttachment.storeOp = vk::AttachmentStoreOp::eStore;

        depthAttachment.imageView = depthImage.imageView;
        depthAttachment.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
        depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        depthAttachment.clearValue.depthStencil.depth = 0.f;

        while (!glfwWindowShouldClose(device.window)) {
            glfwPollEvents();
            draw();
        }
    }

    void Application::draw() {
        auto currentFrameTime = static_cast<float>(glfwGetTime());
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        device.wait_on_work();

        VkImage& currentSwapchainImage = device.get_swapchain_image();
        if (device.resizeRequested) {
            return;
        }

        FrameData& currentFrame = device.get_current_frame();

        core::process_input(device.window);

        auto model = glm::mat4(1.0f);
        glm::mat4 view = core::camera.get_view_matrix();
        glm::mat4 perspective = glm::perspective(glm::radians(core::camera.zoom), static_cast<float>(device.width) / static_cast<float>(device.height), 10000.f, 0.1f);

        Frustum frustum = compute_frustum(view);

        glm::vec3 lightPos{1.0f, cos(glfwGetTime() * 2), 5.0f};

        SceneData sceneData{model, view, perspective, frustum, lightPos};

        commands::UploadContext uploadContext(device.get_handle(), currentFrame.commandBuffer, device.allocator);
        uploadContext.begin();
        uploadContext.upload_uniform(&sceneData, sizeof(SceneData), sceneDataBuffer);

        commands::ComputeContext computeContext(currentFrame.commandBuffer);
        computeContext.bind_pipeline(drawImagePipeline);
        computeContext.image_barrier(drawHandle, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);
        computeContext.dispatch(std::ceil(device.width / 16.0f), std::ceil(device.height / 16.0f), 1);

        commands::GraphicsContext graphicsContext(currentFrame.commandBuffer);

        graphicsContext.image_barrier(drawHandle, vk::ImageLayout::eGeneral, vk::ImageLayout::eColorAttachmentOptimal);
        graphicsContext.image_barrier(depthHandle, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal);

        for (const auto& mesh : testMeshes) {
            graphicsContext.set_up_render_pass(drawImageExtent, &drawAttachment, &depthAttachment);
            graphicsContext.bind_pipeline(trianglePipeline);

            graphicsContext.set_viewport(drawImageExtent, 0.0f, 1.0f);
            graphicsContext.set_scissor(drawImageExtent);

            PushConstants pushConstants {mesh->mesh.deviceAddress};
            graphicsContext.set_push_constants(vk::ShaderStageFlagBits::eVertex, 0, pushConstants);
            graphicsContext.bind_index_buffer(mesh->mesh.indexBuffer.buffer);
            for (auto& surface : mesh->surfaces) {
                graphicsContext.draw(surface.count, surface.startIndex);
            }
        }

        graphicsContext.image_barrier(drawHandle, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal);
        graphicsContext.image_barrier(currentSwapchainImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        graphicsContext.copy_image(drawHandle, currentSwapchainImage, drawImageExtent, drawImageExtent);
        graphicsContext.image_barrier(currentSwapchainImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR);
        graphicsContext.end();

        device.submit_graphics_work(graphicsContext, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eAllGraphics);

        device.present();
    }

    void Application::init_descriptors() {
        vk_check(device.get_handle().resetFences(1, &device.immediateFence), "Failed to reset fences");
        commands::UploadContext uploadContext(device.get_handle(), device.immediateCommandBuffer, device.allocator);
        uploadContext.begin();
        testMeshes = meshes::loadGltfMeshes(R"(../assets/MetalRoughSpheres.glb)", uploadContext).value();
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
            builder.add_binding(1, vk::DescriptorType::eUniformBuffer);
            trianglePipeline.descriptorLayout = builder.build(device.get_handle(), vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

            trianglePipeline.set = descriptorAllocator.allocate(device.device, trianglePipeline.descriptorLayout);

            DescriptorWriter writer;
            writer.write_image(0, device.drawImage.imageView, nullptr, vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage);
            writer.write_buffer(1, sceneDataBuffer.buffer, sizeof(SceneData), 0, vk::DescriptorType::eUniformBuffer);
            writer.update_set(device.device, trianglePipeline.set);
        }

        /*device.primaryDeletionQueue.push_function([&]() {
            vmaDestroyBuffer(device.allocator, meshBuffer.vertexBuffer.buffer, meshBuffer.vertexBuffer.allocation);
            vmaDestroyBuffer(device.allocator, meshBuffer.indexBuffer.buffer, meshBuffer.indexBuffer.allocation);
        });*/
    }
}
