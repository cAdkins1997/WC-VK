
#ifndef APPLICATION_H
#define APPLICATION_H

#include "device.h"
#include "context.h"

#include "pipelines/descriptors.h"

class Application {
public:
    Application();
    ~Application();

    void run();
    void draw();

private:
    void init();
    void init_descriptors();

private:
    Device device;
    Buffer buffer;

    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
};


#endif //APPLICATION_H
