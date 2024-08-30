
#ifndef APPLICATION_H
#define APPLICATION_H

#include "device/device.h"
#include "context.h"

#include "pipelines/descriptors.h"

class Application {
public:
    Application();
    ~Application();

    void run();
    void draw();
private:

    void immediate_reset() const;
    void init_immediate_commands();

    Device device;

    VkCommandPool immediateCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer immediateCommandBuffer = VK_NULL_HANDLE;
    VkFence immediateFence = VK_NULL_HANDLE;
};


#endif //APPLICATION_H
