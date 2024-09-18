
#pragma once
#include <EASTL/vector.h>
#include "../vkcommon.h"

class PipelineBuilder {
public:
    PipelineBuilder() { clear(); }

    void clear();
    vk::Pipeline build_pipeline(vk::Device device);

    void set_shader(vk::ShaderModule vertexShader, vk::ShaderModule fragmentShader);
    void set_input_topology(vk::PrimitiveTopology topology);
    void set_polygon_mode(vk::PolygonMode mode);
    void set_cull_mode(vk::CullModeFlags cullMode, vk::FrontFace frontFace);
    void set_multisampling_none();
    void set_color_attachment_format(vk::Format format);
    void set_depth_format(vk::Format format);
    void disable_depthtest();

    eastl::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
    vk::PipelineRasterizationStateCreateInfo rasterizer;
    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    vk::PipelineMultisampleStateCreateInfo multisampling;
    vk::PipelineLayout pipelineLayout;
    vk::PipelineDepthStencilStateCreateInfo depthStencil;
    vk::PipelineRenderingCreateInfo renderInfo;
    vk::Format colorAttachmentformat{};
};
