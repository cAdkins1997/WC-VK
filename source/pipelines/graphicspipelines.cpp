
#include "graphicspipelines.h"

void PipelineBuilder::clear() {
    shaderStages.clear();
}

vk::Pipeline PipelineBuilder::build_pipeline(vk::Device device) {
    vk::PipelineViewportStateCreateInfo viewportState;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    vk::PipelineColorBlendStateCreateInfo colorBlending;
    colorBlending.logicOpEnable = vk::False;

    colorBlending.logicOp = vk::LogicOp::eCopy;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    std::array<vk::VertexInputAttributeDescription, 2> vertexInputAttribDesc;
    vertexInputAttribDesc[0].binding = 0;
    vertexInputAttribDesc[0].location = 0;
    vertexInputAttribDesc[0].format = vk::Format::eR32G32Sfloat;
    vertexInputAttribDesc[0].offset = offsetof(Vertex, pos);

    vertexInputAttribDesc[1].binding = 0;
    vertexInputAttribDesc[1].location = 1;
    vertexInputAttribDesc[1].format = vk::Format::eR32G32B32Sfloat;
    vertexInputAttribDesc[1].offset = offsetof(Vertex, color);

    std::array<vk::VertexInputBindingDescription, 1> vertexingBindingDescs;
    vertexingBindingDescs[0].binding = 0;
    vertexingBindingDescs[0].inputRate = vk::VertexInputRate::eVertex;
    vertexingBindingDescs[0].stride = sizeof(Vertex);

    vk::PipelineVertexInputStateCreateInfo vertexInputCI;
    vertexInputCI.vertexAttributeDescriptionCount = 2;
    vertexInputCI.vertexBindingDescriptionCount = 1;
    vertexInputCI.pVertexAttributeDescriptions =  vertexInputAttribDesc.data();
    vertexInputCI.pVertexBindingDescriptions = vertexingBindingDescs.data();

    vk::GraphicsPipelineCreateInfo pipelineCI;
    pipelineCI.pNext = &renderInfo;
    pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineCI.pStages = shaderStages.data();
    pipelineCI.pVertexInputState = &vertexInputCI;
    pipelineCI.pInputAssemblyState = &inputAssembly;
    pipelineCI.pViewportState = &viewportState;
    pipelineCI.pRasterizationState = &rasterizer;
    pipelineCI.pMultisampleState = &multisampling;
    pipelineCI.pColorBlendState = &colorBlending;
    pipelineCI.pDepthStencilState = &depthStencil;
    pipelineCI.layout = pipelineLayout;

    vk::DynamicState state[]{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamicInfo;
    dynamicInfo.pDynamicStates = &state[0];
    dynamicInfo.dynamicStateCount = 2;
    pipelineCI.pDynamicState = &dynamicInfo;

    return device.createGraphicsPipeline(VK_NULL_HANDLE, pipelineCI).value;
}

void PipelineBuilder::set_shader(vk::ShaderModule vertexShader, vk::ShaderModule fragmentShader) {
    shaderStages.clear();
    shaderStages.push_back(vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, fragmentShader, "main"));
    shaderStages.push_back(vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, vertexShader, "main"));
}

void PipelineBuilder::set_input_topology(vk::PrimitiveTopology topology) {
    inputAssembly.topology = topology;
    inputAssembly.primitiveRestartEnable = vk::False;
}

void PipelineBuilder::set_polygon_mode(vk::PolygonMode mode) {
    rasterizer.polygonMode = mode;
    rasterizer.lineWidth = 1.f;
}

void PipelineBuilder::set_cull_mode(vk::CullModeFlags cullMode, vk::FrontFace frontFace) {
    rasterizer.cullMode = cullMode;
    rasterizer.frontFace = frontFace;
}

void PipelineBuilder::set_multisampling_none() {
    multisampling.sampleShadingEnable = vk::False;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = vk::False;
    multisampling.alphaToOneEnable = vk::False;
}

void PipelineBuilder::set_color_attachment_format(vk::Format format) {
    colorAttachmentformat = format;
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachmentFormats = &colorAttachmentformat;
}

void PipelineBuilder::set_depth_format(vk::Format format) {
    renderInfo.depthAttachmentFormat = format;
}

void PipelineBuilder::disable_depthtest() {
    depthStencil.depthTestEnable = vk::False;
    depthStencil.depthWriteEnable = vk::False;
    depthStencil.depthCompareOp = vk::CompareOp::eNever;
    depthStencil.depthBoundsTestEnable = vk::False;
    depthStencil.stencilTestEnable = vk::False;
    depthStencil.minDepthBounds = 0.f;
    depthStencil.maxDepthBounds = 1.f;
}

void PipelineBuilder::disable_blending() {
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = vk::False;
}


