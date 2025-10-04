//
// Created by Anton on 04.07.2025.
//
#pragma once
#include "Shapes.hpp"
#include "../../include/core/ResourceManager.hpp"

class Plane : public Shapes {
public:
    Plane(const std::vector<Texture>& textures = {}) {
        mesh = std::make_shared<Mesh>(GetVertices(),GetIndices(),textures.empty() ? GetDefaultTextures() : textures);
    }

private:
    static std::vector<Texture> GetDefaultTextures() {
        std::vector<Texture> textures;
        Texture diffuse;
        diffuse.id = ResourceManager::GetTexture("resources/images/container2.png");
        diffuse.type = "texture_diffuse";
        diffuse.path = "resources/images/container2.png";
        textures.push_back(diffuse);

        Texture specular;
        specular.id = ResourceManager::GetTexture("resources/images/container2_specular.png");
        specular.type = "texture_specular";
        specular.path = "resources/images/container2_specular.png";
        textures.push_back(specular);

        return textures;
    }

    static std::vector<Vertex> GetVertices() {
        static const float rawVertices[] = {
                // Position         // Normal      // TexCoords
                -0.5f, 0.0f, -0.5f,  0,1,0,   0.0f, 0.0f,
                0.5f, 0.0f, -0.5f,  0,1,0,   1.0f, 0.0f,
                0.5f, 0.0f,  0.5f,  0,1,0,   1.0f, 1.0f,
                -0.5f, 0.0f,  0.5f,  0,1,0,   0.0f, 1.0f
        };

        std::vector<Vertex> vertices;

        for (size_t i = 0; i < 4; ++i) {
            Vertex v;
            v.position = glm::vec3(rawVertices[i * 8 + 0], rawVertices[i * 8 + 1], rawVertices[i * 8 + 2]);
            v.normal   = glm::vec3(rawVertices[i * 8 + 3], rawVertices[i * 8 + 4], rawVertices[i * 8 + 5]);
            v.texCoords= glm::vec2(rawVertices[i * 8 + 6], rawVertices[i * 8 + 7]);
            vertices.push_back(v);
        }

        return vertices;
    }

    static std::vector<unsigned int> GetIndices() {
        static const unsigned int idx[] = {
                0, 1, 2,
                2, 3, 0
        };

        return std::vector<unsigned int>(idx, idx + 6);
    }
};
