
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
    void init_descriptors();

private:
    Device device;
    Buffer buffer;
};


#endif //APPLICATION_H
