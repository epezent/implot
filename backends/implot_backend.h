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

#include "../implot.h"

#ifdef IMPLOT_BACKEND_ENABLE_OPENGL3
	#define IMPLOT_BACKEND_ENABLED
	#define IMPLOT_BACKEND_HAS_HEATMAP
	#define IMPLOT_BACKEND_HAS_COLORMAP
#endif

namespace ImPlot {
namespace Backend {

//-----------------------------------------------------------------------------
// [SECTION] Misc backend functions
//-----------------------------------------------------------------------------

/**
 * @brief Struct to hold backend-related context data
 *
 * A backend may store in this struct any data it needs, with no constraints. A
 * pointer to this struct will be stored inside ImPlot's context and can be
 * accessed at any time. This pointer will be set to the returned value of @ref
 * CreateContext(). All resources held by this struct must be freed inside @ref
 * DestroyContext().
 */
struct ContextData;

/**
 * @brief Create backend context
 *
 * Creates and intializes the backend context. The returned pointer will be saved
 * in ImPlot's context and can be accessed later.
 */
IMPLOT_API void* CreateContext();

/**
 * @brief Destroy backend context
 *
 * Destroys and frees any memory or resources needed by the backend. After this
 * call returns, no more calls to any backend function will be performed.
 */
IMPLOT_API void DestroyContext();

/** @brief Bust plot cache. Called from @ref ImPlot::BustPlotCache() */
IMPLOT_API void BustPlotCache();

/** @brief Bust item cache. Called from @ref ImPlot::BustItemCache() */
IMPLOT_API void BustItemCache();

//-----------------------------------------------------------------------------
// [SECTION] Colormap functions
//-----------------------------------------------------------------------------

/**
 * @brief Add a colormap
 *
 * Adds a colormap to be handled by the backend.
 *
 * @param keys   Colors for this colormap, in RGBA format
 * @param count  Number of colors in this colormap
 * @param qual   Qualitative: whether the colormap is continuous (`false`) or
 *               not (`true`)
 */
IMPLOT_API void AddColormap(const ImU32* keys, int count, bool qual);

//-----------------------------------------------------------------------------
// [SECTION] Heatmap functions
//-----------------------------------------------------------------------------

/**
 * @brief Set heatmap data
 *
 * Sets the data of the heatmap with the given plot ID.
 *
 * @param plotID  ID of the heatmap to update. This ID is unique, but it is not
 *                continuous nor always positive.
 * @param values  Data of the heatmap to be set.`values[0]` corresponds with the
 *                top-left corner of the data.
 * @param rows    Number of rows of this heatmap
 * @param cols    Number of columns of this heatmap
 */
IMPLOT_API void SetHeatmapData(int plotID, const ImS8*   values, int rows, int cols);

/** @copydoc SetHeatmapData(int,const ImU8*,int,int) */
IMPLOT_API void SetHeatmapData(int plotID, const ImU8*   values, int rows, int cols);

/** @copydoc SetHeatmapData(int,const ImS16*,int,int) */
IMPLOT_API void SetHeatmapData(int plotID, const ImS16*  values, int rows, int cols);

/** @copydoc SetHeatmapData(int,const ImU16*,int,int) */
IMPLOT_API void SetHeatmapData(int plotID, const ImU16*  values, int rows, int cols);

/** @copydoc SetHeatmapData(int,const ImS32*,int,int) */
IMPLOT_API void SetHeatmapData(int plotID, const ImS32*  values, int rows, int cols);

/** @copydoc SetHeatmapData(int,const ImU32*,int,int) */
IMPLOT_API void SetHeatmapData(int plotID, const ImU32*  values, int rows, int cols);

/** @copydoc SetHeatmapData(int,const float*,int,int) */
IMPLOT_API void SetHeatmapData(int plotID, const float*  values, int rows, int cols);

/** @copydoc SetHeatmapData(int,const double*,int,int) */
IMPLOT_API void SetHeatmapData(int plotID, const double* values, int rows, int cols);

/** @copydoc SetHeatmapData(int,const ImS64*,int,int) */
IMPLOT_API void SetHeatmapData(int plotID, const ImS64*  values, int rows, int cols);

/** @copydoc SetHeatmapData(int,const ImU64*,int,int) */
IMPLOT_API void SetHeatmapData(int plotID, const ImU64*  values, int rows, int cols);

/**
 * @brief Render heatmap
 *
 * Renders the heatmap by submitting the appropriate commands to the current
 * draw list.
 *
 * @param plotID      ID of the heatmap to be rendered
 * @param DrawList    Draw list where to submit the render commands
 * @param bounds_min  Minimum bounds of the heatmap (without clipping)
 * @param bounds_max  Maximum bounds of the heatmap (without clipping)
 * @param scale_min   Minimum value of the heatmap
 * @param scale_max   Maximum value of the heatmap
 * @param colormap    Colormap to be used when rendering this heatmap
 *
 * @note There might be values greater than `scale_max` or lower than `scale_min`.
 *       The shader used for rendering should clamp this values appropriately.
 */
IMPLOT_API void RenderHeatmap(
	int plotID, ImDrawList& DrawList, const ImVec2& bounds_min, const ImVec2& bounds_max,
	float scale_min, float scale_max, ImPlotColormap colormap);

}
}

namespace ImPlot {
namespace Backend {

#ifndef IMPLOT_BACKEND_ENABLED

inline void* CreateContext() { return nullptr; }
inline void DestroyContext() {}

inline void BustPlotCache() {}
inline void BustItemCache() {}

#endif

#ifndef IMPLOT_BACKEND_HAS_COLORMAP

inline void AddColormap(const ImU32*, int, bool) {}

#endif

}
}
