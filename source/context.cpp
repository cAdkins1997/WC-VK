
#include "context.h"

void GraphicsContext::begin() {
    VkCommandBufferBeginInfo beginInfo = vkinit::command_buffer_BI(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkRect2D renderingArea;
    renderingArea.extent = {1920, 1080};
    renderingArea.offset = {0, 0};
    VkRenderingInfo renderingInfo = vkinit::rendering_info(0, renderingArea, 1, 0, 0);
}
