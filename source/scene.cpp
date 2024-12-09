
#include "scene.h"
#include <EASTL/optional.h>

using namespace nlohmann;
using namespace wcvk;
using namespace wcvk::allocators;
using namespace wcvk::scene;

static void* allocate_and_zero(Allocator* allocator, size_t size) {
    void* result = allocator->allocate( size, 64 );
    memset(result, 0, size);
    return result;
}

static void load_string(json& data, const char* key, StringBuffer& stringBuffer, Allocator* allocator) {
    if (auto it = data.find(key); it == data.end())
        return;

    std::string value = data.value(key, "");

    stringBuffer.init(value.length() + 1, *allocator);
    stringBuffer.append(value.c_str());
}

static void load_int(json& data, const char* key, int32_t& value) {
    if (const auto it = data.find(key); it == data.end()) {
        value = INT32_MAX;
        return;
    }

    value = data.value(key, 0);
}

static void load_float(json& data, const char* key, float& value) {
    auto it = data.find(key);
    if (it == data.end()) {
        return;
    }

    value = data.value(key, 0.0f);
}

static void load_bool(json& data, const char* key, bool& value) {
    auto it = data.find(key);
    if (it == data.end()) {
        value = false;
        return;
    }

    value = data.value(key, false);
}

static void load_type(json& data, const char* key, Accessor::Type& type) {
    eastl::string value = data.value( key, "" ).c_str();
    if ( value == "SCALAR" ) {
        type = Accessor::Type::Scalar;
    }
    else if ( value == "VEC2" ) {
        type = Accessor::Type::Vec2;
    }
    else if ( value == "VEC3" ) {
        type = Accessor::Type::Vec3;
    }
    else if ( value == "VEC4" ) {
        type = Accessor::Type::Vec4;
    }
    else if ( value == "MAT2" ) {
        type = Accessor::Type::Mat2;
    }
    else if ( value == "MAT3" ) {
        type = Accessor::Type::Mat3;
    }
    else if ( value == "MAT4" ) {
        type = Accessor::Type::Mat4;
    }
}

static void load_int_array(json& data, const char* key, uint32_t& count, int32_t** array, Allocator* allocator) {
    auto it = data.find( key );
    if ( it == data.end() ) {
        count = 0;
        *array = nullptr;
        return;
    }

    json json_array = data.at(key);

    count = json_array.size();

    auto values = static_cast<int32_t*>(allocate_and_zero(allocator, sizeof(int32_t) * count));

    for (size_t i = 0; i < count; ++i) {
        values[i] = json_array.at(i);
    }

    *array = values;
}

static void load_float_array(json& data, const char* key, uint32_t& count, float** array, Allocator* allocator) {
    auto it = data.find(key);
    if ( it == data.end() ) {
        count = 0;
        *array = nullptr;
        return;
    }

    json json_array =  data.at( key );

    count = json_array.size();

    auto values = static_cast<float *>(allocate_and_zero(allocator, sizeof(float) * count));

    for (size_t i = 0; i < count; ++i) {
        values[i] = json_array.at(i);
    }

    *array = values;
}

static void load_asset(json& data, Asset* asset, Allocator* allocator) {
    json jsonAsset = data["asset"];

    load_string(jsonAsset, "copyright", asset->copyright, allocator);
    load_string(jsonAsset, "generator", asset->generator, allocator);
    load_string(jsonAsset, "minVersion", asset->version, allocator);
    load_string(jsonAsset, "version", asset->minVersion, allocator);
}

static void load_scene(json& data, Scene& scene, Allocator* allocator) {
    load_int_array(data, "nodes", scene.nodesCount, &scene.nodes, allocator);
}


static void load_scenes(json& data, glTF& gltfData, Allocator* allocator ) {
    json scenes = data[ "scenes" ];

    size_t sceneCount = scenes.size();
    gltfData.scenes = static_cast<Scene*>(allocate_and_zero(allocator, sizeof(Scene) * sceneCount));
    gltfData.sceneCount = sceneCount;

    for (size_t i = 0; i < sceneCount; ++i ) {
        load_scene(scenes[i], gltfData.scenes[i], allocator);
    }
}

static void load_node(json data, Node& node, Allocator* allocator) {
    load_int(data, "camera", node.camera);
    load_int(data, "mesh", node.camera);\
    load_int(data, "skin", node.skin);
    load_int_array( data, "children", node.childrenCount, &node.children, allocator );
    load_float_array( data, "matrix", node.matrixCount, &node.matrix, allocator );
    load_float_array( data, "rotation", node.rotationCount, &node.rotation, allocator );
    load_float_array( data, "scale", node.scaleCount, &node.scale, allocator );
    load_float_array( data, "translation", node.translationCount, &node.translation, allocator );
    load_float_array( data, "weights", node.translationCount, &node.weights, allocator );
    load_string(data, "name", node.name, allocator);
}

static void load_nodes(json& data, glTF& gltfData, Allocator* allocator) {
    json array = data["nodes"];

    size_t arrayCount = array.size();
    gltfData.nodes = static_cast<Node*>(allocate_and_zero(allocator, sizeof(Node) * arrayCount));
    gltfData.nodeCount = arrayCount;

    for (size_t i = 0; i < arrayCount; ++i ) {
        load_node(array[i], gltfData.nodes[i], allocator);
    }
}

static void load_buffer(json& data, scene::Buffer& buffer, Allocator* allocator) {
    load_string(data, "uri", buffer.uri, allocator );
    load_int(data, "byteLength", buffer.byteLength);
    load_string(data, "name", buffer.name, allocator );
}

static void load_buffers(json& data, glTF& gltfData, Allocator* allocator) {
    json buffers = data["buffers"];

    size_t bufferCount = buffers.size();
    gltfData.buffers = static_cast<scene::Buffer*>(allocate_and_zero(allocator, sizeof(scene::Buffer) * bufferCount));
    gltfData.bufferCount = bufferCount;

    for (size_t i = 0; i < bufferCount; ++i ) {
        load_buffer(buffers[i], gltfData.buffers[i], allocator );
    }
}

static void load_buffer_view(json& data, BufferView& bufferView, Allocator* allocator) {
    load_int(data, "buffer", bufferView.buffer);
    load_int(data, "byteLength", bufferView.byteLength);
    load_int(data, "byteOffset", bufferView.byteOffset);
    load_int(data, "byteStride", bufferView.byteStride);
    load_int(data, "target", bufferView.target);
    load_string(data, "name", bufferView.name, allocator);
}

static void load_buffer_views(json& data, glTF& gltfData, Allocator* allocator) {
    json bufferViews = data["bufferViews"];

    size_t bufferCount = bufferViews.size();
    gltfData.bufferViewCount = bufferCount;
    gltfData.bufferViews = static_cast<BufferView*>(allocate_and_zero(allocator, sizeof(BufferView) * bufferCount));

    for (size_t i = 0; i < bufferCount; ++i ) {
        load_buffer_view(bufferViews[i], gltfData.bufferViews[i], allocator);
    }
}

static void load_mesh_primitive( json& data, MeshPrimitive& mesh_primitive, Allocator* allocator ) {
    load_int(data, "indices", mesh_primitive.indices );
    load_int(data, "material", mesh_primitive.material );
    load_int(data, "mode", mesh_primitive.mode );

    json attributes = data["attributes"];

    const size_t arrayCount = attributes.size();
    std::cout << "Attribute Count: " << arrayCount << std::endl;
    eastl::vector<MeshPrimitive::Attribute> attributeData(arrayCount);
    mesh_primitive.attributes = attributeData.data();
    mesh_primitive.attributeCount = attributeData.capacity();


    uint32_t index = 0;
    for (auto json_attribute : attributes.items()) {
        const std::string key = json_attribute.key();
        MeshPrimitive::Attribute& attribute = mesh_primitive.attributes[index];

        attribute.key.init(key.size() + 1, *allocator);
        attribute.key.append(key.c_str());
        attribute.accessor_index = json_attribute.value();

        ++index;
    }
}

static void load_mesh_primitives( json& data, scene::Mesh& mesh, Allocator* allocator ) {
    json array = data["primitives"];

    const size_t arrayCount = array.size();
    eastl::vector<MeshPrimitive> meshPrimitives(arrayCount);
    mesh.primitives = meshPrimitives.data();
    mesh.primitivesCount = meshPrimitives.size();

    for (size_t i = 0; i < arrayCount; ++i) {
        load_mesh_primitive(array[i], mesh.primitives[i], allocator);
    }
}

static void load_mesh(json& data, scene::Mesh& mesh, Allocator* allocator ) {
    load_mesh_primitives(data, mesh, allocator);
    load_float_array(data, "weights", mesh.weightsCount, &mesh.weights, allocator);
    load_string(data, "name", mesh.name, allocator);
}

static void load_meshes(json& data, glTF& gltfData, Allocator* allocator) {
    json array = data["meshes"];

    const size_t arrayCount = array.size();
    eastl::vector<scene::Mesh> meshes(arrayCount);
    gltfData.meshes = meshes.data();
    gltfData.meshCount = meshes.capacity();

    for (size_t i = 0; i < meshes.capacity(); ++i) {
        load_mesh(array[i], gltfData.meshes[i], allocator);
    }
}

static void load_accessor(json& data, Accessor& accessor, Allocator* allocator) {
    load_int(data, "bufferView", accessor.bufferView);
    load_int(data, "byteOffset", accessor.byteOffset);
    load_int(data, "componentType", accessor.componentType);
    load_int(data, "count", accessor.count);
    load_int(data, "sparse", accessor.sparse);
    load_float_array(data, "max", accessor.maxCount, &accessor.max, allocator);
    load_float_array(data, "min", accessor.minCount, &accessor.min, allocator);
    load_bool(data, "normalized", accessor.normalized);
    load_type(data, "type", accessor.type);
}

static void load_accessors(json& data, glTF& gltfData, Allocator* allocator) {
    json array = data["accessors"];

    size_t arrayCount = array.size();
    gltfData.accessors = static_cast<Accessor*>(allocator->allocate(sizeof(Accessor) * arrayCount, alignof(Accessor)));
    gltfData.accessorCount = arrayCount;

    for (size_t i = 0; i < arrayCount; ++i) {
        load_accessor(array[i], gltfData.accessors[i], allocator );
    }
}

static void load_texure_info(json& data, const char* key, TextureInfo* textureInfo, Allocator* allocator ) {
    if (auto it = data.find( key ); it == data.end() ) {
        textureInfo = nullptr;
        return;
    }

    textureInfo = static_cast<TextureInfo*>(allocator->allocate(sizeof(Texture), 64));
    load_int(data, "index", textureInfo->index);
    load_int(data, "texCoord", textureInfo->texCoord);
}

static void load_material_normal_texture_info(json& data, const char* key, MaterialNormalTextureInfo* textureInfo, Allocator* allocator ) {
    if (auto it = data.find( key ); it == data.end() ) {
        textureInfo = nullptr;
        return;
    }

    textureInfo = static_cast<MaterialNormalTextureInfo*>(allocator->allocate(sizeof(MaterialNormalTextureInfo), 64));

    load_int(data, "index", textureInfo->index);
    load_int(data, "texCoord",  textureInfo->texCoord);
    load_float(data, "scale", textureInfo->scale);
}

static void load_material_occlusion_texture_info(json& data, const char* key, MaterialOcclusionTextureInfo* matNormalOcclusionTextureInfo, Allocator* allocator) {
    if (auto it = data.find( key ); it == data.end() ) {
        matNormalOcclusionTextureInfo = nullptr;
        return;
    }

    matNormalOcclusionTextureInfo = static_cast<MaterialOcclusionTextureInfo*>(allocator->allocate(sizeof(MaterialOcclusionTextureInfo), 64));

    load_int(data, "index", matNormalOcclusionTextureInfo->index);
    load_int(data, "texCoord", matNormalOcclusionTextureInfo->texCoord);
    load_float(data, "strength", matNormalOcclusionTextureInfo->strength);
}

static void load_material_PBR_MR(json& data, const char* key, MaterialPBRMetallicRoughness* pbrTexInfo, Allocator* allocator ) {
    auto it = data.find(key);
    if (it == data.end())
    {
        pbrTexInfo = nullptr;
        return;
    }

    pbrTexInfo = static_cast<MaterialPBRMetallicRoughness*>(allocator->allocate(sizeof(MaterialPBRMetallicRoughness), 64));

   load_float_array( *it, "baseColorFactor", pbrTexInfo->baseColorFactorCount, &pbrTexInfo->baseColorFactor, allocator);
   load_texure_info( *it, "baseColorTexture", pbrTexInfo->baseColorTexture, allocator);
    load_float(data, "metallicFactor", pbrTexInfo->metallicFactor);
    load_float(data, "roughnessFactor", pbrTexInfo->roughnessFactor);
   load_texure_info( *it, "metallicRoughnessTexture", pbrTexInfo->metallicRoughnessTexture, allocator);
}

static void load_material(json& data, Material& material, Allocator* allocator) {
   load_float_array(data, "emissiveFactor", material.emissiveFactorCount, &material.emissiveFactor, allocator);
    load_float(data, "alphaCutoff", material.alphaCutoff);
   load_string(data, "alphaMode", material.alphaMode, allocator);
    load_bool(data, "doubleSided", material.doubleSided);

   load_texure_info(data, "emissiveTexture", material.emissiveTexture, allocator);
   load_material_normal_texture_info(data, "normalTexture", material.normalTexture, allocator);
   load_material_occlusion_texture_info(data, "occlusionTexture", material.occlusionTexture, allocator);
   load_material_PBR_MR(data, "pbrMetallicRoughness", material.pbrMetallicRoughness, allocator);

   load_string(data, "name", material.name, allocator);
}

static void load_materials(json& data, glTF& gltfData, Allocator* allocator) {
    json array = data["materials"];

    size_t arrayCount = array.size();
    gltfData.materials = static_cast<Material*>(allocate_and_zero(allocator, sizeof(Material) * arrayCount));
    gltfData.materialCount = arrayCount;

    for (size_t i = 0; i < arrayCount; ++i ) {
        load_material(array[i], gltfData.materials[i], allocator);
    }
}

static void load_texture(json& data, Texture& texture, Allocator* allocator) {
    load_int(data, "sampler", texture.sampler);
    load_int(data, "source", texture.source);
   load_string( data, "name", texture.name, allocator );
}

static void load_textures(json& data, glTF& gltfData, Allocator* allocator) {
    json array = data["textures"];

    size_t arrayCount = array.size();
    gltfData.textures = static_cast<Texture*>(allocate_and_zero(allocator, sizeof(Texture) * arrayCount));
    gltfData.textureCount = arrayCount;

    for (size_t i = 0; i < arrayCount; ++i) {
        load_texture(array[i], gltfData.textures[i], allocator);
    }
}

static void load_image(json& data, scene::Image& image, Allocator* allocator) {
    load_int(data, "bufferView", image.bufferView);
    load_string(data, "mimeType", image.mimeType, allocator);
    load_string(data, "uri", image.uri, allocator);
}

static void load_images(json& data, glTF& gltfData, Allocator* allocator) {
    json array = data["images"];

    size_t arrayCount = array.size();
    gltfData.images = static_cast<scene::Image*>(allocate_and_zero(allocator, sizeof(scene::Image) * arrayCount));
    gltfData.imageCount = arrayCount;

    for (size_t i = 0; i < arrayCount; ++i) {
        load_image(array[i], gltfData.images[i], allocator);
    }
}

static void load_sampler(json& data, Sampler& sampler, Allocator* allocator) {
    load_int(data, "magFilter", sampler.magFilter);
    load_int(data, "minFilter", sampler.minFilter);
    load_int(data, "wrapS", sampler.wrapS);
    load_int(data, "wrapT", sampler.wrapT);
}

static void load_samplers(json& data, glTF& gltfData, Allocator* allocator) {
    json array = data["samplers"];

    size_t arrayCount = array.size();
    gltfData.samplers = static_cast<Sampler*>(allocate_and_zero(allocator, sizeof(Sampler) * arrayCount));
    gltfData.samplerCount = arrayCount;

    for (size_t i = 0; i < arrayCount; ++i) {
        load_sampler(array[i], gltfData.samplers[i], allocator);
    }
}

glTF scene::gltf_load_file(const std::string_view filePath) {
    glTF result{};

    std::ifstream file(filePath.data(), std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        std::printf( "Error: file %s does not exists.\n", filePath.data());
        return result;
    }

    Allocator* heapAllocator = &MemoryService::instance()->system_allocator;

    int64_t fileSize = file.tellg();
    Array<char> buffer;
    buffer.init(heapAllocator, fileSize * sizeof(char) + 64, fileSize);

    file.seekg(0);
    file.read(buffer.data, fileSize);
    file.close();

    json gltfData = json::parse(buffer.data);

    result.allocator.init(megabyte(2));
    Allocator* allocator = &result.allocator;

    for (const auto& properties : gltfData.items()) {
        std::cout << properties.key().c_str() << std::endl;
        if (properties.key() == "asset") {
            load_asset(gltfData, &result.asset, allocator);
        }
        else if (properties.key() == "scenes") {
            load_scenes(gltfData, result, allocator);
        }
        else if (properties.key() == "buffers") {
            load_buffers(gltfData,  result, allocator);
        }
        else if (properties.key() == "bufferViews") {
            load_buffer_views(gltfData, result, allocator);
        }
        else if (properties.key() == "nodes") {
            load_nodes(gltfData, result, allocator);
        }
        else if (properties.key() == "meshes") {
            load_meshes(gltfData, result, allocator);
        }
        else if (properties.key() == "accessors") {
            load_accessors(gltfData, result, allocator);
        }
        else if (properties.key() == "materials") {
            load_materials(gltfData, result, allocator);
        }
        else if (properties.key() == "textures") {
            load_textures(gltfData, result, allocator);
        }
        else if (properties.key() == "images") {
            load_images(gltfData, result, allocator);
        }
        else if (properties.key() == "samplers") {
            load_samplers(gltfData, result, allocator);
        }
    }

    heapAllocator->deallocate(buffer.data);

    return result;
}

void scene::gltf_free(glTF &scene) {
    scene.allocator.shutdown();
}

int64_t scene::get_data_offset(int32_t accessor_offset, int32_t buffer_view_offset) {\
    int32_t byte_offset = buffer_view_offset == INT_MAX ? 0 : buffer_view_offset;
    byte_offset += accessor_offset == INT_MAX ? 0 : accessor_offset;
    return byte_offset;
}

/*int64_t scene::gltf_get_attribute_accessor_index(MeshPrimitive::Attribute *attributes, uint32_t attribute_count,  const char *attribute_name) {
    for (size_t i = 0; i < attribute_count; ++i) {
        MeshPrimitive::Attribute& attribute = attributes[i];
        if (strcmp(attribute.key.data(), attribute_name) == 0 ) {
            return attribute.accessor_index;
        }
    }

    return -1;
}*/