//
// Created by Anton on 09.07.2025.
//
#pragma once

#include <vector>
#include <string>
#include <fstream>
#include "Mesh.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include "../core/ResourceManager.hpp"

class Model : public GameObject
{
public:
    Model(const std::string& path)
    {
        loadModel(path);
    }
    void Draw(Shader &shader);
    std::vector<Mesh*> GetMeshes() {
        std::vector<Mesh*> result;
        for (auto& mesh : meshes) {
            result.push_back(&mesh);
        }
        return result;
    }
private:
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Texture> textures_loaded;

    void loadModel(std::string path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                         std::string typeName);
};
