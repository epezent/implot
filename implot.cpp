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

// ImPlot v0.1 WIP

#ifdef _MSC_VER
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include <implot.h>
#include <imgui_internal.h>

#define IM_NORMALIZE2F_OVER_ZERO(VX, VY)                                                           \
    {                                                                                              \
        float d2 = VX * VX + VY * VY;                                                              \
        if (d2 > 0.0f) {                                                                           \
            float inv_len = 1.0f / ImSqrt(d2);                                                     \
            VX *= inv_len;                                                                         \
            VY *= inv_len;                                                                         \
        }                                                                                          \
    }

// Special Color used to specific that a plot item color should set determined automatically.
#define IM_COL_AUTO ImVec4(0,0,0,-1)

ImPlotStyle::ImPlotStyle() {
    LineWeight = 1;
    Marker = ImMarker_None;
    MarkerSize = 5;
    MarkerWeight = 1;
    ErrorBarSize = 5;
    ErrorBarWeight = 1.5;
    DigitalBitHeight = 8;

    Colors[ImPlotCol_Line]          = IM_COL_AUTO;
    Colors[ImPlotCol_Fill]          = IM_COL_AUTO;
    Colors[ImPlotCol_MarkerOutline] = IM_COL_AUTO;
    Colors[ImPlotCol_MarkerFill]    = IM_COL_AUTO;
    Colors[ImPlotCol_ErrorBar]      = IM_COL_AUTO;
    Colors[ImPlotCol_FrameBg]       = IM_COL_AUTO;
    Colors[ImPlotCol_PlotBg]        = IM_COL_AUTO;
    Colors[ImPlotCol_PlotBorder]    = IM_COL_AUTO;
    Colors[ImPlotCol_XAxis]         = IM_COL_AUTO;
    Colors[ImPlotCol_YAxis]         = IM_COL_AUTO;
    Colors[ImPlotCol_Selection]     = ImVec4(1,1,0,1);
    Colors[ImPlotCol_Query]         = ImVec4(0,1,0,1);
}

ImPlotRange::ImPlotRange() {
    XMin = XMax = YMin = YMax = NAN;
}

bool ImPlotRange::Contains(const ImVec2& p) {
    return p.x >= XMin && p.x <= XMax && p.y >= YMin && p.y <= YMax;
}

namespace ImGui {

namespace {

//-----------------------------------------------------------------------------
// Private Utils
//-----------------------------------------------------------------------------

/// Returns true if a flag is set
template <typename TSet, typename TFlag>
inline bool HasFlag(TSet set, TFlag flag) {
    return (set & flag) == flag;
}

/// Flips a flag in a flagset
template <typename TSet, typename TFlag> 
inline void FlipFlag(TSet& set, TFlag flag) {
    HasFlag(set, flag) ? set &= ~flag : set |= flag;
}

/// Linearly remaps float x from [x0 x1] to [y0 y1].
inline float Remap(float x, float x0, float x1, float y0, float y1) {
    return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
}

/// Turns NANs to 0s
inline float ConstrainNan(float val) {
    return val == NAN || val == -NAN ? 0 : val;
}

/// Turns INFINITYs to FLT_MAXs
inline float ConstrainInf(float val) {
    return val == INFINITY ? FLT_MAX : val == -INFINITY ? -FLT_MAX : val;
}

/// Turns numbers less than or equal to 0 to 0.001 (sort of arbitrary, is there a better way?)
inline float ConstrainLog(float val) {
    return val <= 0 ? 0.001f : val;
}

/// Returns true if val is NAN or INFINITY
inline bool NanOrInf(float val) {
    return val == INFINITY || val == NAN || val == -INFINITY  || val == -NAN;
}

/// Utility function to that rounds x to powers of 2,5 and 10 for generating axis labels
/// Taken from Graphics Gems 1 Chapter 11.2, "Nice Numbers for Graph Labels"
inline double NiceNum(double x, bool round) {
    double f;  /* fractional part of x */
    double nf; /* nice, rounded fraction */
    int expv = (int)floor(log10(x));
    f = x / ImPow(10.0, (double)expv); /* between 1 and 10 */
    if (round)
        if (f < 1.5)
            nf = 1;
        else if (f < 3)
            nf = 2;
        else if (f < 7)
            nf = 5;
        else
            nf = 10;
    else if (f <= 1)
        nf = 1;
    else if (f <= 2)
        nf = 2;
    else if (f <= 5)
        nf = 5;
    else
        nf = 10;
    return nf * ImPow(10.0, expv);
}

/// Draws vertical text. The position is the bottom left of the text rect.
inline void AddTextVertical(ImDrawList *DrawList, const char *text, ImVec2 pos, ImU32 text_color) {
    pos.x                   = IM_ROUND(pos.x);
    pos.y                   = IM_ROUND(pos.y);
    ImFont *           font = GImGui->Font;
    const ImFontGlyph *glyph;
    char               c;
    while ((c = *text++)) {
        glyph = font->FindGlyph(c);
        if (!glyph)
            continue;

        DrawList->PrimReserve(6, 4);
        DrawList->PrimQuadUV(
            pos + ImVec2(glyph->Y0, -glyph->X0), pos + ImVec2(glyph->Y0, -glyph->X1),
            pos + ImVec2(glyph->Y1, -glyph->X1), pos + ImVec2(glyph->Y1, -glyph->X0),

            ImVec2(glyph->U0, glyph->V0), ImVec2(glyph->U1, glyph->V0),
            ImVec2(glyph->U1, glyph->V1), ImVec2(glyph->U0, glyph->V1), text_color);
        pos.y -= glyph->AdvanceX;
    }
}

/// Calculates the size of vertical text
inline ImVec2 CalcTextSizeVertical(const char *text) {
    ImVec2 sz = CalcTextSize(text);
    return ImVec2(sz.y, sz.x);
}

} // private namespace

//-----------------------------------------------------------------------------
// Forwards
//-----------------------------------------------------------------------------

ImVec4 NextColor();

//-----------------------------------------------------------------------------
// Structs
//-----------------------------------------------------------------------------


/// Tick mark info
struct ImTick {
    ImTick(double value, bool major, bool render_label = true) { 
        PlotPos = value;
        Major = major;
        RenderLabel = render_label;
    }
    double PlotPos;
    float  PixelPos;
    bool   Major;
    ImVec2 Size;
    int    TextOffset;
    bool   RenderLabel;
};

struct ImPlotItem {
    ImPlotItem() {
        Show = true; 
        Highlight = false;
        Color = NextColor(); 
        NameOffset = -1; 
        ID = 0;  
    }
    ~ImPlotItem() { ID = 0; }
    bool Show;
    bool Highlight;
    ImVec4 Color;
    int NameOffset;
    ImGuiID ID;
};

/// Plot axis structure. You shouldn't need to construct this!
struct ImPlotAxis {
    ImPlotAxis() { 
        Dragging = false;
        Min = 0; 
        Max = 1; 
        Divisions = 3; 
        Subdivisions = 10; 
        Flags = ImAxisFlags_Default; 
    }
    float Range() { return Max - Min; }
    bool Dragging;
    float Min;
    float Max;
    int Divisions;
    int Subdivisions;
    ImAxisFlags Flags;
};

/// Holds Plot state information that must persist between frames
struct ImPlot {
    ImPlot() {
        Selecting = Querying = Queried = DraggingQuery = false;
        SelectStart =  QueryStart = ImVec2(0,0);
        Flags = ImPlotFlags_Default;
        ColorIdx = 0;
    }
    ImPool<ImPlotItem> Items;

    ImRect BB_Legend;
    bool Selecting;
    ImVec2 SelectStart;
    bool Querying;
    bool Queried;
    ImVec2 QueryStart;
    ImRect QueryRect; // relative to BB_grid!!
    bool DraggingQuery;
    ImPlotRange QueryRange;

    ImPlotAxis XAxis;
    ImPlotAxis YAxis;    
    inline ImPlotAxis& Axis(int idx) { return (&XAxis)[idx]; } 

    ImPlotFlags Flags;
    int ColorIdx;
};

struct ImNextPlotData {
    ImNextPlotData() { 
        HasXRange = false;
        HasYRange = false;
    }
    ImGuiCond XRangeCond;
    ImGuiCond YRangeCond;
    bool HasXRange;
    bool HasYRange;
    float XMin, XMax, YMin, YMax;
};

/// Holds Plot state information that must persist only between calls to BeginPlot()/EndPlot()
struct ImPlotContext {
    ImPlotContext() {
        CurrentPlot = NULL;
        FitThisFrame = FitX = FitY = false;
        RestorePlotPalette();
    }

    /// ALl Plots    
    ImPool<ImPlot> Plots;
    /// Current Plot
    ImPlot* CurrentPlot;
    // Legend
    ImVector<int> LegendIndices;    
    ImGuiTextBuffer LegendLabels;
    // Bounding regions    
    ImRect BB_Frame;
    ImRect BB_Canvas;
    ImRect BB_Grid;
    // Hover states
    bool Hov_Frame;
    bool Hov_Grid;
    // Cached Colors
    ImU32 Col_Frame, Col_Bg, Col_Border, 
          Col_Txt, Col_TxtDis, 
          Col_SlctBg, Col_SlctBd,
          Col_QryBg, Col_QryBd,
          Col_XMajor, Col_XMinor, Col_XTxt,
          Col_YMajor, Col_YMinor, Col_YTxt;  
    // Tick marks  
    ImVector<ImTick> XTicks,  YTicks;
    ImGuiTextBuffer XTickLabels, YTickLabels;
    // Transformation cache
    ImRect PixelRange;
    ImVec2 M;          // linear scale (slope)
    ImVec2 LogDen;     // log scale denominator

    // Data extents
    ImRect Extents;
    bool FitThisFrame; bool FitX; bool FitY;
    int VisibleItemCount;
    // Render flags
    bool RenderX, RenderY;
    // Mouse pos
    ImVec2 LastMousePos;
    // Style
    ImVector<ImVec4> ColorMap;
    ImPlotStyle Style;
    ImVector<ImGuiColorMod> ColorModifiers;  // Stack for PushStyleColor()/PopStyleColor()
    ImVector<ImGuiStyleMod> StyleModifiers;  // Stack for PushStyleVar()/PopStyleVar()
    ImNextPlotData NextPlotData;        
    // Digital plot item count
    int DigitalPlotItemCnt;
};

/// Global plot context
static ImPlotContext gp;

//-----------------------------------------------------------------------------
// Utils
//-----------------------------------------------------------------------------

/// Returns the next unused default plot color
ImVec4 NextColor() {
    ImVec4 col  = gp.ColorMap[gp.CurrentPlot->ColorIdx % gp.ColorMap.size()];
    gp.CurrentPlot->ColorIdx++;
    return col;
}

inline void FitPoint(const ImVec2& p) {
    if (!NanOrInf(p.x)) {
        gp.Extents.Min.x = p.x < gp.Extents.Min.x ? p.x : gp.Extents.Min.x;
        gp.Extents.Max.x = p.x > gp.Extents.Max.x ? p.x : gp.Extents.Max.x;
    }
    if (!NanOrInf(p.y)) {
        gp.Extents.Min.y = p.y < gp.Extents.Min.y ? p.y : gp.Extents.Min.y;
        gp.Extents.Max.y = p.y > gp.Extents.Max.y ? p.y : gp.Extents.Max.y;
    }
}

//-----------------------------------------------------------------------------
// Coordinate Transforms
//-----------------------------------------------------------------------------

inline void UpdateTransformCache() {
    // get pixels for transforms
    gp.PixelRange = ImRect(HasFlag(gp.CurrentPlot->XAxis.Flags, ImAxisFlags_Invert) ? gp.BB_Grid.Max.x : gp.BB_Grid.Min.x,
                           HasFlag(gp.CurrentPlot->YAxis.Flags, ImAxisFlags_Invert) ? gp.BB_Grid.Min.y : gp.BB_Grid.Max.y,
                           HasFlag(gp.CurrentPlot->XAxis.Flags, ImAxisFlags_Invert) ? gp.BB_Grid.Min.x : gp.BB_Grid.Max.x,
                           HasFlag(gp.CurrentPlot->YAxis.Flags, ImAxisFlags_Invert) ? gp.BB_Grid.Max.y : gp.BB_Grid.Min.y);   

    gp.M.x       = (gp.PixelRange.Max.x - gp.PixelRange.Min.x) / (gp.CurrentPlot->XAxis.Max - gp.CurrentPlot->XAxis.Min);
    gp.M.y       = (gp.PixelRange.Max.y - gp.PixelRange.Min.y) / (gp.CurrentPlot->YAxis.Max - gp.CurrentPlot->YAxis.Min);
    gp.LogDen.x  = log10(gp.CurrentPlot->XAxis.Max / gp.CurrentPlot->XAxis.Min);
    gp.LogDen.y  = log10(gp.CurrentPlot->YAxis.Max / gp.CurrentPlot->YAxis.Min);
}

inline ImVec2 PixelsToPlot(float x, float y) {
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PixelsToPlot() Needs to be called between BeginPlot() and EndPlot()!");
    ImVec2 plt;
    plt.x = (x - gp.PixelRange.Min.x) / gp.M.x + gp.CurrentPlot->XAxis.Min;
    plt.y = (y - gp.PixelRange.Min.y) / gp.M.y + gp.CurrentPlot->YAxis.Min;
    if (HasFlag(gp.CurrentPlot->XAxis.Flags, ImAxisFlags_LogScale)) {
        float t = (plt.x - gp.CurrentPlot->XAxis.Min) / (gp.CurrentPlot->XAxis.Max - gp.CurrentPlot->XAxis.Min);
        plt.x = pow(10.0f, t * gp.LogDen.x) * gp.CurrentPlot->XAxis.Min;
    }
    if (HasFlag(gp.CurrentPlot->YAxis.Flags, ImAxisFlags_LogScale)) {
        float t = (plt.y - gp.CurrentPlot->YAxis.Min) / (gp.CurrentPlot->YAxis.Max - gp.CurrentPlot->YAxis.Min);
        plt.y = pow(10.0f, t * gp.LogDen.y) * gp.CurrentPlot->YAxis.Min;
    }
    return plt;
}

inline ImVec2 PlotToPixels(float x, float y) {
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotToPixels() Needs to be called between BeginPlot() and EndPlot()!");
    ImVec2 pix;
    if (HasFlag(gp.CurrentPlot->XAxis.Flags, ImAxisFlags_LogScale)) {
        float t = log10(x / gp.CurrentPlot->XAxis.Min) / gp.LogDen.x;   
        x       = ImLerp(gp.CurrentPlot->XAxis.Min, gp.CurrentPlot->XAxis.Max, t);
    }             
    if (HasFlag(gp.CurrentPlot->YAxis.Flags, ImAxisFlags_LogScale)) {
        float t = log10(y / gp.CurrentPlot->YAxis.Min) / gp.LogDen.y;   
        y       = ImLerp(gp.CurrentPlot->YAxis.Min, gp.CurrentPlot->YAxis.Max, t);
    }
    pix.x = gp.PixelRange.Min.x + gp.M.x * (x - gp.CurrentPlot->XAxis.Min);
    pix.y = gp.PixelRange.Min.y + gp.M.y * (y - gp.CurrentPlot->YAxis.Min);
    return pix;
}

ImVec2 PixelsToPlot(const ImVec2& pix) {
    return PixelsToPlot(pix.x, pix.y);
}

ImVec2 PlotToPixels(const ImVec2& plt) {
    return PlotToPixels(plt.x, plt.y);
}

struct Plt2PixLinLin {
    static inline ImVec2 Transform(const ImVec2& plt) { return Transform(plt.x, plt.y); }
    static inline ImVec2 Transform(float x, float y) {
        return { gp.PixelRange.Min.x + gp.M.x * (x - gp.CurrentPlot->XAxis.Min), 
                 gp.PixelRange.Min.y + gp.M.y * (y - gp.CurrentPlot->YAxis.Min) };
    }
};

struct Plt2PixLinLinObj {
    Plt2PixLinLinObj() {
        a = gp.PixelRange.Min.x;
        b = gp.M.x;
        c = gp.CurrentPlot->XAxis.Min;
        d = gp.PixelRange.Min.y;
        e = gp.M.y;
        f = gp.CurrentPlot->YAxis.Min;
    }
    inline ImVec2 Transform(const ImVec2& plt) { return Transform(plt.x, plt.y); }
    inline ImVec2 Transform(float x, float y) {
        return { a + b * (x - c), d + e * (y - f) };
    }
    float a, b, c, d, e, f;
};


struct Plt2PixLogLin {
    static inline ImVec2 Transform(const ImVec2& plt) { return Transform(plt.x, plt.y); }
    static inline ImVec2 Transform(float x, float y) {
        float t = log10(x / gp.CurrentPlot->XAxis.Min) / gp.LogDen.x;   
        x       = ImLerp(gp.CurrentPlot->XAxis.Min, gp.CurrentPlot->XAxis.Max, t);
        return { gp.PixelRange.Min.x + gp.M.x * (x - gp.CurrentPlot->XAxis.Min), 
                 gp.PixelRange.Min.y + gp.M.y * (y - gp.CurrentPlot->YAxis.Min) };
    }
};

struct Plt2PixLinLog {
    static inline ImVec2 Transform(const ImVec2& plt) { return Transform(plt.x, plt.y); }
    static inline ImVec2 Transform(float x, float y) {
        float t = log10(y / gp.CurrentPlot->YAxis.Min) / gp.LogDen.y;
        y       = ImLerp(gp.CurrentPlot->YAxis.Min, gp.CurrentPlot->YAxis.Max, t);
        return { gp.PixelRange.Min.x + gp.M.x * (x - gp.CurrentPlot->XAxis.Min), 
                 gp.PixelRange.Min.y + gp.M.y * (y - gp.CurrentPlot->YAxis.Min) };
    }
};

struct Plt2PixLogLog {
    static inline ImVec2 Transform(const ImVec2& plt) { return Transform(plt.x, plt.y); }
    static inline ImVec2 Transform(float x, float y) {
        float t = log10(x / gp.CurrentPlot->XAxis.Min) / gp.LogDen.x;   
        x       = ImLerp(gp.CurrentPlot->XAxis.Min, gp.CurrentPlot->XAxis.Max, t);
        t       = log10(y / gp.CurrentPlot->YAxis.Min) / gp.LogDen.y;
        y       = ImLerp(gp.CurrentPlot->YAxis.Min, gp.CurrentPlot->YAxis.Max, t);
        return { gp.PixelRange.Min.x + gp.M.x * (x - gp.CurrentPlot->XAxis.Min), 
                 gp.PixelRange.Min.y + gp.M.y * (y - gp.CurrentPlot->YAxis.Min) };
    }
};

//-----------------------------------------------------------------------------
// Legend Utils
//-----------------------------------------------------------------------------

ImPlotItem* RegisterItem(const char* label_id) {
    ImGuiID id = ImGui::GetID(label_id);
    ImPlotItem* item = gp.CurrentPlot->Items.GetOrAddByKey(id);
    int idx = gp.CurrentPlot->Items.GetIndex(item);
    item->ID = id;
    gp.LegendIndices.push_back(idx);
    item->NameOffset = gp.LegendLabels.size();
    gp.LegendLabels.append(label_id, label_id + strlen(label_id) + 1);
    if (item->Show)
        gp.VisibleItemCount++;
    return item;
}

int GetLegendCount() {
    return gp.LegendIndices.size();
}

ImPlotItem* GetLegendItem(int i) {
    return gp.CurrentPlot->Items.GetByIndex(gp.LegendIndices[i]);
}

const char* GetLegendLabel(int i) {
    ImPlotItem* item  = gp.CurrentPlot->Items.GetByIndex(gp.LegendIndices[i]);
    IM_ASSERT(item->NameOffset != -1 && item->NameOffset < gp.LegendLabels.Buf.Size);
    return gp.LegendLabels.Buf.Data + item->NameOffset;
}

//-----------------------------------------------------------------------------
// Tick Utils
//-----------------------------------------------------------------------------

inline void GetTicks(float tMin, float tMax, int nMajor, int nMinor, bool logscale, ImVector<ImTick> &out) {
    out.shrink(0);
    if (logscale) {
        if (tMin <= 0 || tMax <= 0)
            return;
        int exp_min = (int)(ImFloor(log10(tMin)));
        int exp_max = (int)(ImCeil(log10(tMax)));
        for (int e = exp_min - 1; e < exp_max + 1; ++e) {
            double major1 = ImPow(10, (double)(e));
            double major2 = ImPow(10, (double)(e + 1));
            double interval = (major2 - major1) / 9;
            if (major1 >= (tMin - FLT_EPSILON) && major1 <= (tMax + FLT_EPSILON))
                out.push_back(ImTick(major1, true));
            for (int i = 1; i < 9; ++i) {
                double minor = major1 + i * interval;
                if (minor >= (tMin - FLT_EPSILON) && minor <= (tMax + FLT_EPSILON))
                    out.push_back(ImTick(minor, false, false));
            }
        }
    }
    else {
        const double range    = NiceNum(tMax - tMin, 0);
        const double interval = NiceNum(range / (nMajor - 1), 1);
        const double graphmin = floor(tMin / interval) * interval;
        const double graphmax = ceil(tMax / interval) * interval;
        for (double major = graphmin; major < graphmax + 0.5 * interval; major += interval) {
            if (major >= tMin && major <= tMax)
                out.push_back(ImTick(major, true));
            for (int i = 1; i < nMinor; ++i) {
                double minor = major + i * interval / nMinor;
                if (minor >= tMin && minor <= tMax)
                    out.push_back(ImTick(minor, false));
            }
        }
    }
}

inline void LabelTicks(ImVector<ImTick> &ticks, bool scientific, ImGuiTextBuffer& buffer) {
    buffer.Buf.resize(0);
    char temp[32];
    for (ImTick &tk : ticks) {
        if (tk.RenderLabel) {
            tk.TextOffset = buffer.size();
            if (scientific)
                sprintf(temp, "%.0e", tk.PlotPos);
            else
                sprintf(temp, "%g", tk.PlotPos);
            buffer.append(temp, temp + strlen(temp) + 1);
            tk.Size = CalcTextSize(buffer.Buf.Data + tk.TextOffset);
        }
    }
}

//-----------------------------------------------------------------------------
// BeginPlot()
//-----------------------------------------------------------------------------

bool BeginPlot(const char* title, const char* x_label, const char* y_label, const ImVec2& size, ImPlotFlags flags, ImAxisFlags x_flags, ImAxisFlags y_flags) {

    IM_ASSERT_USER_ERROR(gp.CurrentPlot == NULL, "Mismatched BeginPlot()/EndPlot()!");

    // FRONT MATTER  -----------------------------------------------------------

    ImGuiContext &G      = *GImGui;
    ImGuiWindow * Window = G.CurrentWindow;
    if (Window->SkipItems) {
        gp.NextPlotData = ImNextPlotData();
        return false;
    }

    const ImGuiID     ID       = Window->GetID(title);
    const ImGuiStyle &Style    = G.Style;
    const ImGuiIO &   IO       = GetIO();    

    bool just_created = gp.Plots.GetByKey(ID) == NULL;    
    gp.CurrentPlot = gp.Plots.GetOrAddByKey(ID);
    ImPlot &plot = *gp.CurrentPlot;

    if (just_created) {
        plot.Flags       = flags;
        plot.XAxis.Flags = x_flags;
        plot.YAxis.Flags = y_flags;
    }

    // capture scroll with a child region
    if (!HasFlag(plot.Flags, ImPlotFlags_NoChild)) {
        ImGui::BeginChild(title, size);
        Window = ImGui::GetCurrentWindow();
        Window->ScrollMax.y = 1.0f;
    }

    ImDrawList &DrawList = *Window->DrawList;

    // NextPlotData -----------------------------------------------------------

    if (gp.NextPlotData.HasXRange) {
        if (just_created || gp.NextPlotData.XRangeCond == ImGuiCond_Always)
        {
            plot.XAxis.Min = gp.NextPlotData.XMin;
            plot.XAxis.Max = gp.NextPlotData.XMax;
        }
    }

    if (gp.NextPlotData.HasYRange) {
        if (just_created || gp.NextPlotData.YRangeCond == ImGuiCond_Always)
        {
            plot.YAxis.Min = gp.NextPlotData.YMin;
            plot.YAxis.Max = gp.NextPlotData.YMax;
        }
    }

    // AXIS STATES ------------------------------------------------------------

    const bool flip_x     = HasFlag(plot.XAxis.Flags, ImAxisFlags_Invert);
    const bool lock_x_min = HasFlag(plot.XAxis.Flags, ImAxisFlags_LockMin);
    const bool lock_x_max = HasFlag(plot.XAxis.Flags, ImAxisFlags_LockMax);
    const bool lock_x     = (lock_x_min && lock_x_max) || (gp.NextPlotData.HasXRange && gp.NextPlotData.XRangeCond == ImGuiCond_Always);

    const bool flip_y     = HasFlag(plot.YAxis.Flags, ImAxisFlags_Invert);
    const bool lock_y_min = HasFlag(plot.YAxis.Flags, ImAxisFlags_LockMin);
    const bool lock_y_max = HasFlag(plot.YAxis.Flags, ImAxisFlags_LockMax);
    const bool lock_y     = (lock_y_min && lock_y_max) || (gp.NextPlotData.HasYRange && gp.NextPlotData.YRangeCond == ImGuiCond_Always);

    const bool lock_plot  = lock_x && lock_y;
    
    // CONSTRAINTS ------------------------------------------------------------

    plot.XAxis.Min = ConstrainNan(ConstrainInf(plot.XAxis.Min));
    plot.XAxis.Max = ConstrainNan(ConstrainInf(plot.XAxis.Max));
    plot.YAxis.Min = ConstrainNan(ConstrainInf(plot.YAxis.Min));
    plot.YAxis.Max = ConstrainNan(ConstrainInf(plot.YAxis.Max));

    if (HasFlag(plot.XAxis.Flags, ImAxisFlags_LogScale))
        plot.XAxis.Min = ConstrainLog(plot.XAxis.Min);
    if (HasFlag(plot.XAxis.Flags, ImAxisFlags_LogScale))
        plot.XAxis.Max = ConstrainLog(plot.XAxis.Max);
    if (HasFlag(plot.YAxis.Flags, ImAxisFlags_LogScale))
        plot.YAxis.Min = ConstrainLog(plot.YAxis.Min);
    if (HasFlag(plot.YAxis.Flags, ImAxisFlags_LogScale))
        plot.YAxis.Max = ConstrainLog(plot.YAxis.Max);

    if (plot.XAxis.Max <= plot.XAxis.Min)
        plot.XAxis.Max = plot.XAxis.Min + FLT_EPSILON;
    if (plot.YAxis.Max <= plot.YAxis.Min)
        plot.YAxis.Max = plot.YAxis.Min + FLT_EPSILON;

    // adaptive divisions
    if (HasFlag(plot.XAxis.Flags, ImAxisFlags_Adaptive)) {
        plot.XAxis.Divisions = (int)IM_ROUND(0.003 * gp.BB_Canvas.GetWidth());
        if (plot.XAxis.Divisions < 2)
            plot.XAxis.Divisions = 2; 
    }
    if (HasFlag(plot.YAxis.Flags, ImAxisFlags_Adaptive)) {
        plot.YAxis.Divisions = (int)IM_ROUND(0.003 * gp.BB_Canvas.GetHeight());
        if (plot.YAxis.Divisions < 2)
            plot.YAxis.Divisions = 2;
    }

    // COLORS -----------------------------------------------------------------

    gp.Col_Frame  = gp.Style.Colors[ImPlotCol_FrameBg].w     == -1 ? GetColorU32(ImGuiCol_FrameBg)    : GetColorU32(gp.Style.Colors[ImPlotCol_FrameBg]);
    gp.Col_Bg     = gp.Style.Colors[ImPlotCol_PlotBg].w      == -1 ? GetColorU32(ImGuiCol_WindowBg)   : GetColorU32(gp.Style.Colors[ImPlotCol_PlotBg]);
    gp.Col_Border = gp.Style.Colors[ImPlotCol_PlotBorder].w  == -1 ? GetColorU32(ImGuiCol_Text, 0.5f) : GetColorU32(gp.Style.Colors[ImPlotCol_PlotBorder]);

    const ImVec4 col_xAxis = gp.Style.Colors[ImPlotCol_XAxis].w == -1 ? ImGui::GetStyle().Colors[ImGuiCol_Text] * ImVec4(1, 1, 1, 0.25f) : gp.Style.Colors[ImPlotCol_XAxis];
    gp.Col_XMajor = GetColorU32(col_xAxis);
    gp.Col_XMinor = GetColorU32(col_xAxis * ImVec4(1, 1, 1, 0.25f));
    gp.Col_XTxt   = GetColorU32({col_xAxis.x, col_xAxis.y, col_xAxis.z, 1});

    const ImVec4 col_yAxis = gp.Style.Colors[ImPlotCol_YAxis].w == -1 ? ImGui::GetStyle().Colors[ImGuiCol_Text] * ImVec4(1, 1, 1, 0.25f) : gp.Style.Colors[ImPlotCol_YAxis];
    gp.Col_YMajor = GetColorU32(col_yAxis);
    gp.Col_YMinor = GetColorU32(col_yAxis * ImVec4(1, 1, 1, 0.25f));
    gp.Col_YTxt   = GetColorU32({col_yAxis.x, col_yAxis.y, col_yAxis.z, 1});

    gp.Col_Txt    = GetColorU32(ImGuiCol_Text);
    gp.Col_TxtDis = GetColorU32(ImGuiCol_TextDisabled);
    gp.Col_SlctBg = GetColorU32(gp.Style.Colors[ImPlotCol_Selection] * ImVec4(1,1,1,0.25f));
    gp.Col_SlctBd = GetColorU32(gp.Style.Colors[ImPlotCol_Selection]);
    gp.Col_QryBg =  GetColorU32(gp.Style.Colors[ImPlotCol_Query] * ImVec4(1,1,1,0.25f));
    gp.Col_QryBd =  GetColorU32(gp.Style.Colors[ImPlotCol_Query]);

    // BB AND HOVER -----------------------------------------------------------

    // frame
    const ImVec2 frame_size = CalcItemSize(size, 100, 100);
    gp.BB_Frame = ImRect(Window->DC.CursorPos, Window->DC.CursorPos + frame_size);
    ItemSize(gp.BB_Frame);
    if (!ItemAdd(gp.BB_Frame, 0, &gp.BB_Frame)) {
        gp.NextPlotData = ImNextPlotData();
        gp.CurrentPlot = NULL;
        if (!HasFlag(plot.Flags, ImPlotFlags_NoChild))
            ImGui::EndChild();
        return false;
    }
    gp.Hov_Frame = ItemHoverable(gp.BB_Frame, ID);
    RenderFrame(gp.BB_Frame.Min, gp.BB_Frame.Max, gp.Col_Frame, true, Style.FrameRounding);

    // canvas bb
    gp.BB_Canvas = ImRect(gp.BB_Frame.Min + Style.WindowPadding, gp.BB_Frame.Max - Style.WindowPadding);     

    gp.RenderX = (HasFlag(plot.XAxis.Flags, ImAxisFlags_GridLines) || 
                    HasFlag(plot.XAxis.Flags, ImAxisFlags_TickMarks) || 
                    HasFlag(plot.XAxis.Flags, ImAxisFlags_TickLabels)) &&  plot.XAxis.Divisions > 1;
    gp.RenderY = (HasFlag(plot.YAxis.Flags, ImAxisFlags_GridLines) || 
                    HasFlag(plot.YAxis.Flags, ImAxisFlags_TickMarks) || 
                    HasFlag(plot.YAxis.Flags, ImAxisFlags_TickLabels)) &&  plot.YAxis.Divisions > 1;

    // get ticks
    if (gp.RenderX)
        GetTicks(plot.XAxis.Min, plot.XAxis.Max, plot.XAxis.Divisions, plot.XAxis.Subdivisions, HasFlag(plot.XAxis.Flags, ImAxisFlags_LogScale), gp.XTicks);
    if (gp.RenderY)
        GetTicks(plot.YAxis.Min, plot.YAxis.Max, plot.YAxis.Divisions, plot.YAxis.Subdivisions, HasFlag(plot.YAxis.Flags, ImAxisFlags_LogScale), gp.YTicks);

    // label ticks
    if (HasFlag(plot.XAxis.Flags, ImAxisFlags_TickLabels))
        LabelTicks(gp.XTicks, HasFlag(plot.XAxis.Flags, ImAxisFlags_Scientific), gp.XTickLabels);

    float max_label_width = 0;
    if (HasFlag(plot.YAxis.Flags, ImAxisFlags_TickLabels)) {
        LabelTicks(gp.YTicks, HasFlag(plot.YAxis.Flags, ImAxisFlags_Scientific), gp.YTickLabels);
        for (ImTick &yt : gp.YTicks)
            max_label_width = yt.Size.x > max_label_width ? yt.Size.x : max_label_width;
    }

    // grid bb
    const ImVec2 title_size = CalcTextSize(title, NULL, true);
    const float txt_off     = 5;
    const float txt_height  = GetTextLineHeight();
    const float pad_top     = title_size.x > 0.0f ? txt_height + txt_off : 0;
    const float pad_bot     = (HasFlag(plot.XAxis.Flags, ImAxisFlags_TickLabels) ? txt_height + txt_off : 0) + (x_label ? txt_height + txt_off : 0);
    const float pad_left    = (HasFlag(plot.YAxis.Flags, ImAxisFlags_TickLabels) ? max_label_width + txt_off : 0) + (y_label ? txt_height + txt_off : 0);
    gp.BB_Grid            = ImRect(gp.BB_Canvas.Min + ImVec2(pad_left, pad_top), gp.BB_Canvas.Max - ImVec2(0, pad_bot));
    gp.Hov_Grid           = gp.BB_Grid.Contains(IO.MousePos);

    // axis region bbs
    const ImRect xAxisRegion_bb(gp.BB_Grid.Min + ImVec2(10, 0), {gp.BB_Grid.Max.x, gp.BB_Frame.Max.y});
    const bool   hov_x_axis_region = xAxisRegion_bb.Contains(IO.MousePos);
    const ImRect yAxisRegion_bb({gp.BB_Frame.Min.x, gp.BB_Grid.Min.y}, gp.BB_Grid.Max - ImVec2(0, 10));
    const bool   hov_y_axis_region = yAxisRegion_bb.Contains(IO.MousePos);

    // legend hovered from last frame
    const bool hov_legend = HasFlag(plot.Flags, ImPlotFlags_Legend) ? gp.Hov_Frame && plot.BB_Legend.Contains(IO.MousePos) : false;   

    bool hov_query = false;
    if (plot.Queried && !plot.Querying) {
        ImRect bb_query;
        if (HasFlag(plot.Flags, ImPlotFlags_PixelQuery)) {
            bb_query      = plot.QueryRect;
            bb_query.Min += gp.BB_Grid.Min;
            bb_query.Max += gp.BB_Grid.Min;
        }
        else {
            UpdateTransformCache();
            ImVec2 p1 = PlotToPixels(plot.QueryRange.XMin, plot.QueryRange.YMin); 
            ImVec2 p2 = PlotToPixels(plot.QueryRange.XMax, plot.QueryRange.YMax);
            bb_query.Min = ImVec2(ImMin(p1.x,p2.x), ImMin(p1.y,p2.y));
            bb_query.Max = ImVec2(ImMax(p1.x,p2.x), ImMax(p1.y,p2.y));
        }
        hov_query = bb_query.Contains(IO.MousePos);
    }

    // QUERY DRAG -------------------------------------------------------------
    if (plot.DraggingQuery && (IO.MouseReleased[0] || !IO.MouseDown[0])) {
        plot.DraggingQuery = false;
    }
    if (plot.DraggingQuery) {        
        SetMouseCursor(ImGuiMouseCursor_ResizeAll);
        if (!HasFlag(plot.Flags, ImPlotFlags_PixelQuery)) {
            ImVec2 p1 = PlotToPixels(plot.QueryRange.XMin, plot.QueryRange.YMin); 
            ImVec2 p2 = PlotToPixels(plot.QueryRange.XMax, plot.QueryRange.YMax);
            plot.QueryRect.Min = ImVec2(ImMin(p1.x,p2.x), ImMin(p1.y,p2.y)) + IO.MouseDelta;
            plot.QueryRect.Max = ImVec2(ImMax(p1.x,p2.x), ImMax(p1.y,p2.y)) + IO.MouseDelta;
            p1 = PixelsToPlot(plot.QueryRect.Min);
            p2 = PixelsToPlot(plot.QueryRect.Max);  
            plot.QueryRect.Min -= gp.BB_Grid.Min;
            plot.QueryRect.Max -= gp.BB_Grid.Min;          
            plot.QueryRange.XMin = ImMin(p1.x, p2.x);
            plot.QueryRange.XMax = ImMax(p1.x, p2.x);
            plot.QueryRange.YMin = ImMin(p1.y, p2.y);
            plot.QueryRange.YMax = ImMax(p1.y, p2.y);
        }
        else {
            plot.QueryRect.Min += IO.MouseDelta;
            plot.QueryRect.Max += IO.MouseDelta;
        }
    }
    if (gp.Hov_Frame && hov_query && !plot.DraggingQuery && !plot.Selecting && !hov_legend) {
        SetMouseCursor(ImGuiMouseCursor_ResizeAll);
        if (IO.MouseDown[0] && !plot.XAxis.Dragging && !plot.YAxis.Dragging) {
            plot.DraggingQuery = true;
        }        
    }    

    // DRAG INPUT -------------------------------------------------------------

    // end drags
    if (plot.XAxis.Dragging && (IO.MouseReleased[0] || !IO.MouseDown[0])) {
        plot.XAxis.Dragging             = false;
        G.IO.MouseDragMaxDistanceSqr[0] = 0; 
    }
    if (plot.YAxis.Dragging && (IO.MouseReleased[0] || !IO.MouseDown[0])) {
        plot.YAxis.Dragging             = false;
        G.IO.MouseDragMaxDistanceSqr[0] = 0;
    }
    // do drag
    if (plot.XAxis.Dragging || plot.YAxis.Dragging) {
        UpdateTransformCache();
        ImVec2 plot_tl = PixelsToPlot(gp.BB_Grid.Min - IO.MouseDelta);
        ImVec2 plot_br = PixelsToPlot(gp.BB_Grid.Max - IO.MouseDelta);
        if (!lock_x && plot.XAxis.Dragging) {
            if (!lock_x_min)
                plot.XAxis.Min = flip_x ? plot_br.x : plot_tl.x;
            if (!lock_x_max)
                plot.XAxis.Max = flip_x ? plot_tl.x : plot_br.x;
        }
        if (!lock_y && plot.YAxis.Dragging) {
            if (!lock_y_min)
                plot.YAxis.Min = flip_y ? plot_tl.y : plot_br.y;
            if (!lock_y_max)
                plot.YAxis.Max = flip_y ? plot_br.y : plot_tl.y;
        }
        if ((lock_x && lock_y) || (lock_x && plot.XAxis.Dragging && !plot.YAxis.Dragging) || (lock_y && plot.YAxis.Dragging && !plot.XAxis.Dragging))
            ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
        else if (lock_x || (!plot.XAxis.Dragging && plot.YAxis.Dragging))
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        else if (lock_y || (!plot.YAxis.Dragging && plot.XAxis.Dragging))
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        else
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
    }
    // start drag
    if (gp.Hov_Frame && hov_x_axis_region && IO.MouseDragMaxDistanceSqr[0] > 5 && !plot.Selecting && !hov_legend && !hov_query && !plot.DraggingQuery)
        plot.XAxis.Dragging = true;
    if (gp.Hov_Frame && hov_y_axis_region && IO.MouseDragMaxDistanceSqr[0] > 5 && !plot.Selecting && !hov_legend && !hov_query && !plot.DraggingQuery)
        plot.YAxis.Dragging = true;

    // SCROLL INPUT -----------------------------------------------------------

    if (gp.Hov_Frame && (hov_x_axis_region || hov_y_axis_region) && IO.MouseWheel != 0) {
        UpdateTransformCache();
        float zoom_rate = 0.1f;
        if (IO.MouseWheel > 0) 
            zoom_rate = (-zoom_rate) / (1.0f + (2.0f * zoom_rate));  
        float tx = Remap(IO.MousePos.x, gp.BB_Grid.Min.x, gp.BB_Grid.Max.x, 0, 1);
        float ty = Remap(IO.MousePos.y, gp.BB_Grid.Min.y, gp.BB_Grid.Max.y, 0, 1);
        ImVec2 plot_tl = PixelsToPlot(gp.BB_Grid.Min - gp.BB_Grid.GetSize() * ImVec2(tx * zoom_rate, ty * zoom_rate));
        ImVec2 plot_br = PixelsToPlot(gp.BB_Grid.Max + gp.BB_Grid.GetSize() * ImVec2((1 - tx) * zoom_rate, (1 - ty) * zoom_rate));
        if (hov_x_axis_region && !lock_x) {
            if (!lock_x_min)
                plot.XAxis.Min = flip_x ? plot_br.x : plot_tl.x;
            if (!lock_x_max)
                plot.XAxis.Max = flip_x ? plot_tl.x : plot_br.x;
        } 
        if (hov_y_axis_region && !lock_y) {
            if (!lock_y_min)
                plot.YAxis.Min = flip_y ? plot_tl.y : plot_br.y;
            if (!lock_y_max)
                plot.YAxis.Max = flip_y ? plot_br.y : plot_tl.y;
        }        
    }

    // BOX-SELECTION AND QUERY ------------------------------------------------

    // confirm selection
    if (plot.Selecting && (IO.MouseReleased[1] || !IO.MouseDown[1])) {
        UpdateTransformCache();
        ImVec2 select_size = plot.SelectStart - IO.MousePos;
        if (HasFlag(plot.Flags, ImPlotFlags_Selection) && ImFabs(select_size.x) > 2 && ImFabs(select_size.y) > 2) {
            ImVec2 p1 = PixelsToPlot(plot.SelectStart);
            ImVec2 p2 = PixelsToPlot(IO.MousePos);
            if (!lock_x_min && !IO.KeyAlt) 
                plot.XAxis.Min = ImMin(p1.x, p2.x);
            if (!lock_x_max && !IO.KeyAlt)
                plot.XAxis.Max = ImMax(p1.x, p2.x);                
            if (!lock_y_min && !IO.KeyShift) 
                plot.YAxis.Min = ImMin(p1.y, p2.y);
            if (!lock_y_max && !IO.KeyShift)
                plot.YAxis.Max = ImMax(p1.y, p2.y);                
        }        
        plot.Selecting = false;
    }
    // bad selection
    if (plot.Selecting && (!HasFlag(plot.Flags, ImPlotFlags_Selection) || lock_plot) && ImLengthSqr(plot.SelectStart - IO.MousePos) > 4) { 
        ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
    }
    // cancel selection
    if (plot.Selecting && (IO.MouseClicked[0] || IO.MouseDown[0])) {
        plot.Selecting = false;
    }
    // begin selection or query
    if (gp.Hov_Frame && gp.Hov_Grid && IO.MouseClicked[1]) {
        plot.SelectStart = IO.MousePos;
        plot.Selecting = true;
    }
    // update query
    if (plot.Querying) {
        UpdateTransformCache();
        plot.QueryRect.Min.x = IO.KeyAlt ? gp.BB_Grid.Min.x :   ImMin(plot.QueryStart.x, IO.MousePos.x);
        plot.QueryRect.Max.x = IO.KeyAlt ? gp.BB_Grid.Max.x :   ImMax(plot.QueryStart.x, IO.MousePos.x);
        plot.QueryRect.Min.y = IO.KeyShift ? gp.BB_Grid.Min.y : ImMin(plot.QueryStart.y, IO.MousePos.y);
        plot.QueryRect.Max.y = IO.KeyShift ? gp.BB_Grid.Max.y : ImMax(plot.QueryStart.y, IO.MousePos.y);
        ImVec2 p1 = PixelsToPlot(plot.QueryRect.Min);
        ImVec2 p2 = PixelsToPlot(plot.QueryRect.Max);
        plot.QueryRect.Min -= gp.BB_Grid.Min;
        plot.QueryRect.Max -= gp.BB_Grid.Min;
        plot.QueryRange.XMin = ImMin(p1.x, p2.x);
        plot.QueryRange.XMax = ImMax(p1.x, p2.x);
        plot.QueryRange.YMin = ImMin(p1.y, p2.y);
        plot.QueryRange.YMax = ImMax(p1.y, p2.y);
    }
    // end query
    if (plot.Querying && (IO.MouseReleased[2] || IO.MouseReleased[1])) {
        plot.Querying = false;
        if (plot.QueryRect.GetWidth() > 2 && plot.QueryRect.GetHeight() > 2) {
            plot.Queried = true;
        }
        else {
            plot.Queried = false;
            plot.QueryRange = ImPlotRange();
        }
    }
    // begin query
    if ((gp.Hov_Frame && gp.Hov_Grid && IO.MouseClicked[2])) {
        plot.QueryRect = ImRect(0,0,0,0);
        plot.QueryRange = ImPlotRange();
        plot.Querying = true;
        plot.Queried  = true;
        plot.QueryStart = IO.MousePos;
    }
    // toggle between select/query
    if (plot.Selecting && IO.KeyCtrl) {
        plot.Selecting = false;
        plot.QueryRect = ImRect(0,0,0,0);
        plot.QueryRange = ImPlotRange();
        plot.Querying = true;
        plot.Queried  = true;
        plot.QueryStart = plot.SelectStart;
    }
    if (plot.Querying && !IO.KeyCtrl && !IO.MouseDown[2]) {
        plot.Selecting = true;
        plot.Querying = false;
        plot.Queried = false;
        plot.QueryRect = ImRect(0,0,0,0);
        plot.QueryRange = ImPlotRange();
    }
    
    // DOUBLE CLICK -----------------------------------------------------------

    if ( IO.MouseDoubleClicked[0] && gp.Hov_Frame && (hov_x_axis_region || hov_y_axis_region) && !hov_legend && !hov_query) {
        gp.FitThisFrame = true;
        gp.FitX = hov_x_axis_region;
        gp.FitY = hov_y_axis_region;
    }
    else {
        gp.FitThisFrame = false;     
        gp.FitX = false;
        gp.FitY = false;
    }

    // FOCUS ------------------------------------------------------------------

    // focus window
    if ((IO.MouseClicked[0] || IO.MouseClicked[1]) && gp.Hov_Frame)
        FocusWindow(GetCurrentWindow());           

    UpdateTransformCache();

    // set mouse position
    gp.LastMousePos = PixelsToPlot(IO.MousePos);

    // RENDER -----------------------------------------------------------------

    // grid bg
    DrawList.AddRectFilled(gp.BB_Grid.Min, gp.BB_Grid.Max, gp.Col_Bg);

    // render axes
    PushPlotClipRect();

    // transform ticks
    if (gp.RenderX) {
        for (ImTick& xt : gp.XTicks)
            xt.PixelPos = PlotToPixels((float)xt.PlotPos, 0).x;
    }
    if (gp.RenderY) {
        for (ImTick& yt : gp.YTicks)
            yt.PixelPos = PlotToPixels(0, (float)yt.PlotPos).y;
    }

    // render grid
    if (HasFlag(plot.XAxis.Flags, ImAxisFlags_GridLines)) {
        for (ImTick &xt : gp.XTicks)
            DrawList.AddLine({xt.PixelPos, gp.BB_Grid.Min.y}, {xt.PixelPos, gp.BB_Grid.Max.y}, xt.Major ? gp.Col_XMajor : gp.Col_XMinor, 1);
    }

    if (HasFlag(plot.YAxis.Flags, ImAxisFlags_GridLines)) {
        for (ImTick &yt : gp.YTicks)
            DrawList.AddLine({gp.BB_Grid.Min.x, yt.PixelPos}, {gp.BB_Grid.Max.x, yt.PixelPos}, yt.Major ? gp.Col_YMajor : gp.Col_YMinor, 1);
    }

    PopPlotClipRect();

    // render title
    if (title_size.x > 0.0f) {
        RenderText(ImVec2(gp.BB_Canvas.GetCenter().x - title_size.x * 0.5f, gp.BB_Canvas.Min.y), title, NULL, true);
    }

    // render labels
    if (HasFlag(plot.XAxis.Flags, ImAxisFlags_TickLabels)) {
        PushClipRect(gp.BB_Frame.Min, gp.BB_Frame.Max, true);
        for (ImTick &xt : gp.XTicks) {
            if (xt.RenderLabel && xt.PixelPos >= gp.BB_Grid.Min.x - 1 && xt.PixelPos <= gp.BB_Grid.Max.x + 1)
                DrawList.AddText({xt.PixelPos - xt.Size.x * 0.5f, gp.BB_Grid.Max.y + txt_off}, gp.Col_XTxt, gp.XTickLabels.Buf.Data + xt.TextOffset);
        }
        ImGui::PopClipRect();
    }
    if (x_label) {
        const ImVec2 xLabel_size = CalcTextSize(x_label);
        const ImVec2 xLabel_pos(gp.BB_Grid.GetCenter().x - xLabel_size.x * 0.5f,
                                gp.BB_Canvas.Max.y - txt_height);
        DrawList.AddText(xLabel_pos, gp.Col_XTxt, x_label);
    }
    if (HasFlag(plot.YAxis.Flags, ImAxisFlags_TickLabels)) {
        PushClipRect(gp.BB_Frame.Min, gp.BB_Frame.Max, true);
        for (ImTick &yt : gp.YTicks) {
            if (yt.RenderLabel && yt.PixelPos >= gp.BB_Grid.Min.y - 1 && yt.PixelPos <= gp.BB_Grid.Max.y + 1)
                DrawList.AddText({gp.BB_Grid.Min.x - txt_off - yt.Size.x, yt.PixelPos - 0.5f * yt.Size.y}, gp.Col_YTxt, gp.YTickLabels.Buf.Data + yt.TextOffset);
        }
        ImGui::PopClipRect();
    }
    if (y_label) {
        const ImVec2 yLabel_size = CalcTextSizeVertical(y_label);
        const ImVec2 yLabel_pos(gp.BB_Canvas.Min.x, gp.BB_Grid.GetCenter().y + yLabel_size.y * 0.5f);
        AddTextVertical(&DrawList, y_label, yLabel_pos, gp.Col_YTxt);
    }

    // PREP -------------------------------------------------------------------

    // push plot ID into stack
    PushID(ID);
    // reset items count
    gp.VisibleItemCount = 0;
    // reset extents
    gp.Extents.Min.x = INFINITY;
    gp.Extents.Min.y = INFINITY;
    gp.Extents.Max.x = -INFINITY;
    gp.Extents.Max.y = -INFINITY;
    // clear item names
    gp.LegendLabels.Buf.resize(0);
    // reset digital plot items count
    gp.DigitalPlotItemCnt = 0;
    return true;
}

//-----------------------------------------------------------------------------
// Context Menu
//-----------------------------------------------------------------------------

inline void AxisMenu(ImPlotAxis& Axis) {
     ImGui::PushItemWidth(75);
    bool lock_min = HasFlag(Axis.Flags, ImAxisFlags_LockMin);
    bool lock_max = HasFlag(Axis.Flags, ImAxisFlags_LockMax);
    bool invert   = HasFlag(Axis.Flags, ImAxisFlags_Invert);
    bool logscale = HasFlag(Axis.Flags, ImAxisFlags_LogScale);
    bool grid     = HasFlag(Axis.Flags, ImAxisFlags_GridLines);
    bool ticks    = HasFlag(Axis.Flags, ImAxisFlags_TickMarks);
    bool labels   = HasFlag(Axis.Flags, ImAxisFlags_TickLabels);
    if (ImGui::Checkbox("##LockMin", &lock_min)) 
        FlipFlag(Axis.Flags, ImAxisFlags_LockMin);
    ImGui::SameLine();
    if (lock_min) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.25f);
    }
    ImGui::DragFloat("Min", &Axis.Min, 0.01f + 0.01f * (Axis.Max - Axis.Min), -INFINITY, Axis.Max - FLT_EPSILON);
    if (lock_min) {
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();    }

    if (ImGui::Checkbox("##LockMax", &lock_max))
        FlipFlag(Axis.Flags, ImAxisFlags_LockMax);
    ImGui::SameLine();
    if (lock_max) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.25f);    }
    ImGui::DragFloat("Max", &Axis.Max, 0.01f + 0.01f * (Axis.Max - Axis.Min), Axis.Min + FLT_EPSILON, INFINITY);
    if (lock_max) {
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    }
    ImGui::Separator();
    if (ImGui::Checkbox("Invert", &invert))
        FlipFlag(Axis.Flags, ImAxisFlags_Invert);
    if (ImGui::Checkbox("Log Scale", &logscale))
        FlipFlag(Axis.Flags, ImAxisFlags_LogScale);
    ImGui::Separator();
    if (ImGui::Checkbox("Grid Lines", &grid))
        FlipFlag(Axis.Flags, ImAxisFlags_GridLines);
    if (ImGui::Checkbox("Tick Marks", &ticks))
        FlipFlag(Axis.Flags, ImAxisFlags_TickMarks);
    if (ImGui::Checkbox("Labels", &labels))
        FlipFlag(Axis.Flags, ImAxisFlags_TickLabels);
}

void PlotContextMenu(ImPlot& plot) {
    if (ImGui::BeginMenu("X-Axis")) {    
        PushID("X");        
        AxisMenu(plot.XAxis);
        PopID();
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Y-Axis")) {
        PushID("Y");      
        AxisMenu(plot.YAxis);
        PopID();
        ImGui::EndMenu();
    }
    ImGui::Separator();
    if ((ImGui::BeginMenu("Settings"))) {
        if (ImGui::MenuItem("Box Select",NULL,HasFlag(plot.Flags, ImPlotFlags_Selection))) {
            FlipFlag(plot.Flags, ImPlotFlags_Selection);
        }
        if (ImGui::MenuItem("Pixel Query",NULL,HasFlag(plot.Flags, ImPlotFlags_PixelQuery))) {
            FlipFlag(plot.Flags, ImPlotFlags_PixelQuery);
        }        
        if (ImGui::MenuItem("Crosshairs",NULL,HasFlag(plot.Flags, ImPlotFlags_Crosshairs))) {
            FlipFlag(plot.Flags, ImPlotFlags_Crosshairs);
        }
        if (ImGui::MenuItem("Mouse Position",NULL,HasFlag(plot.Flags, ImPlotFlags_MousePos))) {
            FlipFlag(plot.Flags, ImPlotFlags_MousePos);
        }
        if (ImGui::MenuItem("Cull Data",NULL,HasFlag(plot.Flags, ImPlotFlags_CullData))) {
            FlipFlag(plot.Flags, ImPlotFlags_CullData);
        }
        if (ImGui::MenuItem("Anti-Aliased Lines",NULL,HasFlag(plot.Flags, ImPlotFlags_AntiAliased))) {
            FlipFlag(plot.Flags, ImPlotFlags_AntiAliased);
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Legend",NULL,HasFlag(plot.Flags, ImPlotFlags_Legend))) {
        FlipFlag(plot.Flags, ImPlotFlags_Legend);
    }
#if 0
    if (ImGui::BeginMenu("Metrics")) {
        ImGui::PushItemWidth(75);
        ImGui::LabelText("Plots", "%d", gp.Plots.GetSize());
        ImGui::LabelText("Color Modifiers", "%d", gp.ColorModifiers.size());
        ImGui::LabelText("Style Modifiers", "%d", gp.StyleModifiers.size());
        ImGui::PopItemWidth();
        ImGui::EndMenu();
    }
#endif

}

//-----------------------------------------------------------------------------
// EndPlot()
//-----------------------------------------------------------------------------

void EndPlot() {

    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "Mismatched BeginPlot()/EndPlot()!");    

    ImPlot &plot = *gp.CurrentPlot;
    ImGuiContext &G      = *GImGui;
    ImGuiWindow * Window = G.CurrentWindow;
    ImDrawList & DrawList = *Window->DrawList;
    const ImGuiIO &   IO = GetIO();

    // AXIS STATES ------------------------------------------------------------

    const bool lock_x_min = HasFlag(plot.XAxis.Flags, ImAxisFlags_LockMin);
    const bool lock_x_max = HasFlag(plot.XAxis.Flags, ImAxisFlags_LockMax);
    const bool lock_x     = (lock_x_min && lock_x_max) || (gp.NextPlotData.HasXRange && gp.NextPlotData.XRangeCond == ImGuiCond_Always);

    const bool lock_y_min = HasFlag(plot.YAxis.Flags, ImAxisFlags_LockMin);
    const bool lock_y_max = HasFlag(plot.YAxis.Flags, ImAxisFlags_LockMax);
    const bool lock_y     = (lock_y_min && lock_y_max) || (gp.NextPlotData.HasYRange && gp.NextPlotData.YRangeCond == ImGuiCond_Always);

    const bool lock_plot  = lock_x && lock_y;

    // FINAL RENDER -----------------------------------------------------------

    PushPlotClipRect();

    // render ticks
    if (HasFlag(plot.XAxis.Flags, ImAxisFlags_TickMarks)) {
        for (ImTick &xt : gp.XTicks)
            DrawList.AddLine({xt.PixelPos, gp.BB_Grid.Max.y},{xt.PixelPos, gp.BB_Grid.Max.y - (xt.Major ? 10.0f : 5.0f)}, gp.Col_Border, 1);
    }
    if (HasFlag(plot.YAxis.Flags, ImAxisFlags_TickMarks)) {
        for (ImTick &yt : gp.YTicks)
            DrawList.AddLine({gp.BB_Grid.Min.x, yt.PixelPos}, {gp.BB_Grid.Min.x + (yt.Major ? 10.0f : 5.0f), yt.PixelPos}, gp.Col_Border, 1);
    }

    // render selection/query
    if (plot.Selecting) {
        ImRect select_bb(ImMin(IO.MousePos, plot.SelectStart), ImMax(IO.MousePos, plot.SelectStart));
        if (plot.Selecting && !lock_plot && HasFlag(plot.Flags, ImPlotFlags_Selection)) {
            if (IO.KeyAlt && IO.KeyShift && select_bb.GetWidth() > 2 && select_bb.GetHeight() > 2) {
                DrawList.AddRectFilled(gp.BB_Grid.Min, gp.BB_Grid.Max, gp.Col_SlctBg);
                DrawList.AddRect(      gp.BB_Grid.Min, gp.BB_Grid.Max, gp.Col_SlctBd);
            }
            else if ((lock_x || IO.KeyAlt) && select_bb.GetHeight() > 2) {
                DrawList.AddRectFilled(ImVec2(gp.BB_Grid.Min.x, select_bb.Min.y), ImVec2(gp.BB_Grid.Max.x, select_bb.Max.y), gp.Col_SlctBg);
                DrawList.AddRect(      ImVec2(gp.BB_Grid.Min.x, select_bb.Min.y), ImVec2(gp.BB_Grid.Max.x, select_bb.Max.y), gp.Col_SlctBd);
            }
            else if ((lock_y || IO.KeyShift) && select_bb.GetWidth() > 2) {
                DrawList.AddRectFilled(ImVec2(select_bb.Min.x, gp.BB_Grid.Min.y), ImVec2(select_bb.Max.x, gp.BB_Grid.Max.y), gp.Col_SlctBg);
                DrawList.AddRect(      ImVec2(select_bb.Min.x, gp.BB_Grid.Min.y), ImVec2(select_bb.Max.x, gp.BB_Grid.Max.y), gp.Col_SlctBd);
            }
            else if (select_bb.GetWidth() > 2 && select_bb.GetHeight() > 2) {
                DrawList.AddRectFilled(select_bb.Min, select_bb.Max, gp.Col_SlctBg);
                DrawList.AddRect(      select_bb.Min, select_bb.Max, gp.Col_SlctBd);
            }
        }
    }

    if (plot.Querying || (HasFlag(plot.Flags, ImPlotFlags_PixelQuery) && plot.Queried)) {
        if (plot.QueryRect.GetWidth() > 2 && plot.QueryRect.GetHeight() > 2) {
            DrawList.AddRectFilled(plot.QueryRect.Min + gp.BB_Grid.Min, plot.QueryRect.Max + gp.BB_Grid.Min, gp.Col_QryBg);
            DrawList.AddRect(      plot.QueryRect.Min + gp.BB_Grid.Min, plot.QueryRect.Max + gp.BB_Grid.Min, gp.Col_QryBd);
        }  
    }
    else if (plot.Queried) {
        ImVec2 p1 = PlotToPixels(plot.QueryRange.XMin, plot.QueryRange.YMin); 
        ImVec2 p2 = PlotToPixels(plot.QueryRange.XMax, plot.QueryRange.YMax);
        ImVec2 Min(ImMin(p1.x,p2.x), ImMin(p1.y,p2.y));
        ImVec2 Max(ImMax(p1.x,p2.x), ImMax(p1.y,p2.y));
        DrawList.AddRectFilled(Min, Max, gp.Col_QryBg);
        DrawList.AddRect(      Min, Max, gp.Col_QryBd);
    }

    // render legend
    const float txt_ht = GetTextLineHeight();
    const ImVec2 legend_offset(10, 10);
    const ImVec2 legend_padding(5, 5);
    const float  legend_icon_size = txt_ht;
    ImRect legend_content_bb;
    int nItems = GetLegendCount();
    bool hov_legend = false;
    if (HasFlag(plot.Flags, ImPlotFlags_Legend) && nItems > 0) {
        // get max width
        float max_label_width = 0;
        for (int i = 0; i < nItems; ++i) {
            const char* label = GetLegendLabel(i);
            ImVec2 labelWidth = CalcTextSize(label, NULL, true);
            max_label_width   = labelWidth.x > max_label_width ? labelWidth.x : max_label_width;
        }
        legend_content_bb = ImRect(gp.BB_Grid.Min + legend_offset, gp.BB_Grid.Min + legend_offset + ImVec2(max_label_width, nItems * txt_ht));
        plot.BB_Legend    = ImRect(legend_content_bb.Min, legend_content_bb.Max + legend_padding * 2 + ImVec2(legend_icon_size, 0));
        hov_legend = HasFlag(plot.Flags, ImPlotFlags_Legend) ? gp.Hov_Frame && plot.BB_Legend.Contains(IO.MousePos) : false;
        // render legend box
        DrawList.AddRectFilled(plot.BB_Legend.Min, plot.BB_Legend.Max, GetColorU32(ImGuiCol_PopupBg));
        DrawList.AddRect(plot.BB_Legend.Min, plot.BB_Legend.Max, gp.Col_Border);
        // render each legend item
        for (int i = 0; i < nItems; ++i) {
            ImPlotItem* item = GetLegendItem(i);
            ImRect icon_bb;
            icon_bb.Min = legend_content_bb.Min + legend_padding + ImVec2(0, i * txt_ht) + ImVec2(2, 2);
            icon_bb.Max = legend_content_bb.Min + legend_padding + ImVec2(0, i * txt_ht) + ImVec2(legend_icon_size - 2, legend_icon_size - 2);
            ImRect label_bb;
            label_bb.Min = legend_content_bb.Min + legend_padding + ImVec2(0, i * txt_ht) + ImVec2(2, 2);
            label_bb.Max = legend_content_bb.Min + legend_padding + ImVec2(0, i * txt_ht) + ImVec2(legend_content_bb.Max.x, legend_icon_size - 2);
            ImU32 col_hl_txt;
            if (HasFlag(plot.Flags, ImPlotFlags_Highlight) && hov_legend && (icon_bb.Contains(IO.MousePos) || label_bb.Contains(IO.MousePos))) {
                item->Highlight = true;
                col_hl_txt = GetColorU32(ImLerp(G.Style.Colors[ImGuiCol_Text], item->Color, 0.25f));
            }
            else
                item->Highlight = false;
            ImU32 iconColor;
            if (hov_legend && icon_bb.Contains(IO.MousePos)) {
                ImVec4 colAlpha = item->Color;
                colAlpha.w    = 0.5f;
                iconColor     = item->Show ? GetColorU32(colAlpha)
                                          : GetColorU32(ImGuiCol_TextDisabled, 0.5f);
                if (IO.MouseClicked[0])
                    item->Show = !item->Show;
            } else {
                iconColor = item->Show ? GetColorU32(item->Color) : gp.Col_TxtDis;
            }
            DrawList.AddRectFilled(icon_bb.Min, icon_bb.Max, iconColor, 1);
            const char* label = GetLegendLabel(i);
            const char* text_display_end = FindRenderedTextEnd(label, NULL);
            if (label != text_display_end)
                DrawList.AddText(legend_content_bb.Min + legend_padding + ImVec2(legend_icon_size, i * txt_ht), 
                                 item->Show ? (item->Highlight ? col_hl_txt : gp.Col_Txt) : gp.Col_TxtDis, label, text_display_end);
        }
    }

    // render crosshairs
    if (HasFlag(plot.Flags, ImPlotFlags_Crosshairs) && gp.Hov_Grid && gp.Hov_Frame &&
        !(plot.XAxis.Dragging || plot.YAxis.Dragging) && !plot.Selecting && !plot.Querying && !hov_legend) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_None);
        ImVec2 xy = IO.MousePos;
        ImVec2 h1(gp.BB_Grid.Min.x, xy.y);
        ImVec2 h2(xy.x - 5, xy.y);
        ImVec2 h3(xy.x + 5, xy.y);
        ImVec2 h4(gp.BB_Grid.Max.x, xy.y);
        ImVec2 v1(xy.x, gp.BB_Grid.Min.y);
        ImVec2 v2(xy.x, xy.y - 5);
        ImVec2 v3(xy.x, xy.y + 5);
        ImVec2 v4(xy.x, gp.BB_Grid.Max.y);
        DrawList.AddLine(h1, h2, gp.Col_Border);
        DrawList.AddLine(h3, h4, gp.Col_Border);
        DrawList.AddLine(v1, v2, gp.Col_Border);
        DrawList.AddLine(v3, v4, gp.Col_Border);
    }

    // render mouse pos
    if (HasFlag(plot.Flags, ImPlotFlags_MousePos) && gp.Hov_Grid) {
        static char buffer[128];
        sprintf(buffer, "%.2f,%.2f", gp.LastMousePos.x, gp.LastMousePos.y);
        ImVec2 size = CalcTextSize(buffer);
        ImVec2 pos  = gp.BB_Grid.Max - size - ImVec2(5, 5);
        DrawList.AddText(pos, gp.Col_Txt, buffer);
    }

    PopPlotClipRect();

    // render border
    DrawList.AddRect(gp.BB_Grid.Min, gp.BB_Grid.Max, gp.Col_Border);

    // FIT DATA --------------------------------------------------------------

    if (gp.FitThisFrame && (gp.VisibleItemCount > 0 || plot.Queried)) {
        if (gp.FitX && !HasFlag(plot.XAxis.Flags, ImAxisFlags_LockMin) && !NanOrInf(gp.Extents.Min.x))
            plot.XAxis.Min = gp.Extents.Min.x;
        if (gp.FitX && !HasFlag(plot.XAxis.Flags, ImAxisFlags_LockMax) && !NanOrInf(gp.Extents.Max.x))
            plot.XAxis.Max = gp.Extents.Max.x;
        if (gp.FitY && !HasFlag(plot.YAxis.Flags, ImAxisFlags_LockMin) && !NanOrInf(gp.Extents.Min.y))
            plot.YAxis.Min = gp.Extents.Min.y;
        if (gp.FitY && !HasFlag(plot.YAxis.Flags, ImAxisFlags_LockMax) && !NanOrInf(gp.Extents.Max.y))
            plot.YAxis.Max = gp.Extents.Max.y;        
    }

    // CONTEXT MENU -----------------------------------------------------------

    if (HasFlag(plot.Flags, ImPlotFlags_ContextMenu) && gp.Hov_Frame && gp.Hov_Grid && IO.MouseDoubleClicked[1] && !hov_legend)
        ImGui::OpenPopup("##Context");
    if (ImGui::BeginPopup("##Context")) {
        PlotContextMenu(plot);
        ImGui::EndPopup();
    }
    // CLEANUP ----------------------------------------------------------------

    // Reset legend items
    gp.LegendIndices.shrink(0);
    // Null current plot/data
    gp.CurrentPlot = NULL;
    // Reset next plot data
    gp.NextPlotData = ImNextPlotData();
    // Pop PushID at the end of BeginPlot
    PopID(); 
    // End child window
    if (!HasFlag(plot.Flags, ImPlotFlags_NoChild))
        ImGui::EndChild();
}

//-----------------------------------------------------------------------------
// MISC API
//-----------------------------------------------------------------------------

void SetNextPlotRange(float x_min, float x_max, float y_min, float y_max, ImGuiCond cond) {
    SetNextPlotRangeX(x_min, x_max, cond);
    SetNextPlotRangeY(y_min, y_max, cond);
}

void SetNextPlotRangeX(float x_min, float x_max, ImGuiCond cond) {
    IM_ASSERT(cond == 0 || ImIsPowerOfTwo(cond)); // Make sure the user doesn't attempt to combine multiple condition flags.
    gp.NextPlotData.HasXRange = true;
    gp.NextPlotData.XRangeCond = cond;
    gp.NextPlotData.XMin = x_min;
    gp.NextPlotData.XMax = x_max;
}

void SetNextPlotRangeY(float y_min, float y_max, ImGuiCond cond) {
    IM_ASSERT(cond == 0 || ImIsPowerOfTwo(cond)); // Make sure the user doesn't attempt to combine multiple condition flags.
    gp.NextPlotData.HasYRange = true;
    gp.NextPlotData.YRangeCond = cond;
    gp.NextPlotData.YMin = y_min;
    gp.NextPlotData.YMax = y_max;
}

ImVec2 GetPlotPos() {
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "GetPlotPos() Needs to be called between BeginPlot() and EndPlot()!");
    return gp.BB_Grid.Min;
}

ImVec2 GetPlotSize() {
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "GetPlotSize() Needs to be called between BeginPlot() and EndPlot()!");
    return gp.BB_Grid.GetSize();
}

void PushPlotClipRect() {
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PushPlotClipRect() Needs to be called between BeginPlot() and EndPlot()!");
    PushClipRect(gp.BB_Grid.Min, gp.BB_Grid.Max, true);
}

void PopPlotClipRect() {
    PopClipRect();
}

bool IsPlotHovered() { 
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "IsPlotHovered() Needs to be called between BeginPlot() and EndPlot()!");
    return gp.Hov_Grid; 
}
ImVec2 GetPlotMousePos() { 
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "GetPlotMousePos() Needs to be called between BeginPlot() and EndPlot()!");
    return gp.LastMousePos; 
}


ImPlotRange GetPlotRange() {
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "GetPlotRange() Needs to be called between BeginPlot() and EndPlot()!");
    ImPlot& plot = *gp.CurrentPlot;
    ImPlotRange range;
    range.XMin = plot.XAxis.Min;
    range.XMax = plot.XAxis.Max;
    range.YMin = plot.YAxis.Min;
    range.YMax = plot.YAxis.Max;
    return range;
}

bool IsPlotQueried() {
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "IsPlotQueried() Needs to be called between BeginPlot() and EndPlot()!");
    return gp.CurrentPlot->Queried;
}

ImPlotRange GetPlotQuery() {
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "GetPlotQuery() Needs to be called between BeginPlot() and EndPlot()!");
    ImPlot& plot = *gp.CurrentPlot;
    if (HasFlag(plot.Flags, ImPlotFlags_PixelQuery)) {
        UpdateTransformCache();
        ImVec2 p1 = PixelsToPlot(plot.QueryRect.Min + gp.BB_Grid.Min);
        ImVec2 p2 = PixelsToPlot(plot.QueryRect.Max + gp.BB_Grid.Min);
        plot.QueryRange.XMin = ImMin(p1.x, p2.x);
        plot.QueryRange.XMax = ImMax(p1.x, p2.x);
        plot.QueryRange.YMin = ImMin(p1.y, p2.y);
        plot.QueryRange.YMax = ImMax(p1.y, p2.y);
    }    
    return plot.QueryRange;
}

//-----------------------------------------------------------------------------
// STYLING
//-----------------------------------------------------------------------------

struct ImPlotStyleVarInfo {
    ImGuiDataType   Type;
    ImU32           Count;
    ImU32           Offset;
    void*           GetVarPtr(ImPlotStyle* style) const { return (void*)((unsigned char*)style + Offset); }
};

static const ImPlotStyleVarInfo GPlotStyleVarInfo[] = 
{
    { ImGuiDataType_Float, 1, (ImU32)IM_OFFSETOF(ImPlotStyle, LineWeight)         }, // ImPlotStyleVar_LineWeight
    { ImGuiDataType_S32,   1, (ImU32)IM_OFFSETOF(ImPlotStyle, Marker)             }, // ImPlotStyleVar_Marker
    { ImGuiDataType_Float, 1, (ImU32)IM_OFFSETOF(ImPlotStyle, MarkerSize)         }, // ImPlotStyleVar_MarkerSize
    { ImGuiDataType_Float, 1, (ImU32)IM_OFFSETOF(ImPlotStyle, MarkerWeight)       }, // ImPlotStyleVar_MarkerWeight
    { ImGuiDataType_Float, 1, (ImU32)IM_OFFSETOF(ImPlotStyle, ErrorBarSize)       }, // ImPlotStyleVar_ErrorBarSize
    { ImGuiDataType_Float, 1, (ImU32)IM_OFFSETOF(ImPlotStyle, ErrorBarWeight)     }, // ImPlotStyleVar_ErrorBarWeight
    { ImGuiDataType_Float, 1, (ImU32)IM_OFFSETOF(ImPlotStyle, DigitalBitHeight)   }  // ImPlotStyleVar_DigitalBitHeight
};

static const ImPlotStyleVarInfo* GetPlotStyleVarInfo(ImPlotStyleVar idx)
{
    IM_ASSERT(idx >= 0 && idx < ImPlotStyleVar_COUNT);
    IM_ASSERT(IM_ARRAYSIZE(GPlotStyleVarInfo) == ImPlotStyleVar_COUNT);
    return &GPlotStyleVarInfo[idx];
}

ImPlotStyle& GetPlotStyle() {
    return gp.Style;
}

void SetPlotPalette(const ImVec4* colors, int num_colors) {
    gp.ColorMap.shrink(0);
    gp.ColorMap.reserve(num_colors);
    for (int i = 0; i < num_colors; ++i) {
        gp.ColorMap.push_back(colors[i]);
    }
}

/// Returns the next unused default plot color
void RestorePlotPalette() {
    static ImVec4 default_colors[10] = {
        {(0.0F), (0.7490196228F), (1.0F), (1.0F)},                    // Blues::DeepSkyBlue,
        {(1.0F), (0.0F), (0.0F), (1.0F)},                             // Reds::Red,
        {(0.4980392158F), (1.0F), (0.0F), (1.0F)},                    // Greens::Chartreuse,
        {(1.0F), (1.0F), (0.0F), (1.0F)},                             // Yellows::Yellow,
        {(0.0F), (1.0F), (1.0F), (1.0F)},                             // Cyans::Cyan,
        {(1.0F), (0.6470588446F), (0.0F), (1.0F)},                    // Oranges::Orange,
        {(1.0F), (0.0F), (1.0F), (1.0F)},                             // Purples::Magenta,
        {(0.5411764979F), (0.1686274558F), (0.8862745166F), (1.0F)},  // Purples::BlueViolet,
        {(0.5f), (0.5f), (0.5f), (1.0F)},                             // Grays::Gray50,
        {(0.8235294223F), (0.7058823705F), (0.5490196347F), (1.0F)}   // Browns::Tan
    };
    SetPlotPalette(default_colors, 10);
}

void PushPlotColor(ImPlotCol idx, ImU32 col) {
    ImGuiColorMod backup;
    backup.Col = idx;
    backup.BackupValue = gp.Style.Colors[idx];
    gp.ColorModifiers.push_back(backup);
    gp.Style.Colors[idx] = ColorConvertU32ToFloat4(col);
}

void PushPlotColor(ImPlotCol idx, const ImVec4& col) {
    ImGuiColorMod backup;
    backup.Col = idx;
    backup.BackupValue = gp.Style.Colors[idx];
    gp.ColorModifiers.push_back(backup);
    gp.Style.Colors[idx] = col;
}

void PopPlotColor(int count) {
    while (count > 0)
    {
        ImGuiColorMod& backup = gp.ColorModifiers.back();
        gp.Style.Colors[backup.Col] = backup.BackupValue;
        gp.ColorModifiers.pop_back();
        count--;
    }
}

void PushPlotStyleVar(ImPlotStyleVar idx, float val) {
    const ImPlotStyleVarInfo* var_info = GetPlotStyleVarInfo(idx);
    if (var_info->Type == ImGuiDataType_Float && var_info->Count == 1) {
        float* pvar = (float*)var_info->GetVarPtr(&gp.Style);
        gp.StyleModifiers.push_back(ImGuiStyleMod(idx, *pvar));
        *pvar = val;
        return;
    }
    IM_ASSERT(0 && "Called PushPlotStyleVar() float variant but variable is not a float!");
}

void PushPlotStyleVar(ImPlotStyleVar idx, int val) {
    const ImPlotStyleVarInfo* var_info = GetPlotStyleVarInfo(idx);
    if (var_info->Type == ImGuiDataType_S32 && var_info->Count == 1) {
        int* pvar = (int*)var_info->GetVarPtr(&gp.Style);
        gp.StyleModifiers.push_back(ImGuiStyleMod(idx, *pvar));
        *pvar = val;
        return;
    }
    else if (var_info->Type == ImGuiDataType_Float && var_info->Count == 1) {
        float* pvar = (float*)var_info->GetVarPtr(&gp.Style);
        gp.StyleModifiers.push_back(ImGuiStyleMod(idx, *pvar));
        *pvar = (float)val;
        return;
    }
    IM_ASSERT(0 && "Called PushPlotStyleVar() int variant but variable is not a int!");
}

void PopPlotStyleVar(int count) {
    while (count > 0) {
        ImGuiStyleMod& backup = gp.StyleModifiers.back();
        const ImPlotStyleVarInfo* info = GetPlotStyleVarInfo(backup.VarIdx);
        void* data = info->GetVarPtr(&gp.Style);
        if (info->Type == ImGuiDataType_Float && info->Count == 1) {
            ((float*)data)[0] = backup.BackupFloat[0];
        }
        else if (info->Type == ImGuiDataType_Float && info->Count == 2) {
             ((float*)data)[0] = backup.BackupFloat[0]; 
             ((float*)data)[1] = backup.BackupFloat[1];   
        }
        else if (info->Type == ImGuiDataType_S32 && info->Count == 1) {
            ((int*)data)[0] = backup.BackupInt[0];
        }
        gp.StyleModifiers.pop_back();
        count--;
    }
}

//-----------------------------------------------------------------------------
// RENDERING FUNCTIONS
//-----------------------------------------------------------------------------

#define SQRT_1_2 0.70710678118f
#define SQRT_3_2 0.86602540378f

inline void TransformMarker(ImVec2* points, int n, const ImVec2& c, float s) {
    for (int i = 0; i < n; ++i) {
        points[i].x = c.x + points[i].x * s;
        points[i].y = c.y + points[i].y * s;
    }
}

inline void MarkerGeneral(ImDrawList& DrawList, ImVec2* points, int n, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    TransformMarker(points, n, c, s);
    if (fill)
        DrawList.AddConvexPolyFilled(points, n, col_fill);
    if (outline && !(fill && col_outline == col_fill)) {
        for (int i = 0; i < n; ++i)
            DrawList.AddLine(points[i], points[(i+1)%n], col_outline, weight);
    }
}

inline void MakerCircle(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[10] = {{1.0f, 0.0f},
                         {0.809017f, 0.58778524f},
                         {0.30901697f, 0.95105654f},
                         {-0.30901703f, 0.9510565f},
                         {-0.80901706f, 0.5877852f},
                         {-1.0f, 0.0f},
                         {-0.80901694f, -0.58778536f},
                         {-0.3090171f, -0.9510565f},
                         {0.30901712f, -0.9510565f},
                         {0.80901694f, -0.5877853f}};
    MarkerGeneral(DrawList, marker, 10, c, s, outline, col_outline, fill, col_fill, weight);
}

inline void MarkerDiamond(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[4] = {{1, 0}, {0, -1}, {-1, 0}, {0, 1}};
    MarkerGeneral(DrawList, marker, 4, c, s, outline, col_outline, fill, col_fill, weight);
}

inline void MarkerSquare(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[4] = {{SQRT_1_2,SQRT_1_2},{SQRT_1_2,-SQRT_1_2},{-SQRT_1_2,-SQRT_1_2},{-SQRT_1_2,SQRT_1_2}};
    MarkerGeneral(DrawList, marker, 4, c, s, outline, col_outline, fill, col_fill, weight);
}

inline void MarkerUp(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[3] = {{SQRT_3_2,0.5f},{0,-1},{-SQRT_3_2,0.5f}};
    MarkerGeneral(DrawList, marker, 3, c, s, outline, col_outline, fill, col_fill, weight);
}

inline void MarkerDown(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[3] = {{SQRT_3_2,-0.5f},{0,1},{-SQRT_3_2,-0.5f}};
    MarkerGeneral(DrawList, marker, 3, c, s, outline, col_outline, fill, col_fill, weight);
}

inline void MarkerLeft(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[3] = {{-1,0}, {0.5, SQRT_3_2}, {0.5, -SQRT_3_2}};
    MarkerGeneral(DrawList, marker, 3, c, s, outline, col_outline, fill, col_fill, weight);
}

inline void MarkerRight(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[3] = {{1,0}, {-0.5, SQRT_3_2}, {-0.5, -SQRT_3_2}};
    MarkerGeneral(DrawList, marker, 3, c, s, outline, col_outline, fill, col_fill, weight);
}

inline void MarkerAsterisk(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[6] = {{SQRT_3_2, 0.5f}, {0, -1}, {-SQRT_3_2, 0.5f}, {SQRT_3_2, -0.5f}, {0, 1},  {-SQRT_3_2, -0.5f}};
    TransformMarker(marker, 6, c, s);
    DrawList.AddLine(marker[0], marker[5], col_outline, weight);
    DrawList.AddLine(marker[1], marker[4], col_outline, weight);
    DrawList.AddLine(marker[2], marker[3], col_outline, weight);
}

inline void MarkerPlus(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[4] = {{1, 0}, {0, -1}, {-1, 0}, {0, 1}};
    TransformMarker(marker, 4, c, s);
    DrawList.AddLine(marker[0], marker[2], col_outline, weight);
    DrawList.AddLine(marker[1], marker[3], col_outline, weight);
}

inline void MarkerCross(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[4] = {{SQRT_1_2,SQRT_1_2},{SQRT_1_2,-SQRT_1_2},{-SQRT_1_2,-SQRT_1_2},{-SQRT_1_2,SQRT_1_2}};
    TransformMarker(marker, 4, c, s);
    DrawList.AddLine(marker[0], marker[2], col_outline, weight);
    DrawList.AddLine(marker[1], marker[3], col_outline, weight);
}

template <typename Transformer, typename Getter>
inline void RenderMarkers(ImDrawList& DrawList, Getter getter, int count, int offset, bool rend_mk_line, ImU32 col_mk_line, bool rend_mk_fill, ImU32 col_mk_fill, bool cull) {
int idx = offset;
    for (int i = 0; i < count; ++i) {      
        ImVec2 c;
        c = Transformer::Transform(getter(idx));
        idx = (idx + 1) % count;
        if (!cull || gp.BB_Grid.Contains(c)) {
            // TODO: Optimize the loop and if statements, this is atrocious
            if (HasFlag(gp.Style.Marker, ImMarker_Circle)) 
                MakerCircle(DrawList, c, gp.Style.MarkerSize, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, gp.Style.MarkerWeight);       
            if (HasFlag(gp.Style.Marker, ImMarker_Square))
                MarkerSquare(DrawList, c, gp.Style.MarkerSize, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, gp.Style.MarkerWeight);     
            if (HasFlag(gp.Style.Marker, ImMarker_Diamond)) 
                MarkerDiamond(DrawList, c, gp.Style.MarkerSize, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, gp.Style.MarkerWeight);
            if (HasFlag(gp.Style.Marker, ImMarker_Up))
                MarkerUp(DrawList, c, gp.Style.MarkerSize, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, gp.Style.MarkerWeight);     
            if (HasFlag(gp.Style.Marker, ImMarker_Down))    
                MarkerDown(DrawList, c, gp.Style.MarkerSize, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, gp.Style.MarkerWeight);  
            if (HasFlag(gp.Style.Marker, ImMarker_Left))
                MarkerLeft(DrawList, c, gp.Style.MarkerSize, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, gp.Style.MarkerWeight);     
            if (HasFlag(gp.Style.Marker, ImMarker_Right))    
                MarkerRight(DrawList, c, gp.Style.MarkerSize, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, gp.Style.MarkerWeight);  
            if (HasFlag(gp.Style.Marker, ImMarker_Cross))
                MarkerCross(DrawList, c, gp.Style.MarkerSize, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, gp.Style.MarkerWeight); 
            if (HasFlag(gp.Style.Marker, ImMarker_Plus))
                MarkerPlus(DrawList, c, gp.Style.MarkerSize, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, gp.Style.MarkerWeight); 
            if (HasFlag(gp.Style.Marker, ImMarker_Asterisk))
                MarkerAsterisk(DrawList, c, gp.Style.MarkerSize, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, gp.Style.MarkerWeight);  
        }
    }  
}

inline void RenderLine(ImDrawList& DrawList, const ImVec2& p1, const ImVec2& p2, float line_weight, ImU32 col_line, ImVec2 uv) {
    // http://assemblyrequired.crashworks.org/timing-square-root/
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    IM_NORMALIZE2F_OVER_ZERO(dx, dy);
    dx *= (line_weight * 0.5f);
    dy *= (line_weight * 0.5f);                    
    DrawList._VtxWritePtr[0].pos.x = p1.x + dy;
    DrawList._VtxWritePtr[0].pos.y = p1.y - dx;
    DrawList._VtxWritePtr[0].uv    = uv;
    DrawList._VtxWritePtr[0].col   = col_line;
    DrawList._VtxWritePtr[1].pos.x = p2.x + dy;
    DrawList._VtxWritePtr[1].pos.y = p2.y - dx;
    DrawList._VtxWritePtr[1].uv    = uv;
    DrawList._VtxWritePtr[1].col   = col_line;
    DrawList._VtxWritePtr[2].pos.x = p2.x - dy;
    DrawList._VtxWritePtr[2].pos.y = p2.y + dx;
    DrawList._VtxWritePtr[2].uv    = uv;
    DrawList._VtxWritePtr[2].col   = col_line;
    DrawList._VtxWritePtr[3].pos.x = p1.x - dy;
    DrawList._VtxWritePtr[3].pos.y = p1.y + dx;
    DrawList._VtxWritePtr[3].uv    = uv;
    DrawList._VtxWritePtr[3].col   = col_line;
    DrawList._VtxWritePtr += 4;
    DrawList._IdxWritePtr[0] = (ImDrawIdx)(DrawList._VtxCurrentIdx);
    DrawList._IdxWritePtr[1] = (ImDrawIdx)(DrawList._VtxCurrentIdx + 1);
    DrawList._IdxWritePtr[2] = (ImDrawIdx)(DrawList._VtxCurrentIdx + 2);
    DrawList._IdxWritePtr[3] = (ImDrawIdx)(DrawList._VtxCurrentIdx);
    DrawList._IdxWritePtr[4] = (ImDrawIdx)(DrawList._VtxCurrentIdx + 2);
    DrawList._IdxWritePtr[5] = (ImDrawIdx)(DrawList._VtxCurrentIdx + 3);
    DrawList._IdxWritePtr += 6;
    DrawList._VtxCurrentIdx += 4;
}

inline void RenderLineAA(ImDrawList& DrawList, const ImVec2& p1, const ImVec2& p2, float line_weight, ImU32 col_line) {
    DrawList.AddLine(p1, p2, col_line, line_weight);
}

template <typename Transformer, typename Getter>
inline void RenderLines(ImDrawList& DrawList, Getter getter, int count, int offset, float line_weight, ImU32 col_line, bool cull) {
// render line segments
    const int    segments  = count - 1;
    int    i1 = offset;
    ImVec2 p1, p2;
    if (HasFlag(gp.CurrentPlot->Flags, ImPlotFlags_AntiAliased)) {
        for (int s = 0; s < segments; ++s) {
            const int i2 = (i1 + 1) % count;
            p1 = Transformer::Transform(getter(i1));
            p2 = Transformer::Transform(getter(i2));
            i1 = i2;
            if (!cull || gp.BB_Grid.Contains(p1) || gp.BB_Grid.Contains(p2))
                RenderLineAA(DrawList, p1, p2, line_weight, col_line);
        }
    }
    else {
        const ImVec2 uv = DrawList._Data->TexUvWhitePixel;
        DrawList.PrimReserve(segments * 6, segments * 4);
        int segments_culled = 0;
        for (int s = 0; s < segments; ++s) {
            const int i2 = (i1 + 1) % count;
            p1 = Transformer::Transform(getter(i1));
            p2 = Transformer::Transform(getter(i2));
            i1 = i2;
            if (!cull || gp.BB_Grid.Contains(p1) || gp.BB_Grid.Contains(p2)) 
                RenderLine(DrawList, p1, p2, line_weight, col_line, uv);                
            else 
                segments_culled++;                
        }
        if (segments_culled > 0) 
            DrawList.PrimUnreserve(segments_culled * 6, segments_culled * 4); 
    }    
}

//-----------------------------------------------------------------------------
// DATA GETTERS
//-----------------------------------------------------------------------------

inline float StrideIndex(const float* data, int idx, int stride) {
    return *(const float*)(const void*)((const unsigned char*)data + (size_t)idx * stride);
}

struct GetterYs {
    GetterYs(const float* ys, int stride) {  Ys = ys; Stride = stride; }
    const float* Ys;
    int Stride;
    inline ImVec2 operator()(int idx) {
        return ImVec2((float)idx, StrideIndex(Ys, idx, Stride));
    }
};

struct Getter2D {
    Getter2D(const float* xs, const float* ys, int stride) { Xs = xs; Ys = ys; Stride = stride; }
    const float* Xs;
    const float* Ys;
    int Stride;
    inline ImVec2 operator()(int idx) {
        return ImVec2(StrideIndex(Xs, idx, Stride), StrideIndex(Ys, idx, Stride));
    }
};

struct GetterImVec2 {
    GetterImVec2(const ImVec2* data) { Data = data; }
    inline ImVec2 operator()(int idx) { return Data[idx]; }
    const ImVec2* Data;
};

struct GetterFuncPtrImVec2 {
    GetterFuncPtrImVec2(ImVec2 (*g)(void* data, int idx), void* d) { getter = g; data = d;}
    ImVec2 operator()(int idx) { return getter(data, idx); }
    ImVec2 (*getter)(void* data, int idx);
    void* data;
};

struct GetterFuncPtrImVec4 {
    GetterFuncPtrImVec4(ImVec4 (*g)(void* data, int idx), void* d) { getter = g; data = d;}
    ImVec4 operator()(int idx) { return getter(data, idx); }
    ImVec4 (*getter)(void* data, int idx);
    void* data;
};

//-----------------------------------------------------------------------------
// PLOT
//-----------------------------------------------------------------------------

template <typename Getter>
inline void PlotEx(const char* label_id, Getter getter, int count, int offset)
{
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "Plot() Needs to be called between BeginPlot() and EndPlot()!");

    ImPlotItem* item = RegisterItem(label_id);
    if (!item->Show)
        return;

    ImDrawList & DrawList = *ImGui::GetWindowDrawList();

    const bool rend_line    = gp.Style.Colors[ImPlotCol_Line].w != 0          && gp.Style.LineWeight > 0;
    const bool rend_mk_line = gp.Style.Colors[ImPlotCol_MarkerOutline].w != 0 && gp.Style.MarkerWeight > 0;
    const bool rend_mk_fill = gp.Style.Colors[ImPlotCol_MarkerFill].w != 0;

    ImU32 col_line    = gp.Style.Colors[ImPlotCol_Line].w == -1 ? GetColorU32(item->Color) : GetColorU32(gp.Style.Colors[ImPlotCol_Line]);
    ImU32 col_mk_line = gp.Style.Colors[ImPlotCol_MarkerOutline].w == -1 ? col_line        : GetColorU32(gp.Style.Colors[ImPlotCol_MarkerOutline]);
    ImU32 col_mk_fill = gp.Style.Colors[ImPlotCol_MarkerFill].w == -1 ?    col_line        : GetColorU32(gp.Style.Colors[ImPlotCol_MarkerFill]);

    const float line_weight = item->Highlight ? gp.Style.LineWeight * 2 : gp.Style.LineWeight;

    if (gp.Style.Colors[ImPlotCol_Line].w != -1)
        item->Color = gp.Style.Colors[ImPlotCol_Line];

    bool cull = HasFlag(gp.CurrentPlot->Flags, ImPlotFlags_CullData);

    // find data extents
    if (gp.FitThisFrame) {
        for (int i = 0; i < count; ++i) {
            ImVec2 p = getter(i);
            FitPoint(p);
        }
    }
    PushPlotClipRect();
    if (count > 1 && rend_line) {
        if (HasFlag(gp.CurrentPlot->XAxis.Flags, ImAxisFlags_LogScale) && HasFlag(gp.CurrentPlot->YAxis.Flags, ImAxisFlags_LogScale))
            RenderLines<Plt2PixLogLog>(DrawList, getter, count, offset, line_weight, col_line, cull);
        else if (HasFlag(gp.CurrentPlot->XAxis.Flags, ImAxisFlags_LogScale))
            RenderLines<Plt2PixLogLin>(DrawList, getter, count, offset, line_weight, col_line, cull);
        else if (HasFlag(gp.CurrentPlot->YAxis.Flags, ImAxisFlags_LogScale))
            RenderLines<Plt2PixLinLog>(DrawList, getter, count, offset, line_weight, col_line, cull);
        else
            RenderLines<Plt2PixLinLin>(DrawList, getter, count, offset, line_weight, col_line, cull);
    }
    // render markers
    if (gp.Style.Marker != ImMarker_None) {
        if (HasFlag(gp.CurrentPlot->XAxis.Flags, ImAxisFlags_LogScale) && HasFlag(gp.CurrentPlot->YAxis.Flags, ImAxisFlags_LogScale))
            RenderMarkers<Plt2PixLogLog>(DrawList, getter, count, offset, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, cull); 
        else if (HasFlag(gp.CurrentPlot->XAxis.Flags, ImAxisFlags_LogScale))
            RenderMarkers<Plt2PixLogLin>(DrawList, getter, count, offset, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, cull); 
        else if (HasFlag(gp.CurrentPlot->YAxis.Flags, ImAxisFlags_LogScale))
            RenderMarkers<Plt2PixLinLog>(DrawList, getter, count, offset, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, cull); 
        else
            RenderMarkers<Plt2PixLinLin>(DrawList, getter, count, offset, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, cull); 
    }
    PopPlotClipRect();
}

void Plot(const char* label_id, const float* values, int count, int offset, int stride) {
    GetterYs getter(values,stride);
    PlotEx(label_id, getter, count, offset);
}

void Plot(const char* label_id, const float* xs, const float* ys, int count, int offset, int stride) {
    Getter2D getter(xs,ys,stride);
    return PlotEx(label_id, getter, count, offset);
}

void Plot(const char* label_id, const ImVec2* data, int count, int offset) {
    GetterImVec2 getter(data);
    return PlotEx(label_id, getter, count, offset);
}

void Plot(const char* label_id, ImVec2 (*getter_func)(void* data, int idx), void* data, int count, int offset) {
    GetterFuncPtrImVec2 getter(getter_func,data);
    return PlotEx(label_id, getter, count, offset);
}

//-----------------------------------------------------------------------------
// PLOT BAR
//-----------------------------------------------------------------------------

struct GetterBarV {
    const float* Ys; float XShift; int Stride;
    GetterBarV(const float* ys, float xshift, int stride) { Ys = ys; XShift = xshift; Stride = stride; }
    inline ImVec2 operator()(int idx) { return ImVec2((float)idx + XShift, StrideIndex(Ys, idx, Stride)); }
};

struct GetterBarH {
    const float* Xs; float YShift; int Stride;
    GetterBarH(const float* xs, float yshift, int stride) { Xs = xs; YShift = yshift; Stride = stride; }
    inline ImVec2 operator()(int idx) { return ImVec2(StrideIndex(Xs, idx, Stride), (float)idx + YShift); }
};


template <typename Getter>
void PlotBarEx(const char* label_id, Getter getter, int count, float width, int offset) {

    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotBar() Needs to be called between BeginPlot() and EndPlot()!");

    ImPlotItem* item = RegisterItem(label_id);
    if (!item->Show)
        return;

    ImDrawList & DrawList = *GetWindowDrawList();

    bool rend_line = gp.Style.Colors[ImPlotCol_Line].w != 0 && gp.Style.LineWeight > 0;
    bool rend_fill = gp.Style.Colors[ImPlotCol_Fill].w != 0;

    ImU32 col_line = gp.Style.Colors[ImPlotCol_Line].w == -1 ? GetColorU32(item->Color) : GetColorU32(gp.Style.Colors[ImPlotCol_Line]);
    ImU32 col_fill = gp.Style.Colors[ImPlotCol_Fill].w == -1 ? col_line                 : GetColorU32(gp.Style.Colors[ImPlotCol_Fill]);

    if (rend_fill && col_line == col_fill)
        rend_line = false;

    if (gp.Style.Colors[ImPlotCol_Line].w != -1)
        item->Color = gp.Style.Colors[ImPlotCol_Line];

    PushPlotClipRect();

    float half_width = width * 0.5f;

    // find data extents
    if (gp.FitThisFrame) {
        for (int i = 0; i < count; ++i) {
            ImVec2 p = getter(i);
            FitPoint(ImVec2(p.x - half_width, p.y));
            FitPoint(ImVec2(p.x + half_width, 0));
        }
    }

    int idx = offset;
    for (int i = 0; i < count; ++i) {      
        ImVec2 p;
        p = getter(idx);
        idx = (idx + 1) % count;
        if (p.y == 0)
            continue;
        ImVec2 a = PlotToPixels(p.x - half_width, p.y);
        ImVec2 b = PlotToPixels(p.x + half_width, 0);
        if (rend_fill)
            DrawList.AddRectFilled(a, b, col_fill);
        if (rend_line)
            DrawList.AddRect(a, b, col_line);
    }
    PopPlotClipRect();
}

void PlotBar(const char* label_id, const float* values, int count, float width, float shift, int offset, int stride) {
    GetterBarV getter(values,shift,stride);
    PlotBarEx(label_id, getter, count, width, offset);
}

void PlotBar(const char* label_id, const float* xs, const float* ys, int count, float width, int offset, int stride) {
    Getter2D getter(xs,ys,stride);
    PlotBarEx(label_id, getter, count, width, offset);
}

void PlotBar(const char* label_id, ImVec2 (*getter_func)(void* data, int idx), void* data, int count, float width, int offset) {
    GetterFuncPtrImVec2 getter(getter_func, data);
    PlotBarEx(label_id, getter, count, width, offset);
}

//-----------------------------------------------------------------------------

template <typename Getter>
void PlotBarHEx(const char* label_id, Getter getter, int count, float height,  int offset) {

    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotBarH() Needs to be called between BeginPlot() and EndPlot()!");

    ImPlotItem* item = RegisterItem(label_id);
    if (!item->Show)
        return;

    ImDrawList & DrawList = *GetWindowDrawList();

    bool rend_line = gp.Style.Colors[ImPlotCol_Line].w != 0 && gp.Style.LineWeight > 0;
    bool rend_fill = gp.Style.Colors[ImPlotCol_Fill].w != 0;

    ImU32 col_line = gp.Style.Colors[ImPlotCol_Line].w == -1 ? GetColorU32(item->Color) : GetColorU32(gp.Style.Colors[ImPlotCol_Line]);
    ImU32 col_fill = gp.Style.Colors[ImPlotCol_Fill].w == -1 ? col_line                 : GetColorU32(gp.Style.Colors[ImPlotCol_Fill]);

    if (rend_fill && col_line == col_fill)
        rend_line = false;

    if (gp.Style.Colors[ImPlotCol_Line].w != -1)
        item->Color = gp.Style.Colors[ImPlotCol_Line];

    PushPlotClipRect();

    float half_height = height * 0.5f;

    // find data extents
    if (gp.FitThisFrame) {
        for (int i = 0; i < count; ++i) {
            ImVec2 p = getter(i);
            FitPoint(ImVec2(0, p.y - half_height));
            FitPoint(ImVec2(p.x, p.y + half_height));
        }
    }

    int idx = offset;
    for (int i = 0; i < count; ++i) {      
        ImVec2 p;
        p = getter(idx);
        idx = (idx + 1) % count;
        if (p.x == 0)
            continue;
        ImVec2 a = PlotToPixels(0, p.y - half_height);
        ImVec2 b = PlotToPixels(p.x, p.y + half_height);
        if (rend_fill)
            DrawList.AddRectFilled(a, b, col_fill);
        if (rend_line)
            DrawList.AddRect(a, b, col_line);
    }
    PopPlotClipRect();
}

void PlotBarH(const char* label_id, const float* values, int count, float height, float shift, int offset, int stride) {
    GetterBarH getter(values,shift,stride);
    PlotBarHEx(label_id, getter, count, height, offset);
}

void PlotBarH(const char* label_id, const float* xs, const float* ys, int count, float height,  int offset, int stride) {
    Getter2D getter(xs,ys,stride);
    PlotBarHEx(label_id, getter, count, height, offset);
}

void PlotBarH(const char* label_id, ImVec2 (*getter_func)(void* data, int idx), void* data, int count, float height,  int offset) {
    GetterFuncPtrImVec2 getter(getter_func, data);
    PlotBarHEx(label_id, getter, count, height, offset);
}

//-----------------------------------------------------------------------------
// PLOT ERROR BARS
//-----------------------------------------------------------------------------

struct GetterError {
    const float* Xs; const float* Ys; const float* Neg; const float* Pos; int Stride;
    GetterError(const float* xs, const float* ys, const float* neg, const float* pos, int stride) {
        Xs = xs; Ys = ys; Neg = neg; Pos = pos; Stride = stride;
    }
    ImVec4 operator()(int idx) {
        return ImVec4(StrideIndex(Xs,  idx, Stride), 
                      StrideIndex(Ys,  idx, Stride),
                      StrideIndex(Neg, idx, Stride),
                      StrideIndex(Pos, idx, Stride));
    }
};

template <typename Getter>
void PlotErrorBarsEx(const char* label_id, Getter getter, int count, int offset) {
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotErrorBars() Needs to be called between BeginPlot() and EndPlot()!");

    ImGuiID id = GetID(label_id);
    ImPlotItem* item = gp.CurrentPlot->Items.GetByKey(id);
    if (item != NULL && item->Show == false)
        return;

    ImDrawList & DrawList = *GetWindowDrawList();

    PushPlotClipRect();

    const ImU32 col = gp.Style.Colors[ImPlotCol_ErrorBar].w == -1 ? GetColorU32(ImGuiCol_Text) : GetColorU32(gp.Style.Colors[ImPlotCol_ErrorBar]);
    const bool rend_whisker = gp.Style.ErrorBarSize > 0;

    const float half_whisker = gp.Style.ErrorBarSize * 0.5f;

    // find data extents
    if (gp.FitThisFrame) {
        for (int i = 0; i < count; ++i) {
            ImVec4 e = getter(i);
            FitPoint(ImVec2(e.x , e.y - e.z));
            FitPoint(ImVec2(e.x , e.y + e.w ));
        }
    }

    int idx = offset;
    for (int i = 0; i < count; ++i) {
        ImVec4 e;
        e = getter(idx);
        idx = (idx + 1) % count;
        ImVec2 p1 = PlotToPixels(e.x, e.y - e.z);
        ImVec2 p2 = PlotToPixels(e.x, e.y + e.w);
        DrawList.AddLine(p1,p2,col, gp.Style.ErrorBarWeight);
        if (rend_whisker) {
            DrawList.AddLine(p1 - ImVec2(half_whisker, 0), p1 + ImVec2(half_whisker, 0), col, gp.Style.ErrorBarWeight);
            DrawList.AddLine(p2 - ImVec2(half_whisker, 0), p2 + ImVec2(half_whisker, 0), col, gp.Style.ErrorBarWeight);
        }
    }
    PopPlotClipRect();
}

void PlotErrorBars(const char* label_id, const float* xs, const float* ys, const float* err, int count, int offset, int stride) {
    GetterError getter(xs, ys, err, err, stride);
    PlotErrorBarsEx(label_id, getter, count, offset);
}

void PlotErrorBars(const char* label_id, const float* xs, const float* ys, const float* neg, const float* pos, int count, int offset, int stride) {
    GetterError getter(xs, ys, neg, pos, stride);
    PlotErrorBarsEx(label_id, getter, count, offset);
}

void PlotErrorBars(const char* label_id, ImVec4 (*getter_func)(void* data, int idx), void* data, int count, int offset) {
    GetterFuncPtrImVec4 getter(getter_func, data);
    PlotErrorBarsEx(label_id, getter, count, offset);
}

//-----------------------------------------------------------------------------
// PLOT MISC
//-----------------------------------------------------------------------------

inline void DrawPieSlice(ImDrawList& DrawList, const ImVec2& center, float radius, float a0, float a1, ImU32 col) {
    static const float resolution = 50 / (2 * IM_PI);
    static ImVec2 buffer[50];
    buffer[0] = PlotToPixels(center);
    int n = ImMax(3, (int)((a1 - a0) * resolution));
    float da = (a1 - a0) / (n - 1);
    for (int i = 0; i < n; ++i) {
        float a = a0 + i * da;
        buffer[i + 1] = PlotToPixels(center.x + radius * cos(a), center.y + radius * sin(a));
    }
    DrawList.AddConvexPolyFilled(buffer, n + 1, col);
}


void PlotPieChart(const char** label_ids, float* values, int count, const ImVec2& center, float radius, bool show_percents, float angle0) {
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotPieChart() Needs to be called between BeginPlot() and EndPlot()!");
    ImDrawList & DrawList = *GetWindowDrawList();

    float sum = 0;
    for (int i = 0; i < count; ++i)
        sum += values[i];
    
    const bool normalize = sum > 1.0f;

    PushPlotClipRect();
    float a0 = angle0 * 2 * IM_PI / 360.0f;
    float a1 = angle0 * 2 * IM_PI / 360.0f;
    for (int i = 0; i < count; ++i) {
        ImPlotItem* item = RegisterItem(label_ids[i]);
        ImU32 col = GetColorU32(item->Color);
        float percent = normalize ? values[i] / sum : values[i];
        a1 = a0 + 2 * IM_PI * percent;
        if (item->Show) {
            if (percent < 0.5) {
                DrawPieSlice(DrawList, center, radius, a0, a1, col);
            }
            else  {
                DrawPieSlice(DrawList, center, radius, a0, a0 + (a1 - a0) * 0.5f, col);
                DrawPieSlice(DrawList, center, radius, a0 + (a1 - a0) * 0.5f, a1, col);
            }
            if (show_percents) {
                static char buffer[8];
                sprintf(buffer, "%.0f%%", percent * 100);
                ImVec2 size = CalcTextSize(buffer);
                float angle = a0 + (a1 - a0) * 0.5f;
                ImVec2 pos = PlotToPixels(center.x + 0.5f * radius * cos(angle), center.y + 0.5f * radius * sin(angle));
                DrawList.AddText(pos - size * 0.5f + ImVec2(1,1), IM_COL32(0,0,0,255), buffer);
                DrawList.AddText(pos - size * 0.5f, IM_COL32(255,255,255,255), buffer);
            }
        }
        a0 = a1;
    }   
    PopPlotClipRect();
}

void PlotLabel(const char* text, float x, float y, bool vertical, const ImVec2& pixel_offset) {
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotLabel() Needs to be called between BeginPlot() and EndPlot()!");
    ImDrawList & DrawList = *ImGui::GetWindowDrawList();
    PushPlotClipRect();
    ImVec2 pos = PlotToPixels({x,y}) + pixel_offset;
    if (vertical)
        AddTextVertical(&DrawList, text, pos, gp.Col_Txt);
    else
        DrawList.AddText(pos, gp.Col_Txt, text);
    PopPlotClipRect();
}

template <typename Getter>
inline void PlotDigitalEx(const char* label_id, Getter getter, int count, int offset)
{
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "Plot() Needs to be called between BeginPlot() and EndPlot()!");

    ImPlotItem* item = RegisterItem(label_id);
    if (!item->Show)
        return;

    ImDrawList & DrawList = *ImGui::GetWindowDrawList();

    const bool rend_line = gp.Style.Colors[ImPlotCol_Line].w != 0 && gp.Style.LineWeight > 0;

    if (gp.Style.Colors[ImPlotCol_Line].w != -1)
        item->Color = gp.Style.Colors[ImPlotCol_Line];

    // find data extents
    if (gp.FitThisFrame) {
        for (int i = 0; i < count; ++i) {
            ImVec2 p = getter(i);
            FitPoint(p);
        }
    }

    ImGui::PushClipRect(gp.BB_Grid.Min, gp.BB_Grid.Max, true);
    bool cull = HasFlag(gp.CurrentPlot->Flags, ImPlotFlags_CullData);

    const float line_weight = item->Highlight ? gp.Style.LineWeight * 2 : gp.Style.LineWeight;

    // render digital signals as "pixel bases" rectangles
    if (count > 1 && rend_line) {
        //
        const float mx = (gp.PixelRange.Max.x - gp.PixelRange.Min.x) / (gp.CurrentPlot->XAxis.Max - gp.CurrentPlot->XAxis.Min);
        int pixY_0 = line_weight;
        int pixY_1 = gp.Style.DigitalBitHeight;
        int pixY_Offset = 20;//20 pixel from bottom due to mouse cursor label
        int pixY_chOffset = pixY_1 + 3; //3 pixels between channels
        ImVec2 pMin, pMax;
        float y0 = (gp.PixelRange.Min.y) + ((-pixY_chOffset * gp.DigitalPlotItemCnt) - pixY_0 - pixY_Offset);
        float y1 = (gp.PixelRange.Min.y) + ((-pixY_chOffset * gp.DigitalPlotItemCnt) - pixY_1 - pixY_Offset);
        const int    segments  = count - 1;
        int    i1 = offset;
        for (int s = 0; s < segments; ++s) {
            const int i2 = (i1 + 1) % count;
            ImVec2 itemData1 = getter(i1);
            ImVec2 itemData2 = getter(i2);
            i1 = i2;
            pMin.x = gp.PixelRange.Min.x + mx * (itemData1.x - gp.CurrentPlot->XAxis.Min);
            pMin.y = (gp.PixelRange.Min.y) + ((-pixY_chOffset * gp.DigitalPlotItemCnt) - pixY_Offset);
            pMax.x = gp.PixelRange.Min.x + mx * (itemData2.x - gp.CurrentPlot->XAxis.Min);
            pMax.y = ((int) itemData1.y == 0) ? y0 : y1;
            //plot only one rectangle for same digital state
            while (((s+2) < segments) && ((int) itemData1.y == (int) itemData2.y)) {
                const int i2 = (i1 + 1) % count;
                itemData2 = getter(i2);
                pMax.x = gp.PixelRange.Min.x + mx * (itemData2.x - gp.CurrentPlot->XAxis.Min);
                i1 = i2;
                s++;
            } 
            //do not extend plot outside plot range
            if (pMin.x < gp.PixelRange.Min.x) pMin.x = gp.PixelRange.Min.x;
            if (pMax.x < gp.PixelRange.Min.x) pMax.x = gp.PixelRange.Min.x;
            if (pMin.x > gp.PixelRange.Max.x) pMin.x = gp.PixelRange.Max.x;
            if (pMax.x > gp.PixelRange.Max.x) pMax.x = gp.PixelRange.Max.x;
            //plot a rectangle that extends up to x2 with y1 height
            if ((pMax.x > pMin.x) && (!cull || gp.BB_Grid.Contains(pMin) || gp.BB_Grid.Contains(pMax))) {
                ImVec4 colAlpha = item->Color;
                colAlpha.w = item->Highlight ? 1.0 : 0.9;
                DrawList.AddRectFilled(pMin, pMax, GetColorU32(colAlpha));
            }
        }
        gp.DigitalPlotItemCnt++;
    }   

    ImGui::PopClipRect();
}

void PlotDigital(const char* label_id, const float* xs, const float* ys, int count, int offset, int stride) {
    Getter2D getter(xs,ys,stride);
    return PlotDigitalEx(label_id, getter, count, offset);
}

void PlotDigital(const char* label_id, ImVec2 (*getter_func)(void* data, int idx), void* data, int count, int offset) {
    GetterFuncPtrImVec2 getter(getter_func,data);
    return PlotDigitalEx(label_id, getter, count, offset);
}

}  // namespace ImGui
