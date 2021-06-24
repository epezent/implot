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

#define IMPLOT_BACKEND_ENABLED
#define IMPLOT_BACKEND_HAS_HEATMAP
#define IMPLOT_BACKEND_HAS_COLORMAP

#if !defined(IMGUI_IMPL_OPENGL_ES2) \
 && !defined(IMGUI_IMPL_OPENGL_ES3) \
 && !defined(IMGUI_IMPL_OPENGL_LOADER_GL3W) \
 && !defined(IMGUI_IMPL_OPENGL_LOADER_GLEW) \
 && !defined(IMGUI_IMPL_OPENGL_LOADER_GLAD) \
 && !defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2) \
 && !defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2) \
 && !defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3) \
 && !defined(IMGUI_IMPL_OPENGL_LOADER_CUSTOM)

// Try to detect GLES on matching platforms
#if defined(__APPLE__)
#include "TargetConditionals.h"
#endif
#if (defined(__APPLE__) && (TARGET_OS_IOS || TARGET_OS_TV)) || (defined(__ANDROID__))
#define IMGUI_IMPL_OPENGL_ES3               // iOS, Android  -> GL ES 3, "#version 300 es"
#elif defined(__EMSCRIPTEN__)
#define IMGUI_IMPL_OPENGL_ES2               // Emscripten    -> GL ES 2, "#version 100"

// Otherwise try to detect supported Desktop OpenGL loaders..
#elif defined(__has_include)
#if __has_include(<GL/glew.h>)
	#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#elif __has_include(<glad/glad.h>)
	#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#elif __has_include(<glad/gl.h>)
	#define IMGUI_IMPL_OPENGL_LOADER_GLAD2
#elif __has_include(<GL/gl3w.h>)
	#define IMGUI_IMPL_OPENGL_LOADER_GL3W
#elif __has_include(<glbinding/glbinding.h>)
	#define IMGUI_IMPL_OPENGL_LOADER_GLBINDING3
#elif __has_include(<glbinding/Binding.h>)
	#define IMGUI_IMPL_OPENGL_LOADER_GLBINDING2
#else
	#error "Cannot detect OpenGL loader!"
#endif
#else
	#define IMGUI_IMPL_OPENGL_LOADER_GL3W   // Default to GL3W embedded in our repository
#endif

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
 * call returns, no more calls to any backend function can be performed.
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
 * @param itemID  ID of the heatmap to update.
 * @param values  Data of the heatmap to be set.`values[0]` corresponds with the
 *                top-left corner of the data.
 * @param rows    Number of rows of this heatmap
 * @param cols    Number of columns of this heatmap
 */
IMPLOT_API void SetHeatmapData(int itemID, const ImS8*   values, int rows, int cols);

/** @copydoc SetHeatmapData(int,const ImU8*,int,int) */
IMPLOT_API void SetHeatmapData(int itemID, const ImU8*   values, int rows, int cols);

/** @copydoc SetHeatmapData(int,const ImS16*,int,int) */
IMPLOT_API void SetHeatmapData(int itemID, const ImS16*  values, int rows, int cols);

/** @copydoc SetHeatmapData(int,const ImU16*,int,int) */
IMPLOT_API void SetHeatmapData(int itemID, const ImU16*  values, int rows, int cols);

/** @copydoc SetHeatmapData(int,const ImS32*,int,int) */
IMPLOT_API void SetHeatmapData(int itemID, const ImS32*  values, int rows, int cols);

/** @copydoc SetHeatmapData(int,const ImU32*,int,int) */
IMPLOT_API void SetHeatmapData(int itemID, const ImU32*  values, int rows, int cols);

/** @copydoc SetHeatmapData(int,const float*,int,int) */
IMPLOT_API void SetHeatmapData(int itemID, const float*  values, int rows, int cols);

/** @copydoc SetHeatmapData(int,const double*,int,int) */
IMPLOT_API void SetHeatmapData(int itemID, const double* values, int rows, int cols);

/** @copydoc SetHeatmapData(int,const ImS64*,int,int) */
IMPLOT_API void SetHeatmapData(int itemID, const ImS64*  values, int rows, int cols);

/** @copydoc SetHeatmapData(int,const ImU64*,int,int) */
IMPLOT_API void SetHeatmapData(int itemID, const ImU64*  values, int rows, int cols);

/**
 * @brief Render heatmap
 *
 * Renders the heatmap using OpenGL acceleration
 *
 * @param itemID      ID of the heatmap to be rendered
 * @param DrawList    Draw list where to submit the render commands
 * @param bounds_min  Minimum bounds of the heatmap (without clipping)
 * @param bounds_max  Maximum bounds of the heatmap (without clipping)
 * @param scale_min   Minimum value of the heatmap
 * @param scale_max   Maximum value of the heatmap
 * @param colormap    Colormap to be used when rendering this heatmap
 *
 * @note There might be values greater than `scale_max` or lower than `scale_min`.
 *       The shader used for rendering will clamp this values.
 */
IMPLOT_API void RenderHeatmap(
	int itemID, ImDrawList& DrawList, const ImVec2& bounds_min, const ImVec2& bounds_max,
	float scale_min, float scale_max, ImPlotColormap colormap);

}
}
