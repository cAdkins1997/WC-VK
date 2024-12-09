#pragma once

namespace wcvk {
    struct Service {

        virtual void init(void* config) {};
        virtual void shutdown() {};
    };

#define DECLARE_SERVICE(Type) static Type* instance();
}
