
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

        vk::PushConstantRange trianglePCRange(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(PushConstants));

        vk::PipelineLayoutCreateInfo trianglePipelineLayoutCI;
        trianglePipelineLayoutCI.pSetLayouts = &trianglePipeline.descriptorLayout;
        trianglePipelineLayoutCI.setLayoutCount = 1;
        trianglePipelineLayoutCI.pPushConstantRanges = &trianglePCRange;
        trianglePipelineLayoutCI.pushConstantRangeCount = 1;

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

        while (!glfwWindowShouldClose(device.window)) {
            glfwPollEvents();
            draw();
        }
    }-

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
        computeContext.dispatch(std::ceil(drawImageExtent.width), std::ceil(drawImageExtent.height), 1);
        computeContext.end();

        commands::GraphicsContext graphicsContext(currentFrame.graphicsCommandBuffer);
        graphicsContext.begin();
        graphicsContext.bind_pipeline(trianglePipeline);
        graphicsContext.bind_index_buffer(meshBuffer.indexBuffer.buffer);

        PushConstants push_constants{glm::mat4{ 1.f }, meshBuffer.deviceAddress};
        graphicsContext.set_push_constants(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, &push_constants);

        graphicsContext.image_barrier(drawHandle, vk::ImageLayout::eGeneral, vk::ImageLayout::eColorAttachmentOptimal);
        vk::RenderingAttachmentInfo drawAttachment(drawImage.imageView, vk::ImageLayout::eColorAttachmentOptimal);

        graphicsContext.set_up_render_pass({drawImageExtent.width, drawImageExtent.height}, &drawAttachment, nullptr);
        graphicsContext.bind_pipeline(trianglePipeline);
        graphicsContext.set_viewport({drawImageExtent.width, drawImageExtent.height}, 0.0f, 1.0f);
        graphicsContext.set_scissor({drawImageExtent.width, drawImageExtent.height});
        graphicsContext.draw();

        graphicsContext.image_barrier(drawHandle, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal);
        graphicsContext.image_barrier(currentSwapchainImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        graphicsContext.copy_image(drawHandle, currentSwapchainImage, {drawImageExtent.width, drawImageExtent.height}, {drawImageExtent.width, drawImageExtent.height});
        graphicsContext.image_barrier(currentSwapchainImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR);
        graphicsContext.end();

        device.submit_compute_work(computeContext, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eComputeShader);
        device.submit_graphics_work(graphicsContext, vk::PipelineStageFlagBits2::eComputeShader, vk::PipelineStageFlagBits2::eAllGraphics);

        device.present();
    }

    void Application::init_descriptors() {
        eastl::vector<Vertex> rectVerts(4);
        rectVerts[0].position = {0.5,-0.5, 0};
        rectVerts[1].position = {0.5,0.5, 0};
        rectVerts[2].position = {-0.5,-0.5, 0};
        rectVerts[3].position = {-0.5,0.5, 0};

        rectVerts[0].color = {0,0, 0,1};
        rectVerts[1].color = { 0.5,0.5,0.5 ,1};
        rectVerts[2].color = { 1,0, 0,1 };
        rectVerts[3].color = { 0,1, 0,1 };

        vertSize = rectVerts.size();

        meshBuffer.vertexBuffer = device.create_buffer(vertSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

        eastl::vector<uint32_t> rectIndices(6);
        rectIndices[0] = 0;
        rectIndices[1] = 1;
        rectIndices[2] = 2;

        rectIndices[3] = 2;
        rectIndices[4] = 1;
        rectIndices[5] = 3;

        indexSize = rectIndices.size();

        meshBuffer.indexBuffer = device.create_buffer(indexSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

        vk::BufferDeviceAddressInfo addressInfo;
        addressInfo.buffer = meshBuffer.vertexBuffer.buffer;
        meshBuffer.deviceAddress = device.get_handle().getBufferAddress(&addressInfo);

        commands::UploadContext uploadContext(device.immediateCommandBuffer, device.allocator);
        uploadContext.begin();
        uploadContext.upload_mesh(meshBuffer.vertexBuffer, meshBuffer.indexBuffer, rectIndices, rectVerts, meshBuffer.deviceAddress);
        uploadContext.end();
        device.submit_upload_work(uploadContext);

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
            //builder.add_binding(1, vk::DescriptorType::eUniformBuffer);
            //builder.add_binding(2, vk::DescriptorType::eUniformBuffer);
            trianglePipeline.descriptorLayout = builder.build(device.device, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

            trianglePipeline.set = descriptorAllocator.allocate(device.device, trianglePipeline.descriptorLayout);

            DescriptorWriter writer;
            writer.write_image(0, device.drawImage.imageView, nullptr, vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage);
            /*writer.write_buffer(1, meshBuffer.vertexBuffer.buffer, vertSize, 0, vk::DescriptorType::eUniformBuffer);
            writer.write_buffer(2, meshBuffer.indexBuffer.buffer, vertSize, 0, vk::DescriptorType::eUniformBuffer);*/
            writer.update_set(device.device, trianglePipeline.set);
        }
    }
}
