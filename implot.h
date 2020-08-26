// MIT License

// Copyright (c) 2020 Evan Pezent

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

// ImPlot v0.5 WIP

#pragma once
#include "imgui.h"

#define IMPLOT_VERSION "0.5 WIP"

//-----------------------------------------------------------------------------
// Forward Declarations and Basic Types
//-----------------------------------------------------------------------------

// Forward declarations
struct ImPlotContext;          // ImPlot context (opaque struct, see implot_internal.h)

// Enums/Flags
typedef int ImPlotFlags;       // -> enum ImPlotFlags_
typedef int ImPlotAxisFlags;   // -> enum ImPlotAxisFlags_
typedef int ImPlotCol;         // -> enum ImPlotCol_
typedef int ImPlotStyleVar;    // -> enum ImPlotStyleVar_
typedef int ImPlotMarker;      // -> enum ImPlotMarker_
typedef int ImPlotColormap;    // -> enum ImPlotColormap_

// Options for plots.
enum ImPlotFlags_ {
    ImPlotFlags_MousePos    = 1 << 0,  // the mouse position, in plot coordinates, will be displayed in the bottom-right
    ImPlotFlags_Legend      = 1 << 1,  // a legend will be displayed in the top-left
    ImPlotFlags_Highlight   = 1 << 2,  // plot items will be highlighted when their legend entry is hovered
    ImPlotFlags_BoxSelect   = 1 << 3,  // the user will be able to box-select with right-mouse
    ImPlotFlags_Query       = 1 << 4,  // the user will be able to draw query rects with middle-mouse
    ImPlotFlags_ContextMenu = 1 << 5,  // the user will be able to open context menus with double-right click
    ImPlotFlags_Crosshairs  = 1 << 6,  // the default mouse cursor will be replaced with a crosshair when hovered
    ImPlotFlags_AntiAliased = 1 << 7,  // plot lines will be software anti-aliased (not recommended for density plots, prefer MSAA)
    ImPlotFlags_NoChild     = 1 << 8,  // a child window region will not be used to capture mouse scroll (can boost performance for single ImGui window applications)
    ImPlotFlags_YAxis2      = 1 << 9,  // enable a 2nd y-axis
    ImPlotFlags_YAxis3      = 1 << 10, // enable a 3rd y-axis
    ImPlotFlags_Default     = ImPlotFlags_MousePos | ImPlotFlags_Legend | ImPlotFlags_Highlight | ImPlotFlags_BoxSelect | ImPlotFlags_ContextMenu
};

// Options for plot axes (X and Y).
enum ImPlotAxisFlags_ {
    ImPlotAxisFlags_GridLines  = 1 << 0, // grid lines will be displayed
    ImPlotAxisFlags_TickMarks  = 1 << 1, // tick marks will be displayed for each grid line
    ImPlotAxisFlags_TickLabels = 1 << 2, // text labels will be displayed for each grid line
    ImPlotAxisFlags_Invert     = 1 << 3, // the axis will be inverted
    ImPlotAxisFlags_LockMin    = 1 << 4, // the axis minimum value will be locked when panning/zooming
    ImPlotAxisFlags_LockMax    = 1 << 5, // the axis maximum value will be locked when panning/zooming
    ImPlotAxisFlags_LogScale   = 1 << 6, // a logartithmic (base 10) axis scale will be used
    ImPlotAxisFlags_Default    = ImPlotAxisFlags_GridLines | ImPlotAxisFlags_TickMarks | ImPlotAxisFlags_TickLabels,
    ImPlotAxisFlags_Auxiliary  = ImPlotAxisFlags_TickMarks | ImPlotAxisFlags_TickLabels,
};

// Plot styling colors.
enum ImPlotCol_ {
    // item related colors
    ImPlotCol_Line,          // plot line/outline color (defaults to next unused color in current colormap)
    ImPlotCol_Fill,          // plot fill color for bars (defaults to the current line color)
    ImPlotCol_MarkerOutline, // marker outline color (defaults to the current line color)
    ImPlotCol_MarkerFill,    // marker fill color (defaults to the current line color)
    ImPlotCol_ErrorBar,      // error bar color (defaults to ImGuiCol_Text)
    // plot related colors
    ImPlotCol_FrameBg,       // plot frame background color (defaults to ImGuiCol_FrameBg)
    ImPlotCol_PlotBg,        // plot area background color (defaults to ImGuiCol_WindowBg)
    ImPlotCol_PlotBorder,    // plot area border color (defaults to ImGuiCol_Border)
    ImPlotCol_LegendBg,      // legend background color (defaults to ImGuiCol_PopupBg)
    ImPlotCol_LegendBorder,  // legend border color (defaults to ImPlotCol_PlotBorder)
    ImPlotCol_LegendText,    // legend text color (defaults to ImPlotCol_InlayText)
    ImPlotCol_TitleText,     // plot title text color (defaults to ImGuiCol_Text)
    ImPlotCol_InlayText,     // color of text appearing inside of plots (defaults to ImGuiCol_Text)
    ImPlotCol_XAxis,         // x-axis label and tick lables color (defaults to ImGuiCol_Text)
    ImPlotCol_XAxisGrid,     // x-axis grid color (defaults to 25% ImPlotCol_XAxis)
    ImPlotCol_YAxis,         // y-axis label and tick labels color (defaults to ImGuiCol_Text)
    ImPlotCol_YAxisGrid,     // y-axis grid color (defaults to 25% ImPlotCol_YAxis)
    ImPlotCol_YAxis2,        // 2nd y-axis label and tick labels color (defaults to ImGuiCol_Text)
    ImPlotCol_YAxisGrid2,    // 2nd y-axis grid/label color (defaults to 25% ImPlotCol_YAxis2)
    ImPlotCol_YAxis3,        // 3rd y-axis label and tick labels color (defaults to ImGuiCol_Text)
    ImPlotCol_YAxisGrid3,    // 3rd y-axis grid/label color (defaults to 25% ImPlotCol_YAxis3)
    ImPlotCol_Selection,     // box-selection color (defaults to yellow)
    ImPlotCol_Query,         // box-query color (defaults to green)
    ImPlotCol_Crosshairs,    // crosshairs color (defaults to ImPlotCol_PlotBorder)
    ImPlotCol_COUNT
};

// Plot styling variables.
enum ImPlotStyleVar_ {
    // item styling variables
    ImPlotStyleVar_LineWeight,       // float,  plot item line weight in pixels
    ImPlotStyleVar_Marker,           // int,    marker specification
    ImPlotStyleVar_MarkerSize,       // float,  marker size in pixels (roughly the marker's "radius")
    ImPlotStyleVar_MarkerWeight,     // float,  plot outline weight of markers in pixels
    ImPlotStyleVar_FillAlpha,        // float,  alpha modifier applied to all plot item fills
    ImPlotStyleVar_ErrorBarSize,     // float,  error bar whisker width in pixels
    ImPlotStyleVar_ErrorBarWeight,   // float,  error bar whisker weight in pixels
    ImPlotStyleVar_DigitalBitHeight, // float,  digital channels bit height (at 1) in pixels
    ImPlotStyleVar_DigitalBitGap,    // float,  digital channels bit padding gap in pixels
    // plot styling variables
    ImPlotStyleVar_PlotBorderSize,   // float,  thickness of border around plot area
    ImPlotStyleVar_MinorAlpha,       // float,  alpha multiplier applied to minor axis grid lines
    ImPlotStyleVar_MajorTickLen,     // ImVec2, major tick lengths for X and Y axes
    ImPlotStyleVar_MinorTickLen,     // ImVec2, minor tick lengths for X and Y axes
    ImPlotStyleVar_MajorTickSize,    // ImVec2, line thickness of major ticks
    ImPlotStyleVar_MinorTickSize,    // ImVec2, line thickness of minor ticks
    ImPlotStyleVar_MajorGridSize,    // ImVec2, line thickness of major grid lines
    ImPlotStyleVar_MinorGridSize,    // ImVec2, line thickness of minor grid lines
    ImPlotStyleVar_PlotPadding,      // ImVec2, padding between widget frame and plot area and/or labels
    ImPlotStyleVar_LabelPadding,     // ImVec2, padding between axes labels, tick labels, and plot edge
    ImPlotStyleVar_LegendPadding,    // ImVec2, legend padding from top-left of plot
    ImPlotStyleVar_InfoPadding,      // ImVec2, padding between plot edge and interior info text
    ImPlotStyleVar_PlotMinSize,      // ImVec2, minimum size plot frame can be when shrunk
    ImPlotStyleVar_COUNT
};

// Marker specifications. You can combine these with binary OR, e.g. ImPlotMarker_Circle | ImPlotMarker_Cross.
enum ImPlotMarker_ {
    ImPlotMarker_None        = 1 << 0,  // no marker
    ImPlotMarker_Circle      = 1 << 1,  // a circle marker
    ImPlotMarker_Square      = 1 << 2,  // a square maker
    ImPlotMarker_Diamond     = 1 << 3,  // a diamond marker
    ImPlotMarker_Up          = 1 << 4,  // an upward-pointing triangle marker
    ImPlotMarker_Down        = 1 << 5,  // an downward-pointing triangle marker
    ImPlotMarker_Left        = 1 << 6,  // an leftward-pointing triangle marker
    ImPlotMarker_Right       = 1 << 7,  // an rightward-pointing triangle marker
    ImPlotMarker_Cross       = 1 << 8,  // a cross marker (not fillable)
    ImPlotMarker_Plus        = 1 << 9,  // a plus marker (not fillable)
    ImPlotMarker_Asterisk    = 1 << 10, // a asterisk marker (not fillable)
};

// Built-in colormaps
enum ImPlotColormap_ {
    ImPlotColormap_Default  = 0,  // ImPlot default colormap         (n=10)
    ImPlotColormap_Deep     = 1,  // a.k.a. seaborn deep             (n=10)
    ImPlotColormap_Dark     = 2,  // a.k.a. matplotlib "Set1"        (n=9)
    ImPlotColormap_Pastel   = 3,  // a.k.a. matplotlib "Pastel1"     (n=9)
    ImPlotColormap_Paired   = 4,  // a.k.a. matplotlib "Paired"      (n=12)
    ImPlotColormap_Viridis  = 5,  // a.k.a. matplotlib "viridis"     (n=11)
    ImPlotColormap_Plasma   = 6,  // a.k.a. matplotlib "plasma"      (n=11)
    ImPlotColormap_Hot      = 7,  // a.k.a. matplotlib/MATLAB "hot"  (n=11)
    ImPlotColormap_Cool     = 8,  // a.k.a. matplotlib/MATLAB "cool" (n=11)
    ImPlotColormap_Pink     = 9,  // a.k.a. matplotlib/MATLAB "pink" (n=11)
    ImPlotColormap_Jet      = 10, // a.k.a. MATLAB "jet"             (n=11)
    ImPlotColormap_COUNT
};

// Double precision version of ImVec2 used by ImPlot. Extensible by end users.
struct ImPlotPoint {
    double x, y;
    ImPlotPoint()  { x = y = 0.0; }
    ImPlotPoint(double _x, double _y)     { x = _x; y = _y; }
    double  operator[] (size_t idx) const { return (&x)[idx]; }
    double& operator[] (size_t idx)       { return (&x)[idx]; }
#ifdef IMPLOT_POINT_CLASS_EXTRA
    IMPLOT_POINT_CLASS_EXTRA     // Define additional constructors and implicit cast operators in imconfig.h
                                 // to convert back and forth between your math types and ImPlotPoint.
#endif
};

// A range defined by a min/max value. Used for plot axes ranges.
struct ImPlotRange {
    double Min, Max;
    ImPlotRange();
    ImPlotRange(double _min, double _max) { Min = _min; Max = _max; }
    bool Contains(double value) const     { return value >= Min && value <= Max; };
    double Size() const                   { return Max - Min; };
};

// Combination of two ranges for X and Y axes.
struct ImPlotLimits {
    ImPlotRange X, Y;
    bool Contains(const ImPlotPoint& p) const { return Contains(p.x, p.y); }
    bool Contains(double x, double y) const   { return X.Contains(x) && Y.Contains(y); }
};

// Plot style structure
struct ImPlotStyle {
    // item styling variables
    float        LineWeight;              // = 1,      item line weight in pixels
    ImPlotMarker Marker;                  // = ImPlotMarker_None, marker specification
    float        MarkerSize;              // = 4,      marker size in pixels (roughly the marker's "radius")
    float        MarkerWeight;            // = 1,      outline weight of markers in pixels
    float        FillAlpha;               // = 1,      alpha modifier applied to plot fills
    float        ErrorBarSize;            // = 5,      error bar whisker width in pixels
    float        ErrorBarWeight;          // = 1.5,    error bar whisker weight in pixels
    float        DigitalBitHeight;        // = 8,      digital channels bit height (at y = 1.0f) in pixels
    float        DigitalBitGap;           // = 4,      digital channels bit padding gap in pixels
    bool         AntiAliasedLines;        // = false,  enable global anti-aliasing on plot lines (overrides ImPlotFlags_AntiAliased)
    // plot styling variables
    float        PlotBorderSize;          // = 1,      line thickness of border around plot area
    float        MinorAlpha;              // = 0.25    alpha multiplier applied to minor axis grid lines
    ImVec2       MajorTickLen;            // = 10,10   major tick lengths for X and Y axes
    ImVec2       MinorTickLen;            // = 5,5     minor tick lengths for X and Y axes
    ImVec2       MajorTickSize;           // = 1,1     line thickness of major ticks
    ImVec2       MinorTickSize;           // = 1,1     line thickness of minor ticks
    ImVec2       MajorGridSize;           // = 1,1     line thickness of major grid lines
    ImVec2       MinorGridSize;           // = 1,1     line thickness of minor grid lines
    ImVec2       PlotPadding;             // = 8,8     padding between widget frame and plot area and/or labels
    ImVec2       LabelPadding;            // = 5,5     padding between axes labels, tick labels, and plot edge
    ImVec2       LegendPadding;           // = 10,10   legend padding from top-left of plot
    ImVec2       InfoPadding;             // = 10,10   padding between plot edge and interior info text
    ImVec2       PlotMinSize;             // = 300,225 minimum size plot frame can be when shrunk
    ImVec4       Colors[ImPlotCol_COUNT]; //           array of plot specific colors
    ImPlotStyle();
};

// Input mapping structure, default values listed in the comments.
struct ImPlotInputMap {
    ImGuiMouseButton PanButton;             // LMB      enables panning when held
    ImGuiKeyModFlags PanMod;                // none     optional modifier that must be held for panning
    ImGuiMouseButton FitButton;             // LMB      fits visible data when double clicked
    ImGuiMouseButton ContextMenuButton;     // RMB      opens plot context menu (if enabled) when double clicked
    ImGuiMouseButton BoxSelectButton;       // RMB      begins box selection when pressed and confirms selection when released
    ImGuiKeyModFlags BoxSelectMod;          // none     optional modifier that must be held for box selection
    ImGuiMouseButton BoxSelectCancelButton; // LMB      cancels active box selection when pressed
    ImGuiMouseButton QueryButton;           // MMB      begins query selection when pressed and end query selection when released
    ImGuiKeyModFlags QueryMod;              // none     optional modifier that must be held for query selection
    ImGuiKeyModFlags QueryToggleMod;        // Ctrl     when held, active box selections turn into queries
    ImGuiKeyModFlags HorizontalMod;         // Alt      expands active box selection/query horizontally to plot edge when held
    ImGuiKeyModFlags VerticalMod;           // Shift    expands active box selection/query vertically to plot edge when held
    ImPlotInputMap();
};

//-----------------------------------------------------------------------------
// ImPlot End-User API
//-----------------------------------------------------------------------------

namespace ImPlot {

//-----------------------------------------------------------------------------
// ImPlot Context
//-----------------------------------------------------------------------------

// Creates a new ImPlot context. Call this after ImGui::CreateContext.
ImPlotContext* CreateContext();
// Destroys an ImPlot context. Call this before ImGui::DestroyContext. NULL = destroy current context
void DestroyContext(ImPlotContext* ctx = NULL);
// Returns the current context. NULL if not context has ben set.
ImPlotContext* GetCurrentContext();
// Sets the current context.
void SetCurrentContext(ImPlotContext* ctx);

//-----------------------------------------------------------------------------
// Begin/End Plot
//-----------------------------------------------------------------------------

// Starts a 2D plotting context. If this function returns true, EndPlot() must
// be called, e.g. "if (BeginPlot(...)) { ... EndPlot(); }". #title_id must
// be unique. If you need to avoid ID collisions or don't want to display a
// title in the plot, use double hashes (e.g. "MyPlot##Hidden"). If #x_label
// and/or #y_label are provided, axes labels will be displayed.
bool BeginPlot(const char* title_id,
               const char* x_label      = NULL,
               const char* y_label      = NULL,
               const ImVec2& size       = ImVec2(-1,0),
               ImPlotFlags flags        = ImPlotFlags_Default,
               ImPlotAxisFlags x_flags  = ImPlotAxisFlags_Default,
               ImPlotAxisFlags y_flags  = ImPlotAxisFlags_Default,
               ImPlotAxisFlags y2_flags = ImPlotAxisFlags_Auxiliary,
               ImPlotAxisFlags y3_flags = ImPlotAxisFlags_Auxiliary);

// Only call EndPlot() if BeginPlot() returns true! Typically called at the end
// of an if statement conditioned on BeginPlot().
void EndPlot();

//-----------------------------------------------------------------------------
// Plot Items
//-----------------------------------------------------------------------------

// Plots a standard 2D line plot.
void PlotLine(const char* label_id, const float* values, int count, int offset = 0, int stride = sizeof(float));
void PlotLine(const char* label_id, const double* values, int count, int offset = 0, int stride = sizeof(double));
void PlotLine(const char* label_id, const float* xs, const float* ys, int count, int offset = 0, int stride = sizeof(float));
void PlotLine(const char* label_id, const double* xs, const double* ys, int count, int offset = 0, int stride = sizeof(double));
void PlotLine(const char* label_id, const ImVec2* data, int count, int offset = 0);
void PlotLine(const char* label_id, const ImPlotPoint* data, int count, int offset = 0);
void PlotLine(const char* label_id, ImPlotPoint (*getter)(void* data, int idx), void* data, int count, int offset = 0);

// Plots a standard 2D scatter plot. Default marker is ImPlotMarker_Circle.
void PlotScatter(const char* label_id, const float* values, int count, int offset = 0, int stride = sizeof(float));
void PlotScatter(const char* label_id, const double* values, int count, int offset = 0, int stride = sizeof(double));
void PlotScatter(const char* label_id, const float* xs, const float* ys, int count, int offset = 0, int stride = sizeof(float));
void PlotScatter(const char* label_id, const double* xs, const double* ys, int count, int offset = 0, int stride = sizeof(double));
void PlotScatter(const char* label_id, const ImVec2* data, int count, int offset = 0);
void PlotScatter(const char* label_id, const ImPlotPoint* data, int count, int offset = 0);
void PlotScatter(const char* label_id, ImPlotPoint (*getter)(void* data, int idx), void* data, int count, int offset = 0);

// Plots a shaded (filled) region between two lines, or a line and a horizontal reference.
void PlotShaded(const char* label_id, const float* values, int count, float y_ref = 0, int offset = 0, int stride = sizeof(float));
void PlotShaded(const char* label_id, const double* values, int count, double y_ref = 0, int offset = 0, int stride = sizeof(double));
void PlotShaded(const char* label_id, const float* xs, const float* ys, int count, float y_ref = 0, int offset = 0, int stride = sizeof(float));
void PlotShaded(const char* label_id, const double* xs, const double* ys, int count, double y_ref = 0, int offset = 0, int stride = sizeof(double));
void PlotShaded(const char* label_id, const float* xs, const float* ys1, const float* ys2, int count, int offset = 0, int stride = sizeof(float));
void PlotShaded(const char* label_id, const double* xs, const double* ys1, const double* ys2, int count, int offset = 0, int stride = sizeof(double));
void PlotShaded(const char* label_id, ImPlotPoint (*getter1)(void* data, int idx), void* data1, ImPlotPoint (*getter2)(void* data, int idx), void* data2, int count, int offset = 0);

// Plots a vertical bar graph. #width and #shift are in X units.
void PlotBars(const char* label_id, const float* values, int count, float width = 0.67f, float shift = 0, int offset = 0, int stride = sizeof(float));
void PlotBars(const char* label_id, const double* values, int count, double width = 0.67f, double shift = 0, int offset = 0, int stride = sizeof(double));
void PlotBars(const char* label_id, const float* xs, const float* ys, int count, float width, int offset = 0, int stride = sizeof(float));
void PlotBars(const char* label_id, const double* xs, const double* ys, int count, double width, int offset = 0, int stride = sizeof(double));
void PlotBars(const char* label_id, ImPlotPoint (*getter)(void* data, int idx), void* data, int count, double width, int offset = 0);

// Plots a horizontal bar graph. #height and #shift are in Y units.
void PlotBarsH(const char* label_id, const float* values, int count, float height = 0.67f, float shift = 0, int offset = 0, int stride = sizeof(float));
void PlotBarsH(const char* label_id, const double* values, int count, double height = 0.67f, double shift = 0, int offset = 0, int stride = sizeof(double));
void PlotBarsH(const char* label_id, const float* xs, const float* ys, int count, float height,  int offset = 0, int stride = sizeof(float));
void PlotBarsH(const char* label_id, const double* xs, const double* ys, int count, double height,  int offset = 0, int stride = sizeof(double));
void PlotBarsH(const char* label_id, ImPlotPoint (*getter)(void* data, int idx), void* data, int count, double height,  int offset = 0);

// Plots vertical error bar. The label_id should be the same as the label_id of the associated line or bar plot.
void PlotErrorBars(const char* label_id, const float* xs, const float* ys, const float* err, int count, int offset = 0, int stride = sizeof(float));
void PlotErrorBars(const char* label_id, const double* xs, const double* ys, const double* err, int count, int offset = 0, int stride = sizeof(double));
void PlotErrorBars(const char* label_id, const float* xs, const float* ys, const float* neg, const float* pos, int count, int offset = 0, int stride = sizeof(float));
void PlotErrorBars(const char* label_id, const double* xs, const double* ys, const double* neg, const double* pos, int count, int offset = 0, int stride = sizeof(double));

// Plots horizontal error bars. The label_id should be the same as the label_id of the associated line or bar plot.
void PlotErrorBarsH(const char* label_id, const float* xs, const float* ys, const float* err, int count, int offset = 0, int stride = sizeof(float));
void PlotErrorBarsH(const char* label_id, const double* xs, const double* ys, const double* err, int count, int offset = 0, int stride = sizeof(double));
void PlotErrorBarsH(const char* label_id, const float* xs, const float* ys, const float* neg, const float* pos, int count, int offset = 0, int stride = sizeof(float));
void PlotErrorBarsH(const char* label_id, const double* xs, const double* ys, const double* neg, const double* pos, int count, int offset = 0, int stride = sizeof(double));

// Plots a pie chart. If the sum of values > 1 or normalize is true, each value will be normalized. Center and radius are in plot units. #label_fmt can be set to NULL for no labels.
void PlotPieChart(const char** label_ids, const float* values, int count, float x, float y, float radius, bool normalize = false, const char* label_fmt = "%.1f", float angle0 = 90);
void PlotPieChart(const char** label_ids, const double* values, int count, double x, double y, double radius, bool normalize = false, const char* label_fmt = "%.1f", double angle0 = 90);

// Plots a 2D heatmap chart. Values are expected to be in row-major order. #label_fmt can be set to NULL for no labels.
void PlotHeatmap(const char* label_id, const float* values, int rows, int cols, float scale_min, float scale_max, const char* label_fmt = "%.1f", const ImPlotPoint& bounds_min = ImPlotPoint(0,0), const ImPlotPoint& bounds_max = ImPlotPoint(1,1));
void PlotHeatmap(const char* label_id, const double* values, int rows, int cols, double scale_min, double scale_max, const char* label_fmt = "%.1f", const ImPlotPoint& bounds_min = ImPlotPoint(0,0), const ImPlotPoint& bounds_max = ImPlotPoint(1,1));

// Plots digital data. Digital plots do not respond to y drag or zoom, and are always referenced to the bottom of the plot.
void PlotDigital(const char* label_id, const float* xs, const float* ys, int count, int offset = 0, int stride = sizeof(float));
void PlotDigital(const char* label_id, const double* xs, const double* ys, int count, int offset = 0, int stride = sizeof(double));
void PlotDigital(const char* label_id, ImPlotPoint (*getter)(void* data, int idx), void* data, int count, int offset = 0);

// Plots a centered text label at point x,y with optional pixel offset. Text color can be changed with ImGui::PushStyleColor(ImGuiCol_Text, ...).
void PlotText(const char* text, float x, float y, bool vertical = false, const ImVec2& pixel_offset = ImVec2(0,0));
void PlotText(const char* text, double x, double y, bool vertical = false, const ImVec2& pixel_offset = ImVec2(0,0));

//-----------------------------------------------------------------------------
// Plot Utils
//-----------------------------------------------------------------------------

// Set the axes range limits of the next plot. Call right before BeginPlot(). If ImGuiCond_Always is used, the axes limits will be locked.
void SetNextPlotLimits(double x_min, double x_max, double y_min, double y_max, ImGuiCond cond = ImGuiCond_Once);
// Set the X axis range limits of the next plot. Call right before BeginPlot(). If ImGuiCond_Always is used, the X axis limits will be locked.
void SetNextPlotLimitsX(double x_min, double x_max, ImGuiCond cond = ImGuiCond_Once);
// Set the Y axis range limits of the next plot. Call right before BeginPlot(). If ImGuiCond_Always is used, the Y axis limits will be locked.
void SetNextPlotLimitsY(double y_min, double y_max, ImGuiCond cond = ImGuiCond_Once, int y_axis = 0);
// Fits the next plot axes to all plotted data if they are unlocked (equivalent to double-clicks).
void FitNextPlotAxes(bool x = true, bool y = true, bool y2 = true, bool y3 = true);

// Set the X axis ticks and optionally the labels for the next plot.
void SetNextPlotTicksX(const double* values, int n_ticks, const char** labels = NULL, bool show_default = false);
void SetNextPlotTicksX(double x_min, double x_max, int n_ticks, const char** labels = NULL, bool show_default = false);

// Set the Y axis ticks and optionally the labels for the next plot.
void SetNextPlotTicksY(const double* values, int n_ticks, const char** labels = NULL, bool show_default = false, int y_axis = 0);
void SetNextPlotTicksY(double y_min, double y_max, int n_ticks, const char** labels = NULL, bool show_default = false, int y_axis = 0);

// Select which Y axis will be used for subsequent plot elements. The default is '0', or the first (left) Y axis. Enable 2nd and 3rd axes with ImPlotFlags_YAxisX.
void SetPlotYAxis(int y_axis);

// Convert pixels to a position in the current plot's coordinate system. A negative y_axis uses the current value of SetPlotYAxis (0 initially).
ImPlotPoint PixelsToPlot(const ImVec2& pix, int y_axis = -1);
ImPlotPoint PixelsToPlot(float x, float y, int y_axis = -1);

// Convert a position in the current plot's coordinate system to pixels. A negative y_axis uses the current value of SetPlotYAxis (0 initially).
ImVec2 PlotToPixels(const ImPlotPoint& plt, int y_axis = -1);
ImVec2 PlotToPixels(double x, double y, int y_axis = -1);

//-----------------------------------------------------------------------------
// Plot Queries
//-----------------------------------------------------------------------------

// Get the current Plot position (top-left) in pixels.
ImVec2 GetPlotPos();
// Get the curent Plot size in pixels.
ImVec2 GetPlotSize();
// Returns true if the plot area in the current plot is hovered.
bool IsPlotHovered();
// Returns true if the XAxis plot area in the current plot is hovered.
bool IsPlotXAxisHovered();
// Returns true if the YAxis[n] plot area in the current plot is hovered.
bool IsPlotYAxisHovered(int y_axis = 0);
// Returns the mouse position in x,y coordinates of the current plot. A negative y_axis uses the current value of SetPlotYAxis (0 initially).
ImPlotPoint GetPlotMousePos(int y_axis = -1);
// Returns the current plot axis range. A negative y_axis uses the current value of SetPlotYAxis (0 initially).
ImPlotLimits GetPlotLimits(int y_axis = -1);
// Returns true if the current plot is being queried.
bool IsPlotQueried();
// Returns the current plot query bounds.
ImPlotLimits GetPlotQuery(int y_axis = -1);
// Returns true if a plot item legend entry is hovered.
bool IsLegendEntryHovered(const char* label_id);

//-----------------------------------------------------------------------------
// Plot and Item Styling and Colormaps
//-----------------------------------------------------------------------------

// Provides access to plot style structure for permanant modifications to colors, sizes, etc.
ImPlotStyle& GetStyle();

// Style colors for current ImGui style (default).
void StyleColorsAuto(ImPlotStyle* dst = NULL);
// Style colors for ImGui "Classic".
void StyleColorsClassic(ImPlotStyle* dst = NULL);
// Style colors for ImGui "Dark".
void StyleColorsDark(ImPlotStyle* dst = NULL);
// Style colors for ImGui "Light".
void StyleColorsLight(ImPlotStyle* dst = NULL);

// Special color used to indicate that a style color should be deduced automatically from ImGui style or ImPlot colormaps.
#define IMPLOT_COL_AUTO ImVec4(0,0,0,-1)

// Temporarily modify a plot color. Don't forget to call PopStyleColor!
void PushStyleColor(ImPlotCol idx, ImU32 col);
// Temporarily modify a plot color. Don't forget to call PopStyleColor!
void PushStyleColor(ImPlotCol idx, const ImVec4& col);
// Undo temporary color modification.
void PopStyleColor(int count = 1);

// Temporarily modify a style variable of float type. Don't forget to call PopStyleVar!
void PushStyleVar(ImPlotStyleVar idx, float val);
// Temporarily modify a style variable of int type. Don't forget to call PopStyleVar!
void PushStyleVar(ImPlotStyleVar idx, int val);
// Temporarily modify a style variable of ImVec2 type. Don't forget to call PopStyleVar!
void PushStyleVar(ImPlotStyleVar idx, const ImVec2& val);
// Undo temporary style modification.
void PopStyleVar(int count = 1);

// Temporarily switch to one of the built-in colormaps.
void PushColormap(ImPlotColormap colormap);
// Temporarily switch to your custom colormap. The pointer data must persist until the matching call to PopColormap!
void PushColormap(const ImVec4* colormap, int size);
// Undo temporary colormap modification.
void PopColormap(int count = 1);
// Permanently sets a custom colormap. The colors will be copied to internal memory. Prefer PushColormap instead of calling this each frame.
void SetColormap(const ImVec4* colormap, int size);
// Permanently switch to one of the built-in colormaps. If samples is greater than 1, the map will be linearly resampled. Don't call this each frame.
void SetColormap(ImPlotColormap colormap, int samples = 0);

// Returns the size of the current colormap.
int GetColormapSize();
// Returns a color from the Color map given an index >= 0 (modulo will be performed)
ImVec4 GetColormapColor(int index);
// Linearly interpolates a color from the current colormap given t between 0 and 1.
ImVec4 LerpColormap(float t);
// Returns the next unused colormap color and advances the colormap. Can be used to skip colors if desired.
ImVec4 NextColormapColor();

const char* GetStyleColorName(ImPlotCol color);
// Returns a null terminated string name for a built-in colormap
const char* GetColormapName(ImPlotColormap colormap);

// Renders a vertical color scale using the current color map.
void ShowColormapScale(double scale_min, double scale_max, float height);

//-----------------------------------------------------------------------------
// Miscellaneous
//-----------------------------------------------------------------------------

// Allows changing how keyboard/mouse interaction works.
ImPlotInputMap& GetInputMap();

// Shows ImPlot style selector dropdown menu.
bool ShowStyleSelector(const char* label);
// Shows ImPlot style editor block (not a window).
void ShowStyleEditor(ImPlotStyle* ref = NULL);
// Add basic help/info block (not a window): how to manipulate ImPlot as an end-user.
void ShowUserGuide();

// Push clip rect for rendering to current plot area.
void PushPlotClipRect();
// Pop plot clip rect.
void PopPlotClipRect();

//-----------------------------------------------------------------------------
// Demo (add implot_demo.cpp to your sources!)
//-----------------------------------------------------------------------------

// Shows the ImPlot demo.
void ShowDemoWindow(bool* p_open = NULL);

}  // namespace ImPlot
