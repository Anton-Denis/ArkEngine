#pragma once
#include "Light.hpp"
#include <string>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class SpotLight : public Light {
public:
    SpotLight()
        : Light(Type::Spot)
    {
        // typische Default-Werte
        direction = {0.0f, -1.0f, 0.0f};
        constant = 1.0f;
        linear = 0.09f;
        quadratic = 0.032f;
        // Cutoffs in Cosinus (Shader erwartet cos Winkel)
        float innerDeg = 12.5f;
        float outerDeg = 17.5f;
        cutOff = glm::cos(glm::radians(innerDeg));
        outerCutOff = glm::cos(glm::radians(outerDeg));
    }

    void UploadToShader(Shader* shader, int index) const override {
        std::string base = "spotLights[" + std::to_string(index) + "]";
        shader->SetVec3(base + ".position", position);
        shader->SetVec3(base + ".direction", direction);
        shader->SetFloat(base + ".cutOff", cutOff);
        shader->SetFloat(base + ".outerCutOff", outerCutOff);
        shader->SetFloat(base + ".constant", constant);
        shader->SetFloat(base + ".linear", linear);
        shader->SetFloat(base + ".quadratic", quadratic);
        shader->SetVec3(base + ".ambient",  color * 0.1f);
        shader->SetVec3(base + ".diffuse",  color * 0.8f);
        shader->SetVec3(base + ".specular", color * 1.0f);
    }
};
