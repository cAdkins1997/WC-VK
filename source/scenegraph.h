#pragma once
#include "glmdefines.h"
#include "vkcommon.h"

#include "device/resources.h"
#include "meshes.h"
#include "context.h"

#include <glm/gtx/quaternion.hpp>

/*namespace scenes {

    struct  Transform {
        glm::vec3 pos{};
        glm::vec3 eulerRot{};
        glm::vec3 scale{};
        glm::mat4 modelMat{1.0f};
        bool isDirty = true;

        [[nodiscard]] glm::mat4 get_model_mat() const;

        void compute_model_matrix();
        void compute_model_matrix(const glm::mat4& parentGlobalModelMatrix);
        void set_local_position(const glm::vec3& newPosition);
    };

    class Node {
    public:
        explicit Node(Mesh* _mesh = nullptr);
        ~Node();

        void add_child(Node* child) { children.push_back(child); child->parent = this; }
        void update_graph_transforms();

        void update(float msec);

        Node* parent{};
        Mesh* mesh{};
        Transform transform;
        std::vector<Node*> children;
    };

    struct GLTFData {
        std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes;
        std::unordered_map<std::string, std::shared_ptr<Image>> images;
        std::shared_ptr<Node> sceneGraph;

        std::vector<vk::Sampler> samplers;
        DescriptorAllocator descriptorPool;
        Buffer materialBuffer;
    };


    void build_scene_graph(std::string_view filePath, wcvk::commands::UploadContext& context, std::unique_ptr<Node> sceneGraph) {
        std::shared_ptr<GLTFData> scene = std::make_shared<GLTFData>();
        GLTFData& file =  *scene.get();

        fastgltf::Parser parser;

        constexpr auto gltfOptions =
            fastgltf::Options::DontRequireValidAssetMember |
            fastgltf::Options::AllowDouble |
                    fastgltf::Options::LoadExternalBuffers;

        auto data = fastgltf::GltfDataBuffer::FromPath(filePath);
        if (data.error() != fastgltf::Error::None) {
            throw std::runtime_error("Failed to read glTF file");
        }

        fastgltf::Asset gltf;
        std::filesystem::path path = filePath;

        auto type = fastgltf::determineGltfFileType(data.get());
        if (type == fastgltf::GltfType::glTF) {
            auto load = parser.loadGltf(data.get(), path.parent_path(), gltfOptions);
            if (load)
                gltf = std::move(load.get());
            else {
                std::cerr << "Failed to load glTF: " << fastgltf::to_underlying(load.error()) << std::endl;
                return {};
            }
        }

    }
}*/