#include "../../../include/core/ui/MonitoringPanel.hpp"
#include "../../../include/core/ui/PanelContext.hpp"
#include "imgui.h"
#include <algorithm>
#include <string>

void MonitoringPanel::PlotSeries(const char* label, const MonitoringMetrics::SampleSeries& series, const char* unitFmt, float scale, bool autoScale) {
    auto ordered = series.ordered();
    if (ordered.empty()) {
        ImGui::Text("%s: --", label);
        return;
    }
    float maxVal = 0.f;
    if (autoScale) {
        for (float v : ordered) maxVal = std::max(maxVal, v);
        if (maxVal <= 0.f) maxVal = 1.f;
    } else {
        maxVal = scale;
    }
    float last = ordered.back();
    ImGui::Text("%s: %.2f %s", label, last, unitFmt);
    std::string plotId = std::string("##plot_") + label;
    ImGui::PlotLines(plotId.c_str(), ordered.data(), (int)ordered.size(), 0, nullptr, 0.0f, maxVal * 1.05f, ImVec2(0,60));
}

void MonitoringPanel::Draw(PanelContext&) {
    auto& metrics = MonitoringMetrics::Instance();
    ImGui::Begin("Monitoring");

    if (ImGui::BeginTabBar("MonTabs")) {
        if (ImGui::BeginTabItem("Frame")) {
            PlotSeries("Frame Time (ms)", metrics.FrameTimeMs(), "ms");
            PlotSeries("CPU Frame (ms)", metrics.CpuFrameMs(), "ms");
            PlotSeries("GPU Frame (ms)", metrics.GpuFrameMs(), "ms");
            PlotSeries("FPS", metrics.Fps(), "fps");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("CPU / Memory")) {
            PlotSeries("CPU Usage (%)", metrics.CpuUsagePercent(), "%");
            PlotSeries("RAM (MB)", metrics.RamMB(), "MB");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("GPU Memory")) {
            PlotSeries("VRAM Total (MB)", metrics.VramMB(), "MB");
            PlotSeries("VRAM Used (MB)", metrics.VramUsedMB(), "MB");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

