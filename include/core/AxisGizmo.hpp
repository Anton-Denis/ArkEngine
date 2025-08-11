#pragma once
#include <imgui.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include <glm/gtc/type_ptr.hpp>

// ------------------------------
// Colors
// ------------------------------
struct GizmoColors {
    ImU32 xFront = IM_COL32(230,  80,  80, 255);
    ImU32 yFront = IM_COL32( 90, 200,  90, 255);
    ImU32 zFront = IM_COL32( 90, 120, 230, 255);
    ImU32 xBack  = IM_COL32(230,  80,  80, 120); // only alpha is used for "back" blending
    ImU32 yBack  = IM_COL32( 90, 200,  90, 120);
    ImU32 zBack  = IM_COL32( 90, 120, 230, 120);
    ImU32 ring   = IM_COL32(255, 255, 255,  40);
    ImU32 bg     = IM_COL32(  0,   0,   0,  60);
    ImU32 text   = IM_COL32(255, 255, 255, 230);
    ImU32 outline= IM_COL32( 20,  20,  20, 200);
};

// ------------------------------
// Helpers
// ------------------------------
inline ImVec2 projPerspective(const glm::vec3& pCam, ImVec2 center, float radius, float focal = 1.6f) {
    float denom = (pCam.z + focal);
    float k = (denom != 0.0f) ? (radius / denom) : 0.0f;
    return ImVec2(center.x + pCam.x * k, center.y - pCam.y * k);
}

inline int  chR(ImU32 c){ return (c >> IM_COL32_R_SHIFT) & 0xFF; }
inline int  chG(ImU32 c){ return (c >> IM_COL32_G_SHIFT) & 0xFF; }
inline int  chB(ImU32 c){ return (c >> IM_COL32_B_SHIFT) & 0xFF; }
inline int  chA(ImU32 c){ return (c >> IM_COL32_A_SHIFT) & 0xFF; }
inline ImU32 makeRGBA(int r,int g,int b,int a){ return IM_COL32(r,g,b,a); }

// Smooth 0..1 (Hermite)
inline float smooth01(float t){ t = std::clamp(t, 0.0f, 1.0f); return t*t*(3.0f - 2.0f*t); }

// Map dirCam.z in [-1..+1] to a smooth factor: 1 at z=-1 (front), 0 at z=+1 (back)
inline float depthFactor(float z){
    float t = (1.0f - z) * 0.5f; // linear map [-1..+1] -> [1..0]
    return smooth01(t);
}

// Blend alpha between "back" and "front" alphas using smooth depth factor.
// RGB from the "front" color (keeps hue stable).
inline ImU32 blendDepthAlpha(ImU32 colFront, ImU32 colBack, float z){
    int r = chR(colFront), g = chG(colFront), b = chB(colFront);
    float t = depthFactor(z); // 0..1
    float aF = (float)chA(colFront);
    float aB = (float)chA(colBack);
    int a = (int)std::round(aB + (aF - aB) * t);
    return makeRGBA(r,g,b,a);
}

// Slight RGB tint for negative axes (so +X vs -X are distinguishable)
inline ImU32 tintRGB(ImU32 c, float mul){
    int r = (int)std::round(std::clamp(chR(c)*mul, 0.0f, 255.0f));
    int g = (int)std::round(std::clamp(chG(c)*mul, 0.0f, 255.0f));
    int b = (int)std::round(std::clamp(chB(c)*mul, 0.0f, 255.0f));
    return makeRGBA(r,g,b,chA(c));
}

// ------------------------------
// Main (six dots; lines+labels only for +axes)
// OpenGL convention: camera looks along -Z => "front" if z < 0
// ------------------------------
inline void ImGui_DrawAxisDotsWithPosLines_Smooth(const glm::mat4& view,ImVec2 screenPos = ImVec2(20, 20),float size = 112.0f,const GizmoColors& colors = GizmoColors(),float focal = 1.6f)
{
    ImDrawList* dl = ImGui::GetForegroundDrawList();

    const ImVec2 p0 = screenPos;
    const ImVec2 p1 = ImVec2(screenPos.x + size, screenPos.y + size);
    const ImVec2 c  = ImVec2((p0.x+p1.x)*0.5f, (p0.y+p1.y)*0.5f);
    const float r   = size * 0.42f;

    // Panel
    dl->AddRectFilled(p0, p1, colors.bg, 8.0f);
    dl->AddCircle(c, r, colors.ring, 64, 1.0f);

    const glm::mat3 R = glm::mat3(view); // world->camera rotation

    struct Dot { ImVec2 pos; float z; ImU32 fill; float radius; bool isPositive; int axis; };
    struct Lab { ImVec2 pos; std::string text; ImU32 col; };
    struct Line{ ImVec2 a,b; ImU32 col; float thick; };

    std::vector<Dot>  dots; dots.reserve(6);
    std::vector<Line> lines; lines.reserve(3);
    std::vector<Lab>  labels; labels.reserve(3);

    auto pushAxis = [&](const glm::vec3& dirW, const char* label,ImU32 colFront, ImU32 colBack, bool isPositive, int axisIdx)
    {
        glm::vec3 dirCam = glm::normalize(R * dirW);
        ImU32 baseCol = blendDepthAlpha(colFront, colBack, dirCam.z);

        if (!isPositive) baseCol = tintRGB(baseCol, 0.92f);

        const float axisLen = 0.92f;
        ImVec2 A = projPerspective(glm::vec3(0), c, r, focal);
        ImVec2 B = projPerspective(dirCam * axisLen, c, r, focal);

        float baseRadius = 9.0f;
        float radius = baseRadius;

        if (isPositive) {
            ImVec2 ts = ImGui::CalcTextSize(label);
            radius = std::max(baseRadius, (std::max(ts.x, ts.y)*0.5f + 3.0f));
        }

        dots.push_back(Dot{B, dirCam.z, baseCol, radius, isPositive, axisIdx});

        if (isPositive) {
            float thick = 2.5f;
            ImU32 lineCol = baseCol; // same smooth alpha
            lines.push_back(Line{A, B, lineCol, thick});

            ImVec2 ts = ImGui::CalcTextSize(label);
            ImVec2 tp = ImVec2(B.x - ts.x*0.5f, B.y - ts.y*0.5f);
            labels.push_back(Lab{tp, label, colors.text});
        }
    };

    pushAxis(glm::vec3( 1,0,0), "X", colors.xFront, colors.xBack, true,  0);
    pushAxis(glm::vec3(-1,0,0), "",   colors.xFront, colors.xBack, false, 0);
    pushAxis(glm::vec3(0, 1,0), "Y", colors.yFront, colors.yBack, true,  1);
    pushAxis(glm::vec3(0,-1,0), "",   colors.yFront, colors.yBack, false, 1);
    pushAxis(glm::vec3(0,0, 1), "Z", colors.zFront, colors.zBack, true,  2);
    pushAxis(glm::vec3(0,0,-1), "",   colors.zFront, colors.zBack, false, 2);

    std::sort(dots.begin(), dots.end(), [](const Dot& a, const Dot& b){ return a.z > b.z; });
    for (const Dot& d : dots){
        dl->AddCircleFilled(d.pos, d.radius, d.fill, 48);
        dl->AddCircle(d.pos, d.radius, colors.outline, 48, 1.0f);
    }

    for (const Line& L : lines){
        dl->AddLine(L.a, L.b, L.col, L.thick);
    }

    for (const Lab& lb : labels){
        dl->AddText(lb.pos, lb.col, lb.text.c_str());
    }
}

inline void ImGui_DrawAxisDots(const glm::mat4& view,ImVec2 screenPos = ImVec2(20, 20),float size = 112.0f,const GizmoColors& colors = GizmoColors(),float focal = 1.6f)
{
    ImGui_DrawAxisDotsWithPosLines_Smooth(view, screenPos, size, colors, focal);
}
