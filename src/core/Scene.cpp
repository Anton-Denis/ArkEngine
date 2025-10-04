#include "../../include/core/Scene.hpp"
#include <fstream>

using json = nlohmann::json;

Scene::~Scene() {
    Clear();
}

void Scene::AddObject(std::shared_ptr<GameObject> obj) {
    objects.push_back(obj);
}

void Scene::RemoveObjectAt(size_t index) {
    if (index < objects.size())
        objects.erase(objects.begin() + index);
}

void Scene::Clear() {
    objects.clear();
}

std::vector<std::shared_ptr<GameObject>>& Scene::GetObjects() {
    return objects;
}

const std::vector<std::shared_ptr<GameObject>>& Scene::GetObjects() const {
    return objects;
}

/*void Scene::Save(const std::string& filename) const {
    json j;
    j["objects"] = json::array();
    for (const auto& obj : objects) {
        json entry;
        // Transform
        entry["position"] = { obj->position.x, obj->position.y, obj->position.z };
        entry["scale"] = { obj->scale.x, obj->scale.y, obj->scale.z };
        entry["rotation"] = { obj->rotation.w, obj->rotation.x, obj->rotation.y, obj->rotation.z };

        // Typ-spezifisch
        if (auto* light = dynamic_cast<Light*>(obj.get())) {
            entry["type"] = "Light";
            entry["lightType"] = static_cast<int>(light->type);
            entry["color"] = { light->color.r, light->color.g, light->color.b };
            entry["direction"] = { light->direction.x, light->direction.y, light->direction.z };
            entry["constant"] = light->constant;
            entry["linear"] = light->linear;
            entry["quadratic"] = light->quadratic;
            entry["cutOff"] = light->cutOff;
            entry["outerCutOff"] = light->outerCutOff;
            // intensity falls vorhanden
            if constexpr (requires { light->intensity; })
                entry["intensity"] = light->intensity;
        } else if (auto* model = dynamic_cast<Model*>(obj.get())) {
            entry["type"] = "Model";
            // Modellpfad speichern (ggf. Methode GetPath() ergänzen)
            // Hier als Platzhalter:
            entry["modelPath"] = model->GetPath();
        } else {
            entry["type"] = "Unknown";
        }
        j["objects"].push_back(entry);
    }
    std::ofstream out(filename);
    out << std::setw(4) << j;
}

void Scene::Load(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) return;
    json j;
    in >> j;
    Clear();
    for (const auto& entry : j["objects"]) {
        std::string type = entry.value("type", "Unknown");
        std::shared_ptr<GameObject> obj;
        if (type == "Light") {
            auto lightType = static_cast<Light::Type>(entry.value("lightType", 0));
            auto light = std::make_shared<Light>(lightType);
            light->color = glm::vec3(entry["color"][0], entry["color"][1], entry["color"][2]);
            light->direction = glm::vec3(entry["direction"][0], entry["direction"][1], entry["direction"][2]);
            light->constant = entry.value("constant", 1.0f);
            light->linear = entry.value("linear", 0.09f);
            light->quadratic = entry.value("quadratic", 0.032f);
            light->cutOff = entry.value("cutOff", 0.0f);
            light->outerCutOff = entry.value("outerCutOff", 0.0f);
            if (entry.contains("intensity"))
                light->intensity = entry["intensity"];
            obj = light;
        } else if (type == "Model") {
            std::string path = entry.value("modelPath", "");
            auto model = ResourceManager::GetModel(path);
            obj = model;
        } else {
            continue; // Unbekannter Typ
        }
        // Transform wiederherstellen
        obj->position = glm::vec3(entry["position"][0], entry["position"][1], entry["position"][2]);
        obj->scale = glm::vec3(entry["scale"][0], entry["scale"][1], entry["scale"][2]);
        obj->rotation = glm::quat(entry["rotation"][0], entry["rotation"][1], entry["rotation"][2], entry["rotation"][3]);
        AddObject(obj);
    }
}*/