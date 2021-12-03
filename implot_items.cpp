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

// ImPlot v0.13 WIP

#include "implot.h"
#include "implot_internal.h"

#ifdef _MSC_VER
#define sprintf sprintf_s
#endif

#define SQRT_1_2 0.70710678118f
#define SQRT_3_2 0.86602540378f

#define IMPLOT_NO_FORCE_INLINE
#ifndef IMPLOT_NO_FORCE_INLINE
    #ifdef _MSC_VER
        #define IMPLOT_INLINE __forceinline
    #elif defined(__GNUC__)
        #define IMPLOT_INLINE inline __attribute__((__always_inline__))
    #elif defined(__CLANG__)
        #if __has_attribute(__always_inline__)
            #define IMPLOT_INLINE inline __attribute__((__always_inline__))
        #else
            #define IMPLOT_INLINE inline
        #endif
    #else
        #define IMPLOT_INLINE inline
    #endif
#else
    #define IMPLOT_INLINE inline
#endif

#if defined __SSE__ || defined __x86_64__ || defined _M_X64
#ifndef IMGUI_ENABLE_SSE
#include <immintrin.h>
#endif
static IMPLOT_INLINE float  ImInvSqrt(float x) { return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x))); }
#else
static IMPLOT_INLINE float  ImInvSqrt(float x) { return 1.0f / sqrtf(x); }
#endif

#define IMPLOT_NORMALIZE2F_OVER_ZERO(VX,VY) do { float d2 = VX*VX + VY*VY; if (d2 > 0.0f) { float inv_len = ImInvSqrt(d2); VX *= inv_len; VY *= inv_len; } } while (0)

// Support for pre-1.82 versions. Users on 1.82+ can use 0 (default) flags to mean "all corners" but in order to support older versions we are more explicit.
#if (IMGUI_VERSION_NUM < 18102) && !defined(ImDrawFlags_RoundCornersAll)
#define ImDrawFlags_RoundCornersAll ImDrawCornerFlags_All
#endif

namespace ImPlot {

//-----------------------------------------------------------------------------
// Utils
//-----------------------------------------------------------------------------

// Calc maximum index size of ImDrawIdx
template <typename T>
struct MaxIdx { static const unsigned int Value; };
template <> const unsigned int MaxIdx<unsigned short>::Value = 65535;
template <> const unsigned int MaxIdx<unsigned int>::Value   = 4294967295;

//-----------------------------------------------------------------------------
// Item Utils
//-----------------------------------------------------------------------------

ImPlotItem* RegisterOrGetItem(const char* label_id, bool* just_created) {
    ImPlotContext& gp = *GImPlot;
    ImPlotItemGroup& Items = *gp.CurrentItems;
    ImGuiID id = Items.GetItemID(label_id);
    if (just_created != NULL)
        *just_created = Items.GetItem(id) == NULL;
    ImPlotItem* item = Items.GetOrAddItem(id);
    if (item->SeenThisFrame)
        return item;
    item->SeenThisFrame = true;
    int idx = Items.GetItemIndex(item);
    item->ID = id;
    if (ImGui::FindRenderedTextEnd(label_id, NULL) != label_id) {
        Items.Legend.Indices.push_back(idx);
        item->NameOffset = Items.Legend.Labels.size();
        Items.Legend.Labels.append(label_id, label_id + strlen(label_id) + 1);
    }
    else {
        item->Show = true;
    }
    return item;
}

ImPlotItem* GetItem(const char* label_id) {
    ImPlotContext& gp = *GImPlot;
    return gp.CurrentItems->GetItem(label_id);
}

bool IsItemHidden(const char* label_id) {
    ImPlotItem* item = GetItem(label_id);
    return item != NULL && !item->Show;
}

ImPlotItem* GetCurrentItem() {
    ImPlotContext& gp = *GImPlot;
    return gp.CurrentItem;
}

void SetNextLineStyle(const ImVec4& col, float weight) {
    ImPlotContext& gp = *GImPlot;
    gp.NextItemData.Colors[ImPlotCol_Line] = col;
    gp.NextItemData.LineWeight             = weight;
}

void SetNextFillStyle(const ImVec4& col, float alpha) {
    ImPlotContext& gp = *GImPlot;
    gp.NextItemData.Colors[ImPlotCol_Fill] = col;
    gp.NextItemData.FillAlpha              = alpha;
}

void SetNextMarkerStyle(ImPlotMarker marker, float size, const ImVec4& fill, float weight, const ImVec4& outline) {
    ImPlotContext& gp = *GImPlot;
    gp.NextItemData.Marker                          = marker;
    gp.NextItemData.Colors[ImPlotCol_MarkerFill]    = fill;
    gp.NextItemData.MarkerSize                      = size;
    gp.NextItemData.Colors[ImPlotCol_MarkerOutline] = outline;
    gp.NextItemData.MarkerWeight                    = weight;
}

void SetNextErrorBarStyle(const ImVec4& col, float size, float weight) {
    ImPlotContext& gp = *GImPlot;
    gp.NextItemData.Colors[ImPlotCol_ErrorBar] = col;
    gp.NextItemData.ErrorBarSize               = size;
    gp.NextItemData.ErrorBarWeight             = weight;
}

ImVec4 GetLastItemColor() {
    ImPlotContext& gp = *GImPlot;
    if (gp.PreviousItem)
        return ImGui::ColorConvertU32ToFloat4(gp.PreviousItem->Color);
    return ImVec4();
}

void BustItemCache() {
    ImPlotContext& gp = *GImPlot;
    for (int p = 0; p < gp.Plots.GetBufSize(); ++p) {
        ImPlotPlot& plot = *gp.Plots.GetByIndex(p);
        plot.Items.Reset();
    }
    for (int p = 0; p < gp.Subplots.GetBufSize(); ++p) {
        ImPlotSubplot& subplot = *gp.Subplots.GetByIndex(p);
        subplot.Items.Reset();
    }
}

void BustColorCache(const char* plot_title_id) {
    ImPlotContext& gp = *GImPlot;
    if (plot_title_id == NULL) {
        BustItemCache();
    }
    else {
        ImGuiID id = ImGui::GetCurrentWindow()->GetID(plot_title_id);
        ImPlotPlot* plot = gp.Plots.GetByKey(id);
        if (plot != NULL)
            plot->Items.Reset();
        else {
            ImPlotSubplot* subplot = gp.Subplots.GetByKey(id);
            if (subplot != NULL)
                subplot->Items.Reset();
        }
    }
}

//-----------------------------------------------------------------------------
// Begin/EndItem
//-----------------------------------------------------------------------------

static const float ITEM_HIGHLIGHT_LINE_SCALE = 2.0f;
static const float ITEM_HIGHLIGHT_MARK_SCALE = 1.25f;

// Begins a new item. Returns false if the item should not be plotted.
bool BeginItem(const char* label_id, ImPlotCol recolor_from) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotX() needs to be called between BeginPlot() and EndPlot()!");
    SetupLock();
    bool just_created;
    ImPlotItem* item = RegisterOrGetItem(label_id, &just_created);
    // set current item
    gp.CurrentItem = item;
    ImPlotNextItemData& s = gp.NextItemData;
    // set/override item color
    if (recolor_from != -1) {
        if (!IsColorAuto(s.Colors[recolor_from]))
            item->Color = ImGui::ColorConvertFloat4ToU32(s.Colors[recolor_from]);
        else if (!IsColorAuto(gp.Style.Colors[recolor_from]))
            item->Color = ImGui::ColorConvertFloat4ToU32(gp.Style.Colors[recolor_from]);
        else if (just_created)
            item->Color = NextColormapColorU32();
    }
    else if (just_created) {
        item->Color = NextColormapColorU32();
    }
    // hide/show item
    if (gp.NextItemData.HasHidden) {
        if (just_created || gp.NextItemData.HiddenCond == ImGuiCond_Always)
            item->Show = !gp.NextItemData.Hidden;
    }
    if (!item->Show) {
        // reset next item data
        gp.NextItemData.Reset();
        gp.PreviousItem = item;
        gp.CurrentItem  = NULL;
        return false;
    }
    else {
        ImVec4 item_color = ImGui::ColorConvertU32ToFloat4(item->Color);
        // stage next item colors
        s.Colors[ImPlotCol_Line]           = IsColorAuto(s.Colors[ImPlotCol_Line])          ? ( IsColorAuto(ImPlotCol_Line)           ? item_color                 : gp.Style.Colors[ImPlotCol_Line]          ) : s.Colors[ImPlotCol_Line];
        s.Colors[ImPlotCol_Fill]           = IsColorAuto(s.Colors[ImPlotCol_Fill])          ? ( IsColorAuto(ImPlotCol_Fill)           ? item_color                 : gp.Style.Colors[ImPlotCol_Fill]          ) : s.Colors[ImPlotCol_Fill];
        s.Colors[ImPlotCol_MarkerOutline]  = IsColorAuto(s.Colors[ImPlotCol_MarkerOutline]) ? ( IsColorAuto(ImPlotCol_MarkerOutline)  ? s.Colors[ImPlotCol_Line]   : gp.Style.Colors[ImPlotCol_MarkerOutline] ) : s.Colors[ImPlotCol_MarkerOutline];
        s.Colors[ImPlotCol_MarkerFill]     = IsColorAuto(s.Colors[ImPlotCol_MarkerFill])    ? ( IsColorAuto(ImPlotCol_MarkerFill)     ? s.Colors[ImPlotCol_Line]   : gp.Style.Colors[ImPlotCol_MarkerFill]    ) : s.Colors[ImPlotCol_MarkerFill];
        s.Colors[ImPlotCol_ErrorBar]       = IsColorAuto(s.Colors[ImPlotCol_ErrorBar])      ? ( GetStyleColorVec4(ImPlotCol_ErrorBar)                                                                         ) : s.Colors[ImPlotCol_ErrorBar];
        // stage next item style vars
        s.LineWeight         = s.LineWeight       < 0 ? gp.Style.LineWeight       : s.LineWeight;
        s.Marker             = s.Marker           < 0 ? gp.Style.Marker           : s.Marker;
        s.MarkerSize         = s.MarkerSize       < 0 ? gp.Style.MarkerSize       : s.MarkerSize;
        s.MarkerWeight       = s.MarkerWeight     < 0 ? gp.Style.MarkerWeight     : s.MarkerWeight;
        s.FillAlpha          = s.FillAlpha        < 0 ? gp.Style.FillAlpha        : s.FillAlpha;
        s.ErrorBarSize       = s.ErrorBarSize     < 0 ? gp.Style.ErrorBarSize     : s.ErrorBarSize;
        s.ErrorBarWeight     = s.ErrorBarWeight   < 0 ? gp.Style.ErrorBarWeight   : s.ErrorBarWeight;
        s.DigitalBitHeight   = s.DigitalBitHeight < 0 ? gp.Style.DigitalBitHeight : s.DigitalBitHeight;
        s.DigitalBitGap      = s.DigitalBitGap    < 0 ? gp.Style.DigitalBitGap    : s.DigitalBitGap;
        // apply alpha modifier(s)
        s.Colors[ImPlotCol_Fill].w       *= s.FillAlpha;
        s.Colors[ImPlotCol_MarkerFill].w *= s.FillAlpha; // TODO: this should be separate, if it at all
        // apply highlight mods
        if (item->LegendHovered) {
            if (!ImHasFlag(gp.CurrentItems->Legend.Flags, ImPlotLegendFlags_NoHighlightItem)) {
                s.LineWeight   *= ITEM_HIGHLIGHT_LINE_SCALE;
                s.MarkerSize   *= ITEM_HIGHLIGHT_MARK_SCALE;
                s.MarkerWeight *= ITEM_HIGHLIGHT_LINE_SCALE;
                // TODO: how to highlight fills?
            }
            if (!ImHasFlag(gp.CurrentItems->Legend.Flags, ImPlotLegendFlags_NoHighlightAxis)) {
                if (gp.CurrentPlot->EnabledAxesX() > 1)
                    gp.CurrentPlot->Axes[gp.CurrentPlot->CurrentX].ColorHiLi = item->Color;
                if (gp.CurrentPlot->EnabledAxesY() > 1)
                    gp.CurrentPlot->Axes[gp.CurrentPlot->CurrentY].ColorHiLi = item->Color;
            }
        }
        // set render flags
        s.RenderLine       = s.Colors[ImPlotCol_Line].w          > 0 && s.LineWeight > 0;
        s.RenderFill       = s.Colors[ImPlotCol_Fill].w          > 0;
        s.RenderMarkerLine = s.Colors[ImPlotCol_MarkerOutline].w > 0 && s.MarkerWeight > 0;
        s.RenderMarkerFill = s.Colors[ImPlotCol_MarkerFill].w    > 0;
        // push rendering clip rect
        PushPlotClipRect();
        return true;
    }
}

// Ends an item (call only if BeginItem returns true)
void EndItem() {
    ImPlotContext& gp = *GImPlot;
    // pop rendering clip rect
    PopPlotClipRect();
    // reset next item data
    gp.NextItemData.Reset();
    // set current item
    gp.PreviousItem = gp.CurrentItem;
    gp.CurrentItem  = NULL;
}

//-----------------------------------------------------------------------------
// INDEXERS
//-----------------------------------------------------------------------------

// Offsets and strides a data buffer
template <typename T>
IMPLOT_INLINE T IndexData(const T* data, int idx, int count, int offset, int stride) {
    const int s = ((offset == 0) << 0) | ((stride == sizeof(float)) << 1);
    switch (s) {
        case 3 : return data[idx];
        case 2 : return data[(offset + idx) % count];
        case 1 : return *(const T*)(const void*)((const unsigned char*)data + (size_t)((idx) ) * stride);
        case 0 : return *(const T*)(const void*)((const unsigned char*)data + (size_t)((offset + idx) % count) * stride);
        default: return T(0);
    }
}

//-----------------------------------------------------------------------------
// GETTERS
//-----------------------------------------------------------------------------

// Getters can be thought of as iterators that convert user data (e.g. raw arrays)
// to ImPlotPoints

template <typename T>
struct GetterIdx {
    GetterIdx(const T* data, int count, int offset = 0, int stride = sizeof(T)) :
        Data(data),
        Count(count),
        Offset(count ? ImPosMod(offset, count) : 0),
        Stride(stride)
    { }
    template <typename I> IMPLOT_INLINE double operator()(I idx) const {
        return (double)IndexData(Data, idx, Count, Offset, Stride);
    }
    const T* Data;
    int Count;
    int Offset;
    int Stride;
};

struct GetterLin {
    GetterLin(double m, double b) : M(m), B(b) { }
    template <typename I> IMPLOT_INLINE double operator()(I idx) const {
        return M * idx + B;
    }
    const double M;
    const double B;
};

struct GetterRef {
    GetterRef(double ref) : Ref(ref) { }
    template <typename I> IMPLOT_INLINE double operator()(I) const { return Ref; }
    const double Ref;
};

template <typename TGetterX, typename TGetterY>
struct GetterXY {
    GetterXY(TGetterX x, TGetterY y, int count) : GetterX(x), GetterY(y), Count(count) { }
    template <typename I> IMPLOT_INLINE ImPlotPoint operator()(I idx) const {
        return ImPlotPoint(GetterX(idx),GetterY(idx));
    }
    const TGetterX GetterX;
    const TGetterY GetterY;
    const int Count;
};

// Interprets an array of Y points as ImPlotPoints where the X value is the index
template <typename T>
struct GetterXs {
    GetterXs(const T* xs, int count, double yscale, double y0, int offset, int stride) :
        Xs(xs),
        Count(count),
        YScale(yscale),
        Y0(y0),
        Offset(count ? ImPosMod(offset, count) : 0),
        Stride(stride)
    { }
    template <typename I> IMPLOT_INLINE ImPlotPoint operator()(I idx) const {
        return ImPlotPoint((double)IndexData(Xs, idx, Count, Offset, Stride), Y0 + YScale * idx);
    }
    const T* const Xs;
    const int Count;
    const double YScale;
    const double Y0;
    const int Offset;
    const int Stride;
};

/// Interprets a user's function pointer as ImPlotPoints
struct GetterFuncPtr {
    GetterFuncPtr(ImPlotPoint (*getter)(void* data, int idx), void* data, int count) :
        Getter(getter),
        Data(data),
        Count(count)
    { }
    template <typename I> IMPLOT_INLINE ImPlotPoint operator()(I idx) const {
        return Getter(Data, idx);
    }
    ImPlotPoint (* const Getter)(void* data, int idx);
    void* const Data;
    const int Count;
};

template <typename _Getter>
struct GetterOverrideX {
    GetterOverrideX(_Getter getter, double x) : Getter(getter), X(x), Count(getter.Count) { }
    template <typename I> IMPLOT_INLINE ImPlotPoint operator()(I idx) const {
        ImPlotPoint p = Getter(idx);
        p.x = X;
        return p;
    }
    const _Getter Getter;
    const double X;
    const int Count;
};

template <typename _Getter>
struct GetterOverrideY {
    GetterOverrideY(_Getter getter, double y) : Getter(getter), Y(y), Count(getter.Count) { }
    template <typename I> IMPLOT_INLINE ImPlotPoint operator()(I idx) const {
        ImPlotPoint p = Getter(idx);
        p.y = Y;
        return p;
    }
    const _Getter Getter;
    const double Y;
    const int Count;
};

template <typename _Getter>
struct GetterLoop {
    GetterLoop(_Getter getter) : Getter(getter), Count(getter.Count + 1) { }
    template <typename I> IMPLOT_INLINE ImPlotPoint operator()(I idx) const {
        idx = idx % (Count - 1);
        return Getter(idx);
    }
    const _Getter Getter;
    const int Count;
};

template <typename T>
struct GetterError {
    GetterError(const T* xs, const T* ys, const T* neg, const T* pos, int count, int offset, int stride) :
        Xs(xs),
        Ys(ys),
        Neg(neg),
        Pos(pos),
        Count(count),
        Offset(count ? ImPosMod(offset, count) : 0),
        Stride(stride)
    { }
    template <typename I> IMPLOT_INLINE ImPlotPointError operator()(I idx) const {
        return ImPlotPointError((double)IndexData(Xs,  idx, Count, Offset, Stride),
                                (double)IndexData(Ys,  idx, Count, Offset, Stride),
                                (double)IndexData(Neg, idx, Count, Offset, Stride),
                                (double)IndexData(Pos, idx, Count, Offset, Stride));
    }
    const T* const Xs;
    const T* const Ys;
    const T* const Neg;
    const T* const Pos;
    const int Count;
    const int Offset;
    const int Stride;
};

//-----------------------------------------------------------------------------
// TRANSFORMERS
//-----------------------------------------------------------------------------

// Transforms convert points in plot space (i.e. ImPlotPoint) to pixel space (i.e. ImVec2)

struct TransformerLin {
    TransformerLin(double pixMin, double pltMin, double,       double m, double       , double        ) : 
        PixMin(pixMin), PltMin(pltMin), M(m) { }
    template <typename T> IMPLOT_INLINE float operator()(T p) const { return (float)(PixMin + M * (p - PltMin)); }
    double PixMin, PltMin, M;
};

struct TransformerLog {
    TransformerLog(double pixMin, double pltMin, double pltMax, double m, double scaMin, double scaMax) : 
        ScaMin(scaMin), ScaMax(scaMax), PltMin(pltMin), PltMax(pltMax), PixMin(pixMin), M(m) { }
    template <typename T> IMPLOT_INLINE float operator()(T p) const {
        p = p <= 0.0 ? IMPLOT_LOG_ZERO : p;
        double s = ImLog10(p);
        double t = (s - ScaMin) / (ScaMax - ScaMin);
        p = PltMin + (PltMax - PltMin) * t;
        return (float)(PixMin + M * (p - PltMin));
    }
    double ScaMin, ScaMax, PltMin, PltMax, PixMin, M;
};

struct TransformerSym {
    TransformerSym(double pixMin, double pltMin, double pltMax, double m, double scaMin, double scaMax) : 
        ScaMin(scaMin), ScaMax(scaMax), PltMin(pltMin), PltMax(pltMax), PixMin(pixMin), M(m) { }
    template <typename T> IMPLOT_INLINE float operator()(T p) const {
        double s = 2.0 * ImAsinh(p / 2.0);
        double t = (s - ScaMin) / (ScaMax - ScaMin);
        p = PltMin + (PltMax - PltMin) * t;
        return (float)(PixMin + M * (p - PltMin));
    }
    double ScaMin, ScaMax, PltMin, PltMax, PixMin, M;
};

template <typename TransformerX, typename TransformerY>
struct TransformerXY {
    TransformerXY(const ImPlotAxis& x_axis, const ImPlotAxis& y_axis) :
        Tx(x_axis.PixelMin,
           x_axis.Range.Min,
           x_axis.Range.Max,
           x_axis.ScaleToPixel,
           x_axis.ScaleMin,
           x_axis.ScaleMax),
        Ty(y_axis.PixelMin,
           y_axis.Range.Min,
           y_axis.Range.Max,
           y_axis.ScaleToPixel,
           y_axis.ScaleMin,
           y_axis.ScaleMax)
    { }

    TransformerXY(const ImPlotPlot& plot) :
        TransformerXY(plot.Axes[plot.CurrentX], plot.Axes[plot.CurrentY])
    { }

    TransformerXY() :
        TransformerXY(*GImPlot->CurrentPlot)
    { }

    template <typename P> IMPLOT_INLINE ImVec2 operator()(const P& plt) const {
        ImVec2 out;
        out.x = Tx(plt.x);
        out.y = Ty(plt.y);
        return out;
    }

    template <typename T> IMPLOT_INLINE ImVec2 operator()(T x, T y) const {
        ImVec2 out;
        out.x = Tx(x);
        out.y = Ty(y);
        return out;
    }

    TransformerX Tx;
    TransformerY Ty;
};

typedef TransformerXY<TransformerLin,TransformerLin> TransformerLinLin;
typedef TransformerXY<TransformerLog,TransformerLin> TransformerLogLin;
typedef TransformerXY<TransformerSym,TransformerLin> TransformerSymLin;

typedef TransformerXY<TransformerLin,TransformerLog> TransformerLinLog;
typedef TransformerXY<TransformerLog,TransformerLog> TransformerLogLog;
typedef TransformerXY<TransformerSym,TransformerLog> TransformerSymLog;

typedef TransformerXY<TransformerLin,TransformerSym> TransformerLinSym;
typedef TransformerXY<TransformerLog,TransformerSym> TransformerLogSym;
typedef TransformerXY<TransformerSym,TransformerSym> TransformerSymSym;

//-----------------------------------------------------------------------------
// PRIMITIVE RENDERERS
//-----------------------------------------------------------------------------

IMPLOT_INLINE void PrimLine(const ImVec2& P1, const ImVec2& P2, float half_weight, ImU32 col, ImDrawList& draw_list, ImVec2 uv) {
    float dx = P2.x - P1.x;
    float dy = P2.y - P1.y;
    IMPLOT_NORMALIZE2F_OVER_ZERO(dx, dy);
    dx *= half_weight;
    dy *= half_weight;
    draw_list._VtxWritePtr[0].pos.x = P1.x + dy;
    draw_list._VtxWritePtr[0].pos.y = P1.y - dx;
    draw_list._VtxWritePtr[0].uv    = uv;
    draw_list._VtxWritePtr[0].col   = col;
    draw_list._VtxWritePtr[1].pos.x = P2.x + dy;
    draw_list._VtxWritePtr[1].pos.y = P2.y - dx;
    draw_list._VtxWritePtr[1].uv    = uv;
    draw_list._VtxWritePtr[1].col   = col;
    draw_list._VtxWritePtr[2].pos.x = P2.x - dy;
    draw_list._VtxWritePtr[2].pos.y = P2.y + dx;
    draw_list._VtxWritePtr[2].uv    = uv;
    draw_list._VtxWritePtr[2].col   = col;
    draw_list._VtxWritePtr[3].pos.x = P1.x - dy;
    draw_list._VtxWritePtr[3].pos.y = P1.y + dx;
    draw_list._VtxWritePtr[3].uv    = uv;
    draw_list._VtxWritePtr[3].col   = col;
    draw_list._VtxWritePtr += 4;
    draw_list._IdxWritePtr[0] = (ImDrawIdx)(draw_list._VtxCurrentIdx);
    draw_list._IdxWritePtr[1] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 1);
    draw_list._IdxWritePtr[2] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 2);
    draw_list._IdxWritePtr[3] = (ImDrawIdx)(draw_list._VtxCurrentIdx);
    draw_list._IdxWritePtr[4] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 2);
    draw_list._IdxWritePtr[5] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 3);
    draw_list._IdxWritePtr += 6;
    draw_list._VtxCurrentIdx += 4;
}

IMPLOT_INLINE void PrimRectFilled(const ImVec2& Pmin, const ImVec2& Pmax, ImU32 col, ImDrawList& draw_list, ImVec2 uv) {
    draw_list._VtxWritePtr[0].pos   = Pmin;
    draw_list._VtxWritePtr[0].uv    = uv;
    draw_list._VtxWritePtr[0].col   = col;
    draw_list._VtxWritePtr[1].pos   = Pmax;
    draw_list._VtxWritePtr[1].uv    = uv;
    draw_list._VtxWritePtr[1].col   = col;
    draw_list._VtxWritePtr[2].pos.x = Pmin.x;
    draw_list._VtxWritePtr[2].pos.y = Pmax.y;
    draw_list._VtxWritePtr[2].uv    = uv;
    draw_list._VtxWritePtr[2].col   = col;
    draw_list._VtxWritePtr[3].pos.x = Pmax.x;
    draw_list._VtxWritePtr[3].pos.y = Pmin.y;
    draw_list._VtxWritePtr[3].uv    = uv;
    draw_list._VtxWritePtr[3].col   = col;
    draw_list._VtxWritePtr += 4;
    draw_list._IdxWritePtr[0] = (ImDrawIdx)(draw_list._VtxCurrentIdx);
    draw_list._IdxWritePtr[1] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 1);
    draw_list._IdxWritePtr[2] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 2);
    draw_list._IdxWritePtr[3] = (ImDrawIdx)(draw_list._VtxCurrentIdx);
    draw_list._IdxWritePtr[4] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 1);
    draw_list._IdxWritePtr[5] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 3);
    draw_list._IdxWritePtr += 6;
    draw_list._VtxCurrentIdx += 4;
}

template <int _IdxConsumed, int _VtxConsumed, class _Transformer>
struct RendererBase {
    RendererBase(int prims) : Prims(prims) { }
    const int Prims;
    _Transformer Transformer;
    static const int IdxConsumed = _IdxConsumed;
    static const int VtxConsumed = _VtxConsumed;
};

template <class _Getter, class _Transformer>
struct RendererLineStrip : RendererBase<6,4,_Transformer> {
    IMPLOT_INLINE RendererLineStrip(const _Getter& getter, ImU32 col, float weight) :
        RendererBase<6,4,_Transformer>(getter.Count - 1),
        Getter(getter),
        Col(col),
        HalfWeight(weight/2)
    {
        P1 = this->Transformer(Getter(0));
    }
    IMPLOT_INLINE bool operator()(ImDrawList& draw_list, const ImRect& cull_rect, const ImVec2& uv, int prim) const {
        ImVec2 P2 = this->Transformer(Getter(prim + 1));
        if (!cull_rect.Overlaps(ImRect(ImMin(P1, P2), ImMax(P1, P2)))) {
            P1 = P2;
            return false;
        }
        PrimLine(P1,P2,HalfWeight,Col,draw_list,uv);
        P1 = P2;
        return true;
    }
    const _Getter& Getter;
    const ImU32 Col;
    const float HalfWeight;
    mutable ImVec2 P1;
};

template <class _Getter, class _Transformer>
struct RendererLineStripSkip : RendererBase<6,4,_Transformer> {
    IMPLOT_INLINE RendererLineStripSkip(const _Getter& getter, ImU32 col, float weight) :
        RendererBase<6,4,_Transformer>(getter.Count - 1),
        Getter(getter),
        Col(col),
        HalfWeight(weight/2)
    {
        P1 = this->Transformer(Getter(0));
    }
    IMPLOT_INLINE bool operator()(ImDrawList& draw_list, const ImRect& cull_rect, const ImVec2& uv, int prim) const {
        ImVec2 P2 = this->Transformer(Getter(prim + 1));
        if (!cull_rect.Overlaps(ImRect(ImMin(P1, P2), ImMax(P1, P2)))) {
            if (!ImNan(P2.x) && !ImNan(P2.y))
                P1 = P2;
            return false;
        }
        PrimLine(P1,P2,HalfWeight,Col,draw_list,uv);
        if (!ImNan(P2.x) && !ImNan(P2.y))
            P1 = P2;
        return true;
    }
    const _Getter& Getter;
    const ImU32 Col;
    const float HalfWeight;
    mutable ImVec2 P1;
};

template <class _Getter, class _Transformer>
struct RendererLineSegments1 : RendererBase<6,4,_Transformer> {
    IMPLOT_INLINE RendererLineSegments1(const _Getter& getter, ImU32 col, float weight) :
        RendererBase<6,4,_Transformer>(getter.Count / 2),
        Getter(getter),
        Col(col),
        HalfWeight(weight/2)
    { }
    IMPLOT_INLINE bool operator()(ImDrawList& draw_list, const ImRect& cull_rect, const ImVec2& uv, int prim) const {
        ImVec2 P1 = this->Transformer(Getter(prim*2+0));
        ImVec2 P2 = this->Transformer(Getter(prim*2+1));
        if (!cull_rect.Overlaps(ImRect(ImMin(P1, P2), ImMax(P1, P2))))
            return false;
        PrimLine(P1,P2,HalfWeight,Col,draw_list,uv);
        return true;
    }
    const _Getter& Getter;
    const ImU32 Col;
    const float HalfWeight;
};

template <class _Getter1, class _Getter2, class _Transformer>
struct RendererLineSegments2 : RendererBase<6,4,_Transformer> {
    IMPLOT_INLINE RendererLineSegments2(const _Getter1& getter1, const _Getter2& getter2, ImU32 col, float weight) :
        RendererBase<6,4,_Transformer>(ImMin(getter1.Count, getter1.Count)),
        Getter1(getter1),
        Getter2(getter2),
        Col(col),
        HalfWeight(weight/2)
    {}
    IMPLOT_INLINE bool operator()(ImDrawList& draw_list, const ImRect& cull_rect, const ImVec2& uv, int prim) const {
        ImVec2 P1 = this->Transformer(Getter1(prim));
        ImVec2 P2 = this->Transformer(Getter2(prim));
        if (!cull_rect.Overlaps(ImRect(ImMin(P1, P2), ImMax(P1, P2))))
            return false;
        PrimLine(P1,P2,HalfWeight,Col,draw_list,uv);
        return true;
    }
    const _Getter1& Getter1;
    const _Getter2& Getter2;
    const ImU32 Col;
    const float HalfWeight;
};

template <class _Getter1, class _Getter2, class _Transformer>
struct RendererBarsV : RendererBase<6,4,_Transformer> {
    IMPLOT_INLINE RendererBarsV(const _Getter1& getter1, const _Getter2& getter2, ImU32 col, double width) :
        RendererBase<6,4,_Transformer>(ImMin(getter1.Count, getter1.Count)),
        Getter1(getter1),
        Getter2(getter2),
        Col(col),
        HalfWidth(width/2)
    {}
    IMPLOT_INLINE bool operator()(ImDrawList& draw_list, const ImRect& cull_rect, const ImVec2& uv, int prim) const {
        ImPlotPoint p1 = Getter1(prim);
        ImPlotPoint p2 = Getter2(prim);
        p1.x += HalfWidth;
        p2.x -= HalfWidth;
        ImVec2 P1 = this->Transformer(p1);
        ImVec2 P2 = this->Transformer(p2);
        float width_px = ImAbs(P1.x-P2.x);
        if (width_px < 1.0f) {
            P1.x += P1.x > P2.x ? (1-width_px) / 2 : (width_px-1) / 2;
            P2.x += P2.x > P1.x ? (1-width_px) / 2 : (width_px-1) / 2;
        }
        if (!cull_rect.Overlaps(ImRect(ImMin(P1, P2), ImMax(P1, P2))))
            return false;
        PrimRectFilled(P1,P2,Col,draw_list,uv);
        return true;
    }
    const _Getter1& Getter1;
    const _Getter2& Getter2;
    const ImU32 Col;
    const double HalfWidth;
};

template <class _Getter1, class _Getter2, class _Transformer>
struct RendererBarsH : RendererBase<6,4,_Transformer> {
    IMPLOT_INLINE RendererBarsH(const _Getter1& getter1, const _Getter2& getter2, ImU32 col, double height) :
        RendererBase<6,4,_Transformer>(ImMin(getter1.Count, getter1.Count)),
        Getter1(getter1),
        Getter2(getter2),
        Col(col),
        HalfHeight(height/2)
    {}
    IMPLOT_INLINE bool operator()(ImDrawList& draw_list, const ImRect& cull_rect, const ImVec2& uv, int prim) const {
        ImPlotPoint p1 = Getter1(prim);
        ImPlotPoint p2 = Getter2(prim);
        p1.y += HalfHeight;
        p2.y -= HalfHeight;
        ImVec2 P1 = this->Transformer(p1);
        ImVec2 P2 = this->Transformer(p2);
        float height_px = ImAbs(P1.y-P2.y);
        if (height_px < 1.0f) {
            P1.y += P1.y > P2.y ? (1-height_px) / 2 : (height_px-1) / 2;
            P2.y += P2.y > P1.y ? (1-height_px) / 2 : (height_px-1) / 2;
        }
        if (!cull_rect.Overlaps(ImRect(ImMin(P1, P2), ImMax(P1, P2))))
            return false;
        PrimRectFilled(P1,P2,Col,draw_list,uv);
        return true;
    }
    const _Getter1& Getter1;
    const _Getter2& Getter2;
    const ImU32 Col;
    const double HalfHeight;
};

template <class _Getter, class _Transformer>
struct RendererStairsPre : RendererBase<12,8,_Transformer> {
    IMPLOT_INLINE RendererStairsPre(const _Getter& getter, ImU32 col, float weight) :
        RendererBase<12,8,_Transformer>(getter.Count - 1),
        Getter(getter),
        Col(col),
        HalfWeight(weight * 0.5f)
    {
        P1 = this->Transformer(Getter(0));
    }
    IMPLOT_INLINE bool operator()(ImDrawList& draw_list, const ImRect& cull_rect, const ImVec2& uv, int prim) const {
        ImVec2 P2 = this->Transformer(Getter(prim + 1));
        if (!cull_rect.Overlaps(ImRect(ImMin(P1, P2), ImMax(P1, P2)))) {
            P1 = P2;
            return false;
        }
        PrimRectFilled(ImVec2(P1.x - HalfWeight, P1.y), ImVec2(P1.x + HalfWeight, P2.y), Col, draw_list, uv);
        PrimRectFilled(ImVec2(P1.x, P2.y + HalfWeight), ImVec2(P2.x, P2.y - HalfWeight), Col, draw_list, uv);
        P1 = P2;
        return true;
    }
    const _Getter& Getter;
    const ImU32 Col;
    const float HalfWeight;
    mutable ImVec2 P1;
};

template <class _Getter, class _Transformer>
struct RendererStairsPost : RendererBase<12,8,_Transformer> {
    IMPLOT_INLINE RendererStairsPost(const _Getter& getter, ImU32 col, float weight) :
        RendererBase<12,8,_Transformer>(getter.Count - 1),
        Getter(getter),
        Col(col),
        HalfWeight(weight * 0.5f)
    {
        P1 = this->Transformer(Getter(0));
    }
    IMPLOT_INLINE bool operator()(ImDrawList& draw_list, const ImRect& cull_rect, const ImVec2& uv, int prim) const {
        ImVec2 P2 = this->Transformer(Getter(prim + 1));
        if (!cull_rect.Overlaps(ImRect(ImMin(P1, P2), ImMax(P1, P2)))) {
            P1 = P2;
            return false;
        }
        PrimRectFilled(ImVec2(P1.x, P1.y + HalfWeight), ImVec2(P2.x, P1.y - HalfWeight), Col, draw_list, uv);
        PrimRectFilled(ImVec2(P2.x - HalfWeight, P2.y), ImVec2(P2.x + HalfWeight, P1.y), Col, draw_list, uv);
        P1 = P2;
        return true;
    }
    const _Getter& Getter;
    const ImU32 Col;
    const float HalfWeight;
    mutable ImVec2 P1;
};

template <class _Getter1, class _Getter2, class _Transformer>
struct RendererShaded : RendererBase<6,5,_Transformer> {
    IMPLOT_INLINE RendererShaded(const _Getter1& getter1, const _Getter2& getter2, ImU32 col) :
        RendererBase<6,5,_Transformer>(ImMin(getter1.Count, getter2.Count) - 1),
        Getter1(getter1),
        Getter2(getter2),
        Col(col)
    {
        P11 = this->Transformer(Getter1(0));
        P12 = this->Transformer(Getter2(0));
    }

    IMPLOT_INLINE bool operator()(ImDrawList& draw_list, const ImRect& cull_rect, const ImVec2& uv, int prim) const {
        ImVec2 P21 = this->Transformer(Getter1(prim+1));
        ImVec2 P22 = this->Transformer(Getter2(prim+1));
        ImRect rect(ImMin(ImMin(ImMin(P11,P12),P21),P22), ImMax(ImMax(ImMax(P11,P12),P21),P22));
        if (!cull_rect.Overlaps(rect)) {
            P11 = P21;
            P12 = P22;
            return false;
        }
        const int intersect = (P11.y > P12.y && P22.y > P21.y) || (P12.y > P11.y && P21.y > P22.y);
        ImVec2 intersection = Intersection(P11,P21,P12,P22);
        draw_list._VtxWritePtr[0].pos = P11;
        draw_list._VtxWritePtr[0].uv  = uv;
        draw_list._VtxWritePtr[0].col = Col;
        draw_list._VtxWritePtr[1].pos = P21;
        draw_list._VtxWritePtr[1].uv  = uv;
        draw_list._VtxWritePtr[1].col = Col;
        draw_list._VtxWritePtr[2].pos = intersection;
        draw_list._VtxWritePtr[2].uv  = uv;
        draw_list._VtxWritePtr[2].col = Col;
        draw_list._VtxWritePtr[3].pos = P12;
        draw_list._VtxWritePtr[3].uv  = uv;
        draw_list._VtxWritePtr[3].col = Col;
        draw_list._VtxWritePtr[4].pos = P22;
        draw_list._VtxWritePtr[4].uv  = uv;
        draw_list._VtxWritePtr[4].col = Col;
        draw_list._VtxWritePtr += 5;
        draw_list._IdxWritePtr[0] = (ImDrawIdx)(draw_list._VtxCurrentIdx);
        draw_list._IdxWritePtr[1] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 1 + intersect);
        draw_list._IdxWritePtr[2] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 3);
        draw_list._IdxWritePtr[3] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 1);
        draw_list._IdxWritePtr[4] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 4);
        draw_list._IdxWritePtr[5] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 3 - intersect);
        draw_list._IdxWritePtr += 6;
        draw_list._VtxCurrentIdx += 5;
        P11 = P21;
        P12 = P22;
        return true;
    }
    const _Getter1& Getter1;
    const _Getter2& Getter2;
    const ImU32 Col;
    mutable ImVec2 P11;
    mutable ImVec2 P12;
};

struct RectC {
    ImPlotPoint Pos;
    ImPlotPoint HalfSize;
    ImU32 Color;
};

template <typename _Getter, typename _Transformer>
struct RendererRectC : RendererBase<6,4,_Transformer> {
    IMPLOT_INLINE RendererRectC(const _Getter& getter) :
        RendererBase<6,4,_Transformer>(getter.Count),
        Getter(getter)
    {}
    IMPLOT_INLINE bool operator()(ImDrawList& draw_list, const ImRect& cull_rect, const ImVec2& uv, int prim) const {
        RectC rect = Getter(prim);
        ImVec2 P1 = this->Transformer(rect.Pos.x - rect.HalfSize.x , rect.Pos.y - rect.HalfSize.y);
        ImVec2 P2 = this->Transformer(rect.Pos.x + rect.HalfSize.x , rect.Pos.y + rect.HalfSize.y);
        if ((rect.Color & IM_COL32_A_MASK) == 0 || !cull_rect.Overlaps(ImRect(ImMin(P1, P2), ImMax(P1, P2))))
            return false;
        PrimRectFilled(P1,P2,rect.Color,draw_list,uv);
        return true;
    }
    const _Getter& Getter;
};

/// Renders primitive shapes in bulk as efficiently as possible.
template <class _Renderer>
IMPLOT_INLINE void RenderPrimitives(const _Renderer& renderer, ImDrawList& draw_list, const ImRect& cull_rect) {
    unsigned int prims        = renderer.Prims;
    unsigned int prims_culled = 0;
    unsigned int idx          = 0;
    const ImVec2 uv = draw_list._Data->TexUvWhitePixel;
    while (prims) {
        // find how many can be reserved up to end of current draw command's limit
        unsigned int cnt = ImMin(prims, (MaxIdx<ImDrawIdx>::Value - draw_list._VtxCurrentIdx) / _Renderer::VtxConsumed);
        // make sure at least this many elements can be rendered to avoid situations where at the end of buffer this slow path is not taken all the time
        if (cnt >= ImMin(64u, prims)) {
            if (prims_culled >= cnt)
                prims_culled -= cnt; // reuse previous reservation
            else {
                // add more elements to previous reservation
                draw_list.PrimReserve((cnt - prims_culled) * _Renderer::IdxConsumed, (cnt - prims_culled) * _Renderer::VtxConsumed); 
                prims_culled = 0;
            }
        }
        else
        {
            if (prims_culled > 0) {
                draw_list.PrimUnreserve(prims_culled * _Renderer::IdxConsumed, prims_culled * _Renderer::VtxConsumed);
                prims_culled = 0;
            }
            cnt = ImMin(prims, (MaxIdx<ImDrawIdx>::Value - 0/*draw_list._VtxCurrentIdx*/) / _Renderer::VtxConsumed);
            // reserve new draw command
            draw_list.PrimReserve(cnt * _Renderer::IdxConsumed, cnt * _Renderer::VtxConsumed); 
        }
        prims -= cnt;
        for (unsigned int ie = idx + cnt; idx != ie; ++idx) {
            if (!renderer(draw_list, cull_rect, uv, idx))
                prims_culled++;
        }
    }
    if (prims_culled > 0)
        draw_list.PrimUnreserve(prims_culled * _Renderer::IdxConsumed, prims_culled * _Renderer::VtxConsumed);
}

template <template <class,class> class _Renderer, class _Getter, typename ...Args>
IMPLOT_INLINE void RenderPrimitives1(const _Getter& getter, Args... args) {
    ImDrawList& draw_list = *GetPlotDrawList();
    const ImRect& cull_rect = GetCurrentPlot()->PlotRect;
    switch (GetCurrentScale()) {
        case ImPlotScale_LinLin : RenderPrimitives(_Renderer<_Getter,TransformerLinLin>(getter, args...), draw_list, cull_rect); break;
        case ImPlotScale_LogLin : RenderPrimitives(_Renderer<_Getter,TransformerLogLin>(getter, args...), draw_list, cull_rect); break;
        case ImPlotScale_SymLin : RenderPrimitives(_Renderer<_Getter,TransformerSymLin>(getter, args...), draw_list, cull_rect); break;;
        case ImPlotScale_LinLog : RenderPrimitives(_Renderer<_Getter,TransformerLinLog>(getter, args...), draw_list, cull_rect); break;
        case ImPlotScale_LogLog : RenderPrimitives(_Renderer<_Getter,TransformerLogLog>(getter, args...), draw_list, cull_rect); break;
        case ImPlotScale_SymLog : RenderPrimitives(_Renderer<_Getter,TransformerSymLog>(getter, args...), draw_list, cull_rect); break;;
        case ImPlotScale_LinSym : RenderPrimitives(_Renderer<_Getter,TransformerLinSym>(getter, args...), draw_list, cull_rect); break;;
        case ImPlotScale_LogSym : RenderPrimitives(_Renderer<_Getter,TransformerLogSym>(getter, args...), draw_list, cull_rect); break;;
        case ImPlotScale_SymSym : RenderPrimitives(_Renderer<_Getter,TransformerSymSym>(getter, args...), draw_list, cull_rect); break;;
    }
}

template <template <class,class,class> class _Renderer, class _Getter1, class _Getter2, typename ...Args>
IMPLOT_INLINE void RenderPrimitives2(const _Getter1& getter1, const _Getter2& getter2, Args... args) {
    ImDrawList& draw_list = *GetPlotDrawList();
    const ImRect& cull_rect = GetCurrentPlot()->PlotRect;
    switch (GetCurrentScale()) {
        case ImPlotScale_LinLin : RenderPrimitives(_Renderer<_Getter1,_Getter2,TransformerLinLin>(getter1, getter2, args...), draw_list, cull_rect); break;
        case ImPlotScale_LogLin : RenderPrimitives(_Renderer<_Getter1,_Getter2,TransformerLogLin>(getter1, getter2, args...), draw_list, cull_rect); break;
        case ImPlotScale_LinLog : RenderPrimitives(_Renderer<_Getter1,_Getter2,TransformerLinLog>(getter1, getter2, args...), draw_list, cull_rect); break;
        case ImPlotScale_LogLog : RenderPrimitives(_Renderer<_Getter1,_Getter2,TransformerLogLog>(getter1, getter2, args...), draw_list, cull_rect); break;
    }
}

/*

template <typename Getter, typename Transformer>
IMPLOT_INLINE void RenderLineStrip(const Getter& getter, const Transformer& transformer, ImDrawList& draw_list, float line_weight, ImU32 col) {
    ImPlotContext& gp = *GImPlot;
    if (ImHasFlag(gp.CurrentPlot->Flags, ImPlotFlags_AntiAliased) || gp.Style.AntiAliasedLines) {
        ImVec2 p1 = transformer(getter(0));
        for (int i = 1; i < getter.Count; ++i) {
            ImVec2 p2 = transformer(getter(i));
            if (gp.CurrentPlot->PlotRect.Overlaps(ImRect(ImMin(p1, p2), ImMax(p1, p2))))
                draw_list.AddLine(p1, p2, col, line_weight);
            p1 = p2;
        }
    }
    else {
        RenderPrimitives(RendererLineStrip<Getter,Transformer>(getter, col, line_weight), draw_list, gp.CurrentPlot->PlotRect);
    }
}

template <typename Getter, typename Transformer>
IMPLOT_INLINE void RenderLineStripSkip(const Getter &getter, const Transformer &transformer, ImDrawList &draw_list, float line_weight, ImU32 col) {
    ImPlotContext &gp = *GImPlot;
    if (ImHasFlag(gp.CurrentPlot->Flags, ImPlotFlags_AntiAliased) || gp.Style.AntiAliasedLines) {
        ImVec2 p1 = transformer(getter(0));
        for (int i = 1; i < getter.Count; ++i) {
            ImVec2 p2 = transformer(getter(i));
            if (gp.CurrentPlot->PlotRect.Overlaps(ImRect(ImMin(p1, p2), ImMax(p1, p2))))
                draw_list.AddLine(p1, p2, col, line_weight);
            if (!ImNan(p2.x) && !ImNan(p2.y)) 
                p1 = p2;
        }
    } 
    else {
        RenderPrimitives(RendererLineStripSkip<Getter, Transformer>(getter, transformer, col, line_weight), draw_list, gp.CurrentPlot->PlotRect);
    }
}

template <typename Getter, typename Transformer>
IMPLOT_INLINE void RenderLineSegments(const Getter& getter, const Transformer& transformer, ImDrawList& draw_list, float line_weight, ImU32 col) {
    ImPlotContext& gp = *GImPlot;
    if (ImHasFlag(gp.CurrentPlot->Flags, ImPlotFlags_AntiAliased) || gp.Style.AntiAliasedLines) {
        const int I = getter.Count / 2;
        for (int i = 0; i < I; ++i) {
            ImVec2 p1 = transformer(getter(2*i+0));
            ImVec2 p2 = transformer(getter(2*i+1));
            if (gp.CurrentPlot->PlotRect.Overlaps(ImRect(ImMin(p1, p2), ImMax(p1, p2))))
                draw_list.AddLine(p1, p2, col, line_weight);
        }
    }
    else {
        RenderPrimitives(RendererLineSegments1<Getter,Transformer>(getter, transformer, col, line_weight), draw_list, gp.CurrentPlot->PlotRect);
    }
}

template <typename Getter1, typename Getter2, typename Transformer>
IMPLOT_INLINE void RenderLineSegments2(const Getter1& getter1, const Getter2& getter2, const Transformer& transformer, ImDrawList& draw_list, float line_weight, ImU32 col) {
    ImPlotContext& gp = *GImPlot;
    if (ImHasFlag(gp.CurrentPlot->Flags, ImPlotFlags_AntiAliased) || gp.Style.AntiAliasedLines) {
        int I = ImMin(getter1.Count, getter2.Count);
        for (int i = 0; i < I; ++i) {
            ImVec2 p1 = transformer(getter1(i));
            ImVec2 p2 = transformer(getter2(i));
            if (gp.CurrentPlot->PlotRect.Overlaps(ImRect(ImMin(p1, p2), ImMax(p1, p2))))
                draw_list.AddLine(p1, p2, col, line_weight);
        }
    }
    else {
        RenderPrimitives(RendererLineSegments2<Getter1,Getter2,Transformer>(getter1, getter2, transformer, col, line_weight), draw_list, gp.CurrentPlot->PlotRect);
    }
}

template <typename Getter, typename Transformer>
IMPLOT_INLINE void RenderStairs(const Getter& getter, const Transformer& transformer, ImDrawList& draw_list, float line_weight, ImU32 col, bool pre) {
    ImPlotContext& gp = *GImPlot;
    if (pre)
        RenderPrimitives(RendererStairsPre<Getter,Transformer>(getter, transformer, col, line_weight), draw_list, gp.CurrentPlot->PlotRect);    
    else
        RenderPrimitives(RendererStairsPost<Getter,Transformer>(getter, transformer, col, line_weight), draw_list, gp.CurrentPlot->PlotRect);    
}

*/

//-----------------------------------------------------------------------------
// MARKER RENDERERS
//-----------------------------------------------------------------------------

IMPLOT_INLINE void TransformMarker(ImVec2* points, int n, const ImVec2& c, float s) {
    for (int i = 0; i < n; ++i) {
        points[i].x = c.x + points[i].x * s;
        points[i].y = c.y + points[i].y * s;
    }
}

IMPLOT_INLINE void RenderMarkerGeneral(ImDrawList& draw_list, ImVec2* points, int n, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    TransformMarker(points, n, c, s);
    if (fill)
        draw_list.AddConvexPolyFilled(points, n, col_fill);
    if (outline && !(fill && col_outline == col_fill)) {
        for (int i = 0; i < n; ++i)
            draw_list.AddLine(points[i], points[(i+1)%n], col_outline, weight);
    }
}

IMPLOT_INLINE void RenderMarkerCircle(ImDrawList& draw_list, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[10] = {ImVec2(1.0f, 0.0f),
                         ImVec2(0.809017f, 0.58778524f),
                         ImVec2(0.30901697f, 0.95105654f),
                         ImVec2(-0.30901703f, 0.9510565f),
                         ImVec2(-0.80901706f, 0.5877852f),
                         ImVec2(-1.0f, 0.0f),
                         ImVec2(-0.80901694f, -0.58778536f),
                         ImVec2(-0.3090171f, -0.9510565f),
                         ImVec2(0.30901712f, -0.9510565f),
                         ImVec2(0.80901694f, -0.5877853f)};
    RenderMarkerGeneral(draw_list, marker, 10, c, s, outline, col_outline, fill, col_fill, weight);
}

IMPLOT_INLINE void RenderMarkerDiamond(ImDrawList& draw_list, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[4] = {ImVec2(1, 0), ImVec2(0, -1), ImVec2(-1, 0), ImVec2(0, 1)};
    RenderMarkerGeneral(draw_list, marker, 4, c, s, outline, col_outline, fill, col_fill, weight);
}

IMPLOT_INLINE void RenderMarkerSquare(ImDrawList& draw_list, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[4] = {ImVec2(SQRT_1_2,SQRT_1_2),ImVec2(SQRT_1_2,-SQRT_1_2),ImVec2(-SQRT_1_2,-SQRT_1_2),ImVec2(-SQRT_1_2,SQRT_1_2)};
    RenderMarkerGeneral(draw_list, marker, 4, c, s, outline, col_outline, fill, col_fill, weight);
}

IMPLOT_INLINE void RenderMarkerUp(ImDrawList& draw_list, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[3] = {ImVec2(SQRT_3_2,0.5f),ImVec2(0,-1),ImVec2(-SQRT_3_2,0.5f)};
    RenderMarkerGeneral(draw_list, marker, 3, c, s, outline, col_outline, fill, col_fill, weight);
}

IMPLOT_INLINE void RenderMarkerDown(ImDrawList& draw_list, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[3] = {ImVec2(SQRT_3_2,-0.5f),ImVec2(0,1),ImVec2(-SQRT_3_2,-0.5f)};
    RenderMarkerGeneral(draw_list, marker, 3, c, s, outline, col_outline, fill, col_fill, weight);
}

IMPLOT_INLINE void RenderMarkerLeft(ImDrawList& draw_list, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[3] = {ImVec2(-1,0), ImVec2(0.5, SQRT_3_2), ImVec2(0.5, -SQRT_3_2)};
    RenderMarkerGeneral(draw_list, marker, 3, c, s, outline, col_outline, fill, col_fill, weight);
}

IMPLOT_INLINE void RenderMarkerRight(ImDrawList& draw_list, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[3] = {ImVec2(1,0), ImVec2(-0.5, SQRT_3_2), ImVec2(-0.5, -SQRT_3_2)};
    RenderMarkerGeneral(draw_list, marker, 3, c, s, outline, col_outline, fill, col_fill, weight);
}

IMPLOT_INLINE void RenderMarkerAsterisk(ImDrawList& draw_list, const ImVec2& c, float s, bool /*outline*/, ImU32 col_outline, bool /*fill*/, ImU32 /*col_fill*/, float weight) {
    ImVec2 marker[6] = {ImVec2(SQRT_3_2, 0.5f), ImVec2(0, -1), ImVec2(-SQRT_3_2, 0.5f), ImVec2(SQRT_3_2, -0.5f), ImVec2(0, 1),  ImVec2(-SQRT_3_2, -0.5f)};
    TransformMarker(marker, 6, c, s);
    draw_list.AddLine(marker[0], marker[5], col_outline, weight);
    draw_list.AddLine(marker[1], marker[4], col_outline, weight);
    draw_list.AddLine(marker[2], marker[3], col_outline, weight);
}

IMPLOT_INLINE void RenderMarkerPlus(ImDrawList& draw_list, const ImVec2& c, float s, bool /*outline*/, ImU32 col_outline, bool /*fill*/, ImU32 /*col_fill*/, float weight) {
    ImVec2 marker[4] = {ImVec2(1, 0), ImVec2(0, -1), ImVec2(-1, 0), ImVec2(0, 1)};
    TransformMarker(marker, 4, c, s);
    draw_list.AddLine(marker[0], marker[2], col_outline, weight);
    draw_list.AddLine(marker[1], marker[3], col_outline, weight);
}

IMPLOT_INLINE void RenderMarkerCross(ImDrawList& draw_list, const ImVec2& c, float s, bool /*outline*/, ImU32 col_outline, bool /*fill*/, ImU32 /*col_fill*/, float weight) {
    ImVec2 marker[4] = {ImVec2(SQRT_1_2,SQRT_1_2),ImVec2(SQRT_1_2,-SQRT_1_2),ImVec2(-SQRT_1_2,-SQRT_1_2),ImVec2(-SQRT_1_2,SQRT_1_2)};
    TransformMarker(marker, 4, c, s);
    draw_list.AddLine(marker[0], marker[2], col_outline, weight);
    draw_list.AddLine(marker[1], marker[3], col_outline, weight);
}

template <typename Transformer, typename Getter>
IMPLOT_INLINE void RenderMarkers(Getter getter, Transformer transformer, ImDrawList& draw_list, ImPlotMarker marker, float size, bool rend_mk_line, ImU32 col_mk_line, float weight, bool rend_mk_fill, ImU32 col_mk_fill) {
    static void (*marker_table[ImPlotMarker_COUNT])(ImDrawList&, const ImVec2&, float s, bool, ImU32, bool, ImU32, float) = {
        RenderMarkerCircle,
        RenderMarkerSquare,
        RenderMarkerDiamond ,
        RenderMarkerUp ,
        RenderMarkerDown ,
        RenderMarkerLeft,
        RenderMarkerRight,
        RenderMarkerCross,
        RenderMarkerPlus,
        RenderMarkerAsterisk
    };
    ImPlotContext& gp = *GImPlot;
    const ImRect& rect = gp.CurrentPlot->PlotRect;
    for (int i = 0; i < getter.Count; ++i) {
        ImVec2 c = transformer(getter(i));
        if (c.x >= rect.Min.x && c.y >= rect.Min.y && c.x <= rect.Max.x && c.y <= rect.Max.y)
            marker_table[marker](draw_list, c, size, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, weight);
    }
}

//-----------------------------------------------------------------------------
// PLOT LINE
//-----------------------------------------------------------------------------

template <typename Getter>
IMPLOT_INLINE void PlotLineEx(const char* label_id, const Getter& getter, ImPlotLineFlags flags) {
    if (BeginItem(label_id, ImPlotCol_Line)) {
        if (FitThisFrame()) {
            for (int i = 0; i < getter.Count; ++i) {
                ImPlotPoint p = getter(i);
                FitPoint(p);
            }
        }
        const ImPlotNextItemData& s = GetItemData();
        ImDrawList& draw_list = *GetPlotDrawList();
        if (getter.Count > 1 && s.RenderLine) {
            const ImU32 col_line    = ImGui::GetColorU32(s.Colors[ImPlotCol_Line]);
            if (ImHasFlag(flags,ImPlotLineFlags_Segments)) {
                RenderPrimitives1<RendererLineSegments1>(getter,col_line,s.LineWeight);   
            }
            else if (ImHasFlag(flags, ImPlotLineFlags_Loop)) {
                if (ImHasFlag(flags, ImPlotLineFlags_SkipNaN)) 
                    RenderPrimitives1<RendererLineStripSkip>(GetterLoop<Getter>(getter),col_line,s.LineWeight);        
                else 
                    RenderPrimitives1<RendererLineStrip>(GetterLoop<Getter>(getter),col_line,s.LineWeight);                      
            }
            else {
                if (ImHasFlag(flags, ImPlotLineFlags_SkipNaN)) 
                    RenderPrimitives1<RendererLineStripSkip>(getter,col_line,s.LineWeight);   
                else 
                    RenderPrimitives1<RendererLineStrip>(getter,col_line,s.LineWeight);                
            }
        }
        // render markers
        if (s.Marker != ImPlotMarker_None) {
            // uncomment lines below to render markers over plot rect border
            if (ImHasFlag(flags, ImPlotLineFlags_NoClip)) {
                PopPlotClipRect();
                PushPlotClipRect(s.MarkerSize);
            }
            const ImU32 col_line = ImGui::GetColorU32(s.Colors[ImPlotCol_MarkerOutline]);
            const ImU32 col_fill = ImGui::GetColorU32(s.Colors[ImPlotCol_MarkerFill]);
            switch (GetCurrentScale()) {
                case ImPlotScale_LinLin: RenderMarkers(getter, TransformerLinLin(), draw_list, s.Marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
                case ImPlotScale_LogLin: RenderMarkers(getter, TransformerLogLin(), draw_list, s.Marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
                case ImPlotScale_LinLog: RenderMarkers(getter, TransformerLinLog(), draw_list, s.Marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
                case ImPlotScale_LogLog: RenderMarkers(getter, TransformerLogLog(), draw_list, s.Marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
            }
        }
        EndItem();
    }
}

template <typename T>
void PlotLine(const char* label_id, const T* values, int count, double xscale, double x0, ImPlotLineFlags flags, int offset, int stride) {
    GetterXY<GetterLin,GetterIdx<T>> getter(GetterLin(xscale,x0),GetterIdx<T>(values,count,offset,stride),count);
    PlotLineEx(label_id, getter, flags);
}

template IMPLOT_API void PlotLine<ImS8> (const char* label_id, const ImS8* values, int count, double xscale, double x0, ImPlotLineFlags flags, int offset, int stride);
template IMPLOT_API void PlotLine<ImU8> (const char* label_id, const ImU8* values, int count, double xscale, double x0, ImPlotLineFlags flags, int offset, int stride);
template IMPLOT_API void PlotLine<ImS16>(const char* label_id, const ImS16* values, int count, double xscale, double x0, ImPlotLineFlags flags, int offset, int stride);
template IMPLOT_API void PlotLine<ImU16>(const char* label_id, const ImU16* values, int count, double xscale, double x0, ImPlotLineFlags flags, int offset, int stride);
template IMPLOT_API void PlotLine<ImS32>(const char* label_id, const ImS32* values, int count, double xscale, double x0, ImPlotLineFlags flags, int offset, int stride);
template IMPLOT_API void PlotLine<ImU32>(const char* label_id, const ImU32* values, int count, double xscale, double x0, ImPlotLineFlags flags, int offset, int stride);
template IMPLOT_API void PlotLine<ImS64>(const char* label_id, const ImS64* values, int count, double xscale, double x0, ImPlotLineFlags flags, int offset, int stride);
template IMPLOT_API void PlotLine<ImU64>(const char* label_id, const ImU64* values, int count, double xscale, double x0, ImPlotLineFlags flags, int offset, int stride);
template IMPLOT_API void PlotLine<float>(const char* label_id, const float* values, int count, double xscale, double x0, ImPlotLineFlags flags, int offset, int stride);
template IMPLOT_API void PlotLine<double>(const char* label_id, const double* values, int count, double xscale, double x0, ImPlotLineFlags flags, int offset, int stride);

template <typename T>
void PlotLine(const char* label_id, const T* xs, const T* ys, int count, ImPlotLineFlags flags, int offset, int stride) {
    GetterXY<GetterIdx<T>,GetterIdx<T>> getter(GetterIdx<T>(xs,count,offset,stride),GetterIdx<T>(ys,count,offset,stride),count);
    return PlotLineEx(label_id, getter, flags);
}

template IMPLOT_API void PlotLine<ImS8>(const char* label_id, const ImS8* xs, const ImS8* ys, int count, ImPlotLineFlags flags, int offset, int stride);
template IMPLOT_API void PlotLine<ImU8>(const char* label_id, const ImU8* xs, const ImU8* ys, int count, ImPlotLineFlags flags, int offset, int stride);
template IMPLOT_API void PlotLine<ImS16>(const char* label_id, const ImS16* xs, const ImS16* ys, int count, ImPlotLineFlags flags, int offset, int stride);
template IMPLOT_API void PlotLine<ImU16>(const char* label_id, const ImU16* xs, const ImU16* ys, int count, ImPlotLineFlags flags, int offset, int stride);
template IMPLOT_API void PlotLine<ImS32>(const char* label_id, const ImS32* xs, const ImS32* ys, int count, ImPlotLineFlags flags, int offset, int stride);
template IMPLOT_API void PlotLine<ImU32>(const char* label_id, const ImU32* xs, const ImU32* ys, int count, ImPlotLineFlags flags, int offset, int stride);
template IMPLOT_API void PlotLine<ImS64>(const char* label_id, const ImS64* xs, const ImS64* ys, int count, ImPlotLineFlags flags, int offset, int stride);
template IMPLOT_API void PlotLine<ImU64>(const char* label_id, const ImU64* xs, const ImU64* ys, int count, ImPlotLineFlags flags, int offset, int stride);
template IMPLOT_API void PlotLine<float>(const char* label_id, const float* xs, const float* ys, int count, ImPlotLineFlags flags, int offset, int stride);
template IMPLOT_API void PlotLine<double>(const char* label_id, const double* xs, const double* ys, int count, ImPlotLineFlags flags, int offset, int stride);

// custom
void PlotLineG(const char* label_id, ImPlotGetter getter_func, void* data, int count, ImPlotLineFlags flags) {
    GetterFuncPtr getter(getter_func,data, count);
    return PlotLineEx(label_id, getter, flags);
}

//-----------------------------------------------------------------------------
// PLOT SCATTER
//-----------------------------------------------------------------------------

template <typename Getter>
IMPLOT_INLINE void PlotScatterEx(const char* label_id, const Getter& getter, ImPlotScatterFlags flags) {
    if (BeginItem(label_id, ImPlotCol_MarkerOutline)) {
        if (FitThisFrame()) {
            for (int i = 0; i < getter.Count; ++i) {
                ImPlotPoint p = getter(i);
                FitPoint(p);
            }
        }
        const ImPlotNextItemData& s = GetItemData();
        ImDrawList& draw_list = *GetPlotDrawList();
        ImPlotMarker marker = s.Marker == ImPlotMarker_None ? ImPlotMarker_Circle : s.Marker;
        if (marker != ImPlotMarker_None) {
            if (ImHasFlag(flags,ImPlotScatterFlags_NoClip)) {
                PopPlotClipRect();
                PushPlotClipRect(s.MarkerSize);
            }
            const ImU32 col_line = ImGui::GetColorU32(s.Colors[ImPlotCol_MarkerOutline]);
            const ImU32 col_fill = ImGui::GetColorU32(s.Colors[ImPlotCol_MarkerFill]);
            switch (GetCurrentScale()) {
                case ImPlotScale_LinLin: RenderMarkers(getter, TransformerLinLin(), draw_list, marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
                case ImPlotScale_LogLin: RenderMarkers(getter, TransformerLogLin(), draw_list, marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
                case ImPlotScale_LinLog: RenderMarkers(getter, TransformerLinLog(), draw_list, marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
                case ImPlotScale_LogLog: RenderMarkers(getter, TransformerLogLog(), draw_list, marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
            }
        }
        EndItem();
    }
}

template <typename T>
void PlotScatter(const char* label_id, const T* values, int count, double xscale, double x0, ImPlotScatterFlags flags, int offset, int stride) {
    GetterXY<GetterLin,GetterIdx<T>> getter(GetterLin(xscale,x0),GetterIdx<T>(values,count,offset,stride),count);
    PlotScatterEx(label_id, getter, flags);
}

template IMPLOT_API void PlotScatter<ImS8>(const char* label_id, const ImS8* values, int count, double xscale, double x0, ImPlotScatterFlags flags, int offset, int stride);
template IMPLOT_API void PlotScatter<ImU8>(const char* label_id, const ImU8* values, int count, double xscale, double x0, ImPlotScatterFlags flags, int offset, int stride);
template IMPLOT_API void PlotScatter<ImS16>(const char* label_id, const ImS16* values, int count, double xscale, double x0, ImPlotScatterFlags flags, int offset, int stride);
template IMPLOT_API void PlotScatter<ImU16>(const char* label_id, const ImU16* values, int count, double xscale, double x0, ImPlotScatterFlags flags, int offset, int stride);
template IMPLOT_API void PlotScatter<ImS32>(const char* label_id, const ImS32* values, int count, double xscale, double x0, ImPlotScatterFlags flags, int offset, int stride);
template IMPLOT_API void PlotScatter<ImU32>(const char* label_id, const ImU32* values, int count, double xscale, double x0, ImPlotScatterFlags flags, int offset, int stride);
template IMPLOT_API void PlotScatter<ImS64>(const char* label_id, const ImS64* values, int count, double xscale, double x0, ImPlotScatterFlags flags, int offset, int stride);
template IMPLOT_API void PlotScatter<ImU64>(const char* label_id, const ImU64* values, int count, double xscale, double x0, ImPlotScatterFlags flags, int offset, int stride);
template IMPLOT_API void PlotScatter<float>(const char* label_id, const float* values, int count, double xscale, double x0, ImPlotScatterFlags flags, int offset, int stride);
template IMPLOT_API void PlotScatter<double>(const char* label_id, const double* values, int count, double xscale, double x0, ImPlotScatterFlags flags, int offset, int stride);

template <typename T>
void PlotScatter(const char* label_id, const T* xs, const T* ys, int count, ImPlotScatterFlags flags, int offset, int stride) {
    GetterXY<GetterIdx<T>,GetterIdx<T>> getter(GetterIdx<T>(xs,count,offset,stride),GetterIdx<T>(ys,count,offset,stride),count);
    return PlotScatterEx(label_id, getter, flags);
}

template IMPLOT_API void PlotScatter<ImS8>(const char* label_id, const ImS8* xs, const ImS8* ys, int count, ImPlotScatterFlags flags, int offset, int stride);
template IMPLOT_API void PlotScatter<ImU8>(const char* label_id, const ImU8* xs, const ImU8* ys, int count, ImPlotScatterFlags flags, int offset, int stride);
template IMPLOT_API void PlotScatter<ImS16>(const char* label_id, const ImS16* xs, const ImS16* ys, int count, ImPlotScatterFlags flags, int offset, int stride);
template IMPLOT_API void PlotScatter<ImU16>(const char* label_id, const ImU16* xs, const ImU16* ys, int count, ImPlotScatterFlags flags, int offset, int stride);
template IMPLOT_API void PlotScatter<ImS32>(const char* label_id, const ImS32* xs, const ImS32* ys, int count, ImPlotScatterFlags flags, int offset, int stride);
template IMPLOT_API void PlotScatter<ImU32>(const char* label_id, const ImU32* xs, const ImU32* ys, int count, ImPlotScatterFlags flags, int offset, int stride);
template IMPLOT_API void PlotScatter<ImS64>(const char* label_id, const ImS64* xs, const ImS64* ys, int count, ImPlotScatterFlags flags, int offset, int stride);
template IMPLOT_API void PlotScatter<ImU64>(const char* label_id, const ImU64* xs, const ImU64* ys, int count, ImPlotScatterFlags flags, int offset, int stride);
template IMPLOT_API void PlotScatter<float>(const char* label_id, const float* xs, const float* ys, int count, ImPlotScatterFlags flags, int offset, int stride);
template IMPLOT_API void PlotScatter<double>(const char* label_id, const double* xs, const double* ys, int count, ImPlotScatterFlags flags, int offset, int stride);

// custom
void PlotScatterG(const char* label_id, ImPlotGetter getter_func, void* data, int count, ImPlotScatterFlags flags) {
    GetterFuncPtr getter(getter_func,data, count);
    return PlotScatterEx(label_id, getter, flags);
}

//-----------------------------------------------------------------------------
// PLOT STAIRS
//-----------------------------------------------------------------------------

template <typename Getter>
IMPLOT_INLINE void PlotStairsEx(const char* label_id, const Getter& getter, ImPlotStairsFlags flags) {
    if (BeginItem(label_id, ImPlotCol_Line)) {
        if (FitThisFrame()) {
            for (int i = 0; i < getter.Count; ++i) {
                ImPlotPoint p = getter(i);
                FitPoint(p);
            }
        }
        const ImPlotNextItemData& s = GetItemData();
        ImDrawList& draw_list = *GetPlotDrawList();
        if (getter.Count > 1 && s.RenderLine) {
            const ImU32 col_line = ImGui::GetColorU32(s.Colors[ImPlotCol_Line]);
            if (ImHasFlag(flags, ImPlotStairsFlags_PreStep))
                RenderPrimitives1<RendererStairsPre>(getter,col_line,s.LineWeight);    
            else     
                RenderPrimitives1<RendererStairsPost>(getter,col_line,s.LineWeight);       
        }
        // render markers
        if (s.Marker != ImPlotMarker_None) {
            PopPlotClipRect();
            PushPlotClipRect(s.MarkerSize);
            const ImU32 col_line = ImGui::GetColorU32(s.Colors[ImPlotCol_MarkerOutline]);
            const ImU32 col_fill = ImGui::GetColorU32(s.Colors[ImPlotCol_MarkerFill]);
            switch (GetCurrentScale()) {
                case ImPlotScale_LinLin: RenderMarkers(getter, TransformerLinLin(), draw_list, s.Marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
                case ImPlotScale_LogLin: RenderMarkers(getter, TransformerLogLin(), draw_list, s.Marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
                case ImPlotScale_LinLog: RenderMarkers(getter, TransformerLinLog(), draw_list, s.Marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
                case ImPlotScale_LogLog: RenderMarkers(getter, TransformerLogLog(), draw_list, s.Marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
            }
        }
        EndItem();
    }
}

template <typename T>
void PlotStairs(const char* label_id, const T* values, int count, double xscale, double x0, ImPlotStairsFlags flags, int offset, int stride) {
    GetterXY<GetterLin,GetterIdx<T>> getter(GetterLin(xscale,x0),GetterIdx<T>(values,count,offset,stride),count);
    PlotStairsEx(label_id, getter, flags);
}

template IMPLOT_API void PlotStairs<ImS8> (const char* label_id, const ImS8* values, int count, double xscale, double x0, ImPlotStairsFlags flags, int offset, int stride);
template IMPLOT_API void PlotStairs<ImU8> (const char* label_id, const ImU8* values, int count, double xscale, double x0, ImPlotStairsFlags flags, int offset, int stride);
template IMPLOT_API void PlotStairs<ImS16>(const char* label_id, const ImS16* values, int count, double xscale, double x0, ImPlotStairsFlags flags, int offset, int stride);
template IMPLOT_API void PlotStairs<ImU16>(const char* label_id, const ImU16* values, int count, double xscale, double x0, ImPlotStairsFlags flags, int offset, int stride);
template IMPLOT_API void PlotStairs<ImS32>(const char* label_id, const ImS32* values, int count, double xscale, double x0, ImPlotStairsFlags flags, int offset, int stride);
template IMPLOT_API void PlotStairs<ImU32>(const char* label_id, const ImU32* values, int count, double xscale, double x0, ImPlotStairsFlags flags, int offset, int stride);
template IMPLOT_API void PlotStairs<ImS64>(const char* label_id, const ImS64* values, int count, double xscale, double x0, ImPlotStairsFlags flags, int offset, int stride);
template IMPLOT_API void PlotStairs<ImU64>(const char* label_id, const ImU64* values, int count, double xscale, double x0, ImPlotStairsFlags flags, int offset, int stride);
template IMPLOT_API void PlotStairs<float>(const char* label_id, const float* values, int count, double xscale, double x0, ImPlotStairsFlags flags, int offset, int stride);
template IMPLOT_API void PlotStairs<double>(const char* label_id, const double* values, int count, double xscale, double x0, ImPlotStairsFlags flags, int offset, int stride);

template <typename T>
void PlotStairs(const char* label_id, const T* xs, const T* ys, int count, ImPlotStairsFlags flags, int offset, int stride) {
    GetterXY<GetterIdx<T>,GetterIdx<T>> getter(GetterIdx<T>(xs,count,offset,stride),GetterIdx<T>(ys,count,offset,stride),count);
    return PlotStairsEx(label_id, getter, flags);
}

template IMPLOT_API void PlotStairs<ImS8>(const char* label_id, const ImS8* xs, const ImS8* ys, int count, ImPlotStairsFlags flags, int offset, int stride);
template IMPLOT_API void PlotStairs<ImU8>(const char* label_id, const ImU8* xs, const ImU8* ys, int count, ImPlotStairsFlags flags, int offset, int stride);
template IMPLOT_API void PlotStairs<ImS16>(const char* label_id, const ImS16* xs, const ImS16* ys, int count, ImPlotStairsFlags flags, int offset, int stride);
template IMPLOT_API void PlotStairs<ImU16>(const char* label_id, const ImU16* xs, const ImU16* ys, int count, ImPlotStairsFlags flags, int offset, int stride);
template IMPLOT_API void PlotStairs<ImS32>(const char* label_id, const ImS32* xs, const ImS32* ys, int count, ImPlotStairsFlags flags, int offset, int stride);
template IMPLOT_API void PlotStairs<ImU32>(const char* label_id, const ImU32* xs, const ImU32* ys, int count, ImPlotStairsFlags flags, int offset, int stride);
template IMPLOT_API void PlotStairs<ImS64>(const char* label_id, const ImS64* xs, const ImS64* ys, int count, ImPlotStairsFlags flags, int offset, int stride);
template IMPLOT_API void PlotStairs<ImU64>(const char* label_id, const ImU64* xs, const ImU64* ys, int count, ImPlotStairsFlags flags, int offset, int stride);
template IMPLOT_API void PlotStairs<float>(const char* label_id, const float* xs, const float* ys, int count, ImPlotStairsFlags flags, int offset, int stride);
template IMPLOT_API void PlotStairs<double>(const char* label_id, const double* xs, const double* ys, int count, ImPlotStairsFlags flags, int offset, int stride);

// custom
void PlotStairsG(const char* label_id, ImPlotGetter getter_func, void* data, int count, ImPlotStairsFlags flags) {
    GetterFuncPtr getter(getter_func,data, count);
    return PlotStairsEx(label_id, getter, flags);
}

//-----------------------------------------------------------------------------
// PLOT SHADED
//-----------------------------------------------------------------------------

template <typename Getter1, typename Getter2>
IMPLOT_INLINE void PlotShadedEx(const char* label_id, const Getter1& getter1, const Getter2& getter2, ImPlotShadedFlags, bool fit2) {
    if (BeginItem(label_id, ImPlotCol_Fill)) {
        if (FitThisFrame()) {
            for (int i = 0; i < getter1.Count; ++i)
                FitPoint(getter1(i));
            if (fit2) {
                for (int i = 0; i < getter2.Count; ++i)
                    FitPoint(getter2(i));
            }
        }
        const ImPlotNextItemData& s = GetItemData();
        if (s.RenderFill) {
            const ImU32 col = ImGui::GetColorU32(s.Colors[ImPlotCol_Fill]);
            RenderPrimitives2<RendererShaded>(getter1,getter2,col);
        }
        EndItem();
    }
}

template <typename T>
void PlotShaded(const char* label_id, const T* values, int count, double y_ref, double xscale, double x0, ImPlotShadedFlags flags, int offset, int stride) {
    bool fit2 = true;
    if (y_ref == -HUGE_VAL) {
        fit2 = false;
        y_ref = GetPlotLimits(IMPLOT_AUTO,IMPLOT_AUTO).Y.Min;
    }
    if (y_ref == HUGE_VAL) {
        fit2 = false;
        y_ref = GetPlotLimits(IMPLOT_AUTO,IMPLOT_AUTO).Y.Max;
    }
    GetterXY<GetterLin,GetterIdx<T>> getter1(GetterLin(xscale,x0),GetterIdx<T>(values,count,offset,stride),count);
    GetterXY<GetterLin,GetterRef>    getter2(GetterLin(xscale,x0),GetterRef(y_ref),count);
    PlotShadedEx(label_id, getter1, getter2, flags, fit2);
}

template IMPLOT_API void PlotShaded<ImS8>(const char* label_id, const ImS8* values, int count, double y_ref, double xscale, double x0, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<ImU8>(const char* label_id, const ImU8* values, int count, double y_ref, double xscale, double x0, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<ImS16>(const char* label_id, const ImS16* values, int count, double y_ref, double xscale, double x0, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<ImU16>(const char* label_id, const ImU16* values, int count, double y_ref, double xscale, double x0, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<ImS32>(const char* label_id, const ImS32* values, int count, double y_ref, double xscale, double x0, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<ImU32>(const char* label_id, const ImU32* values, int count, double y_ref, double xscale, double x0, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<ImS64>(const char* label_id, const ImS64* values, int count, double y_ref, double xscale, double x0, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<ImU64>(const char* label_id, const ImU64* values, int count, double y_ref, double xscale, double x0, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<float>(const char* label_id, const float* values, int count, double y_ref, double xscale, double x0, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<double>(const char* label_id, const double* values, int count, double y_ref, double xscale, double x0, ImPlotShadedFlags flags, int offset, int stride);

template <typename T>
void PlotShaded(const char* label_id, const T* xs, const T* ys, int count, double y_ref, ImPlotShadedFlags flags, int offset, int stride) {
    bool fit2 = true;
    if (y_ref == -HUGE_VAL) {
        fit2 = false;
        y_ref = GetPlotLimits(IMPLOT_AUTO,IMPLOT_AUTO).Y.Min;
    }
    if (y_ref == HUGE_VAL) {
        fit2 = false;
        y_ref = GetPlotLimits(IMPLOT_AUTO,IMPLOT_AUTO).Y.Max;
    }
    GetterXY<GetterIdx<T>,GetterIdx<T>> getter1(GetterIdx<T>(xs,count,offset,stride),GetterIdx<T>(ys,count,offset,stride),count);
    GetterXY<GetterIdx<T>,GetterRef>    getter2(GetterIdx<T>(xs,count,offset,stride),GetterRef(y_ref),count);
    PlotShadedEx(label_id, getter1, getter2, flags, fit2);
}

template IMPLOT_API void PlotShaded<ImS8>(const char* label_id, const ImS8* xs, const ImS8* ys, int count, double y_ref, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<ImU8>(const char* label_id, const ImU8* xs, const ImU8* ys, int count, double y_ref, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<ImS16>(const char* label_id, const ImS16* xs, const ImS16* ys, int count, double y_ref, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<ImU16>(const char* label_id, const ImU16* xs, const ImU16* ys, int count, double y_ref, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<ImS32>(const char* label_id, const ImS32* xs, const ImS32* ys, int count, double y_ref, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<ImU32>(const char* label_id, const ImU32* xs, const ImU32* ys, int count, double y_ref, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<ImS64>(const char* label_id, const ImS64* xs, const ImS64* ys, int count, double y_ref, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<ImU64>(const char* label_id, const ImU64* xs, const ImU64* ys, int count, double y_ref, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<float>(const char* label_id, const float* xs, const float* ys, int count, double y_ref, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<double>(const char* label_id, const double* xs, const double* ys, int count, double y_ref, ImPlotShadedFlags flags, int offset, int stride);

template <typename T>
void PlotShaded(const char* label_id, const T* xs, const T* ys1, const T* ys2, int count, ImPlotShadedFlags flags, int offset, int stride) {
    GetterXY<GetterIdx<T>,GetterIdx<T>> getter1(GetterIdx<T>(xs,count,offset,stride),GetterIdx<T>(ys1,count,offset,stride),count);
    GetterXY<GetterIdx<T>,GetterIdx<T>> getter2(GetterIdx<T>(xs,count,offset,stride),GetterIdx<T>(ys2,count,offset,stride),count);
    PlotShadedEx(label_id, getter1, getter2, flags, true);
}

template IMPLOT_API void PlotShaded<ImS8>(const char* label_id, const ImS8* xs, const ImS8* ys1, const ImS8* ys2, int count, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<ImU8>(const char* label_id, const ImU8* xs, const ImU8* ys1, const ImU8* ys2, int count, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<ImS16>(const char* label_id, const ImS16* xs, const ImS16* ys1, const ImS16* ys2, int count, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<ImU16>(const char* label_id, const ImU16* xs, const ImU16* ys1, const ImU16* ys2, int count, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<ImS32>(const char* label_id, const ImS32* xs, const ImS32* ys1, const ImS32* ys2, int count, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<ImU32>(const char* label_id, const ImU32* xs, const ImU32* ys1, const ImU32* ys2, int count, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<ImS64>(const char* label_id, const ImS64* xs, const ImS64* ys1, const ImS64* ys2, int count, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<ImU64>(const char* label_id, const ImU64* xs, const ImU64* ys1, const ImU64* ys2, int count, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<float>(const char* label_id, const float* xs, const float* ys1, const float* ys2, int count, ImPlotShadedFlags flags, int offset, int stride);
template IMPLOT_API void PlotShaded<double>(const char* label_id, const double* xs, const double* ys1, const double* ys2, int count, ImPlotShadedFlags flags, int offset, int stride);

// custom
void PlotShadedG(const char* label_id, ImPlotGetter getter_func1, void* data1, ImPlotGetter getter_func2, void* data2, int count, ImPlotShadedFlags flags) {
    GetterFuncPtr getter1(getter_func1, data1, count);
    GetterFuncPtr getter2(getter_func2, data2, count);
    PlotShadedEx(label_id, getter1, getter2, flags, true);
}

//-----------------------------------------------------------------------------
// PLOT BAR
//-----------------------------------------------------------------------------

template <typename Getter1, typename Getter2>
void PlotBarsEx(const char* label_id, const Getter1& getter1, const Getter2 getter2, double width, ImPlotBarsFlags) {
    if (BeginItem(label_id, ImPlotCol_Fill)) {
        const double half_width = width / 2;
        if (FitThisFrame()) {
            for (int i = 0; i < getter1.Count; ++i) {
                ImPlotPoint p1 = getter1(i);
                ImPlotPoint p2 = getter2(i);
                FitPoint(ImPlotPoint(p1.x - half_width, p1.y));
                FitPoint(ImPlotPoint(p2.x + half_width, p2.y));
            }
        }
        const ImPlotNextItemData& s = GetItemData();
        ImU32 col_line  = ImGui::GetColorU32(s.Colors[ImPlotCol_Line]);
        ImU32 col_fill  = ImGui::GetColorU32(s.Colors[ImPlotCol_Fill]);
        bool  rend_line = s.RenderLine;
        if (s.RenderFill && col_line == col_fill)
            rend_line = false;
        if (s.RenderFill)
            RenderPrimitives2<RendererBarsV>(getter1,getter2,col_fill,width);
        // if (rend_line) 
            // TODO
        EndItem();
    }
}


template <typename Getter1, typename Getter2>
void PlotBarsHEx(const char* label_id, const Getter1& getter1, const Getter2& getter2, double height, ImPlotBarsFlags) {
    if (BeginItem(label_id, ImPlotCol_Fill)) {
        const double half_height = height / 2;
        if (FitThisFrame()) {
            for (int i = 0; i < getter1.Count; ++i) {
                ImPlotPoint p1 = getter1(i);
                ImPlotPoint p2 = getter2(i);
                FitPoint(ImPlotPoint(p1.x, p1.y - half_height));
                FitPoint(ImPlotPoint(p2.x, p2.y + half_height));
            }
        }
        const ImPlotNextItemData& s = GetItemData();
        ImU32 col_line  = ImGui::GetColorU32(s.Colors[ImPlotCol_Line]);
        ImU32 col_fill  = ImGui::GetColorU32(s.Colors[ImPlotCol_Fill]);
        bool  rend_line = s.RenderLine;
        if (s.RenderFill && col_line == col_fill)
            rend_line = false;
        if (s.RenderFill)
            RenderPrimitives2<RendererBarsH>(getter1,getter2,col_fill,height);
        // if (rend_line) 
            // TODO
        EndItem();
    }
}

template <typename T>
void PlotBars(const char* label_id, const T* values, int count, double bar_size, double shift, ImPlotBarsFlags flags, int offset, int stride) {
    if (ImHasFlag(flags, ImPlotBarsFlags_Horizontal)) {
        GetterXY<GetterIdx<T>,GetterLin> getter1(GetterIdx<T>(values,count,offset,stride),GetterLin(1.0,shift),count);
        GetterXY<GetterRef,GetterLin>    getter2(GetterRef(0),GetterLin(1.0,shift),count);
        PlotBarsHEx(label_id, getter1, getter2, bar_size, flags);
    }
    else {
        GetterXY<GetterLin,GetterIdx<T>> getter1(GetterLin(1.0,shift),GetterIdx<T>(values,count,offset,stride),count);
        GetterXY<GetterLin,GetterRef>    getter2(GetterLin(1.0,shift),GetterRef(0),count);
        PlotBarsEx(label_id, getter1, getter2, bar_size, flags);
    }
}

template IMPLOT_API void PlotBars<ImS8>(const char* label_id, const ImS8* values, int count, double bar_size, double shift, ImPlotBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotBars<ImU8>(const char* label_id, const ImU8* values, int count, double bar_size, double shift, ImPlotBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotBars<ImS16>(const char* label_id, const ImS16* values, int count, double bar_size, double shift, ImPlotBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotBars<ImU16>(const char* label_id, const ImU16* values, int count, double bar_size, double shift, ImPlotBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotBars<ImS32>(const char* label_id, const ImS32* values, int count, double bar_size, double shift, ImPlotBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotBars<ImU32>(const char* label_id, const ImU32* values, int count, double bar_size, double shift, ImPlotBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotBars<ImS64>(const char* label_id, const ImS64* values, int count, double bar_size, double shift, ImPlotBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotBars<ImU64>(const char* label_id, const ImU64* values, int count, double bar_size, double shift, ImPlotBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotBars<float>(const char* label_id, const float* values, int count, double bar_size, double shift, ImPlotBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotBars<double>(const char* label_id, const double* values, int count, double bar_size, double shift, ImPlotBarsFlags flags, int offset, int stride);

template <typename T>
void PlotBars(const char* label_id, const T* xs, const T* ys, int count, double bar_size, ImPlotBarsFlags flags, int offset, int stride) {
    if (ImHasFlag(flags, ImPlotBarsFlags_Horizontal)) {
        GetterXY<GetterIdx<T>,GetterIdx<T>> getter1(GetterIdx<T>(xs,count,offset,stride),GetterIdx<T>(ys,count,offset,stride),count);
        GetterXY<GetterRef,   GetterIdx<T>> getter2(GetterRef(0),GetterIdx<T>(ys,count,offset,stride),count);
        PlotBarsHEx(label_id, getter1, getter2, bar_size, flags);
    }
    else {
        GetterXY<GetterIdx<T>,GetterIdx<T>> getter1(GetterIdx<T>(xs,count,offset,stride),GetterIdx<T>(ys,count,offset,stride),count);
        GetterXY<GetterIdx<T>,GetterRef>    getter2(GetterIdx<T>(xs,count,offset,stride),GetterRef(0),count);
        PlotBarsEx(label_id, getter1, getter2, bar_size, flags);
    }
}

template IMPLOT_API void PlotBars<ImS8>(const char* label_id, const ImS8* xs, const ImS8* ys, int count, double bar_size, ImPlotBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotBars<ImU8>(const char* label_id, const ImU8* xs, const ImU8* ys, int count, double bar_size, ImPlotBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotBars<ImS16>(const char* label_id, const ImS16* xs, const ImS16* ys, int count, double bar_size, ImPlotBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotBars<ImU16>(const char* label_id, const ImU16* xs, const ImU16* ys, int count, double bar_size, ImPlotBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotBars<ImS32>(const char* label_id, const ImS32* xs, const ImS32* ys, int count, double bar_size, ImPlotBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotBars<ImU32>(const char* label_id, const ImU32* xs, const ImU32* ys, int count, double bar_size, ImPlotBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotBars<ImS64>(const char* label_id, const ImS64* xs, const ImS64* ys, int count, double bar_size, ImPlotBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotBars<ImU64>(const char* label_id, const ImU64* xs, const ImU64* ys, int count, double bar_size, ImPlotBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotBars<float>(const char* label_id, const float* xs, const float* ys, int count, double bar_size, ImPlotBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotBars<double>(const char* label_id, const double* xs, const double* ys, int count, double bar_size, ImPlotBarsFlags flags, int offset, int stride);

void PlotBarsG(const char* label_id, ImPlotGetter getter_func, void* data, int count, double bar_size, ImPlotBarsFlags flags) {
    if (ImHasFlag(flags, ImPlotBarsFlags_Horizontal)) {
        GetterFuncPtr getter1(getter_func, data, count);
        GetterOverrideX<GetterFuncPtr> getter2(getter1,0);
        PlotBarsHEx(label_id, getter1, getter2, bar_size, flags);
    }
    else {
        GetterFuncPtr getter1(getter_func, data, count);
        GetterOverrideY<GetterFuncPtr> getter2(getter1,0);
        PlotBarsEx(label_id, getter1, getter2, bar_size, flags);
    }
}

//-----------------------------------------------------------------------------
// PLOT BAR GROUPS
//-----------------------------------------------------------------------------

template <typename T>
void PlotBarGroups(const char* const label_ids[], const T* values, int item_count, int group_count, double group_size, double shift, ImPlotBarGroupsFlags flags) {
    const bool horz = ImHasFlag(flags, ImPlotBarGroupsFlags_Horizontal);
    const bool stack = ImHasFlag(flags, ImPlotBarGroupsFlags_Stacked);
    if (ImHasFlag(flags, ImPlotBarGroupsFlags_Stacked)) {
        SetupLock();
        GImPlot->TempDouble1.resize(4*group_count);
        double* temp = GImPlot->TempDouble1.Data;
        double* neg =      &temp[0];
        double* pos =      &temp[group_count];
        double* curr_min = &temp[group_count*2];
        double* curr_max = &temp[group_count*3];
        for (int g = 0; g < group_count*2; ++g)
            temp[g] = 0;
        if (horz) {
            for (int i = 0; i < item_count; ++i) {
                if (!IsItemHidden(label_ids[i])) {
                    for (int g = 0; g < group_count; ++g) {
                        double v = (double)values[i*group_count+g];
                        if (v > 0) {
                            curr_min[g] = pos[g];
                            curr_max[g] = curr_min[g] + v;
                            pos[g]      += v;
                        }
                        else {
                            curr_max[g] = neg[g];
                            curr_min[g] = curr_max[g] + v;
                            neg[g]      += v;
                        }
                    }
                }
                GetterXY<GetterIdx<double>,GetterLin> getter1(GetterIdx<double>(curr_min,group_count),GetterLin(1.0,shift),group_count);
                GetterXY<GetterIdx<double>,GetterLin> getter2(GetterIdx<double>(curr_max,group_count),GetterLin(1.0,shift),group_count);
                PlotBarsHEx(label_ids[i],getter1,getter2,group_size,0);
            }
        }
        else {
            for (int i = 0; i < item_count; ++i) {
                if (!IsItemHidden(label_ids[i])) {
                    for (int g = 0; g < group_count; ++g) {
                        double v = (double)values[i*group_count+g];
                        if (v > 0) {
                            curr_min[g] = pos[g];
                            curr_max[g] = curr_min[g] + v;
                            pos[g]      += v;
                        }
                        else {
                            curr_max[g] = neg[g];
                            curr_min[g] = curr_max[g] + v;
                            neg[g]      += v;
                        }
                    }
                }
                GetterXY<GetterLin,GetterIdx<double>> getter1(GetterLin(1.0,shift),GetterIdx<double>(curr_min,group_count),group_count);
                GetterXY<GetterLin,GetterIdx<double>> getter2(GetterLin(1.0,shift),GetterIdx<double>(curr_max,group_count),group_count);
                PlotBarsEx(label_ids[i],getter1,getter2,group_size,0);
            }
        }
    }
    else {
        const double subsize = group_size / item_count;
        if (horz) {
            for (int i = 0; i < item_count; ++i) {
                const double subshift = (i+0.5)*subsize - group_size/2;
                PlotBars(label_ids[i],&values[i*group_count],group_count,subsize,subshift+shift,ImPlotBarsFlags_Horizontal);
            }
        }
        else {
            for (int i = 0; i < item_count; ++i) {
                const double subshift = (i+0.5)*subsize - group_size/2;
                PlotBars(label_ids[i],&values[i*group_count],group_count,subsize,subshift+shift);
            }
        }
    }
}

template IMPLOT_API void PlotBarGroups<ImS8>(const char* const label_ids[], const ImS8* values, int items, int groups, double width, double shift, ImPlotBarGroupsFlags flags);
template IMPLOT_API void PlotBarGroups<ImU8>(const char* const label_ids[], const ImU8* values, int items, int groups, double width, double shift, ImPlotBarGroupsFlags flags);
template IMPLOT_API void PlotBarGroups<ImS16>(const char* const label_ids[], const ImS16* values, int items, int groups, double width, double shift, ImPlotBarGroupsFlags flags);
template IMPLOT_API void PlotBarGroups<ImU16>(const char* const label_ids[], const ImU16* values, int items, int groups, double width, double shift, ImPlotBarGroupsFlags flags);
template IMPLOT_API void PlotBarGroups<ImS32>(const char* const label_ids[], const ImS32* values, int items, int groups, double width, double shift, ImPlotBarGroupsFlags flags);
template IMPLOT_API void PlotBarGroups<ImU32>(const char* const label_ids[], const ImU32* values, int items, int groups, double width, double shift, ImPlotBarGroupsFlags flags);
template IMPLOT_API void PlotBarGroups<ImS64>(const char* const label_ids[], const ImS64* values, int items, int groups, double width, double shift, ImPlotBarGroupsFlags flags);
template IMPLOT_API void PlotBarGroups<ImU64>(const char* const label_ids[], const ImU64* values, int items, int groups, double width, double shift, ImPlotBarGroupsFlags flags);
template IMPLOT_API void PlotBarGroups<float>(const char* const label_ids[], const float* values, int items, int groups, double width, double shift, ImPlotBarGroupsFlags flags);
template IMPLOT_API void PlotBarGroups<double>(const char* const label_ids[], const double* values, int items, int groups, double width, double shift, ImPlotBarGroupsFlags flags);

//-----------------------------------------------------------------------------
// PLOT ERROR BARS
//-----------------------------------------------------------------------------

template <typename Getter>
void PlotErrorBarsEx(const char* label_id, const Getter& getter, ImPlotErrorBarsFlags flags) {
    if (BeginItem(label_id)) {

        const ImPlotNextItemData& s = GetItemData();
        ImDrawList& draw_list = *GetPlotDrawList();
        const ImU32 col = ImGui::GetColorU32(s.Colors[ImPlotCol_ErrorBar]);
        const bool rend_whisker  = s.ErrorBarSize > 0;
        const float half_whisker = s.ErrorBarSize * 0.5f;

        if (ImHasFlag(flags, ImPlotErrorBarsFlags_Horizontal)) {
            if (FitThisFrame()) {
                for (int i = 0; i < getter.Count; ++i) {
                    ImPlotPointError e = getter(i);
                    FitPoint(ImPlotPoint(e.X - e.Neg, e.Y));
                    FitPoint(ImPlotPoint(e.X + e.Pos, e.Y));
                }
            }
            for (int i = 0; i < getter.Count; ++i) {
                ImPlotPointError e = getter(i);
                ImVec2 p1 = PlotToPixels(e.X - e.Neg, e.Y,IMPLOT_AUTO,IMPLOT_AUTO);
                ImVec2 p2 = PlotToPixels(e.X + e.Pos, e.Y,IMPLOT_AUTO,IMPLOT_AUTO);
                draw_list.AddLine(p1, p2, col, s.ErrorBarWeight);
                if (rend_whisker) {
                    draw_list.AddLine(p1 - ImVec2(0, half_whisker), p1 + ImVec2(0, half_whisker), col, s.ErrorBarWeight);
                    draw_list.AddLine(p2 - ImVec2(0, half_whisker), p2 + ImVec2(0, half_whisker), col, s.ErrorBarWeight);
                }
            }            
        }
        else {
            if (FitThisFrame()) {
                for (int i = 0; i < getter.Count; ++i) {
                    ImPlotPointError e = getter(i);
                    FitPoint(ImPlotPoint(e.X , e.Y - e.Neg));
                    FitPoint(ImPlotPoint(e.X , e.Y + e.Pos ));
                }
            }        
            for (int i = 0; i < getter.Count; ++i) {
                ImPlotPointError e = getter(i);
                ImVec2 p1 = PlotToPixels(e.X, e.Y - e.Neg,IMPLOT_AUTO,IMPLOT_AUTO);
                ImVec2 p2 = PlotToPixels(e.X, e.Y + e.Pos,IMPLOT_AUTO,IMPLOT_AUTO);
                draw_list.AddLine(p1,p2,col, s.ErrorBarWeight);
                if (rend_whisker) {
                    draw_list.AddLine(p1 - ImVec2(half_whisker, 0), p1 + ImVec2(half_whisker, 0), col, s.ErrorBarWeight);
                    draw_list.AddLine(p2 - ImVec2(half_whisker, 0), p2 + ImVec2(half_whisker, 0), col, s.ErrorBarWeight);
                }
            }
        }
        EndItem();
    }
}

template <typename T>
void PlotErrorBars(const char* label_id, const T* xs, const T* ys, const T* err, int count, ImPlotErrorBarsFlags flags, int offset, int stride) {
    GetterError<T> getter(xs, ys, err, err, count, offset, stride);
    PlotErrorBarsEx(label_id, getter, flags);
}

template IMPLOT_API void PlotErrorBars<ImS8>(const char* label_id, const ImS8* xs, const ImS8* ys, const ImS8* err, int count, ImPlotErrorBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotErrorBars<ImU8>(const char* label_id, const ImU8* xs, const ImU8* ys, const ImU8* err, int count, ImPlotErrorBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotErrorBars<ImS16>(const char* label_id, const ImS16* xs, const ImS16* ys, const ImS16* err, int count, ImPlotErrorBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotErrorBars<ImU16>(const char* label_id, const ImU16* xs, const ImU16* ys, const ImU16* err, int count, ImPlotErrorBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotErrorBars<ImS32>(const char* label_id, const ImS32* xs, const ImS32* ys, const ImS32* err, int count, ImPlotErrorBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotErrorBars<ImU32>(const char* label_id, const ImU32* xs, const ImU32* ys, const ImU32* err, int count, ImPlotErrorBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotErrorBars<ImS64>(const char* label_id, const ImS64* xs, const ImS64* ys, const ImS64* err, int count, ImPlotErrorBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotErrorBars<ImU64>(const char* label_id, const ImU64* xs, const ImU64* ys, const ImU64* err, int count, ImPlotErrorBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotErrorBars<float>(const char* label_id, const float* xs, const float* ys, const float* err, int count, ImPlotErrorBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotErrorBars<double>(const char* label_id, const double* xs, const double* ys, const double* err, int count, ImPlotErrorBarsFlags flags, int offset, int stride);

template <typename T>
void PlotErrorBars(const char* label_id, const T* xs, const T* ys, const T* neg, const T* pos, int count, ImPlotErrorBarsFlags flags, int offset, int stride) {
    GetterError<T> getter(xs, ys, neg, pos, count, offset, stride);
    PlotErrorBarsEx(label_id, getter, flags);
}

template IMPLOT_API void PlotErrorBars<ImS8>(const char* label_id, const ImS8* xs, const ImS8* ys, const ImS8* neg, const ImS8* pos, int count, ImPlotErrorBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotErrorBars<ImU8>(const char* label_id, const ImU8* xs, const ImU8* ys, const ImU8* neg, const ImU8* pos, int count, ImPlotErrorBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotErrorBars<ImS16>(const char* label_id, const ImS16* xs, const ImS16* ys, const ImS16* neg, const ImS16* pos, int count, ImPlotErrorBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotErrorBars<ImU16>(const char* label_id, const ImU16* xs, const ImU16* ys, const ImU16* neg, const ImU16* pos, int count, ImPlotErrorBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotErrorBars<ImS32>(const char* label_id, const ImS32* xs, const ImS32* ys, const ImS32* neg, const ImS32* pos, int count, ImPlotErrorBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotErrorBars<ImU32>(const char* label_id, const ImU32* xs, const ImU32* ys, const ImU32* neg, const ImU32* pos, int count, ImPlotErrorBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotErrorBars<ImS64>(const char* label_id, const ImS64* xs, const ImS64* ys, const ImS64* neg, const ImS64* pos, int count, ImPlotErrorBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotErrorBars<ImU64>(const char* label_id, const ImU64* xs, const ImU64* ys, const ImU64* neg, const ImU64* pos, int count, ImPlotErrorBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotErrorBars<float>(const char* label_id, const float* xs, const float* ys, const float* neg, const float* pos, int count, ImPlotErrorBarsFlags flags, int offset, int stride);
template IMPLOT_API void PlotErrorBars<double>(const char* label_id, const double* xs, const double* ys, const double* neg, const double* pos, int count, ImPlotErrorBarsFlags flags, int offset, int stride);

//-----------------------------------------------------------------------------
// PLOT STEMS
//-----------------------------------------------------------------------------

template <typename GetterM, typename GetterB>
IMPLOT_INLINE void PlotStemsEx(const char* label_id, const GetterM& get_mark, const GetterB& get_base) {
    if (BeginItem(label_id, ImPlotCol_Line)) {
        if (FitThisFrame()) {
            for (int i = 0; i < get_base.Count; ++i) {
                FitPoint(get_mark(i));
                FitPoint(get_base(i));
            }
        }
        const ImPlotNextItemData& s = GetItemData();
        ImDrawList& draw_list = *GetPlotDrawList();
        // render stems
        if (s.RenderLine) {
            const ImU32 col_line = ImGui::GetColorU32(s.Colors[ImPlotCol_Line]);
            RenderPrimitives2<RendererLineSegments2>(get_mark, get_base, col_line, s.LineWeight);
        }
        // render markers
        ImPlotMarker marker = s.Marker == ImPlotMarker_None ? ImPlotMarker_Circle : s.Marker;
        if (marker != ImPlotMarker_None) {
            PopPlotClipRect();
            PushPlotClipRect(s.MarkerSize);
            const ImU32 col_line = ImGui::GetColorU32(s.Colors[ImPlotCol_MarkerOutline]);
            const ImU32 col_fill = ImGui::GetColorU32(s.Colors[ImPlotCol_MarkerFill]);
            switch (GetCurrentScale()) {
                case ImPlotScale_LinLin: RenderMarkers(get_mark, TransformerLinLin(), draw_list, marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
                case ImPlotScale_LogLin: RenderMarkers(get_mark, TransformerLogLin(), draw_list, marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
                case ImPlotScale_LinLog: RenderMarkers(get_mark, TransformerLinLog(), draw_list, marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
                case ImPlotScale_LogLog: RenderMarkers(get_mark, TransformerLogLog(), draw_list, marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
            }
        }
        EndItem();
    }
}

template <typename T>
void PlotStems(const char* label_id, const T* values, int count, double y_ref, double xscale, double x0, int offset, int stride) {
    GetterXY<GetterLin,GetterIdx<T>> get_mark(GetterLin(xscale,x0),GetterIdx<T>(values,count,offset,stride),count);
    GetterXY<GetterLin,GetterRef>    get_base(GetterLin(xscale,x0),GetterRef(y_ref),count);
    PlotStemsEx(label_id, get_mark, get_base);
}

template IMPLOT_API void PlotStems<ImS8>(const char* label_id, const ImS8* values, int count, double y_ref, double xscale, double x0, int offset, int stride);
template IMPLOT_API void PlotStems<ImU8>(const char* label_id, const ImU8* values, int count, double y_ref, double xscale, double x0, int offset, int stride);
template IMPLOT_API void PlotStems<ImS16>(const char* label_id, const ImS16* values, int count, double y_ref, double xscale, double x0, int offset, int stride);
template IMPLOT_API void PlotStems<ImU16>(const char* label_id, const ImU16* values, int count, double y_ref, double xscale, double x0, int offset, int stride);
template IMPLOT_API void PlotStems<ImS32>(const char* label_id, const ImS32* values, int count, double y_ref, double xscale, double x0, int offset, int stride);
template IMPLOT_API void PlotStems<ImU32>(const char* label_id, const ImU32* values, int count, double y_ref, double xscale, double x0, int offset, int stride);
template IMPLOT_API void PlotStems<ImS64>(const char* label_id, const ImS64* values, int count, double y_ref, double xscale, double x0, int offset, int stride);
template IMPLOT_API void PlotStems<ImU64>(const char* label_id, const ImU64* values, int count, double y_ref, double xscale, double x0, int offset, int stride);
template IMPLOT_API void PlotStems<float>(const char* label_id, const float* values, int count, double y_ref, double xscale, double x0, int offset, int stride);
template IMPLOT_API void PlotStems<double>(const char* label_id, const double* values, int count, double y_ref, double xscale, double x0, int offset, int stride);

template <typename T>
void PlotStems(const char* label_id, const T* xs, const T* ys, int count, double y_ref, int offset, int stride) {
    GetterXY<GetterIdx<T>,GetterIdx<T>> get_mark(GetterIdx<T>(xs,count,offset,stride),GetterIdx<T>(ys,count,offset,stride),count);
    GetterXY<GetterIdx<T>,GetterRef>    get_base(GetterIdx<T>(xs,count,offset,stride),GetterRef(y_ref),count);
    PlotStemsEx(label_id, get_mark, get_base);
}

template IMPLOT_API void PlotStems<ImS8>(const char* label_id, const ImS8* xs, const ImS8* ys, int count, double y_ref, int offset, int stride);
template IMPLOT_API void PlotStems<ImU8>(const char* label_id, const ImU8* xs, const ImU8* ys, int count, double y_ref, int offset, int stride);
template IMPLOT_API void PlotStems<ImS16>(const char* label_id, const ImS16* xs, const ImS16* ys, int count, double y_ref, int offset, int stride);
template IMPLOT_API void PlotStems<ImU16>(const char* label_id, const ImU16* xs, const ImU16* ys, int count, double y_ref, int offset, int stride);
template IMPLOT_API void PlotStems<ImS32>(const char* label_id, const ImS32* xs, const ImS32* ys, int count, double y_ref, int offset, int stride);
template IMPLOT_API void PlotStems<ImU32>(const char* label_id, const ImU32* xs, const ImU32* ys, int count, double y_ref, int offset, int stride);
template IMPLOT_API void PlotStems<ImS64>(const char* label_id, const ImS64* xs, const ImS64* ys, int count, double y_ref, int offset, int stride);
template IMPLOT_API void PlotStems<ImU64>(const char* label_id, const ImU64* xs, const ImU64* ys, int count, double y_ref, int offset, int stride);
template IMPLOT_API void PlotStems<float>(const char* label_id, const float* xs, const float* ys, int count, double y_ref, int offset, int stride);
template IMPLOT_API void PlotStems<double>(const char* label_id, const double* xs, const double* ys, int count, double y_ref, int offset, int stride);

//-----------------------------------------------------------------------------
// INFINITE LINES
//-----------------------------------------------------------------------------

template <typename T>
void PlotInfLines(const char* label_id, const T* values, int count, ImPlotInfLinesFlags flags, int offset, int stride) {
    if (ImHasFlag(flags, ImPlotInfLinesFlags_Horizontal)) {
        if (BeginItem(label_id, ImPlotCol_Line)) {
            const ImPlotRect lims = GetPlotLimits(IMPLOT_AUTO,IMPLOT_AUTO);
            GetterXY<GetterRef,GetterIdx<T>> get_min(GetterRef(lims.X.Min),GetterIdx<T>(values,count,offset,stride),count);
            GetterXY<GetterRef,GetterIdx<T>> get_max(GetterRef(lims.X.Max),GetterIdx<T>(values,count,offset,stride),count);
            if (FitThisFrame()) {
                for (int i = 0; i < get_min.Count; ++i)
                    FitPointY(get_min(i).y);
            }
            const ImPlotNextItemData& s = GetItemData();
            if (s.RenderLine) {
                const ImU32 col_line = ImGui::GetColorU32(s.Colors[ImPlotCol_Line]);
                RenderPrimitives2<RendererLineSegments2>(get_min, get_max, col_line, s.LineWeight); 
            }
            EndItem();
        }
    }
    else {
        if (BeginItem(label_id, ImPlotCol_Line)) {
            const ImPlotRect lims = GetPlotLimits(IMPLOT_AUTO,IMPLOT_AUTO);
            GetterXY<GetterIdx<T>,GetterRef> get_min(GetterIdx<T>(values,count,offset,stride),GetterRef(lims.Y.Min),count);
            GetterXY<GetterIdx<T>,GetterRef> get_max(GetterIdx<T>(values,count,offset,stride),GetterRef(lims.Y.Max),count);
            if (FitThisFrame()) {
                for (int i = 0; i < get_min.Count; ++i)
                    FitPointX(get_min(i).x);
            }
            const ImPlotNextItemData& s = GetItemData();
            if (s.RenderLine) {
                const ImU32 col_line = ImGui::GetColorU32(s.Colors[ImPlotCol_Line]);
                RenderPrimitives2<RendererLineSegments2>(get_min, get_max, col_line, s.LineWeight);
            }
            EndItem();
        }
    }
}

template IMPLOT_API void PlotInfLines<ImS8>(const char* label_id, const ImS8* xs, int count, ImPlotInfLinesFlags flags, int offset, int stride);
template IMPLOT_API void PlotInfLines<ImU8>(const char* label_id, const ImU8* xs, int count, ImPlotInfLinesFlags flags, int offset, int stride);
template IMPLOT_API void PlotInfLines<ImS16>(const char* label_id, const ImS16* xs, int count, ImPlotInfLinesFlags flags, int offset, int stride);
template IMPLOT_API void PlotInfLines<ImU16>(const char* label_id, const ImU16* xs, int count, ImPlotInfLinesFlags flags, int offset, int stride);
template IMPLOT_API void PlotInfLines<ImS32>(const char* label_id, const ImS32* xs, int count, ImPlotInfLinesFlags flags, int offset, int stride);
template IMPLOT_API void PlotInfLines<ImU32>(const char* label_id, const ImU32* xs, int count, ImPlotInfLinesFlags flags, int offset, int stride);
template IMPLOT_API void PlotInfLines<ImS64>(const char* label_id, const ImS64* xs, int count, ImPlotInfLinesFlags flags, int offset, int stride);
template IMPLOT_API void PlotInfLines<ImU64>(const char* label_id, const ImU64* xs, int count, ImPlotInfLinesFlags flags, int offset, int stride);
template IMPLOT_API void PlotInfLines<float>(const char* label_id, const float* xs, int count, ImPlotInfLinesFlags flags, int offset, int stride);
template IMPLOT_API void PlotInfLines<double>(const char* label_id, const double* xs, int count, ImPlotInfLinesFlags flags, int offset, int stride);

//-----------------------------------------------------------------------------
// PLOT PIE CHART
//-----------------------------------------------------------------------------

IMPLOT_INLINE void RenderPieSlice(ImDrawList& draw_list, const ImPlotPoint& center, double radius, double a0, double a1, ImU32 col) {
    static const float resolution = 50 / (2 * IM_PI);
    static ImVec2 buffer[50];
    buffer[0] = PlotToPixels(center,IMPLOT_AUTO,IMPLOT_AUTO);
    int n = ImMax(3, (int)((a1 - a0) * resolution));
    double da = (a1 - a0) / (n - 1);
    for (int i = 0; i < n; ++i) {
        double a = a0 + i * da;
        buffer[i + 1] = PlotToPixels(center.x + radius * cos(a), center.y + radius * sin(a),IMPLOT_AUTO,IMPLOT_AUTO);
    }
    draw_list.AddConvexPolyFilled(buffer, n + 1, col);
}

template <typename T>
void PlotPieChart(const char* const label_ids[], const T* values, int count, double x, double y, double radius, const char* fmt, double angle0, ImPlotPieChartFlags flags) {
    IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot != NULL, "PlotPieChart() needs to be called between BeginPlot() and EndPlot()!");
    ImDrawList & draw_list = *GetPlotDrawList();
    double sum = 0;
    for (int i = 0; i < count; ++i)
        sum += (double)values[i];
    const bool normalize = ImHasFlag(flags,ImPlotPieChartFlags_Normalize) || sum > 1.0;
    ImPlotPoint center(x,y);
    PushPlotClipRect();
    double a0 = angle0 * 2 * IM_PI / 360.0;
    double a1 = angle0 * 2 * IM_PI / 360.0;
    for (int i = 0; i < count; ++i) {
        double percent = normalize ? (double)values[i] / sum : (double)values[i];
        a1 = a0 + 2 * IM_PI * percent;
        if (BeginItem(label_ids[i])) {
            if (FitThisFrame()) {
                FitPoint(ImPlotPoint(x-radius,y-radius));
                FitPoint(ImPlotPoint(x+radius,y+radius));
            }
            ImU32 col = GetCurrentItem()->Color;
            if (percent < 0.5) {
                RenderPieSlice(draw_list, center, radius, a0, a1, col);
            }
            else  {
                RenderPieSlice(draw_list, center, radius, a0, a0 + (a1 - a0) * 0.5, col);
                RenderPieSlice(draw_list, center, radius, a0 + (a1 - a0) * 0.5, a1, col);
            }
            EndItem();
        }
        a0 = a1;
    }
    if (fmt != NULL) {
        a0 = angle0 * 2 * IM_PI / 360.0;
        a1 = angle0 * 2 * IM_PI / 360.0;
        char buffer[32];
        for (int i = 0; i < count; ++i) {
            ImPlotItem* item = GetItem(label_ids[i]);
            double percent = normalize ? (double)values[i] / sum : (double)values[i];
            a1 = a0 + 2 * IM_PI * percent;
            if (item->Show) {
                sprintf(buffer, fmt, (double)values[i]);
                ImVec2 size = ImGui::CalcTextSize(buffer);
                double angle = a0 + (a1 - a0) * 0.5;
                ImVec2 pos = PlotToPixels(center.x + 0.5 * radius * cos(angle), center.y + 0.5 * radius * sin(angle),IMPLOT_AUTO,IMPLOT_AUTO);
                ImU32 col  = CalcTextColor(ImGui::ColorConvertU32ToFloat4(item->Color));
                draw_list.AddText(pos - size * 0.5f, col, buffer);
            }
            a0 = a1;
        }
    }
    PopPlotClipRect();
}

template IMPLOT_API void PlotPieChart<ImS8>(const char* const label_ids[], const ImS8* values, int count, double x, double y, double radius, const char* fmt, double angle0, ImPlotPieChartFlags flags);
template IMPLOT_API void PlotPieChart<ImU8>(const char* const label_ids[], const ImU8* values, int count, double x, double y, double radius, const char* fmt, double angle0, ImPlotPieChartFlags flags);
template IMPLOT_API void PlotPieChart<ImS16>(const char* const label_ids[], const ImS16* values, int count, double x, double y, double radius, const char* fmt, double angle0, ImPlotPieChartFlags flags);
template IMPLOT_API void PlotPieChart<ImU16>(const char* const label_ids[], const ImU16* values, int count, double x, double y, double radius, const char* fmt, double angle0, ImPlotPieChartFlags flags);
template IMPLOT_API void PlotPieChart<ImS32>(const char* const label_ids[], const ImS32* values, int count, double x, double y, double radius, const char* fmt, double angle0, ImPlotPieChartFlags flags);
template IMPLOT_API void PlotPieChart<ImU32>(const char* const label_ids[], const ImU32* values, int count, double x, double y, double radius, const char* fmt, double angle0, ImPlotPieChartFlags flags);
template IMPLOT_API void PlotPieChart<ImS64>(const char* const label_ids[], const ImS64* values, int count, double x, double y, double radius, const char* fmt, double angle0, ImPlotPieChartFlags flags);
template IMPLOT_API void PlotPieChart<ImU64>(const char* const label_ids[], const ImU64* values, int count, double x, double y, double radius, const char* fmt, double angle0, ImPlotPieChartFlags flags);
template IMPLOT_API void PlotPieChart<float>(const char* const label_ids[], const float* values, int count, double x, double y, double radius, const char* fmt, double angle0, ImPlotPieChartFlags flags);
template IMPLOT_API void PlotPieChart<double>(const char* const label_ids[], const double* values, int count, double x, double y, double radius, const char* fmt, double angle0, ImPlotPieChartFlags flags);

//-----------------------------------------------------------------------------
// PLOT HEATMAP
//-----------------------------------------------------------------------------

template <typename T>
struct GetterHeatmapRowMaj {
    GetterHeatmapRowMaj(const T* values, int rows, int cols, double scale_min, double scale_max, double width, double height, double xref, double yref, double ydir) :
        Values(values),
        Count(rows*cols),
        Rows(rows),
        Cols(cols),
        ScaleMin(scale_min),
        ScaleMax(scale_max),
        Width(width),
        Height(height),
        XRef(xref),
        YRef(yref),
        YDir(ydir),
        HalfSize(Width*0.5, Height*0.5)
    { }
    template <typename I> IMPLOT_INLINE RectC operator()(I idx) const {
        double val = (double)Values[idx];
        const int r = idx / Cols;
        const int c = idx % Cols;
        const ImPlotPoint p(XRef + HalfSize.x + c*Width, YRef + YDir * (HalfSize.y + r*Height));
        RectC rect;
        rect.Pos = p;
        rect.HalfSize = HalfSize;
        const float t = ImClamp((float)ImRemap01(val, ScaleMin, ScaleMax),0.0f,1.0f);
        rect.Color = GImPlot->ColormapData.LerpTable(GImPlot->Style.Colormap, t);
        return rect;
    }
    const T* const Values;
    const int Count, Rows, Cols;
    const double ScaleMin, ScaleMax, Width, Height, XRef, YRef, YDir;
    const ImPlotPoint HalfSize;
};

template <typename T>
struct GetterHeatmapColMaj {
    GetterHeatmapColMaj(const T* values, int rows, int cols, double scale_min, double scale_max, double width, double height, double xref, double yref, double ydir) :
        Values(values),
        Count(rows*cols),
        Rows(rows),
        Cols(cols),
        ScaleMin(scale_min),
        ScaleMax(scale_max),
        Width(width),
        Height(height),
        XRef(xref),
        YRef(yref),
        YDir(ydir),
        HalfSize(Width*0.5, Height*0.5)
    { }
    template <typename I> IMPLOT_INLINE RectC operator()(I idx) const {
        double val = (double)Values[idx];
        const int r = idx % Cols;
        const int c = idx / Cols;
        const ImPlotPoint p(XRef + HalfSize.x + c*Width, YRef + YDir * (HalfSize.y + r*Height));
        RectC rect;
        rect.Pos = p;
        rect.HalfSize = HalfSize;
        const float t = ImClamp((float)ImRemap01(val, ScaleMin, ScaleMax),0.0f,1.0f);
        rect.Color = GImPlot->ColormapData.LerpTable(GImPlot->Style.Colormap, t);
        return rect;
    }
    const T* const Values;
    const int Count, Rows, Cols;
    const double ScaleMin, ScaleMax, Width, Height, XRef, YRef, YDir;
    const ImPlotPoint HalfSize;
};

template <typename T, typename Transformer>
void RenderHeatmap(Transformer transformer, ImDrawList& draw_list, const T* values, int rows, int cols, double scale_min, double scale_max, const char* fmt, const ImPlotPoint& bounds_min, const ImPlotPoint& bounds_max, bool reverse_y, bool col_maj) {
    ImPlotContext& gp = *GImPlot;
    if (scale_min == 0 && scale_max == 0) {
        T temp_min, temp_max;
        ImMinMaxArray(values,rows*cols,&temp_min,&temp_max);
        scale_min = (double)temp_min;
        scale_max = (double)temp_max;
    }
    if (scale_min == scale_max) {
        ImVec2 a = transformer(bounds_min);
        ImVec2 b = transformer(bounds_max);
        ImU32  col = GetColormapColorU32(0,gp.Style.Colormap);
        draw_list.AddRectFilled(a, b, col);
        return;
    }
    const double yref = reverse_y ? bounds_max.y : bounds_min.y;
    const double ydir = reverse_y ? -1 : 1;
    if (col_maj) {
        GetterHeatmapColMaj<T> getter(values, rows, cols, scale_min, scale_max, (bounds_max.x - bounds_min.x) / cols, (bounds_max.y - bounds_min.y) / rows, bounds_min.x, yref, ydir);
        RenderPrimitives1<RendererRectC>(getter);
    }
    else {
        GetterHeatmapRowMaj<T> getter(values, rows, cols, scale_min, scale_max, (bounds_max.x - bounds_min.x) / cols, (bounds_max.y - bounds_min.y) / rows, bounds_min.x, yref, ydir);
        RenderPrimitives1<RendererRectC>(getter);
    }
    // labels
    if (fmt != NULL) {
        const double w = (bounds_max.x - bounds_min.x) / cols;
        const double h = (bounds_max.y - bounds_min.y) / rows;
        const ImPlotPoint half_size(w*0.5,h*0.5);
        int i = 0;
        if (col_maj) {
            for (int c = 0; c < cols; ++c) {
                for (int r = 0; r < rows; ++r) {
                    ImPlotPoint p;
                    p.x = bounds_min.x + 0.5*w + c*w;
                    p.y = yref + ydir * (0.5*h + r*h);
                    ImVec2 px = transformer(p);
                    char buff[32];
                    sprintf(buff, fmt, values[i]);
                    ImVec2 size = ImGui::CalcTextSize(buff);
                    double t = ImClamp(ImRemap01((double)values[i], scale_min, scale_max),0.0,1.0);
                    ImVec4 color = SampleColormap((float)t);
                    ImU32 col = CalcTextColor(color);
                    draw_list.AddText(px - size * 0.5f, col, buff);
                    i++;
                }
            }
        }
        else {
            for (int r = 0; r < rows; ++r) {
                for (int c = 0; c < cols; ++c) {
                    ImPlotPoint p;
                    p.x = bounds_min.x + 0.5*w + c*w;
                    p.y = yref + ydir * (0.5*h + r*h);
                    ImVec2 px = transformer(p);
                    char buff[32];
                    sprintf(buff, fmt, values[i]);
                    ImVec2 size = ImGui::CalcTextSize(buff);
                    double t = ImClamp(ImRemap01((double)values[i], scale_min, scale_max),0.0,1.0);
                    ImVec4 color = SampleColormap((float)t);
                    ImU32 col = CalcTextColor(color);
                    draw_list.AddText(px - size * 0.5f, col, buff);
                    i++;
                }
            }
        }
    }
}

template <typename T>
void PlotHeatmap(const char* label_id, const T* values, int rows, int cols, double scale_min, double scale_max, const char* fmt, const ImPlotPoint& bounds_min, const ImPlotPoint& bounds_max, ImPlotHeatmapFlags flags) {
    if (BeginItem(label_id)) {
        if (FitThisFrame()) {
            FitPoint(bounds_min);
            FitPoint(bounds_max);
        }
        ImDrawList& draw_list = *GetPlotDrawList();
        const bool col_maj = ImHasFlag(flags, ImPlotHeatmapFlags_ColMajor);
        switch (GetCurrentScale()) {
            case ImPlotScale_LinLin: RenderHeatmap(TransformerLinLin(), draw_list, values, rows, cols, scale_min, scale_max, fmt, bounds_min, bounds_max, true, col_maj); break;
            case ImPlotScale_LogLin: RenderHeatmap(TransformerLogLin(), draw_list, values, rows, cols, scale_min, scale_max, fmt, bounds_min, bounds_max, true, col_maj); break;
            case ImPlotScale_LinLog: RenderHeatmap(TransformerLinLog(), draw_list, values, rows, cols, scale_min, scale_max, fmt, bounds_min, bounds_max, true, col_maj); break;
            case ImPlotScale_LogLog: RenderHeatmap(TransformerLogLog(), draw_list, values, rows, cols, scale_min, scale_max, fmt, bounds_min, bounds_max, true, col_maj); break;
        }
        EndItem();
    }
}

template IMPLOT_API void PlotHeatmap<ImS8>(const char* label_id, const ImS8* values, int rows, int cols, double scale_min, double scale_max, const char* fmt, const ImPlotPoint& bounds_min, const ImPlotPoint& bounds_max, ImPlotHeatmapFlags flags);
template IMPLOT_API void PlotHeatmap<ImU8>(const char* label_id, const ImU8* values, int rows, int cols, double scale_min, double scale_max, const char* fmt, const ImPlotPoint& bounds_min, const ImPlotPoint& bounds_max, ImPlotHeatmapFlags flags);
template IMPLOT_API void PlotHeatmap<ImS16>(const char* label_id, const ImS16* values, int rows, int cols, double scale_min, double scale_max, const char* fmt, const ImPlotPoint& bounds_min, const ImPlotPoint& bounds_max, ImPlotHeatmapFlags flags);
template IMPLOT_API void PlotHeatmap<ImU16>(const char* label_id, const ImU16* values, int rows, int cols, double scale_min, double scale_max, const char* fmt, const ImPlotPoint& bounds_min, const ImPlotPoint& bounds_max, ImPlotHeatmapFlags flags);
template IMPLOT_API void PlotHeatmap<ImS32>(const char* label_id, const ImS32* values, int rows, int cols, double scale_min, double scale_max, const char* fmt, const ImPlotPoint& bounds_min, const ImPlotPoint& bounds_max, ImPlotHeatmapFlags flags);
template IMPLOT_API void PlotHeatmap<ImU32>(const char* label_id, const ImU32* values, int rows, int cols, double scale_min, double scale_max, const char* fmt, const ImPlotPoint& bounds_min, const ImPlotPoint& bounds_max, ImPlotHeatmapFlags flags);
template IMPLOT_API void PlotHeatmap<ImS64>(const char* label_id, const ImS64* values, int rows, int cols, double scale_min, double scale_max, const char* fmt, const ImPlotPoint& bounds_min, const ImPlotPoint& bounds_max, ImPlotHeatmapFlags flags);
template IMPLOT_API void PlotHeatmap<ImU64>(const char* label_id, const ImU64* values, int rows, int cols, double scale_min, double scale_max, const char* fmt, const ImPlotPoint& bounds_min, const ImPlotPoint& bounds_max, ImPlotHeatmapFlags flags);
template IMPLOT_API void PlotHeatmap<float>(const char* label_id, const float* values, int rows, int cols, double scale_min, double scale_max, const char* fmt, const ImPlotPoint& bounds_min, const ImPlotPoint& bounds_max, ImPlotHeatmapFlags flags);
template IMPLOT_API void PlotHeatmap<double>(const char* label_id, const double* values, int rows, int cols, double scale_min, double scale_max, const char* fmt, const ImPlotPoint& bounds_min, const ImPlotPoint& bounds_max, ImPlotHeatmapFlags flags);

//-----------------------------------------------------------------------------
// PLOT HISTOGRAM
//-----------------------------------------------------------------------------

template <typename T>
double PlotHistogram(const char* label_id, const T* values, int count, int bins, double bar_scale, ImPlotRange range, ImPlotHistogramFlags flags) {

    const bool cumulative = ImHasFlag(flags, ImPlotHistogramFlags_Cumulative);
    const bool density    = ImHasFlag(flags, ImPlotHistogramFlags_Density);
    const bool outliers   = !ImHasFlag(flags, ImPlotHistogramFlags_NoOutliers);

    if (count <= 0 || bins == 0)
        return 0;

    if (range.Min == 0 && range.Max == 0) {
        T Min, Max;
        ImMinMaxArray(values, count, &Min, &Max);
        range.Min = (double)Min;
        range.Max = (double)Max;
    }

    double width;
    if (bins < 0)
        CalculateBins(values, count, bins, range, bins, width);
    else
        width = range.Size() / bins;

    ImVector<double>& bin_centers = GImPlot->TempDouble1;
    ImVector<double>& bin_counts  = GImPlot->TempDouble2;
    bin_centers.resize(bins);
    bin_counts.resize(bins);
    int below = 0;

    for (int b = 0; b < bins; ++b) {
        bin_centers[b] = range.Min + b * width + width * 0.5;
        bin_counts[b] = 0;
    }
    int counted = 0;
    double max_count = 0;
    for (int i = 0; i < count; ++i) {
        double val = (double)values[i];
        if (range.Contains(val)) {
            const int b = ImClamp((int)((val - range.Min) / width), 0, bins - 1);
            bin_counts[b] += 1.0;
            if (bin_counts[b] > max_count)
                max_count = bin_counts[b];
            counted++;
        }
        else if (val < range.Min) {
            below++;
        }
    }
    if (cumulative && density) {
        if (outliers)
            bin_counts[0] += below;
        for (int b = 1; b < bins; ++b)
            bin_counts[b] += bin_counts[b-1];
        double scale = 1.0 / (outliers ? count : counted);
        for (int b = 0; b < bins; ++b)
            bin_counts[b] *= scale;
        max_count = bin_counts[bins-1];
    }
    else if (cumulative) {
        if (outliers)
            bin_counts[0] += below;
        for (int b = 1; b < bins; ++b)
            bin_counts[b] += bin_counts[b-1];
        max_count = bin_counts[bins-1];
    }
    else if (density) {
        double scale = 1.0 / ((outliers ? count : counted) * width);
        for (int b = 0; b < bins; ++b)
            bin_counts[b] *= scale;
        max_count *= scale;
    }
    if (ImHasFlag(flags, ImPlotHistogramFlags_Horizontal))
        PlotBars(label_id, &bin_counts.Data[0], &bin_centers.Data[0], bins, bar_scale*width,ImPlotBarsFlags_Horizontal);
    else
        PlotBars(label_id, &bin_centers.Data[0], &bin_counts.Data[0], bins, bar_scale*width);
    return max_count;
}

template IMPLOT_API double PlotHistogram<ImS8>(const char* label_id, const ImS8* values, int count, int bins, double bar_scale, ImPlotRange range, ImPlotHistogramFlags flags);
template IMPLOT_API double PlotHistogram<ImU8>(const char* label_id, const ImU8* values, int count, int bins, double bar_scale, ImPlotRange range, ImPlotHistogramFlags flags);
template IMPLOT_API double PlotHistogram<ImS16>(const char* label_id, const ImS16* values, int count, int bins, double bar_scale, ImPlotRange range, ImPlotHistogramFlags flags);
template IMPLOT_API double PlotHistogram<ImU16>(const char* label_id, const ImU16* values, int count, int bins, double bar_scale, ImPlotRange range, ImPlotHistogramFlags flags);
template IMPLOT_API double PlotHistogram<ImS32>(const char* label_id, const ImS32* values, int count, int bins, double bar_scale, ImPlotRange range, ImPlotHistogramFlags flags);
template IMPLOT_API double PlotHistogram<ImU32>(const char* label_id, const ImU32* values, int count, int bins, double bar_scale, ImPlotRange range, ImPlotHistogramFlags flags);
template IMPLOT_API double PlotHistogram<ImS64>(const char* label_id, const ImS64* values, int count, int bins, double bar_scale, ImPlotRange range, ImPlotHistogramFlags flags);
template IMPLOT_API double PlotHistogram<ImU64>(const char* label_id, const ImU64* values, int count, int bins, double bar_scale, ImPlotRange range, ImPlotHistogramFlags flags);
template IMPLOT_API double PlotHistogram<float>(const char* label_id, const float* values, int count, int bins, double bar_scale, ImPlotRange range, ImPlotHistogramFlags flags);
template IMPLOT_API double PlotHistogram<double>(const char* label_id, const double* values, int count, int bins, double bar_scale, ImPlotRange range, ImPlotHistogramFlags flags);

//-----------------------------------------------------------------------------
// PLOT HISTOGRAM 2D
//-----------------------------------------------------------------------------

template <typename T>
double PlotHistogram2D(const char* label_id, const T* xs, const T* ys, int count, int x_bins, int y_bins, ImPlotRect range, ImPlotHistogramFlags flags) {

    // const bool cumulative = ImHasFlag(flags, ImPlotHistogramFlags_Cumulative); NOT SUPPORTED
    const bool density  = ImHasFlag(flags, ImPlotHistogramFlags_Density);
    const bool outliers = !ImHasFlag(flags, ImPlotHistogramFlags_NoOutliers);
    const bool col_maj  = ImHasFlag(flags, ImPlotHistogramFlags_ColMajor);

    if (count <= 0 || x_bins == 0 || y_bins == 0)
        return 0;

    if (range.X.Min == 0 && range.X.Max == 0) {
        T Min, Max;
        ImMinMaxArray(xs, count, &Min, &Max);
        range.X.Min = (double)Min;
        range.X.Max = (double)Max;
    }
    if (range.Y.Min == 0 && range.Y.Max == 0) {
        T Min, Max;
        ImMinMaxArray(ys, count, &Min, &Max);
        range.Y.Min = (double)Min;
        range.Y.Max = (double)Max;
    }

    double width, height;
    if (x_bins < 0)
        CalculateBins(xs, count, x_bins, range.X, x_bins, width);
    else
        width = range.X.Size() / x_bins;
    if (y_bins < 0)
        CalculateBins(ys, count, y_bins, range.Y, y_bins, height);
    else
        height = range.Y.Size() / y_bins;

    const int bins = x_bins * y_bins;

    ImVector<double>& bin_counts = GImPlot->TempDouble1;
    bin_counts.resize(bins);

    for (int b = 0; b < bins; ++b)
        bin_counts[b] = 0;

    int counted = 0;
    double max_count = 0;
    for (int i = 0; i < count; ++i) {
        if (range.Contains((double)xs[i], (double)ys[i])) {
            const int xb = ImClamp( (int)((double)(xs[i] - range.X.Min) / width)  , 0, x_bins - 1);
            const int yb = ImClamp( (int)((double)(ys[i] - range.Y.Min) / height) , 0, y_bins - 1);
            const int b  = yb * x_bins + xb;
            bin_counts[b] += 1.0;
            if (bin_counts[b] > max_count)
                max_count = bin_counts[b];
            counted++;
        }
    }
    if (density) {
        double scale = 1.0 / ((outliers ? count : counted) * width * height);
        for (int b = 0; b < bins; ++b)
            bin_counts[b] *= scale;
        max_count *= scale;
    }

    if (BeginItem(label_id)) {
        if (FitThisFrame()) {
            FitPoint(range.Min());
            FitPoint(range.Max());
        }
        ImDrawList& draw_list = *GetPlotDrawList();
        switch (GetCurrentScale()) {
            case ImPlotScale_LinLin: RenderHeatmap(TransformerLinLin(), draw_list, &bin_counts.Data[0], y_bins, x_bins, 0, max_count, NULL, range.Min(), range.Max(), false, col_maj); break;
            case ImPlotScale_LogLin: RenderHeatmap(TransformerLogLin(), draw_list, &bin_counts.Data[0], y_bins, x_bins, 0, max_count, NULL, range.Min(), range.Max(), false, col_maj); break;
            case ImPlotScale_LinLog: RenderHeatmap(TransformerLinLog(), draw_list, &bin_counts.Data[0], y_bins, x_bins, 0, max_count, NULL, range.Min(), range.Max(), false, col_maj); break;
            case ImPlotScale_LogLog: RenderHeatmap(TransformerLogLog(), draw_list, &bin_counts.Data[0], y_bins, x_bins, 0, max_count, NULL, range.Min(), range.Max(), false, col_maj); break;
        }
        EndItem();
    }
    return max_count;
}

template IMPLOT_API double PlotHistogram2D<ImS8>(const char* label_id,   const ImS8*   xs, const ImS8*   ys, int count, int x_bins, int y_bins, ImPlotRect range, ImPlotHistogramFlags flags);
template IMPLOT_API double PlotHistogram2D<ImU8>(const char* label_id,   const ImU8*   xs, const ImU8*   ys, int count, int x_bins, int y_bins, ImPlotRect range, ImPlotHistogramFlags flags);
template IMPLOT_API double PlotHistogram2D<ImS16>(const char* label_id,  const ImS16*  xs, const ImS16*  ys, int count, int x_bins, int y_bins, ImPlotRect range, ImPlotHistogramFlags flags);
template IMPLOT_API double PlotHistogram2D<ImU16>(const char* label_id,  const ImU16*  xs, const ImU16*  ys, int count, int x_bins, int y_bins, ImPlotRect range, ImPlotHistogramFlags flags);
template IMPLOT_API double PlotHistogram2D<ImS32>(const char* label_id,  const ImS32*  xs, const ImS32*  ys, int count, int x_bins, int y_bins, ImPlotRect range, ImPlotHistogramFlags flags);
template IMPLOT_API double PlotHistogram2D<ImU32>(const char* label_id,  const ImU32*  xs, const ImU32*  ys, int count, int x_bins, int y_bins, ImPlotRect range, ImPlotHistogramFlags flags);
template IMPLOT_API double PlotHistogram2D<ImS64>(const char* label_id,  const ImS64*  xs, const ImS64*  ys, int count, int x_bins, int y_bins, ImPlotRect range, ImPlotHistogramFlags flags);
template IMPLOT_API double PlotHistogram2D<ImU64>(const char* label_id,  const ImU64*  xs, const ImU64*  ys, int count, int x_bins, int y_bins, ImPlotRect range, ImPlotHistogramFlags flags);
template IMPLOT_API double PlotHistogram2D<float>(const char* label_id,  const float*  xs, const float*  ys, int count, int x_bins, int y_bins, ImPlotRect range, ImPlotHistogramFlags flags);
template IMPLOT_API double PlotHistogram2D<double>(const char* label_id, const double* xs, const double* ys, int count, int x_bins, int y_bins, ImPlotRect range, ImPlotHistogramFlags flags);

//-----------------------------------------------------------------------------
// PLOT DIGITAL
//-----------------------------------------------------------------------------

// TODO: Make this behave like all the other plot types (.e. not fixed in y axis)

template <typename Getter>
IMPLOT_INLINE void PlotDigitalEx(const char* label_id, Getter getter, ImPlotDigitalFlags) {
    if (BeginItem(label_id, ImPlotCol_Fill)) {
        ImPlotContext& gp = *GImPlot;
        ImDrawList& draw_list = *GetPlotDrawList();
        const ImPlotNextItemData& s = GetItemData();
        if (getter.Count > 1 && s.RenderFill) {
            ImPlotPlot& plot   = *gp.CurrentPlot;
            ImPlotAxis& x_axis = plot.Axes[plot.CurrentX];
            ImPlotAxis& y_axis = plot.Axes[plot.CurrentY];

            int pixYMax = 0;
            ImPlotPoint itemData1 = getter(0);
            for (int i = 0; i < getter.Count; ++i) {
                ImPlotPoint itemData2 = getter(i);
                if (ImNanOrInf(itemData1.y)) {
                    itemData1 = itemData2;
                    continue;
                }
                if (ImNanOrInf(itemData2.y)) itemData2.y = ImConstrainNan(ImConstrainInf(itemData2.y));
                int pixY_0 = (int)(s.LineWeight);
                itemData1.y = ImMax(0.0, itemData1.y);
                float pixY_1_float = s.DigitalBitHeight * (float)itemData1.y;
                int pixY_1 = (int)(pixY_1_float); //allow only positive values
                int pixY_chPosOffset = (int)(ImMax(s.DigitalBitHeight, pixY_1_float) + s.DigitalBitGap);
                pixYMax = ImMax(pixYMax, pixY_chPosOffset);
                ImVec2 pMin = PlotToPixels(itemData1,IMPLOT_AUTO,IMPLOT_AUTO);
                ImVec2 pMax = PlotToPixels(itemData2,IMPLOT_AUTO,IMPLOT_AUTO);
                int pixY_Offset = 0; //20 pixel from bottom due to mouse cursor label
                pMin.y = (y_axis.PixelMin) + ((-gp.DigitalPlotOffset)                   - pixY_Offset);
                pMax.y = (y_axis.PixelMin) + ((-gp.DigitalPlotOffset) - pixY_0 - pixY_1 - pixY_Offset);
                //plot only one rectangle for same digital state
                while (((i+2) < getter.Count) && (itemData1.y == itemData2.y)) {
                    const int in = (i + 1);
                    itemData2 = getter(in);
                    if (ImNanOrInf(itemData2.y)) break;
                    pMax.x = PlotToPixels(itemData2,IMPLOT_AUTO,IMPLOT_AUTO).x;
                    i++;
                }
                //do not extend plot outside plot range
                if (pMin.x < x_axis.PixelMin) pMin.x = x_axis.PixelMin;
                if (pMax.x < x_axis.PixelMin) pMax.x = x_axis.PixelMin;
                if (pMin.x > x_axis.PixelMax) pMin.x = x_axis.PixelMax;
                if (pMax.x > x_axis.PixelMax) pMax.x = x_axis.PixelMax;
                //plot a rectangle that extends up to x2 with y1 height
                if ((pMax.x > pMin.x) && (gp.CurrentPlot->PlotRect.Contains(pMin) || gp.CurrentPlot->PlotRect.Contains(pMax))) {
                    // ImVec4 colAlpha = item->Color;
                    // colAlpha.w = item->Highlight ? 1.0f : 0.9f;
                    draw_list.AddRectFilled(pMin, pMax, ImGui::GetColorU32(s.Colors[ImPlotCol_Fill]));
                }
                itemData1 = itemData2;
            }
            gp.DigitalPlotItemCnt++;
            gp.DigitalPlotOffset += pixYMax;
        }
        EndItem();
    }
}


template <typename T>
void PlotDigital(const char* label_id, const T* xs, const T* ys, int count, ImPlotDigitalFlags flags, int offset, int stride) {
    GetterXY<GetterIdx<T>,GetterIdx<T>> getter(GetterIdx<T>(xs,count,offset,stride),GetterIdx<T>(ys,count,offset,stride),count);
    return PlotDigitalEx(label_id, getter, flags);
}

template IMPLOT_API void PlotDigital<ImS8>(const char* label_id, const ImS8* xs, const ImS8* ys, int count, ImPlotDigitalFlags flags, int offset, int stride);
template IMPLOT_API void PlotDigital<ImU8>(const char* label_id, const ImU8* xs, const ImU8* ys, int count, ImPlotDigitalFlags flags, int offset, int stride);
template IMPLOT_API void PlotDigital<ImS16>(const char* label_id, const ImS16* xs, const ImS16* ys, int count, ImPlotDigitalFlags flags, int offset, int stride);
template IMPLOT_API void PlotDigital<ImU16>(const char* label_id, const ImU16* xs, const ImU16* ys, int count, ImPlotDigitalFlags flags, int offset, int stride);
template IMPLOT_API void PlotDigital<ImS32>(const char* label_id, const ImS32* xs, const ImS32* ys, int count, ImPlotDigitalFlags flags, int offset, int stride);
template IMPLOT_API void PlotDigital<ImU32>(const char* label_id, const ImU32* xs, const ImU32* ys, int count, ImPlotDigitalFlags flags, int offset, int stride);
template IMPLOT_API void PlotDigital<ImS64>(const char* label_id, const ImS64* xs, const ImS64* ys, int count, ImPlotDigitalFlags flags, int offset, int stride);
template IMPLOT_API void PlotDigital<ImU64>(const char* label_id, const ImU64* xs, const ImU64* ys, int count, ImPlotDigitalFlags flags, int offset, int stride);
template IMPLOT_API void PlotDigital<float>(const char* label_id, const float* xs, const float* ys, int count, ImPlotDigitalFlags flags, int offset, int stride);
template IMPLOT_API void PlotDigital<double>(const char* label_id, const double* xs, const double* ys, int count, ImPlotDigitalFlags flags, int offset, int stride);

// custom
void PlotDigitalG(const char* label_id, ImPlotGetter getter_func, void* data, int count, ImPlotDigitalFlags flags) {
    GetterFuncPtr getter(getter_func,data,count);
    return PlotDigitalEx(label_id, getter, flags);
}

//-----------------------------------------------------------------------------
// PLOT IMAGE
//-----------------------------------------------------------------------------

void PlotImage(const char* label_id, ImTextureID user_texture_id, const ImPlotPoint& bmin, const ImPlotPoint& bmax, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, ImPlotImageFlags) {
    if (BeginItem(label_id)) {
        if (FitThisFrame()) {
            FitPoint(bmin);
            FitPoint(bmax);
        }
        ImU32 tint_col32 = ImGui::ColorConvertFloat4ToU32(tint_col);
        GetCurrentItem()->Color = tint_col32;
        ImDrawList& draw_list = *GetPlotDrawList();
        ImVec2 p1 = PlotToPixels(bmin.x, bmax.y,IMPLOT_AUTO,IMPLOT_AUTO);
        ImVec2 p2 = PlotToPixels(bmax.x, bmin.y,IMPLOT_AUTO,IMPLOT_AUTO);
        PushPlotClipRect();
        draw_list.AddImage(user_texture_id, p1, p2, uv0, uv1, tint_col32);
        PopPlotClipRect();
        EndItem();
    }
}

//-----------------------------------------------------------------------------
// PLOT TEXT
//-----------------------------------------------------------------------------

// double
void PlotText(const char* text, double x, double y, const ImVec2& pixel_offset, ImPlotTextFlags flags) {
    IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot != NULL, "PlotText() needs to be called between BeginPlot() and EndPlot()!");
    SetupLock();
    ImDrawList & draw_list = *GetPlotDrawList();
    PushPlotClipRect();
    ImU32 colTxt = GetStyleColorU32(ImPlotCol_InlayText);
    if (ImHasFlag(flags,ImPlotTextFlags_Vertical)) {
        ImVec2 siz = CalcTextSizeVertical(text) * 0.5f;
        ImVec2 ctr = siz * 0.5f;
        ImVec2 pos = PlotToPixels(ImPlotPoint(x,y),IMPLOT_AUTO,IMPLOT_AUTO) + ImVec2(-ctr.x, ctr.y) + pixel_offset;
        if (FitThisFrame()) {
            FitPoint(PixelsToPlot(pos));
            FitPoint(PixelsToPlot(pos.x + siz.x, pos.y - siz.y));
        }
        AddTextVertical(&draw_list, pos, colTxt, text);
    }
    else {
        ImVec2 siz = ImGui::CalcTextSize(text);
        ImVec2 pos = PlotToPixels(ImPlotPoint(x,y),IMPLOT_AUTO,IMPLOT_AUTO) - siz * 0.5f + pixel_offset;
        if (FitThisFrame()) {
            FitPoint(PixelsToPlot(pos));
            FitPoint(PixelsToPlot(pos+siz));
        }
        draw_list.AddText(pos, colTxt, text);
    }
    PopPlotClipRect();
}

//-----------------------------------------------------------------------------
// PLOT DUMMY
//-----------------------------------------------------------------------------

void PlotDummy(const char* label_id, ImPlotDummyFlags) {
    if (BeginItem(label_id, ImPlotCol_Line))
        EndItem();
}

} // namespace ImPlot
