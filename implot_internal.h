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

// You may use this file to debug, understand or extend ImPlot features but we 
// don't provide any guarantee of forward compatibility!

//-----------------------------------------------------------------------------
// [SECTION] Header mess
//-----------------------------------------------------------------------------

#pragma once

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "imgui_internal.h"

#ifndef IMPLOT_VERSION
#error Must include implot.h before implot_internal.h
#endif

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations
//-----------------------------------------------------------------------------

struct ImPlotTick;
struct ImPlotAxis;
struct ImPlotAxisState;
struct ImPlotAxisColor;
struct ImPlotItem;
struct ImPlotState;
struct ImPlotNextPlotData;

//-----------------------------------------------------------------------------
// [SECTION] Context pointer
//-----------------------------------------------------------------------------

extern ImPlotContext* GImPlot; // Current implicit context pointer

//-----------------------------------------------------------------------------
// [SECTION] Macros
//-----------------------------------------------------------------------------

// The maximum number of supported y-axes (DO NOT CHANGE THIS)
#define MAX_Y_AXES 3

//-----------------------------------------------------------------------------
// [SECTION] Generic helpers
//-----------------------------------------------------------------------------

// Computes the common (base-10) logarithm
static inline float  ImLog10(float x)  { return log10f(x); }
static inline double ImLog10(double x) { return log10(x);  }

// Returns true if a flag is set
template <typename TSet, typename TFlag>
inline bool ImHasFlag(TSet set, TFlag flag) { return (set & flag) == flag; }

// Flips a flag in a flagset
template <typename TSet, typename TFlag>
inline void ImFlipFlag(TSet& set, TFlag flag) { ImHasFlag(set, flag) ? set &= ~flag : set |= flag; }

// Linearly remaps x from [x0 x1] to [y0 y1].
template <typename T>
inline T ImRemap(T x, T x0, T x1, T y0, T y1) { return y0 + (x - x0) * (y1 - y0) / (x1 - x0); }

// Returns always positive modulo (assumes r != 0)
inline int ImPosMod(int l, int r) { return (l % r + r) % r; }

// Offset calculator helper
template <int Count>
struct ImOffsetCalculator {
    ImOffsetCalculator(int* sizes) {
        Offsets[0] = 0;
        for (int i = 1; i < Count; ++i)
            Offsets[i] = Offsets[i-1] + sizes[i-1];
    }
    int Offsets[Count];
};

// Character buffer writer helper
struct ImBufferWriter
{
    char*  Buffer;
    size_t Size;
    size_t Pos;

    ImBufferWriter(char* buffer, size_t size) {
        Buffer = buffer;
        Size = size;
        Pos = 0;
    }

    void Write(const char* fmt, ...) IM_FMTARGS(2) {
        va_list argp;
        va_start(argp, fmt);
        const int written = ::vsnprintf(&Buffer[Pos], Size - Pos - 1, fmt, argp);
        if (written > 0)
          Pos += ImMin(size_t(written), Size-Pos-1);
        va_end(argp);
    }
};

// Fixed size array
template <typename T, int N>
struct ImArray {
    inline T&           operator[](int i)       { return Data[i]; }
    inline const T&     operator[](int i) const { return Data[i]; }
    T Data[N];
    const int Size = N;
};

//-----------------------------------------------------------------------------
// [SECTION] ImPlot Structs
//-----------------------------------------------------------------------------

// ImPlotPoint with positive/negative error values
struct ImPlotPointError 
{
    double X, Y, Neg, Pos;

    ImPlotPointError(double x, double y, double neg, double pos) {
        X = x; Y = y; Neg = neg; Pos = pos;
    }
};

// Tick mark info
struct ImPlotTick
{
    double PlotPos;
    float  PixelPos;
    ImVec2 Size;
    int    TextOffset;
    bool   Major;
    bool   RenderLabel;
    bool   Labeled;

    ImPlotTick(double value, bool major, bool render_label = true) {
        PlotPos     = value;
        Major       = major;
        RenderLabel = render_label;
        Labeled     = false;
    }
};

// Axis state information that must persist after EndPlot
struct ImPlotAxis
{
    ImPlotAxisFlags Flags;
    ImPlotAxisFlags PreviousFlags;
    ImPlotRange     Range;
    bool            Dragging;
    bool            Hovered;

    ImPlotAxis() {
        Flags     = PreviousFlags = ImPlotAxisFlags_Default;
        Range.Min = 0;
        Range.Max = 1;
        Dragging  = false;
        Hovered   = false;
    }
};

// Axis state information only needed between BeginPlot/EndPlot
struct ImPlotAxisState
{
    ImPlotAxis* Axis;
    ImGuiCond   RangeCond;
    int         PresentSoFar;
    bool        HasRange;
    bool        Present;
    bool        Invert;
    bool        LockMin;
    bool        LockMax;
    bool        Lock;

    ImPlotAxisState(ImPlotAxis* axis, bool has_range, ImGuiCond range_cond, bool present, int previous_present) {
        Axis         = axis;
        HasRange     = has_range;
        RangeCond    = range_cond;
        Present      = present;
        PresentSoFar = previous_present + (Present ? 1 : 0);
        Invert       = ImHasFlag(Axis->Flags, ImPlotAxisFlags_Invert);
        LockMin      = ImHasFlag(Axis->Flags, ImPlotAxisFlags_LockMin) || (HasRange && RangeCond == ImGuiCond_Always);
        LockMax      = ImHasFlag(Axis->Flags, ImPlotAxisFlags_LockMax) || (HasRange && RangeCond == ImGuiCond_Always);
        Lock         = !Present || ((LockMin && LockMax) || (HasRange && RangeCond == ImGuiCond_Always));
    }

    ImPlotAxisState() { }
};

struct ImPlotAxisColor
{
    ImU32 Major, Minor, Txt;
    ImPlotAxisColor() { Major = Minor = Txt = 0; }
};

// State information for Plot items
struct ImPlotItem
{
    ImGuiID ID;
    ImVec4  Color;
    bool    Show;
    bool    Highlight;
    bool    SeenThisFrame;
    int     NameOffset;

    ImPlotItem() {
        ID            = 0;
        Color         = ImPlot::NextColormapColor();
        Show          = true;
        SeenThisFrame = false;
        Highlight     = false;
        NameOffset    = -1;
    }

    ~ImPlotItem() { ID = 0; }
};

// Holds Plot state information that must persist after EndPlot
struct ImPlotState
{
    ImPlotFlags        Flags;
    ImPlotFlags        PreviousFlags;
    ImPlotAxis         XAxis;
    ImPlotAxis         YAxis[MAX_Y_AXES];
    ImPool<ImPlotItem> Items;
    ImVec2             SelectStart;
    ImVec2             QueryStart;
    ImRect             QueryRect;
    ImRect             BB_Legend;
    bool               Selecting;
    bool               Querying;
    bool               Queried;
    bool               DraggingQuery;
    int                ColormapIdx;
    int                CurrentYAxis;

    ImPlotState() {
        Flags        = PreviousFlags = ImPlotFlags_Default;
        SelectStart  = QueryStart = ImVec2(0,0);
        Selecting    = Querying = Queried = DraggingQuery = false;
        ColormapIdx  = CurrentYAxis = 0;
    }
};

// Temporary data storage for upcoming plot
struct ImPlotNextPlotData
{
    ImGuiCond   XRangeCond;
    ImGuiCond   YRangeCond[MAX_Y_AXES];
    ImPlotRange X;
    ImPlotRange Y[MAX_Y_AXES];
    bool        HasXRange;
    bool        HasYRange[MAX_Y_AXES];
    bool        ShowDefaultTicksX;
    bool        ShowDefaultTicksY[MAX_Y_AXES];
    bool        DragDisable;

    ImPlotNextPlotData() {
        HasXRange         = false;
        ShowDefaultTicksX = true;
        for (int i = 0; i < MAX_Y_AXES; ++i) {
            HasYRange[i]         = false;
            ShowDefaultTicksY[i] = true;
        }
        DragDisable = false;
    }
};

// Holds state information that must persist between calls to BeginPlot()/EndPlot()
struct ImPlotContext {
    // Plot States
    ImPool<ImPlotState> Plots;
    ImPlotState*        CurrentPlot;

    // Legend
    ImVector<int>   LegendIndices;
    ImGuiTextBuffer LegendLabels;

    // Bounding Boxes
    ImRect BB_Frame;
    ImRect BB_Canvas;
    ImRect BB_Plot;

    // Cached Colors
    ImU32 Col_Frame;
    ImU32 Col_Bg;
    ImU32 Col_Border;
    ImU32 Col_Txt;
    ImU32 Col_TxtDis;
    ImU32 Col_SlctBg;
    ImU32 Col_SlctBd;
    ImU32 Col_QryBg;
    ImU32 Col_QryBd;

    // Axis States
    ImPlotAxisColor Col_X;
    ImPlotAxisColor Col_Y[MAX_Y_AXES];
    ImPlotAxisState X;
    ImPlotAxisState Y[MAX_Y_AXES];

    // Tick Marks and Labels
    ImVector<ImPlotTick> XTicks;
    ImVector<ImPlotTick> YTicks[MAX_Y_AXES];
    ImGuiTextBuffer      XTickLabels;
    ImGuiTextBuffer      YTickLabels[MAX_Y_AXES];
    float                AxisLabelReference[MAX_Y_AXES];

    // Transformations and Data Extents
    ImRect      PixelRange[MAX_Y_AXES];
    double      Mx;
    double      My[MAX_Y_AXES];
    double      LogDenX;
    double      LogDenY[MAX_Y_AXES];
    ImPlotRange ExtentsX;
    ImPlotRange ExtentsY[MAX_Y_AXES];

    // Data Fitting Flags
    bool FitThisFrame;
    bool FitX;
    bool FitY[MAX_Y_AXES];

    // Hover states
    bool Hov_Frame;
    bool Hov_Plot;

    // Axis Rendering Flags
    bool RenderX;
    bool RenderY[MAX_Y_AXES];

    // Axis Locking Flags
    bool LockPlot;
    bool ChildWindowMade;

    // Style and Colormaps
    ImPlotStyle             Style;
    ImVector<ImGuiColorMod> ColorModifiers;
    ImVector<ImGuiStyleMod> StyleModifiers;
    ImVec4*                 Colormap;
    int                     ColormapSize;

    // Misc
    int                VisibleItemCount;
    int                DigitalPlotItemCnt;
    int                DigitalPlotOffset;
    ImPlotNextPlotData NextPlotData;
    ImPlotInputMap     InputMap;
    ImPlotPoint        LastMousePos[MAX_Y_AXES];
};

struct ImPlotAxisScale 
{
    ImPlotPoint Min, Max;

    ImPlotAxisScale(int y_axis, float tx, float ty, float zoom_rate) {
        ImPlotContext& gp = *GImPlot;
        Min = ImPlot::PixelsToPlot(gp.BB_Plot.Min - gp.BB_Plot.GetSize() * ImVec2(tx * zoom_rate, ty * zoom_rate), y_axis);
        Max = ImPlot::PixelsToPlot(gp.BB_Plot.Max + gp.BB_Plot.GetSize() * ImVec2((1 - tx) * zoom_rate, (1 - ty) * zoom_rate), y_axis);
    }
};

//-----------------------------------------------------------------------------
// [SECTION] Internal API
// No guarantee of forward compatibility here!
//-----------------------------------------------------------------------------

namespace ImPlot {

// Initializes an ImPlotContext
void Initialize(ImPlotContext* ctx);
// Resets an ImPlot context for the next call to BeginPlot
void Reset(ImPlotContext* ctx);

// Gets a plot from the current ImPlotContext
ImPlotState* GetPlot(const char* title);
// Gets the current plot from the current ImPlotContext
ImPlotState* GetCurrentPlot();

// Updates plot-to-pixel space transformation variables for the current plot
void UpdateTransformCache();
// Extends the current plots axes so that it encompasses point p
void FitPoint(const ImPlotPoint& p);

// Register or get an existing item from the current plot
ImPlotItem* RegisterOrGetItem(const char* label_id);
// Get the ith plot item from the current plot
ImPlotItem* GetItem(int i);
// Get a plot item from the current plot
ImPlotItem* GetItem(const char* label_id);
// Gets a plot item from a specific plot
ImPlotItem* GetItem(const char* plot_title, const char* item_label_id);

// Returns the number of entries in the current legend
int GetLegendCount();
// Gets the ith entry string for the current legend
const char* GetLegendLabel(int i);

// Populates a list of ImPlotTicks with automatically spaced ticks
void AddDefaultTicks(const ImPlotRange& range, int nMajor, int nMinor, bool logscale, ImVector<ImPlotTick> &out);
// Populates a list of ImPlotTicks with custom spaced and labeled ticks
void AddCustomTicks(const double* values, const char** labels, int n, ImVector<ImPlotTick>& ticks, ImGuiTextBuffer& buffer);
// Creates label information for a list of ImPlotTick
void LabelTicks(ImVector<ImPlotTick> &ticks, bool scientific, ImGuiTextBuffer& buffer);
// Calculates the maximum width of a list of ImPlotTick
float MaxTickLabelWidth(ImVector<ImPlotTick>& ticks);
// Rounds x to powers of 2,5 and 10 for generating axis labels (from Graphics Gems 1 Chapter 11.2)
double NiceNum(double x, bool round);

// Updates axis ticks, lins, and label colors
void UpdateAxisColor(int axis_flag, ImPlotAxisColor* col);
// Sets the colormap for a particular ImPlotContext
void SetColormapEx(ImPlotColormap colormap, int samples, ImPlotContext* ctx);
void SetColormapEx(const ImVec4* colors, int num_colors, ImPlotContext* ctx);

// Draws vertical text. The position is the bottom left of the text rect.
void AddTextVertical(ImDrawList *DrawList, const char *text, ImVec2 pos, ImU32 text_color);
// Calculates the size of vertical text
ImVec2 CalcTextSizeVertical(const char *text);
// Returns white or black text given background color
inline ImU32 CalcTextColor(const ImVec4& bg) { return (bg.x * 0.299 + bg.y * 0.587 + bg.z * 0.114) > 0.729 ? IM_COL32_BLACK : IM_COL32_WHITE; }

// Returns true if val is NAN or INFINITY
inline bool NanOrInf(double val) { return val == HUGE_VAL || val == -HUGE_VAL || isnan(val); }
// Turns NANs to 0s
inline double ConstrainNan(double val) { return isnan(val) ? 0 : val; }
// Turns infinity to floating point maximums
inline double ConstrainInf(double val) { return val == HUGE_VAL ?  DBL_MAX : val == -HUGE_VAL ? - DBL_MAX : val; }
// Turns numbers less than or equal to 0 to 0.001 (sort of arbitrary, is there a better way?)
inline double ConstrainLog(double val) { return val <= 0 ? 0.001f : val; }

// Computes order of magnitude of double.
inline int OrderOfMagnitude(double val) { return val == 0 ? 0 : (int)(floor(log10(fabs(val)))); }
// Returns the precision required for a order of magnitude.
inline int OrderToPrecision(int order) { return order > 0 ? 0 : 1 - order; }
// Returns a floating point precision to use given a value
inline int Precision(double val) { return OrderToPrecision(OrderOfMagnitude(val)); }

// Returns the intersection point of two lines A and B (assumes they are not parallel!)
inline ImVec2 Intersection(const ImVec2& a1, const ImVec2& a2, const ImVec2& b1, const ImVec2& b2) {
    float v1 = (a1.x * a2.y - a1.y * a2.x);
    float v2 = (b1.x * b2.y - b1.y * b2.x);
    float v3 = ((a1.x - a2.x) * (b1.y - b2.y) - (a1.y - a2.y) * (b1.x - b2.x));
    return ImVec2((v1 * (b1.x - b2.x) - v2 * (a1.x - a2.x)) / v3, (v1 * (b1.y - b2.y) - v2 * (a1.y - a2.y)) / v3);
}

// Fills a buffer with n samples linear interpolated from vmin to vmax
template <typename T>
void FillRange(ImVector<T>& buffer, int n, T vmin, T vmax) {
    buffer.resize(n);
    T step = (vmax - vmin) / (n - 1);
    for (int i = 0; i < n; ++i) {
        buffer[i] = vmin + i * step;
    }
}

// Offsets and strides a data buffer
template <typename T>
inline T OffsetAndStride(const T* data, int idx, int count, int offset, int stride) {
    idx = ImPosMod(offset + idx, count);
    return *(const T*)(const void*)((const unsigned char*)data + (size_t)idx * stride);
}

// Returns true if a style color is set to be automaticaly determined
inline bool ColorIsAuto(ImPlotCol idx) {
    return GImPlot->Style.Colors[idx].w == -1;
}

// Recolors an item legend icon from an the current ImPlotCol if it is not automatic (i.e. alpha != -1)
inline void TryRecolorItem(ImPlotItem* item, ImPlotCol idx) {
    if (GImPlot->Style.Colors[idx].w != -1)
        item->Color = GImPlot->Style.Colors[idx];
}

// Returns true if lines will render (e.g. basic lines, bar outlines)
inline bool WillLineRender() {
    return GImPlot->Style.Colors[ImPlotCol_Line].w != 0 && GImPlot->Style.LineWeight > 0;
}

// Returns true if fills will render (e.g. shaded plots, bar fills)
inline bool WillFillRender() {
    return GImPlot->Style.Colors[ImPlotCol_Fill].w != 0 && GImPlot->Style.FillAlpha > 0;
}

// Returns true if marker outlines will render
inline bool WillMarkerOutlineRender() {
    return GImPlot->Style.Colors[ImPlotCol_MarkerOutline].w != 0 && GImPlot->Style.MarkerWeight > 0;
}

// Returns true if mark fill will render
inline bool WillMarkerFillRender() {
    return GImPlot->Style.Colors[ImPlotCol_MarkerFill].w != 0 && GImPlot->Style.FillAlpha > 0;
}

// Gets the line color for an item
inline ImVec4 GetLineColor(ImPlotItem* item) {
    return ColorIsAuto(ImPlotCol_Line) ? item->Color : GImPlot->Style.Colors[ImPlotCol_Line];
}

// Gets the fill color for an item
inline ImVec4 GetItemFillColor(ImPlotItem* item) {
    ImVec4 col = ColorIsAuto(ImPlotCol_Fill) ? item->Color : GImPlot->Style.Colors[ImPlotCol_Fill];
    col.w *= GImPlot->Style.FillAlpha;
    return col;
}

// Gets the marker outline color for an item
inline ImVec4 GetMarkerOutlineColor(ImPlotItem* item) {
    return ColorIsAuto(ImPlotCol_MarkerOutline) ? GetLineColor(item) : GImPlot->Style.Colors[ImPlotCol_MarkerOutline];
}

// Gets the marker fill color for an item
inline ImVec4 GetMarkerFillColor(ImPlotItem* item) {
    ImVec4 col = ColorIsAuto(ImPlotCol_MarkerFill) ?  GetLineColor(item) :GImPlot->Style.Colors[ImPlotCol_MarkerFill];
    col.w *= GImPlot->Style.FillAlpha;
    return col;
}

// Gets the error bar color
inline ImVec4 GetErrorBarColor() {
    return ColorIsAuto(ImPlotCol_ErrorBar) ? ImGui::GetStyleColorVec4(ImGuiCol_Text) : GImPlot->Style.Colors[ImPlotCol_ErrorBar];
}

//-----------------------------------------------------------------------------
// [SECTION] Internal / Experimental Plotters
// No guarantee of forward compatibility here!
//-----------------------------------------------------------------------------

// Plots axis-aligned, filled rectangles. Every two consecutive points defines opposite corners of a single rectangle.
void PlotRects(const char* label_id, const float* xs, const float* ys, int count, int offset = 0, int stride = sizeof(float));
void PlotRects(const char* label_id, const double* xs, const double* ys, int count, int offset = 0, int stride = sizeof(double));
void PlotRects(const char* label_id, ImPlotPoint (*getter)(void* data, int idx), void* data, int count, int offset = 0);

} // namespace ImPlot
