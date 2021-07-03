// MIT License

// Copyright (c) 2021 Evan Pezent

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// ImPlot v0.10 WIP

#pragma once

#if defined(IMPLOT_BACKEND_ENABLE_OPENGL3)
    #define IMPLOT_BACKEND_ENABLED
    #define IMPLOT_BACKEND_HAS_HEATMAP
    #define IMPLOT_BACKEND_HAS_COLORMAP
#elif defined(IMPLOT_BACKEND_ENABLE_METAL)
#endif

namespace ImPlot {
namespace Backend {

#ifdef IMPLOT_BACKEND_ENABLED
    void* CreateContext();
    void DestroyContext();
    void BustPlotCache();
    void BustItemCache();
#else
    inline void* CreateContext() { return nullptr; }
    inline void DestroyContext() {}
    inline void BustPlotCache() {}
    inline void BustItemCache() {}
#endif

#ifdef IMPLOT_BACKEND_HAS_COLORMAP
    void AddColormap(const ImU32*, int, bool);
#else
    inline void AddColormap(const ImU32*, int, bool) {}
#endif

#ifdef IMPLOT_BACKEND_HAS_HEATMAP
    void SetHeatmapData(int itemID, const ImS16*  values, int rows, int cols);
    void SetHeatmapData(int itemID, const ImS32*  values, int rows, int cols);
    void SetHeatmapData(int itemID, const ImS64*  values, int rows, int cols);
    void SetHeatmapData(int itemID, const ImS8*   values, int rows, int cols);
    void SetHeatmapData(int itemID, const ImU16*  values, int rows, int cols);
    void SetHeatmapData(int itemID, const ImU32*  values, int rows, int cols);
    void SetHeatmapData(int itemID, const ImU64*  values, int rows, int cols);
    void SetHeatmapData(int itemID, const ImU8*   values, int rows, int cols);
    void SetHeatmapData(int itemID, const double* values, int rows, int cols);
    void SetHeatmapData(int itemID, const float*  values, int rows, int cols);
    void RenderHeatmap(
        int itemID, ImDrawList& DrawList, const ImVec2& bounds_min, const ImVec2& bounds_max,
        float scale_min, float scale_max, ImPlotColormap colormap, bool reverse_y);
    void SetAxisLog(int itemID, bool x_is_log, bool y_is_log, const ImPlotPoint& bounds_min, const ImPlotPoint& bounds_max);
#endif

}
}
