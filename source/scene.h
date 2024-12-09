#pragma once
#include "allocator.h"
#include "structures/array.h"
#include "structures/string.h"
#include "../source/device/device.hpp"
#include <nlohmann/json.hpp>

namespace wcvk::scene {

    struct Asset {
        StringBuffer copyright;
        StringBuffer generator;
        StringBuffer minVersion;
        StringBuffer version;
    };

    struct AccessorSparse {
        uint32_t count;
        uint32_t indices;
        uint32_t values;
    };

    struct BufferView {
        enum Target {
            ARRAY_BUFFER = 34962 /* Vertex Data */, ELEMENT_ARRAY_BUFFER = 34963 /* Index Data */
        };

        int32_t buffer{};
        int32_t byteLength{};
        int32_t byteOffset{};
        int32_t byteStride{};
        int32_t target{};
        StringBuffer name;
    };

    struct Image {
        int32_t bufferView{};
        StringBuffer mimeType;
        StringBuffer uri;
    };

    struct Node {
        int32_t camera{};
        uint32_t childrenCount{};
        int32_t* children{};
        uint32_t matrixCount{};
        float* matrix{};
        int32_t mesh{};
        uint32_t rotationCount{};
        float* rotation{};
        uint32_t scaleCount{};
        float* scale{};
        int32_t skin{};
        uint32_t translationCount{};
        float* translation{};
        float* weights{};
        StringBuffer name;
    };

    struct TextureInfo {
        int32_t index;
        int32_t texCoord;
    };

    struct MaterialPBRMetallicRoughness {
        uint32_t baseColorFactorCount;
        float* baseColorFactor;
        TextureInfo* baseColorTexture;
        float metallicFactor;
        TextureInfo* metallicRoughnessTexture;
        float roughnessFactor;
    };

    struct MeshPrimitive {
        struct Attribute {
            StringBuffer key;
            int32_t accessor_index{};
        };

        uint32_t attributeCount;
        Attribute* attributes;
        int32_t indices;
        int32_t material;
        int32_t mode;
    };

    struct AccessorSparseIndices {
        int32_t bufferView;
        int32_t byteOffset;
        int32_t componentType;
    };

    struct Accessor {
        enum ComponentType {
            BYTE = 5120, UNSIGNED_BYTE = 5121, SHORT = 5122, UNSIGNED_SHORT = 5123, UNSIGNED_INT = 5125, FLOAT = 5126
        };

        enum Type {
            Scalar, Vec2, Vec3, Vec4, Mat2, Mat3, Mat4
        };

        int32_t bufferView;
        int32_t byteOffset;

        int32_t componentType;
        int32_t count;
        uint32_t maxCount;
        float* max;
        uint32_t minCount;
        float* min;
        bool normalized;
        int32_t sparse;
        Type type;
    };

    struct Texture {
        int32_t sampler{};
        int32_t source{};
        StringBuffer name;
    };

    struct MaterialNormalTextureInfo {
        int32_t index;
        int32_t texCoord;
        float scale;
    };

    struct Mesh {
        uint32_t primitivesCount{};
        MeshPrimitive* primitives{};
        uint32_t weightsCount{};
        float* weights{};
        StringBuffer name;
    };

    struct MaterialOcclusionTextureInfo {
        int32_t index;
        int32_t texCoord;
        float strength;
    };

    struct Material {
        float alphaCutoff{};
        StringBuffer alphaMode;
        bool doubleSided{};
        uint32_t emissiveFactorCount{};
        float* emissiveFactor{};
        TextureInfo* emissiveTexture{};
        MaterialNormalTextureInfo* normalTexture{};
        MaterialOcclusionTextureInfo* occlusionTexture{};
        MaterialPBRMetallicRoughness* pbrMetallicRoughness{};
        StringBuffer name;
    };

    struct Buffer {
        int32_t byteLength{};
        StringBuffer uri;
        StringBuffer name;
    };

    struct AccessorSparseValues {
         int32_t bufferView;
         int32_t byteOffset;
    };

    struct Scene {
        uint32_t nodesCount;
        int32_t* nodes;
    };

    struct Sampler {
        enum Filter {
            NEAREST = 9728, LINEAR = 9729, NEAREST_MIPMAP_NEAREST = 9984, LINEAR_MIPMAP_NEAREST = 9985, NEAREST_MIPMAP_LINEAR = 9986, LINEAR_MIPMAP_LINEAR = 9987
        };

        enum Wrap {
            CLAMP_TO_EDGE = 33071, MIRRORED_REPEAT = 33648, REPEAT = 10497
        };

        int32_t magFilter;
        int32_t minFilter;
        int32_t wrapS;
        int32_t wrapT;
    };

    struct glTF {
        uint32_t accessorCount;
        Accessor* accessors;
        Asset asset;
        uint32_t bufferViewCount;
        BufferView* bufferViews;
        uint32_t bufferCount;
        Buffer* buffers;
        uint32_t extensionsRequiredCount;
        StringBuffer* extensionsRequired;
        uint32_t extensionsUsedCount;
        StringBuffer* extensionsUsed;
        uint32_t imageCount;
        Image* images;
        uint32_t materialCount;
        Material* materials;
        uint32_t meshCount;
        Mesh* meshes;
        uint32_t nodeCount;
        Node* nodes;
        uint32_t samplerCount;
        Sampler* samplers;
        int32_t scene;
        uint32_t sceneCount;
        Scene* scenes;
        uint32_t textureCount;
        Texture* textures;
        allocators::LinearAllocator allocator;
    };

    glTF gltf_load_file(std::string_view filePath);

    void gltf_free(glTF& scene);

    int64_t get_data_offset(int32_t accessor_offset, int32_t buffer_view_offset );

    //int64_t gltf_get_attribute_accessor_index(MeshPrimitive::Attribute* attributes, uint32_t attribute_count, const char* attribute_name);
};
