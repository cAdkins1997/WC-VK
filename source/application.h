
#ifndef APPLICATION_H
#define APPLICATION_H

#include "device/device.hpp"
#include "context.h"

#include "pipelines/descriptors.h"

namespace wcvk {
    class Application {
    public:
        Application();
        ~Application();

        void run();
        void draw();

    private:
        void init_descriptors();

        DescriptorAllocator descriptorAllocator;
        Pipeline drawImagePipeline;
        Pipeline trianglePipeline;

    private:
        core::Device device;
    };
}

#endif //APPLICATION_H
