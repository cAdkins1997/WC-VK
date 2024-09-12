
#ifndef APPLICATION_H
#define APPLICATION_H

#include "device/device.hpp"
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
    void init_descriptors();

    //Device device;
    wcvk::Device deviceHpp;


    VkCommandPool immediateCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer immediateCommandBuffer = VK_NULL_HANDLE;
    VkFence immediateFence = VK_NULL_HANDLE;

    VkDescriptorSetLayout dsLayout;

    Buffer uniformDSBuffer;
};


#endif //APPLICATION_H
