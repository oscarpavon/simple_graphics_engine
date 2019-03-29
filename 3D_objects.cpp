#include "3D_objects.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>


void mesh::load_model(std::string model_path){
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, model_path.c_str())) {
            throw std::runtime_error(warn + err);
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex = {};

                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.color = {1.0f, 1.0f, 1.0f};

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }
}

void mesh::SetLocation(float x, float y, float z){
		Location.x = x;
		Location.y = y;
		Location.z = z;

        glm::mat4 myMatrix = glm::translate(glm::mat4(), glm::vec3(Location.x,Location.y,Location.z));
        glm::vec4 myVector(10.0f, 10.0f, 10.0f, 0.0f);
        glm::vec4 transformedVector = myMatrix * myVector; // guess the result
       // model_matrix = transformedVector;
       model_matrix = glm::translate(model_matrix, glm::vec3(Location.x,Location.y,Location.z));
	}