#pragma once
#include "IPanel.hpp"
#include "../MonitoringMetrics.hpp"

class MonitoringPanel : public IPanel {
public:
    const char* Name() const override { return "Monitoring"; }
    void Draw(PanelContext& ctx) override;
private:
    void PlotSeries(const char* label, const MonitoringMetrics::SampleSeries& series, const char* unitFmt, float scale = 0.f, bool autoScale = true);
};
