#pragma once
#include <vector>
#include <chrono>

class MonitoringMetrics {
public:
    static MonitoringMetrics& Instance();

    void BeginFrameCpu();
    void EndFrameCpu(double frameTimeSeconds);
    void BeginFrameGpu();
    void EndFrameGpu();

    void UpdateProcessCpu();
    void UpdateMemory();

    struct SampleSeries {
        std::vector<float> data; // ring buffer semantics
        size_t cursor = 0;
        bool filled = false;
        void init(size_t n) { data.assign(n, 0.f); cursor = 0; filled = false; }
        void push(float v) { if (data.empty()) return; data[cursor] = v; cursor = (cursor+1)%data.size(); if (cursor==0) filled=true; }
        std::vector<float> ordered() const { if (!filled) return std::vector<float>(data.begin(), data.begin()+cursor); std::vector<float> out; out.reserve(data.size()); for (size_t i=cursor;i<data.size();++i) out.push_back(data[i]); for (size_t i=0;i<cursor;++i) out.push_back(data[i]); return out; }
    };

    const SampleSeries& FrameTimeMs() const { return frameTimeMs; }
    const SampleSeries& CpuFrameMs() const { return cpuFrameMs; }
    const SampleSeries& GpuFrameMs() const { return gpuFrameMs; }
    const SampleSeries& Fps() const { return fpsSeries; }
    const SampleSeries& CpuUsagePercent() const { return cpuUsageSeries; }
    const SampleSeries& RamMB() const { return ramMBSeries; }
    const SampleSeries& VramMB() const { return vramMBSeries; }
    const SampleSeries& VramUsedMB() const { return vramUsedMBSeries; }

    float LastCpuUsage() const { return lastCpuUsage; }
    float LastRamMB() const { return lastRamMB; }
    float LastVramTotalMB() const { return lastVramTotalMB; }
    float LastVramUsedMB() const { return lastVramUsedMB; }
    float LastGpuFrameMs() const { return lastGpuFrameMs; }

private:
    MonitoringMetrics();

    // CPU usage calc state
#ifdef _WIN32
    unsigned long long lastKernelTime = 0;
    unsigned long long lastUserTime = 0;
    unsigned long long lastTimeStamp = 0; // in 100ns units via GetSystemTimeAsFileTime
    int cpuCount = 1;
#endif

    // GPU timer query
    unsigned int gpuQuery[2] = {0,0};
    bool gpuQueryActive = false;

    // Series
    SampleSeries frameTimeMs;
    SampleSeries cpuFrameMs;
    SampleSeries gpuFrameMs;
    SampleSeries fpsSeries;
    SampleSeries cpuUsageSeries;
    SampleSeries ramMBSeries;
    SampleSeries vramMBSeries;      // total
    SampleSeries vramUsedMBSeries;  // used

    float lastCpuUsage = 0.f;
    float lastRamMB = 0.f;
    float lastVramTotalMB = 0.f;
    float lastVramUsedMB = 0.f;
    float lastGpuFrameMs = 0.f;
    std::chrono::high_resolution_clock::time_point cpuFrameStart;
};
