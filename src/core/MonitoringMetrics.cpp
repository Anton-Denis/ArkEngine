#include "../../include/core/MonitoringMetrics.hpp"
#include <algorithm>
#include <cstring>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif
#include "glad/glad.h"

// Helper: Extension-Erkennung ohne GLAD Variablen
static bool HasGLExtension(const char* name) {
    if (!name) return false;
    GLint numExt = 0;
    if (glGetStringi) {
        glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
        for (GLint i=0;i<numExt;++i) {
            const char* ext = (const char*)glGetStringi(GL_EXTENSIONS, i);
            if (ext && std::strcmp(ext, name)==0) return true;
        }
        return false;
    }
    // Fallback (älter): gesamter String (deprecated, aber als Rückfallebene ok)
    const char* all = (const char*)glGetString(GL_EXTENSIONS);
    if (!all) return false;
    std::string s(all);
    std::string needle(name);
    needle.push_back(' ');
    if (s.rfind(needle, 0) == 0) return true; // ganz vorne
    return s.find(std::string(" ")+needle) != std::string::npos;
}

// Definiere fehlende Konstanten sicherheitshalber
#ifndef GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX
#define GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX 0x9048
#endif
#ifndef GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX
#define GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX 0x9049
#endif
#ifndef GL_TEXTURE_FREE_MEMORY_ATI
#define GL_TEXTURE_FREE_MEMORY_ATI 0x87FC
#endif

MonitoringMetrics& MonitoringMetrics::Instance() {
    static MonitoringMetrics inst; return inst;
}

MonitoringMetrics::MonitoringMetrics() {
    const size_t N = 400; // ca. 6-7 Sekunden bei 60 FPS
    frameTimeMs.init(N);
    cpuFrameMs.init(N);
    gpuFrameMs.init(N);
    fpsSeries.init(N);
    cpuUsageSeries.init(N);
    ramMBSeries.init(N);
    vramMBSeries.init(N);
    vramUsedMBSeries.init(N);
#ifdef _WIN32
    SYSTEM_INFO si; GetSystemInfo(&si); cpuCount = (int)si.dwNumberOfProcessors;
#endif
    // OpenGL Timer Queries
    glGenQueries(2, gpuQuery);
}

void MonitoringMetrics::BeginFrameCpu() {
    cpuFrameStart = std::chrono::high_resolution_clock::now();
}

void MonitoringMetrics::EndFrameCpu(double frameTimeSeconds) {
    using namespace std::chrono;
    auto end = high_resolution_clock::now();
    double cpuMs = duration<double, std::milli>(end - cpuFrameStart).count();
    double frameMs = frameTimeSeconds * 1000.0;
    frameTimeMs.push((float)frameMs);
    cpuFrameMs.push((float)cpuMs);
    if (frameMs > 0.0001) fpsSeries.push((float)(1000.0 / frameMs));
    else fpsSeries.push(0.f);
}

void MonitoringMetrics::BeginFrameGpu() {
    if (!gpuQueryActive) {
        glQueryCounter(gpuQuery[0], GL_TIMESTAMP);
        gpuQueryActive = true;
    }
}

void MonitoringMetrics::EndFrameGpu() {
    if (gpuQueryActive) {
        glQueryCounter(gpuQuery[1], GL_TIMESTAMP);
        // Result später abholen (non-blocking)
        GLint available = 0;
        glGetQueryObjectiv(gpuQuery[1], GL_QUERY_RESULT_AVAILABLE, &available);
        if (available) {
            unsigned long long startTime = 0, endTime = 0;
            glGetQueryObjectui64v(gpuQuery[0], GL_QUERY_RESULT, &startTime);
            glGetQueryObjectui64v(gpuQuery[1], GL_QUERY_RESULT, &endTime);
            double gpuMs = (endTime - startTime) / 1000000.0; // ns -> ms
            lastGpuFrameMs = (float)gpuMs;
            gpuFrameMs.push(lastGpuFrameMs);
            gpuQueryActive = false;
        }
    }
}

void MonitoringMetrics::UpdateProcessCpu() {
#ifdef _WIN32
    FILETIME ftNow, ftCreation, ftExit, ftKernel, ftUser;
    GetSystemTimeAsFileTime(&ftNow);
    auto fileTimeToULL = [](const FILETIME& ft){ return ( (unsigned long long)ft.dwHighDateTime << 32) | ft.dwLowDateTime; };
    if (GetProcessTimes(GetCurrentProcess(), &ftCreation, &ftExit, &ftKernel, &ftUser)) {
        unsigned long long now = fileTimeToULL(ftNow);
        unsigned long long k = fileTimeToULL(ftKernel);
        unsigned long long u = fileTimeToULL(ftUser);
        if (lastTimeStamp != 0) {
            unsigned long long sysDelta = now - lastTimeStamp;
            unsigned long long procDelta = (k - lastKernelTime) + (u - lastUserTime);
            if (sysDelta > 0) {
                double percent = (double)procDelta / (double)sysDelta * 100.0 / cpuCount;
                lastCpuUsage = (float)percent;
            }
        }
        lastKernelTime = k; lastUserTime = u; lastTimeStamp = now;
    }
#else
    // Platzhalter für andere Plattformen
    lastCpuUsage = 0.f;
#endif
    cpuUsageSeries.push(lastCpuUsage);
}

void MonitoringMetrics::UpdateMemory() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX pmc; std::memset(&pmc,0,sizeof(pmc));
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        lastRamMB = (float)(pmc.WorkingSetSize / (1024.0*1024.0));
    }
    static bool extChecked = false;
    static bool hasNVX = false;
    static bool hasATI = false;
    if (!extChecked) {
        hasNVX = HasGLExtension("GL_NVX_gpu_memory_info");
        hasATI = HasGLExtension("GL_ATI_meminfo");
        extChecked = true;
    }
    if (hasNVX) {
        GLint totalKB = 0, curKB = 0;
        glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalKB);
        glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &curKB);
        lastVramTotalMB = totalKB / 1024.0f;
        lastVramUsedMB  = (totalKB - curKB) / 1024.0f;
    } else if (hasATI) {
        GLint texFree[4] = {0};
        glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, texFree);
        // Nur freie Memory bekannt -> keine Total/Used zuverlässige Aufschlüsselung
        lastVramTotalMB = 0.f;
        lastVramUsedMB  = 0.f;
    } else {
        lastVramTotalMB = 0.f;
        lastVramUsedMB  = 0.f;
    }
#else
    lastRamMB = 0.f; lastVramTotalMB = 0.f; lastVramUsedMB = 0.f;
#endif
    ramMBSeries.push(lastRamMB);
    vramMBSeries.push(lastVramTotalMB);
    vramUsedMBSeries.push(lastVramUsedMB);
}
