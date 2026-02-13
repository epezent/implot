// MIT License

// Copyright (c) 2020-2024 Evan Pezent
// Copyright (c) 2025-2026 Breno Cunha Queiroz

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

// ImPlot v0.18 WIP

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "implot.h"
#ifndef IMGUI_DISABLE
#include "implot_internal.h"

//-----------------------------------------------------------------------------
// [SECTION] Macros and Defines
//-----------------------------------------------------------------------------

#define SQRT_1_2 0.70710678118f
#define SQRT_3_2 0.86602540378f

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

//-----------------------------------------------------------------------------
// [SECTION] Template instantiation utility
//-----------------------------------------------------------------------------

// By default, templates are instantiated for `float`, `double`, and for the following integer types, which are defined in imgui.h:
//     signed char         ImS8;   // 8-bit signed integer
//     unsigned char       ImU8;   // 8-bit unsigned integer
//     signed short        ImS16;  // 16-bit signed integer
//     unsigned short      ImU16;  // 16-bit unsigned integer
//     signed int          ImS32;  // 32-bit signed integer == int
//     unsigned int        ImU32;  // 32-bit unsigned integer
//     signed   long long  ImS64;  // 64-bit signed integer
//     unsigned long long  ImU64;  // 64-bit unsigned integer
// (note: this list does *not* include `long`, `unsigned long` and `long double`)
//
// You can customize the supported types by defining IMPLOT_CUSTOM_NUMERIC_TYPES at compile time to define your own type list.
//    As an example, you could use the compile time define given by the line below in order to support only float and double.
//        -DIMPLOT_CUSTOM_NUMERIC_TYPES="(float)(double)"
//    In order to support all known C++ types, use:
//        -DIMPLOT_CUSTOM_NUMERIC_TYPES="(signed char)(unsigned char)(signed short)(unsigned short)(signed int)(unsigned int)(signed long)(unsigned long)(signed long long)(unsigned long long)(float)(double)(long double)"

#ifdef IMPLOT_CUSTOM_NUMERIC_TYPES
    #define IMPLOT_NUMERIC_TYPES IMPLOT_CUSTOM_NUMERIC_TYPES
#else
    #define IMPLOT_NUMERIC_TYPES (ImS8)(ImU8)(ImS16)(ImU16)(ImS32)(ImU32)(ImS64)(ImU64)(float)(double)
#endif

// CALL_INSTANTIATE_FOR_NUMERIC_TYPES will duplicate the template instantiation code `INSTANTIATE_MACRO(T)` on supported types.
#define _CAT(x, y) _CAT_(x, y)
#define _CAT_(x,y) x ## y
#define _INSTANTIATE_FOR_NUMERIC_TYPES(chain) _CAT(_INSTANTIATE_FOR_NUMERIC_TYPES_1 chain, _END)
#define _INSTANTIATE_FOR_NUMERIC_TYPES_1(T) INSTANTIATE_MACRO(T) _INSTANTIATE_FOR_NUMERIC_TYPES_2
#define _INSTANTIATE_FOR_NUMERIC_TYPES_2(T) INSTANTIATE_MACRO(T) _INSTANTIATE_FOR_NUMERIC_TYPES_1
#define _INSTANTIATE_FOR_NUMERIC_TYPES_1_END
#define _INSTANTIATE_FOR_NUMERIC_TYPES_2_END
#define CALL_INSTANTIATE_FOR_NUMERIC_TYPES() _INSTANTIATE_FOR_NUMERIC_TYPES(IMPLOT_NUMERIC_TYPES)

namespace ImPlot {

//-----------------------------------------------------------------------------
// [SECTION] Utils
//-----------------------------------------------------------------------------

// Calc maximum index size of ImDrawIdx
template <typename T>
struct MaxIdx { static const unsigned int Value; };
template <> const unsigned int MaxIdx<unsigned short>::Value = 65535;
template <> const unsigned int MaxIdx<unsigned int>::Value   = 4294967295;

template <typename T>
int Stride(const ImPlotSpec& spec) {
    return spec.Stride == IMPLOT_AUTO ? sizeof(T) : spec.Stride;
}

// Finds the min and max value in an unsorted array
template <typename Indexer, typename T>
static inline void ImMinMaxIndexer(const Indexer& values, int count, T* min_out, T* max_out) {
    T Min = values[0]; T Max = values[0];
    for (int i = 1; i < count; ++i) {
        if (values[i] < Min) { Min = values[i]; }
        if (values[i] > Max) { Max = values[i]; }
    }
    *min_out = Min; *max_out = Max;
}

// Finds the mean of a container
template <typename TContainer>
static inline double ImMean(const TContainer& values, int count) {
    double den = 1.0 / count;
    double mu  = 0;
    for (int i = 0; i < count; ++i)
        mu += (double)values[i] * den;
    return mu;
}

// Finds the sample standard deviation of a container
template <typename TContainer>
static inline double ImStdDev(const TContainer& values, int count) {
    double den = 1.0 / (count - 1.0);
    double mu  = ImMean(values, count);
    double x   = 0;
    for (int i = 0; i < count; ++i)
        x += ((double)values[i] - mu) * ((double)values[i] - mu) * den;
    return sqrt(x);
}

IMPLOT_INLINE void GetLineRenderProps(const ImDrawList& draw_list, float& half_weight, ImVec2& tex_uv0, ImVec2& tex_uv1) {
    const bool aa = ImHasFlag(draw_list.Flags, ImDrawListFlags_AntiAliasedLines) &&
                    ImHasFlag(draw_list.Flags, ImDrawListFlags_AntiAliasedLinesUseTex);
    if (aa) {
        ImVec4 tex_uvs = draw_list._Data->TexUvLines[(int)(half_weight*2)];
        tex_uv0 = ImVec2(tex_uvs.x, tex_uvs.y);
        tex_uv1 = ImVec2(tex_uvs.z, tex_uvs.w);
        half_weight += 1;
    }
    else {
        tex_uv0 = tex_uv1 = draw_list._Data->TexUvWhitePixel;
    }
}

IMPLOT_INLINE void PrimLine(ImDrawList& draw_list, const ImVec2& P1, const ImVec2& P2, float half_weight, ImU32 col, const ImVec2& tex_uv0, const ImVec2 tex_uv1) {
    float dx = P2.x - P1.x;
    float dy = P2.y - P1.y;
    IMPLOT_NORMALIZE2F_OVER_ZERO(dx, dy);
    dx *= half_weight;
    dy *= half_weight;
    draw_list._VtxWritePtr[0].pos.x = P1.x + dy;
    draw_list._VtxWritePtr[0].pos.y = P1.y - dx;
    draw_list._VtxWritePtr[0].uv    = tex_uv0;
    draw_list._VtxWritePtr[0].col   = col;
    draw_list._VtxWritePtr[1].pos.x = P2.x + dy;
    draw_list._VtxWritePtr[1].pos.y = P2.y - dx;
    draw_list._VtxWritePtr[1].uv    = tex_uv0;
    draw_list._VtxWritePtr[1].col   = col;
    draw_list._VtxWritePtr[2].pos.x = P2.x - dy;
    draw_list._VtxWritePtr[2].pos.y = P2.y + dx;
    draw_list._VtxWritePtr[2].uv    = tex_uv1;
    draw_list._VtxWritePtr[2].col   = col;
    draw_list._VtxWritePtr[3].pos.x = P1.x - dy;
    draw_list._VtxWritePtr[3].pos.y = P1.y + dx;
    draw_list._VtxWritePtr[3].uv    = tex_uv1;
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

IMPLOT_INLINE void PrimRectFill(ImDrawList& draw_list, const ImVec2& Pmin, const ImVec2& Pmax, ImU32 col, const ImVec2& uv) {
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

IMPLOT_INLINE void PrimRectLine(ImDrawList& draw_list, const ImVec2& Pmin, const ImVec2& Pmax, float weight, ImU32 col, const ImVec2& uv) {

    draw_list._VtxWritePtr[0].pos.x = Pmin.x;
    draw_list._VtxWritePtr[0].pos.y = Pmin.y;
    draw_list._VtxWritePtr[0].uv    = uv;
    draw_list._VtxWritePtr[0].col   = col;

    draw_list._VtxWritePtr[1].pos.x = Pmin.x;
    draw_list._VtxWritePtr[1].pos.y = Pmax.y;
    draw_list._VtxWritePtr[1].uv    = uv;
    draw_list._VtxWritePtr[1].col   = col;

    draw_list._VtxWritePtr[2].pos.x = Pmax.x;
    draw_list._VtxWritePtr[2].pos.y = Pmax.y;
    draw_list._VtxWritePtr[2].uv    = uv;
    draw_list._VtxWritePtr[2].col   = col;

    draw_list._VtxWritePtr[3].pos.x = Pmax.x;
    draw_list._VtxWritePtr[3].pos.y = Pmin.y;
    draw_list._VtxWritePtr[3].uv    = uv;
    draw_list._VtxWritePtr[3].col   = col;

    draw_list._VtxWritePtr[4].pos.x = Pmin.x + weight;
    draw_list._VtxWritePtr[4].pos.y = Pmin.y + weight;
    draw_list._VtxWritePtr[4].uv    = uv;
    draw_list._VtxWritePtr[4].col   = col;

    draw_list._VtxWritePtr[5].pos.x = Pmin.x + weight;
    draw_list._VtxWritePtr[5].pos.y = Pmax.y - weight;
    draw_list._VtxWritePtr[5].uv    = uv;
    draw_list._VtxWritePtr[5].col   = col;

    draw_list._VtxWritePtr[6].pos.x = Pmax.x - weight;
    draw_list._VtxWritePtr[6].pos.y = Pmax.y - weight;
    draw_list._VtxWritePtr[6].uv    = uv;
    draw_list._VtxWritePtr[6].col   = col;

    draw_list._VtxWritePtr[7].pos.x = Pmax.x - weight;
    draw_list._VtxWritePtr[7].pos.y = Pmin.y + weight;
    draw_list._VtxWritePtr[7].uv    = uv;
    draw_list._VtxWritePtr[7].col   = col;

    draw_list._VtxWritePtr += 8;

    draw_list._IdxWritePtr[0] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 0);
    draw_list._IdxWritePtr[1] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 1);
    draw_list._IdxWritePtr[2] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 5);
    draw_list._IdxWritePtr += 3;

    draw_list._IdxWritePtr[0] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 0);
    draw_list._IdxWritePtr[1] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 5);
    draw_list._IdxWritePtr[2] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 4);
    draw_list._IdxWritePtr += 3;

    draw_list._IdxWritePtr[0] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 1);
    draw_list._IdxWritePtr[1] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 2);
    draw_list._IdxWritePtr[2] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 6);
    draw_list._IdxWritePtr += 3;

    draw_list._IdxWritePtr[0] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 1);
    draw_list._IdxWritePtr[1] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 6);
    draw_list._IdxWritePtr[2] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 5);
    draw_list._IdxWritePtr += 3;

    draw_list._IdxWritePtr[0] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 2);
    draw_list._IdxWritePtr[1] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 3);
    draw_list._IdxWritePtr[2] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 7);
    draw_list._IdxWritePtr += 3;

    draw_list._IdxWritePtr[0] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 2);
    draw_list._IdxWritePtr[1] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 7);
    draw_list._IdxWritePtr[2] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 6);
    draw_list._IdxWritePtr += 3;

    draw_list._IdxWritePtr[0] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 3);
    draw_list._IdxWritePtr[1] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 0);
    draw_list._IdxWritePtr[2] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 4);
    draw_list._IdxWritePtr += 3;

    draw_list._IdxWritePtr[0] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 3);
    draw_list._IdxWritePtr[1] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 4);
    draw_list._IdxWritePtr[2] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 7);
    draw_list._IdxWritePtr += 3;

    draw_list._VtxCurrentIdx += 8;
}


//-----------------------------------------------------------------------------
// [SECTION] Item Utils
//-----------------------------------------------------------------------------

ImPlotItem* RegisterOrGetItem(const char* label_id, ImPlotItemFlags flags, bool* just_created) {
    ImPlotContext& gp = *GImPlot;
    ImPlotItemGroup& Items = *gp.CurrentItems;
    ImGuiID id = Items.GetItemID(label_id);
    if (just_created != nullptr)
        *just_created = Items.GetItem(id) == nullptr;
    ImPlotItem* item = Items.GetOrAddItem(id);
    if (item->SeenThisFrame)
        return item;
    item->SeenThisFrame = true;
    int idx = Items.GetItemIndex(item);
    item->ID = id;
    if (!ImHasFlag(flags, ImPlotItemFlags_NoLegend) && ImGui::FindRenderedTextEnd(label_id, nullptr) != label_id) {
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
    return item != nullptr && !item->Show;
}

ImPlotItem* GetCurrentItem() {
    ImPlotContext& gp = *GImPlot;
    return gp.CurrentItem;
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
    if (plot_title_id == nullptr) {
        BustItemCache();
    }
    else {
        ImGuiID id = ImGui::GetCurrentWindow()->GetID(plot_title_id);
        ImPlotPlot* plot = gp.Plots.GetByKey(id);
        if (plot != nullptr)
            plot->Items.Reset();
        else {
            ImPlotSubplot* subplot = gp.Subplots.GetByKey(id);
            if (subplot != nullptr)
                subplot->Items.Reset();
        }
    }
}

//-----------------------------------------------------------------------------
// [SECTION] BeginItem / EndItem
//-----------------------------------------------------------------------------

constexpr float ITEM_HIGHLIGHT_LINE_SCALE = 2.0f;
constexpr float ITEM_HIGHLIGHT_MARK_SCALE = 1.25f;

// Begins a new item. Returns false if the item should not be plotted.
bool BeginItem(const char* label_id, const ImPlotSpec& spec, const ImVec4& item_col, ImPlotMarker item_mkr) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != nullptr, "PlotX() needs to be called between BeginPlot() and EndPlot()!");
    SetupLock();
    bool just_created;
    ImPlotItem* item = RegisterOrGetItem(label_id, spec.Flags, &just_created);
    // set current item
    gp.CurrentItem = item;
    ImPlotNextItemData& s = gp.NextItemData;
    // set/override item color
    if (!IsColorAuto(item_col))
        item->Color = ImGui::ColorConvertFloat4ToU32(item_col);
    else if (just_created)
        item->Color = NextColormapColorU32();
    if (gp.NextItemData.HasHidden) {
        if (just_created || gp.NextItemData.HiddenCond == ImGuiCond_Always)
            item->Show = !gp.NextItemData.Hidden;
    }
    // set/override item marker
    if (item_mkr != ImPlotMarker_Invalid) {
        if (item_mkr != ImPlotMarker_Auto) {
            item->Marker = item_mkr;
        }
        else if (just_created && item_mkr == ImPlotMarker_Auto) {
            item->Marker = NextMarker();
        }
        else if (item_mkr == ImPlotMarker_Auto && item->Marker == ImPlotMarker_None) {
            item->Marker = NextMarker();
        }
    }
    // return false if not shown
    if (!item->Show) {
        // reset next item data
        gp.NextItemData.Reset();
        gp.PreviousItem = item;
        gp.CurrentItem  = nullptr;
        return false;
    }
    else {
        ImVec4 item_color = ImGui::ColorConvertU32ToFloat4(item->Color);
        // stage next item spec
        s.Spec = spec;
        s.Spec.LineColor = IsColorAuto(s.Spec.LineColor) ? item_color : s.Spec.LineColor;
        s.Spec.FillColor = IsColorAuto(s.Spec.FillColor) ? item_color : s.Spec.FillColor;
        s.Spec.FillColor.w *= s.Spec.FillAlpha;
        s.Spec.Marker = item->Marker;
        s.Spec.MarkerLineColor = IsColorAuto(s.Spec.MarkerLineColor) ? s.Spec.LineColor : s.Spec.MarkerLineColor;
        s.Spec.MarkerFillColor = IsColorAuto(s.Spec.MarkerFillColor) ? s.Spec.LineColor : s.Spec.MarkerFillColor;
        s.Spec.MarkerFillColor.w *= s.Spec.FillAlpha;
        // apply highlight mods
        if (item->LegendHovered) {
            if (!ImHasFlag(gp.CurrentItems->Legend.Flags, ImPlotLegendFlags_NoHighlightItem)) {
                s.Spec.LineWeight *= ITEM_HIGHLIGHT_LINE_SCALE;
                s.Spec.MarkerSize *= ITEM_HIGHLIGHT_MARK_SCALE;
                s.Spec.Size *= ITEM_HIGHLIGHT_MARK_SCALE;
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
        s.RenderLine = s.Spec.LineColor.w > 0 && s.Spec.LineWeight > 0;
        s.RenderFill = s.Spec.FillColor.w > 0;
        s.RenderMarkerLine = s.Spec.MarkerLineColor.w > 0 && s.Spec.LineWeight > 0;
        s.RenderMarkerFill = s.Spec.MarkerFillColor.w > 0;
        s.RenderMarkers = s.Spec.Marker >= 0 && (s.RenderMarkerFill || s.RenderMarkerLine);
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
    gp.CurrentItem  = nullptr;
}

//-----------------------------------------------------------------------------
// [SECTION] Indexers
//-----------------------------------------------------------------------------

template <typename T>
IMPLOT_INLINE T IndexData(const T* data, int idx, int count, int offset, int stride) {
    const int s = ((offset == 0) << 0) | ((stride == sizeof(T)) << 1);
    switch (s) {
        case 3 : return data[idx];
        case 2 : return data[(offset + idx) % count];
        case 1 : return *(const T*)(const void*)((const unsigned char*)data + (size_t)((idx) ) * stride);
        case 0 : return *(const T*)(const void*)((const unsigned char*)data + (size_t)((offset + idx) % count) * stride);
        default: return T(0);
    }
}

template <typename T>
struct IndexerIdx {
    IndexerIdx(const T* data, int count, int offset = 0, int stride = sizeof(T)) :
        Data(data),
        Count(count),
        Offset(count ? ImPosMod(offset, count) : 0),
        Stride(stride)
    { }
    template <typename I> IMPLOT_INLINE double operator[](I idx) const {
        return (double)IndexData(Data, idx, Count, Offset, Stride);
    }
    const T* Data;
    int Count;
    int Offset;
    int Stride;
    typedef double value_type;
};

template <typename _Indexer1, typename _Indexer2>
struct IndexerAdd {
    IndexerAdd(const _Indexer1& indexer1, const _Indexer2& indexer2, double scale1 = 1, double scale2 = 1)
        : Indexer1(indexer1),
          Indexer2(indexer2),
          Scale1(scale1),
          Scale2(scale2),
          Count(ImMin(Indexer1.Count, Indexer2.Count))
    { }
    template <typename I> IMPLOT_INLINE double operator[](I idx) const {
        return Scale1 * Indexer1[idx] + Scale2 * Indexer2[idx];
    }
    const _Indexer1& Indexer1;
    const _Indexer2& Indexer2;
    double Scale1;
    double Scale2;
    int Count;
    typedef double value_type;
};

struct IndexerLin {
    IndexerLin(double m, double b) : M(m), B(b) { }
    template <typename I> IMPLOT_INLINE double operator[](I idx) const {
        return M * idx + B;
    }
    const double M;
    const double B;
    typedef double value_type;
};

struct IndexerConst {
    IndexerConst(double ref) : Ref(ref) { }
    template <typename I> IMPLOT_INLINE double operator[](I) const { return Ref; }
    const double Ref;
    typedef double value_type;
};

//-----------------------------------------------------------------------------
// [SECTION] Getters
//-----------------------------------------------------------------------------

template <typename _IndexerX, typename _IndexerY>
struct GetterXY {
    GetterXY(_IndexerX x, _IndexerY y, int count) : IndexerX(x), IndexerY(y), Count(count) { }
    template <typename I> IMPLOT_INLINE ImPlotPoint operator[](I idx) const {
        return ImPlotPoint(IndexerX[idx],IndexerY[idx]);
    }
    const _IndexerX IndexerX;
    const _IndexerY IndexerY;
    const int Count;
    typedef ImPlotPoint value_type;
};

// Double precision point with three coordinates used by ImPlot.
struct ImPlotPoint3D {
  double x, y, z;
  constexpr ImPlotPoint3D()                     : x(0.0), y(0.0), z(0.0) { }
  constexpr ImPlotPoint3D(double _x, double _y, double _z) : x(_x), y(_y), z(_z) { }
  double& operator[] (size_t idx)             { IM_ASSERT(idx == 0 || idx == 1 || idx == 2); return ((double*)(void*)(char*)this)[idx]; }
  double  operator[] (size_t idx) const       { IM_ASSERT(idx == 0 || idx == 1 || idx == 2); return ((const double*)(const void*)(const char*)this)[idx]; }
};

template <typename _IndexerX, typename _IndexerY, typename _IndexerZ>
struct GetterXYZ {
  GetterXYZ(_IndexerX x, _IndexerY y, _IndexerZ z, int count) : IndxerX(x), IndxerY(y), IndxerZ(z), Count(count) { }
  template <typename I> IMPLOT_INLINE ImPlotPoint3D operator()(I idx) const {
    return ImPlotPoint3D(IndxerX[idx],IndxerY[idx],IndxerZ[idx]);
  }
  const _IndexerX IndxerX;
  const _IndexerY IndxerY;
  const _IndexerZ IndxerZ;
  const int Count;
};

/// Interprets a user's function pointer as ImPlotPoints
struct GetterFuncPtr {
    GetterFuncPtr(ImPlotGetter getter, void* data, int count) :
        Getter(getter),
        Data(data),
        Count(count)
    { }
    template <typename I> IMPLOT_INLINE ImPlotPoint operator[](I idx) const {
        return Getter(idx, Data);
    }
    ImPlotGetter Getter;
    void* const Data;
    const int Count;
    typedef ImPlotPoint value_type;
};

template <typename _Getter>
struct GetterOverrideX {
    GetterOverrideX(_Getter getter, double x) : Getter(getter), X(x), Count(getter.Count) { }
    template <typename I> IMPLOT_INLINE ImPlotPoint operator[](I idx) const {
        ImPlotPoint p = Getter[idx];
        p.x = X;
        return p;
    }
    const _Getter Getter;
    const double X;
    const int Count;
    typedef ImPlotPoint value_type;
};

template <typename _Getter>
struct GetterOverrideY {
    GetterOverrideY(_Getter getter, double y) : Getter(getter), Y(y), Count(getter.Count) { }
    template <typename I> IMPLOT_INLINE ImPlotPoint operator[](I idx) const {
        ImPlotPoint p = Getter[idx];
        p.y = Y;
        return p;
    }
    const _Getter Getter;
    const double Y;
    const int Count;
    typedef ImPlotPoint value_type;
};

template <typename _Getter>
struct GetterLoop {
    GetterLoop(_Getter getter) : Getter(getter), Count(getter.Count + 1) { }
    template <typename I> IMPLOT_INLINE ImPlotPoint operator[](I idx) const {
        idx = idx % (Count - 1);
        return Getter[idx];
    }
    const _Getter Getter;
    const int Count;
    typedef ImPlotPoint value_type;
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
    template <typename I> IMPLOT_INLINE ImPlotPointError operator[](I idx) const {
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
    typedef ImPlotPointError value_type;
};

//-----------------------------------------------------------------------------
// [SECTION] Fitters
//-----------------------------------------------------------------------------

template <typename _Getter1>
struct Fitter1 {
    Fitter1(const _Getter1& getter) : Getter(getter) { }
    void Fit(ImPlotAxis& x_axis, ImPlotAxis& y_axis) const {
        for (int i = 0; i < Getter.Count; ++i) {
            ImPlotPoint p = Getter[i];
            x_axis.ExtendFitWith(y_axis, p.x, p.y);
            y_axis.ExtendFitWith(x_axis, p.y, p.x);
        }
    }
    const _Getter1& Getter;
};

template <typename _Getter1>
struct FitterBubbles1 {
  FitterBubbles1(const _Getter1& getter) : Getter(getter) { }
  void Fit(ImPlotAxis& x_axis, ImPlotAxis& y_axis) const {
    for (int i = 0; i < Getter.Count; ++i) {
      ImPlotPoint3D p = Getter(i);
      double half_size = p.z;
      // Fit left and right edges
      x_axis.ExtendFitWith(y_axis, p.x - half_size, p.y);
      x_axis.ExtendFitWith(y_axis, p.x + half_size, p.y);
      // Fit top and bottom edges
      y_axis.ExtendFitWith(x_axis, p.y - half_size, p.x);
      y_axis.ExtendFitWith(x_axis, p.y + half_size, p.x);
    }
  }
  const _Getter1& Getter;
};

template <typename _Getter1>
struct FitterX {
    FitterX(const _Getter1& getter) : Getter(getter) { }
    void Fit(ImPlotAxis& x_axis, ImPlotAxis&) const {
        for (int i = 0; i < Getter.Count; ++i) {
            ImPlotPoint p = Getter[i];
            x_axis.ExtendFit(p.x);
        }
    }
    const _Getter1& Getter;
};

template <typename _Getter1>
struct FitterY {
    FitterY(const _Getter1& getter) : Getter(getter) { }
    void Fit(ImPlotAxis&, ImPlotAxis& y_axis) const {
        for (int i = 0; i < Getter.Count; ++i) {
            ImPlotPoint p = Getter[i];
            y_axis.ExtendFit(p.y);
        }
    }
    const _Getter1& Getter;
};

template <typename _Getter1, typename _Getter2>
struct Fitter2 {
    Fitter2(const _Getter1& getter1, const _Getter2& getter2) : Getter1(getter1), Getter2(getter2) { }
    void Fit(ImPlotAxis& x_axis, ImPlotAxis& y_axis) const {
        for (int i = 0; i < Getter1.Count; ++i) {
            ImPlotPoint p = Getter1[i];
            x_axis.ExtendFitWith(y_axis, p.x, p.y);
            y_axis.ExtendFitWith(x_axis, p.y, p.x);
        }
        for (int i = 0; i < Getter2.Count; ++i) {
            ImPlotPoint p = Getter2[i];
            x_axis.ExtendFitWith(y_axis, p.x, p.y);
            y_axis.ExtendFitWith(x_axis, p.y, p.x);
        }
    }
    const _Getter1& Getter1;
    const _Getter2& Getter2;
};

template <typename _Getter1, typename _Getter2>
struct FitterBarV {
    FitterBarV(const _Getter1& getter1, const _Getter2& getter2, double width) :
        Getter1(getter1),
        Getter2(getter2),
        HalfWidth(width*0.5)
    { }
    void Fit(ImPlotAxis& x_axis, ImPlotAxis& y_axis) const {
        int count = ImMin(Getter1.Count, Getter2.Count);
        for (int i = 0; i < count; ++i) {
            ImPlotPoint p1 = Getter1[i]; p1.x -= HalfWidth;
            ImPlotPoint p2 = Getter2[i]; p2.x += HalfWidth;
            x_axis.ExtendFitWith(y_axis, p1.x, p1.y);
            y_axis.ExtendFitWith(x_axis, p1.y, p1.x);
            x_axis.ExtendFitWith(y_axis, p2.x, p2.y);
            y_axis.ExtendFitWith(x_axis, p2.y, p2.x);
        }
    }
    const _Getter1& Getter1;
    const _Getter2& Getter2;
    const double    HalfWidth;
};

template <typename _Getter1, typename _Getter2>
struct FitterBarH {
    FitterBarH(const _Getter1& getter1, const _Getter2& getter2, double height) :
        Getter1(getter1),
        Getter2(getter2),
        HalfHeight(height*0.5)
    { }
    void Fit(ImPlotAxis& x_axis, ImPlotAxis& y_axis) const {
        int count = ImMin(Getter1.Count, Getter2.Count);
        for (int i = 0; i < count; ++i) {
            ImPlotPoint p1 = Getter1[i]; p1.y -= HalfHeight;
            ImPlotPoint p2 = Getter2[i]; p2.y += HalfHeight;
            x_axis.ExtendFitWith(y_axis, p1.x, p1.y);
            y_axis.ExtendFitWith(x_axis, p1.y, p1.x);
            x_axis.ExtendFitWith(y_axis, p2.x, p2.y);
            y_axis.ExtendFitWith(x_axis, p2.y, p2.x);
        }
    }
    const _Getter1& Getter1;
    const _Getter2& Getter2;
    const double    HalfHeight;
};

struct FitterRect {
    FitterRect(const ImPlotPoint& pmin, const ImPlotPoint& pmax) :
        Pmin(pmin),
        Pmax(pmax)
    { }
    FitterRect(const ImPlotRect& rect) :
        FitterRect(rect.Min(), rect.Max())
    { }
    void Fit(ImPlotAxis& x_axis, ImPlotAxis& y_axis) const {
        x_axis.ExtendFitWith(y_axis, Pmin.x, Pmin.y);
        y_axis.ExtendFitWith(x_axis, Pmin.y, Pmin.x);
        x_axis.ExtendFitWith(y_axis, Pmax.x, Pmax.y);
        y_axis.ExtendFitWith(x_axis, Pmax.y, Pmax.x);
    }
    const ImPlotPoint Pmin;
    const ImPlotPoint Pmax;
};

//-----------------------------------------------------------------------------
// [SECTION] Transformers
//-----------------------------------------------------------------------------

struct Transformer1 {
    Transformer1(double pixMin, double pltMin, double pltMax, double m, double scaMin, double scaMax, ImPlotTransform fwd, void* data) :
        ScaMin(scaMin),
        ScaMax(scaMax),
        PltMin(pltMin),
        PltMax(pltMax),
        PixMin(pixMin),
        M(m),
        TransformFwd(fwd),
        TransformData(data)
    { }

    template <typename T> IMPLOT_INLINE float operator()(T p) const {
        if (TransformFwd != nullptr) {
            double s = TransformFwd(p, TransformData);
            double t = (s - ScaMin) / (ScaMax - ScaMin);
            p = PltMin + (PltMax - PltMin) * t;
        }
        return (float)(PixMin + M * (p - PltMin));
    }

    double ScaMin, ScaMax, PltMin, PltMax, PixMin, M;
    ImPlotTransform TransformFwd;
    void*           TransformData;
};

struct Transformer2 {
    Transformer2(const ImPlotAxis& x_axis, const ImPlotAxis& y_axis) :
        Tx(x_axis.PixelMin,
           x_axis.Range.Min,
           x_axis.Range.Max,
           x_axis.ScaleToPixel,
           x_axis.ScaleMin,
           x_axis.ScaleMax,
           x_axis.TransformForward,
           x_axis.TransformData),
        Ty(y_axis.PixelMin,
           y_axis.Range.Min,
           y_axis.Range.Max,
           y_axis.ScaleToPixel,
           y_axis.ScaleMin,
           y_axis.ScaleMax,
           y_axis.TransformForward,
           y_axis.TransformData)
    { }

    Transformer2(const ImPlotPlot& plot) :
        Transformer2(plot.Axes[plot.CurrentX], plot.Axes[plot.CurrentY])
    { }

    Transformer2() :
        Transformer2(*GImPlot->CurrentPlot)
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

    Transformer1 Tx;
    Transformer1 Ty;
};

//-----------------------------------------------------------------------------
// [SECTION] Renderers
//-----------------------------------------------------------------------------

struct RendererBase {
    RendererBase(int prims, int idx_consumed, int vtx_consumed) :
        Prims(prims),
        IdxConsumed(idx_consumed),
        VtxConsumed(vtx_consumed)
    { }
    const int Prims;
    Transformer2 Transformer;
    const int IdxConsumed;
    const int VtxConsumed;
};

template <class _Getter>
struct RendererLineStrip : RendererBase {
    RendererLineStrip(const _Getter& getter, ImU32 col, float weight) :
        RendererBase(getter.Count - 1, 6, 4),
        Getter(getter),
        Col(col),
        HalfWeight(ImMax(1.0f,weight)*0.5f)
    {
        P1 = this->Transformer(Getter[0]);
    }
    void Init(ImDrawList& draw_list) const {
        GetLineRenderProps(draw_list, HalfWeight, UV0, UV1);
    }
    IMPLOT_INLINE bool Render(ImDrawList& draw_list, const ImRect& cull_rect, int prim) const {
        ImVec2 P2 = this->Transformer(Getter[prim + 1]);
        if (!cull_rect.Overlaps(ImRect(ImMin(P1, P2), ImMax(P1, P2)))) {
            P1 = P2;
            return false;
        }
        PrimLine(draw_list,P1,P2,HalfWeight,Col,UV0,UV1);
        P1 = P2;
        return true;
    }
    const _Getter& Getter;
    const ImU32 Col;
    mutable float HalfWeight;
    mutable ImVec2 P1;
    mutable ImVec2 UV0;
    mutable ImVec2 UV1;
};

template <class _Getter>
struct RendererLineStripSkip : RendererBase {
    RendererLineStripSkip(const _Getter& getter, ImU32 col, float weight) :
        RendererBase(getter.Count - 1, 6, 4),
        Getter(getter),
        Col(col),
        HalfWeight(ImMax(1.0f,weight)*0.5f)
    {
        P1 = this->Transformer(Getter[0]);
    }
    void Init(ImDrawList& draw_list) const {
        GetLineRenderProps(draw_list, HalfWeight, UV0, UV1);
    }
    IMPLOT_INLINE bool Render(ImDrawList& draw_list, const ImRect& cull_rect, int prim) const {
        ImVec2 P2 = this->Transformer(Getter[prim + 1]);
        if (!cull_rect.Overlaps(ImRect(ImMin(P1, P2), ImMax(P1, P2)))) {
            if (!ImNan(P2.x) && !ImNan(P2.y))
                P1 = P2;
            return false;
        }
        PrimLine(draw_list,P1,P2,HalfWeight,Col,UV0,UV1);
        if (!ImNan(P2.x) && !ImNan(P2.y))
            P1 = P2;
        return true;
    }
    const _Getter& Getter;
    const ImU32 Col;
    mutable float HalfWeight;
    mutable ImVec2 P1;
    mutable ImVec2 UV0;
    mutable ImVec2 UV1;
};

template <class _Getter>
struct RendererLineSegments1 : RendererBase {
    RendererLineSegments1(const _Getter& getter, ImU32 col, float weight) :
        RendererBase(getter.Count / 2, 6, 4),
        Getter(getter),
        Col(col),
        HalfWeight(ImMax(1.0f,weight)*0.5f)
    { }
    void Init(ImDrawList& draw_list) const {
        GetLineRenderProps(draw_list, HalfWeight, UV0, UV1);
    }
    IMPLOT_INLINE bool Render(ImDrawList& draw_list, const ImRect& cull_rect, int prim) const {
        ImVec2 P1 = this->Transformer(Getter[prim*2+0]);
        ImVec2 P2 = this->Transformer(Getter[prim*2+1]);
        if (!cull_rect.Overlaps(ImRect(ImMin(P1, P2), ImMax(P1, P2))))
            return false;
        PrimLine(draw_list,P1,P2,HalfWeight,Col,UV0,UV1);
        return true;
    }
    const _Getter& Getter;
    const ImU32 Col;
    mutable float HalfWeight;
    mutable ImVec2 UV0;
    mutable ImVec2 UV1;
};

template <class _Getter1, class _Getter2>
struct RendererLineSegments2 : RendererBase {
    RendererLineSegments2(const _Getter1& getter1, const _Getter2& getter2, ImU32 col, float weight) :
        RendererBase(ImMin(getter1.Count, getter1.Count), 6, 4),
        Getter1(getter1),
        Getter2(getter2),
        Col(col),
        HalfWeight(ImMax(1.0f,weight)*0.5f)
    {}
    void Init(ImDrawList& draw_list) const {
        GetLineRenderProps(draw_list, HalfWeight, UV0, UV1);
    }
    IMPLOT_INLINE bool Render(ImDrawList& draw_list, const ImRect& cull_rect, int prim) const {
        ImVec2 P1 = this->Transformer(Getter1[prim]);
        ImVec2 P2 = this->Transformer(Getter2[prim]);
        if (!cull_rect.Overlaps(ImRect(ImMin(P1, P2), ImMax(P1, P2))))
            return false;
        PrimLine(draw_list,P1,P2,HalfWeight,Col,UV0,UV1);
        return true;
    }
    const _Getter1& Getter1;
    const _Getter2& Getter2;
    const ImU32 Col;
    mutable float HalfWeight;
    mutable ImVec2 UV0;
    mutable ImVec2 UV1;
};

template <class _Getter1, class _Getter2>
struct RendererBarsFillV : RendererBase {
    RendererBarsFillV(const _Getter1& getter1, const _Getter2& getter2, ImU32 col, double width) :
        RendererBase(ImMin(getter1.Count, getter1.Count), 6, 4),
        Getter1(getter1),
        Getter2(getter2),
        Col(col),
        HalfWidth(width/2)
    {}
    void Init(ImDrawList& draw_list) const {
        UV = draw_list._Data->TexUvWhitePixel;
    }
    IMPLOT_INLINE bool Render(ImDrawList& draw_list, const ImRect& cull_rect, int prim) const {
        ImPlotPoint p1 = Getter1[prim];
        ImPlotPoint p2 = Getter2[prim];
        p1.x += HalfWidth;
        p2.x -= HalfWidth;
        ImVec2 P1 = this->Transformer(p1);
        ImVec2 P2 = this->Transformer(p2);
        float width_px = ImAbs(P1.x-P2.x);
        if (width_px < 1.0f) {
            P1.x += P1.x > P2.x ? (1-width_px) / 2 : (width_px-1) / 2;
            P2.x += P2.x > P1.x ? (1-width_px) / 2 : (width_px-1) / 2;
        }
        ImVec2 PMin = ImMin(P1, P2);
        ImVec2 PMax = ImMax(P1, P2);
        if (!cull_rect.Overlaps(ImRect(PMin, PMax)))
            return false;
        PrimRectFill(draw_list,PMin,PMax,Col,UV);
        return true;
    }
    const _Getter1& Getter1;
    const _Getter2& Getter2;
    const ImU32 Col;
    const double HalfWidth;
    mutable ImVec2 UV;
};

template <class _Getter1, class _Getter2>
struct RendererBarsFillH : RendererBase {
    RendererBarsFillH(const _Getter1& getter1, const _Getter2& getter2, ImU32 col, double height) :
        RendererBase(ImMin(getter1.Count, getter1.Count), 6, 4),
        Getter1(getter1),
        Getter2(getter2),
        Col(col),
        HalfHeight(height/2)
    {}
    void Init(ImDrawList& draw_list) const {
        UV = draw_list._Data->TexUvWhitePixel;
    }
    IMPLOT_INLINE bool Render(ImDrawList& draw_list, const ImRect& cull_rect, int prim) const {
        ImPlotPoint p1 = Getter1[prim];
        ImPlotPoint p2 = Getter2[prim];
        p1.y += HalfHeight;
        p2.y -= HalfHeight;
        ImVec2 P1 = this->Transformer(p1);
        ImVec2 P2 = this->Transformer(p2);
        float height_px = ImAbs(P1.y-P2.y);
        if (height_px < 1.0f) {
            P1.y += P1.y > P2.y ? (1-height_px) / 2 : (height_px-1) / 2;
            P2.y += P2.y > P1.y ? (1-height_px) / 2 : (height_px-1) / 2;
        }
        ImVec2 PMin = ImMin(P1, P2);
        ImVec2 PMax = ImMax(P1, P2);
        if (!cull_rect.Overlaps(ImRect(PMin, PMax)))
            return false;
        PrimRectFill(draw_list,PMin,PMax,Col,UV);
        return true;
    }
    const _Getter1& Getter1;
    const _Getter2& Getter2;
    const ImU32 Col;
    const double HalfHeight;
    mutable ImVec2 UV;
};

template <class _Getter1, class _Getter2>
struct RendererBarsLineV : RendererBase {
    RendererBarsLineV(const _Getter1& getter1, const _Getter2& getter2, ImU32 col, double width, float weight) :
        RendererBase(ImMin(getter1.Count, getter1.Count), 24, 8),
        Getter1(getter1),
        Getter2(getter2),
        Col(col),
        HalfWidth(width/2),
        Weight(weight)
    {}
    void Init(ImDrawList& draw_list) const {
        UV = draw_list._Data->TexUvWhitePixel;
    }
    IMPLOT_INLINE bool Render(ImDrawList& draw_list, const ImRect& cull_rect, int prim) const {
        ImPlotPoint p1 = Getter1[prim];
        ImPlotPoint p2 = Getter2[prim];
        p1.x += HalfWidth;
        p2.x -= HalfWidth;
        ImVec2 P1 = this->Transformer(p1);
        ImVec2 P2 = this->Transformer(p2);
        float width_px = ImAbs(P1.x-P2.x);
        if (width_px < 1.0f) {
            P1.x += P1.x > P2.x ? (1-width_px) / 2 : (width_px-1) / 2;
            P2.x += P2.x > P1.x ? (1-width_px) / 2 : (width_px-1) / 2;
        }
        ImVec2 PMin = ImMin(P1, P2);
        ImVec2 PMax = ImMax(P1, P2);
        if (!cull_rect.Overlaps(ImRect(PMin, PMax)))
            return false;
        PrimRectLine(draw_list,PMin,PMax,Weight,Col,UV);
        return true;
    }
    const _Getter1& Getter1;
    const _Getter2& Getter2;
    const ImU32 Col;
    const double HalfWidth;
    const float Weight;
    mutable ImVec2 UV;
};

template <class _Getter1, class _Getter2>
struct RendererBarsLineH : RendererBase {
    RendererBarsLineH(const _Getter1& getter1, const _Getter2& getter2, ImU32 col, double height, float weight) :
        RendererBase(ImMin(getter1.Count, getter1.Count), 24, 8),
        Getter1(getter1),
        Getter2(getter2),
        Col(col),
        HalfHeight(height/2),
        Weight(weight)
    {}
    void Init(ImDrawList& draw_list) const {
        UV = draw_list._Data->TexUvWhitePixel;
    }
    IMPLOT_INLINE bool Render(ImDrawList& draw_list, const ImRect& cull_rect, int prim) const {
        ImPlotPoint p1 = Getter1[prim];
        ImPlotPoint p2 = Getter2[prim];
        p1.y += HalfHeight;
        p2.y -= HalfHeight;
        ImVec2 P1 = this->Transformer(p1);
        ImVec2 P2 = this->Transformer(p2);
        float height_px = ImAbs(P1.y-P2.y);
        if (height_px < 1.0f) {
            P1.y += P1.y > P2.y ? (1-height_px) / 2 : (height_px-1) / 2;
            P2.y += P2.y > P1.y ? (1-height_px) / 2 : (height_px-1) / 2;
        }
        ImVec2 PMin = ImMin(P1, P2);
        ImVec2 PMax = ImMax(P1, P2);
        if (!cull_rect.Overlaps(ImRect(PMin, PMax)))
            return false;
        PrimRectLine(draw_list,PMin,PMax,Weight,Col,UV);
        return true;
    }
    const _Getter1& Getter1;
    const _Getter2& Getter2;
    const ImU32 Col;
    const double HalfHeight;
    const float Weight;
    mutable ImVec2 UV;
};


template <class _Getter>
struct RendererStairsPre : RendererBase {
    RendererStairsPre(const _Getter& getter, ImU32 col, float weight) :
        RendererBase(getter.Count - 1, 12, 8),
        Getter(getter),
        Col(col),
        HalfWeight(ImMax(1.0f,weight)*0.5f)
    {
        P1 = this->Transformer(Getter[0]);
    }
    void Init(ImDrawList& draw_list) const {
        UV = draw_list._Data->TexUvWhitePixel;
    }
    IMPLOT_INLINE bool Render(ImDrawList& draw_list, const ImRect& cull_rect, int prim) const {
        ImVec2 P2 = this->Transformer(Getter[prim + 1]);
        if (!cull_rect.Overlaps(ImRect(ImMin(P1, P2), ImMax(P1, P2)))) {
            P1 = P2;
            return false;
        }
        PrimRectFill(draw_list, ImVec2(P1.x - HalfWeight, P1.y), ImVec2(P1.x + HalfWeight, P2.y), Col, UV);
        PrimRectFill(draw_list, ImVec2(P1.x, P2.y + HalfWeight), ImVec2(P2.x, P2.y - HalfWeight), Col, UV);
        P1 = P2;
        return true;
    }
    const _Getter& Getter;
    const ImU32 Col;
    mutable float HalfWeight;
    mutable ImVec2 P1;
    mutable ImVec2 UV;
};

template <class _Getter>
struct RendererStairsPost : RendererBase {
    RendererStairsPost(const _Getter& getter, ImU32 col, float weight) :
        RendererBase(getter.Count - 1, 12, 8),
        Getter(getter),
        Col(col),
        HalfWeight(ImMax(1.0f,weight) * 0.5f)
    {
        P1 = this->Transformer(Getter[0]);
    }
    void Init(ImDrawList& draw_list) const {
        UV = draw_list._Data->TexUvWhitePixel;
    }
    IMPLOT_INLINE bool Render(ImDrawList& draw_list, const ImRect& cull_rect, int prim) const {
        ImVec2 P2 = this->Transformer(Getter[prim + 1]);
        if (!cull_rect.Overlaps(ImRect(ImMin(P1, P2), ImMax(P1, P2)))) {
            P1 = P2;
            return false;
        }
        PrimRectFill(draw_list, ImVec2(P1.x, P1.y + HalfWeight), ImVec2(P2.x, P1.y - HalfWeight), Col, UV);
        PrimRectFill(draw_list, ImVec2(P2.x - HalfWeight, P2.y), ImVec2(P2.x + HalfWeight, P1.y), Col, UV);
        P1 = P2;
        return true;
    }
    const _Getter& Getter;
    const ImU32 Col;
    mutable float HalfWeight;
    mutable ImVec2 P1;
    mutable ImVec2 UV;
};

template <class _Getter>
struct RendererStairsPreShaded : RendererBase {
    RendererStairsPreShaded(const _Getter& getter, ImU32 col) :
        RendererBase(getter.Count - 1, 6, 4),
        Getter(getter),
        Col(col)
    {
        P1 = this->Transformer(Getter[0]);
        Y0 = this->Transformer(ImPlotPoint(0,0)).y;
    }
    void Init(ImDrawList& draw_list) const {
        UV = draw_list._Data->TexUvWhitePixel;
    }
    IMPLOT_INLINE bool Render(ImDrawList& draw_list, const ImRect& cull_rect, int prim) const {
        ImVec2 P2 = this->Transformer(Getter[prim + 1]);
        ImVec2 PMin(ImMin(P1.x, P2.x), ImMin(Y0, P2.y));
        ImVec2 PMax(ImMax(P1.x, P2.x), ImMax(Y0, P2.y));
        if (!cull_rect.Overlaps(ImRect(PMin, PMax))) {
            P1 = P2;
            return false;
        }
        PrimRectFill(draw_list, PMin, PMax, Col, UV);
        P1 = P2;
        return true;
    }
    const _Getter& Getter;
    const ImU32 Col;
    float Y0;
    mutable ImVec2 P1;
    mutable ImVec2 UV;
};

template <class _Getter>
struct RendererStairsPostShaded : RendererBase {
    RendererStairsPostShaded(const _Getter& getter, ImU32 col) :
        RendererBase(getter.Count - 1, 6, 4),
        Getter(getter),
        Col(col)
    {
        P1 = this->Transformer(Getter[0]);
        Y0 = this->Transformer(ImPlotPoint(0,0)).y;
    }
    void Init(ImDrawList& draw_list) const {
        UV = draw_list._Data->TexUvWhitePixel;
    }
    IMPLOT_INLINE bool Render(ImDrawList& draw_list, const ImRect& cull_rect, int prim) const {
        ImVec2 P2 = this->Transformer(Getter[prim + 1]);
        ImVec2 PMin(ImMin(P1.x, P2.x), ImMin(P1.y, Y0));
        ImVec2 PMax(ImMax(P1.x, P2.x), ImMax(P1.y, Y0));
        if (!cull_rect.Overlaps(ImRect(PMin, PMax))) {
            P1 = P2;
            return false;
        }
        PrimRectFill(draw_list, PMin, PMax, Col, UV);
        P1 = P2;
        return true;
    }
    const _Getter& Getter;
    const ImU32 Col;
    float Y0;
    mutable ImVec2 P1;
    mutable ImVec2 UV;
};



template <class _Getter1, class _Getter2>
struct RendererShaded : RendererBase {
    RendererShaded(const _Getter1& getter1, const _Getter2& getter2, ImU32 col) :
        RendererBase(ImMin(getter1.Count, getter2.Count) - 1, 6, 5),
        Getter1(getter1),
        Getter2(getter2),
        Col(col)
    {
        P11 = this->Transformer(Getter1[0]);
        P12 = this->Transformer(Getter2[0]);
    }
    void Init(ImDrawList& draw_list) const {
        UV = draw_list._Data->TexUvWhitePixel;
    }
    IMPLOT_INLINE bool Render(ImDrawList& draw_list, const ImRect& cull_rect, int prim) const {
        ImVec2 P21 = this->Transformer(Getter1[prim+1]);
        ImVec2 P22 = this->Transformer(Getter2[prim+1]);
        ImRect rect(ImMin(ImMin(ImMin(P11,P12),P21),P22), ImMax(ImMax(ImMax(P11,P12),P21),P22));
        if (!cull_rect.Overlaps(rect)) {
            P11 = P21;
            P12 = P22;
            return false;
        }
        const int intersect = (P11.y > P12.y && P22.y > P21.y) || (P12.y > P11.y && P21.y > P22.y);
        const ImVec2 intersection = intersect == 0 ? ImVec2(0,0) : Intersection(P11,P21,P12,P22);
        draw_list._VtxWritePtr[0].pos = P11;
        draw_list._VtxWritePtr[0].uv  = UV;
        draw_list._VtxWritePtr[0].col = Col;
        draw_list._VtxWritePtr[1].pos = P21;
        draw_list._VtxWritePtr[1].uv  = UV;
        draw_list._VtxWritePtr[1].col = Col;
        draw_list._VtxWritePtr[2].pos = intersection;
        draw_list._VtxWritePtr[2].uv  = UV;
        draw_list._VtxWritePtr[2].col = Col;
        draw_list._VtxWritePtr[3].pos = P12;
        draw_list._VtxWritePtr[3].uv  = UV;
        draw_list._VtxWritePtr[3].col = Col;
        draw_list._VtxWritePtr[4].pos = P22;
        draw_list._VtxWritePtr[4].uv  = UV;
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
    mutable ImVec2 UV;
};

struct RectC {
    ImPlotPoint Pos;
    ImPlotPoint HalfSize;
    ImU32 Color;
};

template <typename _Getter>
struct RendererRectC : RendererBase {
    RendererRectC(const _Getter& getter) :
        RendererBase(getter.Count, 6, 4),
        Getter(getter)
    {}
    void Init(ImDrawList& draw_list) const {
        UV = draw_list._Data->TexUvWhitePixel;
    }
    IMPLOT_INLINE bool Render(ImDrawList& draw_list, const ImRect& cull_rect, int prim) const {
        RectC rect = Getter[prim];
        ImVec2 P1 = this->Transformer(rect.Pos.x - rect.HalfSize.x , rect.Pos.y - rect.HalfSize.y);
        ImVec2 P2 = this->Transformer(rect.Pos.x + rect.HalfSize.x , rect.Pos.y + rect.HalfSize.y);
        if ((rect.Color & IM_COL32_A_MASK) == 0 || !cull_rect.Overlaps(ImRect(ImMin(P1, P2), ImMax(P1, P2))))
            return false;
        PrimRectFill(draw_list,P1,P2,rect.Color,UV);
        return true;
    }
    const _Getter& Getter;
    mutable ImVec2 UV;
};

//-----------------------------------------------------------------------------
// [SECTION] RenderPrimitives
//-----------------------------------------------------------------------------

/// Renders primitive shapes in bulk as efficiently as possible.
template <class _Renderer>
void RenderPrimitivesEx(const _Renderer& renderer, ImDrawList& draw_list, const ImRect& cull_rect) {
    unsigned int prims        = renderer.Prims;
    unsigned int prims_culled = 0;
    unsigned int idx          = 0;
    renderer.Init(draw_list);
    while (prims) {
        // find how many can be reserved up to end of current draw command's limit
        unsigned int cnt = ImMin(prims, (MaxIdx<ImDrawIdx>::Value - draw_list._VtxCurrentIdx) / renderer.VtxConsumed);
        // make sure at least this many elements can be rendered to avoid situations where at the end of buffer this slow path is not taken all the time
        if (cnt >= ImMin(64u, prims)) {
            if (prims_culled >= cnt)
                prims_culled -= cnt; // reuse previous reservation
            else {
                // add more elements to previous reservation
                draw_list.PrimReserve((cnt - prims_culled) * renderer.IdxConsumed, (cnt - prims_culled) * renderer.VtxConsumed);
                prims_culled = 0;
            }
        }
        else
        {
            if (prims_culled > 0) {
                draw_list.PrimUnreserve(prims_culled * renderer.IdxConsumed, prims_culled * renderer.VtxConsumed);
                prims_culled = 0;
            }
            cnt = ImMin(prims, (MaxIdx<ImDrawIdx>::Value - 0/*draw_list._VtxCurrentIdx*/) / renderer.VtxConsumed);
            // reserve new draw command
            draw_list.PrimReserve(cnt * renderer.IdxConsumed, cnt * renderer.VtxConsumed);
        }
        prims -= cnt;
        for (unsigned int ie = idx + cnt; idx != ie; ++idx) {
            if (!renderer.Render(draw_list, cull_rect, idx))
                prims_culled++;
        }
    }
    if (prims_culled > 0)
        draw_list.PrimUnreserve(prims_culled * renderer.IdxConsumed, prims_culled * renderer.VtxConsumed);
}

template <template <class> class _Renderer, class _Getter, typename ...Args>
void RenderPrimitives1(const _Getter& getter, Args... args) {
    ImDrawList& draw_list = *GetPlotDrawList();
    const ImRect& cull_rect = GetCurrentPlot()->PlotRect;
    RenderPrimitivesEx(_Renderer<_Getter>(getter,args...), draw_list, cull_rect);
}

template <template <class,class> class _Renderer, class _Getter1, class _Getter2, typename ...Args>
void RenderPrimitives2(const _Getter1& getter1, const _Getter2& getter2, Args... args) {
    ImDrawList& draw_list = *GetPlotDrawList();
    const ImRect& cull_rect = GetCurrentPlot()->PlotRect;
    RenderPrimitivesEx(_Renderer<_Getter1,_Getter2>(getter1,getter2,args...), draw_list, cull_rect);
}

//-----------------------------------------------------------------------------
// [SECTION] Markers
//-----------------------------------------------------------------------------

template <class _Getter>
struct RendererMarkersFill : RendererBase {
    RendererMarkersFill(const _Getter& getter, const ImVec2* marker, int count, float size, ImU32 col) :
        RendererBase(getter.Count, (count-2)*3, count),
        Getter(getter),
        Marker(marker),
        Count(count),
        Size(size),
        Col(col)
    { }
    void Init(ImDrawList& draw_list) const {
        UV = draw_list._Data->TexUvWhitePixel;
    }
    IMPLOT_INLINE bool Render(ImDrawList& draw_list, const ImRect& cull_rect, int prim) const {
        ImVec2 p = this->Transformer(Getter[prim]);
        if (p.x >= cull_rect.Min.x && p.y >= cull_rect.Min.y && p.x <= cull_rect.Max.x && p.y <= cull_rect.Max.y) {
            for (int i = 0; i < Count; i++) {
                draw_list._VtxWritePtr[0].pos.x = p.x + Marker[i].x * Size;
                draw_list._VtxWritePtr[0].pos.y = p.y + Marker[i].y * Size;
                draw_list._VtxWritePtr[0].uv = UV;
                draw_list._VtxWritePtr[0].col = Col;
                draw_list._VtxWritePtr++;
            }
            for (int i = 2; i < Count; i++) {
                draw_list._IdxWritePtr[0] = (ImDrawIdx)(draw_list._VtxCurrentIdx);
                draw_list._IdxWritePtr[1] = (ImDrawIdx)(draw_list._VtxCurrentIdx + i - 1);
                draw_list._IdxWritePtr[2] = (ImDrawIdx)(draw_list._VtxCurrentIdx + i);
                draw_list._IdxWritePtr += 3;
            }
            draw_list._VtxCurrentIdx += (ImDrawIdx)Count;
            return true;
        }
        return false;
    }
    const _Getter& Getter;
    const ImVec2* Marker;
    const int Count;
    const float Size;
    const ImU32 Col;
    mutable ImVec2 UV;
};


template <class _Getter>
struct RendererMarkersLine : RendererBase {
    RendererMarkersLine(const _Getter& getter, const ImVec2* marker, int count, float size, float weight, ImU32 col) :
        RendererBase(getter.Count, count/2*6, count/2*4),
        Getter(getter),
        Marker(marker),
        Count(count),
        HalfWeight(ImMax(1.0f,weight)*0.5f),
        Size(size),
        Col(col)
    { }
    void Init(ImDrawList& draw_list) const {
        GetLineRenderProps(draw_list, HalfWeight, UV0, UV1);
    }
    IMPLOT_INLINE bool Render(ImDrawList& draw_list, const ImRect& cull_rect, int prim) const {
        ImVec2 p = this->Transformer(Getter[prim]);
        if (p.x >= cull_rect.Min.x && p.y >= cull_rect.Min.y && p.x <= cull_rect.Max.x && p.y <= cull_rect.Max.y) {
            for (int i = 0; i < Count; i = i + 2) {
                ImVec2 p1(p.x + Marker[i].x * Size, p.y + Marker[i].y * Size);
                ImVec2 p2(p.x + Marker[i+1].x * Size, p.y + Marker[i+1].y * Size);
                PrimLine(draw_list, p1, p2, HalfWeight, Col, UV0, UV1);
            }
            return true;
        }
        return false;
    }
    const _Getter& Getter;
    const ImVec2* Marker;
    const int Count;
    mutable float HalfWeight;
    const float Size;
    const ImU32 Col;
    mutable ImVec2 UV0;
    mutable ImVec2 UV1;
};

template <class _Getter>
struct RendererCircleFill : RendererBase {
    RendererCircleFill(const _Getter& getter, ImU32 col) :
        RendererBase(getter.Count, 62*3, 64),
        Getter(getter),
        Col(col)
    { }
    void Init(ImDrawList& draw_list) const {
        UV = draw_list._Data->TexUvWhitePixel;
    }
    IMPLOT_INLINE bool Render(ImDrawList& draw_list, const ImRect& cull_rect, int prim) const {
        ImPlotPoint3D p3D = Getter(prim);
        float size_in_plot_coords = (float)p3D.z;
        float radius_in_plot_coords = size_in_plot_coords;

        // Compute approximate radius in pixels for LOD
        float approx_radius_pixels = (float)ImAbs(radius_in_plot_coords * this->Transformer.Tx.M);
        int num_segments = ImClamp((int)(approx_radius_pixels), 10, 64);

        // Compute bounding box of the bubble in plot coordinates
        ImPlotPoint plot_min(p3D.x - radius_in_plot_coords, p3D.y - radius_in_plot_coords);
        ImPlotPoint plot_max(p3D.x + radius_in_plot_coords, p3D.y + radius_in_plot_coords);

        // Transform bounding box to pixel coordinates
        ImVec2 pixel_min = this->Transformer(plot_min);
        ImVec2 pixel_max = this->Transformer(plot_max);

        // Create bounding rectangle (handle axis inversion)
        ImRect bbox(ImMin(pixel_min, pixel_max), ImMax(pixel_min, pixel_max));

        // Check if bounding box overlaps with cull rectangle
        if (cull_rect.Overlaps(bbox)) {

            const float a_max = IM_PI * 2.0f;
            const float a_step = a_max / num_segments;

            ImDrawIdx vtx_base = (ImDrawIdx)draw_list._VtxCurrentIdx;

            for (int i = 0; i < num_segments; i++) {
                float angle = a_step * i;
                float cos_a = ImCos(angle);
                float sin_a = ImSin(angle);

                // Compute point in plot coordinates
                ImPlotPoint plot_point(p3D.x + cos_a * radius_in_plot_coords,
                                      p3D.y + sin_a * radius_in_plot_coords);
                // Transform to pixel coordinates
                ImVec2 pixel_pos = this->Transformer(plot_point);

                draw_list._VtxWritePtr[0].pos = pixel_pos;
                draw_list._VtxWritePtr[0].uv = UV;
                draw_list._VtxWritePtr[0].col = Col;
                draw_list._VtxWritePtr++;
            }

            for (int i = 2; i < num_segments; i++) {
                draw_list._IdxWritePtr[0] = vtx_base;
                draw_list._IdxWritePtr[1] = (ImDrawIdx)(vtx_base + i - 1);
                draw_list._IdxWritePtr[2] = (ImDrawIdx)(vtx_base + i);
                draw_list._IdxWritePtr += 3;
            }

            draw_list._VtxCurrentIdx += num_segments;

            int unused_vtx = 64 - num_segments;
            int unused_idx = (62 - (num_segments - 2)) * 3;
            if (unused_vtx > 0 || unused_idx > 0) {
                draw_list.PrimUnreserve(unused_idx, unused_vtx);
            }

            return true;
        }
        return false;
    }
    const _Getter& Getter;
    const ImU32 Col;
    mutable ImVec2 UV;
};

template <class _Getter>
struct RendererCircleLine : RendererBase {
  RendererCircleLine(const _Getter& getter, float weight, ImU32 col) :
      RendererBase(getter.Count, 64*6, 64*4),
      Getter(getter),
      HalfWeight(ImMax(1.0f,weight)*0.5f),
      Col(col)
    { }
    void Init(ImDrawList& draw_list) const {
      GetLineRenderProps(draw_list, HalfWeight, UV0, UV1);
    }
    IMPLOT_INLINE bool Render(ImDrawList& draw_list, const ImRect& cull_rect, int prim) const {
        ImPlotPoint3D p3D = Getter(prim);
        float size_in_plot_coords = (float)p3D.z;
        float radius_in_plot_coords = size_in_plot_coords;

        // Compute approximate radius in pixels for LOD
        float approx_radius_pixels = (float)ImAbs(radius_in_plot_coords * this->Transformer.Tx.M);
        int num_segments = ImClamp((int)(approx_radius_pixels), 10, 64);

        // Compute bounding box of the bubble in plot coordinates
        ImPlotPoint plot_min(p3D.x - radius_in_plot_coords, p3D.y - radius_in_plot_coords);
        ImPlotPoint plot_max(p3D.x + radius_in_plot_coords, p3D.y + radius_in_plot_coords);

        // Transform bounding box to pixel coordinates
        ImVec2 pixel_min = this->Transformer(plot_min);
        ImVec2 pixel_max = this->Transformer(plot_max);

        // Create bounding rectangle (handle axis inversion)
        ImRect bbox(ImMin(pixel_min, pixel_max), ImMax(pixel_min, pixel_max));

        // Check if bounding box overlaps with cull rectangle
        if (cull_rect.Overlaps(bbox)) {

            const float a_max = IM_PI * 2.0f;
            const float a_step = a_max / num_segments;

            for (int i = 0; i < num_segments; i++) {
                float angle1 = a_step * i;
                float angle2 = a_step * ((i + 1) % num_segments);

                // Compute points in plot coordinates
                ImPlotPoint plot_point1(p3D.x + ImCos(angle1) * radius_in_plot_coords,
                                       p3D.y + ImSin(angle1) * radius_in_plot_coords);
                ImPlotPoint plot_point2(p3D.x + ImCos(angle2) * radius_in_plot_coords,
                                       p3D.y + ImSin(angle2) * radius_in_plot_coords);

                // Transform to pixel coordinates
                ImVec2 p1 = this->Transformer(plot_point1);
                ImVec2 p2 = this->Transformer(plot_point2);

                PrimLine(draw_list, p1, p2, HalfWeight, Col, UV0, UV1);
            }

            int unused_vtx = (64 - num_segments) * 4;
            int unused_idx = (64 - num_segments) * 6;
            if (unused_vtx > 0 || unused_idx > 0) {
                draw_list.PrimUnreserve(unused_idx, unused_vtx);
            }

            return true;
        }
        return false;
    }
    const _Getter& Getter;
    mutable float HalfWeight;
    const ImU32 Col;
    mutable ImVec2 UV0;
    mutable ImVec2 UV1;
};


static const ImVec2 MARKER_FILL_CIRCLE[10]  = {ImVec2(1.0f, 0.0f), ImVec2(0.809017f, 0.58778524f),ImVec2(0.30901697f, 0.95105654f),ImVec2(-0.30901703f, 0.9510565f),ImVec2(-0.80901706f, 0.5877852f),ImVec2(-1.0f, 0.0f),ImVec2(-0.80901694f, -0.58778536f),ImVec2(-0.3090171f, -0.9510565f),ImVec2(0.30901712f, -0.9510565f),ImVec2(0.80901694f, -0.5877853f)};
static const ImVec2 MARKER_FILL_SQUARE[4]   = {ImVec2(SQRT_1_2,SQRT_1_2), ImVec2(SQRT_1_2,-SQRT_1_2), ImVec2(-SQRT_1_2,-SQRT_1_2), ImVec2(-SQRT_1_2,SQRT_1_2)};
static const ImVec2 MARKER_FILL_DIAMOND[4]  = {ImVec2(1, 0), ImVec2(0, -1), ImVec2(-1, 0), ImVec2(0, 1)};
static const ImVec2 MARKER_FILL_UP[3]       = {ImVec2(SQRT_3_2,0.5f),ImVec2(0,-1),ImVec2(-SQRT_3_2,0.5f)};
static const ImVec2 MARKER_FILL_DOWN[3]     = {ImVec2(SQRT_3_2,-0.5f),ImVec2(0,1),ImVec2(-SQRT_3_2,-0.5f)};
static const ImVec2 MARKER_FILL_LEFT[3]     = {ImVec2(-1,0), ImVec2(0.5, SQRT_3_2), ImVec2(0.5, -SQRT_3_2)};
static const ImVec2 MARKER_FILL_RIGHT[3]    = {ImVec2(1,0), ImVec2(-0.5, SQRT_3_2), ImVec2(-0.5, -SQRT_3_2)};

static const ImVec2 MARKER_LINE_CIRCLE[20]  = {
    ImVec2(1.0f, 0.0f),
    ImVec2(0.809017f, 0.58778524f),
    ImVec2(0.809017f, 0.58778524f),
    ImVec2(0.30901697f, 0.95105654f),
    ImVec2(0.30901697f, 0.95105654f),
    ImVec2(-0.30901703f, 0.9510565f),
    ImVec2(-0.30901703f, 0.9510565f),
    ImVec2(-0.80901706f, 0.5877852f),
    ImVec2(-0.80901706f, 0.5877852f),
    ImVec2(-1.0f, 0.0f),
    ImVec2(-1.0f, 0.0f),
    ImVec2(-0.80901694f, -0.58778536f),
    ImVec2(-0.80901694f, -0.58778536f),
    ImVec2(-0.3090171f, -0.9510565f),
    ImVec2(-0.3090171f, -0.9510565f),
    ImVec2(0.30901712f, -0.9510565f),
    ImVec2(0.30901712f, -0.9510565f),
    ImVec2(0.80901694f, -0.5877853f),
    ImVec2(0.80901694f, -0.5877853f),
    ImVec2(1.0f, 0.0f)
};
constexpr ImVec2 MARKER_LINE_SQUARE[8]   = {ImVec2(SQRT_1_2,SQRT_1_2), ImVec2(SQRT_1_2,-SQRT_1_2), ImVec2(SQRT_1_2,-SQRT_1_2), ImVec2(-SQRT_1_2,-SQRT_1_2), ImVec2(-SQRT_1_2,-SQRT_1_2), ImVec2(-SQRT_1_2,SQRT_1_2), ImVec2(-SQRT_1_2,SQRT_1_2), ImVec2(SQRT_1_2,SQRT_1_2)};
constexpr ImVec2 MARKER_LINE_DIAMOND[8]  = {ImVec2(1, 0), ImVec2(0, -1), ImVec2(0, -1), ImVec2(-1, 0), ImVec2(-1, 0), ImVec2(0, 1), ImVec2(0, 1), ImVec2(1, 0)};
constexpr ImVec2 MARKER_LINE_UP[6]       = {ImVec2(SQRT_3_2,0.5f), ImVec2(0,-1),ImVec2(0,-1),ImVec2(-SQRT_3_2,0.5f),ImVec2(-SQRT_3_2,0.5f),ImVec2(SQRT_3_2,0.5f)};
constexpr ImVec2 MARKER_LINE_DOWN[6]     = {ImVec2(SQRT_3_2,-0.5f),ImVec2(0,1),ImVec2(0,1),ImVec2(-SQRT_3_2,-0.5f), ImVec2(-SQRT_3_2,-0.5f), ImVec2(SQRT_3_2,-0.5f)};
constexpr ImVec2 MARKER_LINE_LEFT[6]     = {ImVec2(-1,0), ImVec2(0.5, SQRT_3_2),  ImVec2(0.5, SQRT_3_2),  ImVec2(0.5, -SQRT_3_2) , ImVec2(0.5, -SQRT_3_2) , ImVec2(-1,0) };
constexpr ImVec2 MARKER_LINE_RIGHT[6]    = {ImVec2(1,0),  ImVec2(-0.5, SQRT_3_2), ImVec2(-0.5, SQRT_3_2), ImVec2(-0.5, -SQRT_3_2), ImVec2(-0.5, -SQRT_3_2), ImVec2(1,0) };
constexpr ImVec2 MARKER_LINE_ASTERISK[6] = {ImVec2(-SQRT_3_2, -0.5f), ImVec2(SQRT_3_2, 0.5f),  ImVec2(-SQRT_3_2, 0.5f), ImVec2(SQRT_3_2, -0.5f), ImVec2(0, -1), ImVec2(0, 1)};
constexpr ImVec2 MARKER_LINE_PLUS[4]     = {ImVec2(-1, 0), ImVec2(1, 0), ImVec2(0, -1), ImVec2(0, 1)};
constexpr ImVec2 MARKER_LINE_CROSS[4]    = {ImVec2(-SQRT_1_2,-SQRT_1_2),ImVec2(SQRT_1_2,SQRT_1_2),ImVec2(SQRT_1_2,-SQRT_1_2),ImVec2(-SQRT_1_2,SQRT_1_2)};

template <typename _Getter>
void RenderMarkers(const _Getter& getter, ImPlotMarker marker, float size, bool rend_fill, ImU32 col_fill, bool rend_line, ImU32 col_line, float weight) {
    if (rend_fill) {
        switch (marker) {
            case ImPlotMarker_Circle  : RenderPrimitives1<RendererMarkersFill>(getter,MARKER_FILL_CIRCLE,10,size,col_fill); break;
            case ImPlotMarker_Square  : RenderPrimitives1<RendererMarkersFill>(getter,MARKER_FILL_SQUARE, 4,size,col_fill); break;
            case ImPlotMarker_Diamond : RenderPrimitives1<RendererMarkersFill>(getter,MARKER_FILL_DIAMOND,4,size,col_fill); break;
            case ImPlotMarker_Up      : RenderPrimitives1<RendererMarkersFill>(getter,MARKER_FILL_UP,     3,size,col_fill); break;
            case ImPlotMarker_Down    : RenderPrimitives1<RendererMarkersFill>(getter,MARKER_FILL_DOWN,   3,size,col_fill); break;
            case ImPlotMarker_Left    : RenderPrimitives1<RendererMarkersFill>(getter,MARKER_FILL_LEFT,   3,size,col_fill); break;
            case ImPlotMarker_Right   : RenderPrimitives1<RendererMarkersFill>(getter,MARKER_FILL_RIGHT,  3,size,col_fill); break;
        }
    }
    if (rend_line) {
        switch (marker) {
            case ImPlotMarker_Circle    : RenderPrimitives1<RendererMarkersLine>(getter,MARKER_LINE_CIRCLE, 20,size,weight,col_line); break;
            case ImPlotMarker_Square    : RenderPrimitives1<RendererMarkersLine>(getter,MARKER_LINE_SQUARE,  8,size,weight,col_line); break;
            case ImPlotMarker_Diamond   : RenderPrimitives1<RendererMarkersLine>(getter,MARKER_LINE_DIAMOND, 8,size,weight,col_line); break;
            case ImPlotMarker_Up        : RenderPrimitives1<RendererMarkersLine>(getter,MARKER_LINE_UP,      6,size,weight,col_line); break;
            case ImPlotMarker_Down      : RenderPrimitives1<RendererMarkersLine>(getter,MARKER_LINE_DOWN,    6,size,weight,col_line); break;
            case ImPlotMarker_Left      : RenderPrimitives1<RendererMarkersLine>(getter,MARKER_LINE_LEFT,    6,size,weight,col_line); break;
            case ImPlotMarker_Right     : RenderPrimitives1<RendererMarkersLine>(getter,MARKER_LINE_RIGHT,   6,size,weight,col_line); break;
            case ImPlotMarker_Asterisk  : RenderPrimitives1<RendererMarkersLine>(getter,MARKER_LINE_ASTERISK,6,size,weight,col_line); break;
            case ImPlotMarker_Plus      : RenderPrimitives1<RendererMarkersLine>(getter,MARKER_LINE_PLUS,    4,size,weight,col_line); break;
            case ImPlotMarker_Cross     : RenderPrimitives1<RendererMarkersLine>(getter,MARKER_LINE_CROSS,   4,size,weight,col_line); break;
        }
    }
}

//-----------------------------------------------------------------------------
// [SECTION] PlotLine
//-----------------------------------------------------------------------------

template <typename _Getter>
void PlotLineEx(const char* label_id, const _Getter& getter, const ImPlotSpec& spec) {
    if (BeginItemEx(label_id, Fitter1<_Getter>(getter), spec, spec.LineColor, spec.Marker)) {
        if (getter.Count <= 0) {
            EndItem();
            return;
        }
        const ImPlotNextItemData& s = GetItemData();
        if (getter.Count > 1) {
            if (ImHasFlag(spec.Flags, ImPlotLineFlags_Shaded) && s.RenderFill) {
                const ImU32 col_fill = ImGui::GetColorU32(s.Spec.FillColor);
                GetterOverrideY<_Getter> getter2(getter, 0);
                RenderPrimitives2<RendererShaded>(getter,getter2,col_fill);
            }
            if (s.RenderLine) {
                const ImU32 col_line = ImGui::GetColorU32(s.Spec.LineColor);
                if (ImHasFlag(spec.Flags,ImPlotLineFlags_Segments)) {
                    RenderPrimitives1<RendererLineSegments1>(getter,col_line,s.Spec.LineWeight);
                }
                else if (ImHasFlag(spec.Flags, ImPlotLineFlags_Loop)) {
                    if (ImHasFlag(spec.Flags, ImPlotLineFlags_SkipNaN))
                        RenderPrimitives1<RendererLineStripSkip>(GetterLoop<_Getter>(getter),col_line,s.Spec.LineWeight);
                    else
                        RenderPrimitives1<RendererLineStrip>(GetterLoop<_Getter>(getter),col_line,s.Spec.LineWeight);
                }
                else {
                    if (ImHasFlag(spec.Flags, ImPlotLineFlags_SkipNaN))
                        RenderPrimitives1<RendererLineStripSkip>(getter,col_line,s.Spec.LineWeight);
                    else
                        RenderPrimitives1<RendererLineStrip>(getter,col_line,s.Spec.LineWeight);
                }
            }
        }
        // render markers
        if (s.RenderMarkers) {
            if (ImHasFlag(spec.Flags, ImPlotLineFlags_NoClip)) {
                PopPlotClipRect();
                PushPlotClipRect(s.Spec.MarkerSize);
            }
            const ImU32 col_line = ImGui::GetColorU32(s.Spec.MarkerLineColor);
            const ImU32 col_fill = ImGui::GetColorU32(s.Spec.MarkerFillColor);
            RenderMarkers<_Getter>(getter, s.Spec.Marker, s.Spec.MarkerSize, s.RenderMarkerFill, col_fill, s.RenderMarkerLine, col_line, s.Spec.LineWeight);
        }
        EndItem();
    }
}

template <typename T>
void PlotLine(const char* label_id, const T* values, int count, double xscale, double x0, const ImPlotSpec& spec) {
    GetterXY<IndexerLin,IndexerIdx<T>> getter(IndexerLin(xscale,x0),IndexerIdx<T>(values,count,spec.Offset,Stride<T>(spec)),count);
    PlotLineEx(label_id, getter, spec);
}

template <typename T>
void PlotLine(const char* label_id, const T* xs, const T* ys, int count, const ImPlotSpec& spec) {
    GetterXY<IndexerIdx<T>,IndexerIdx<T>> getter(IndexerIdx<T>(xs,count,spec.Offset,Stride<T>(spec)),IndexerIdx<T>(ys,count,spec.Offset,Stride<T>(spec)),count);
    PlotLineEx(label_id, getter, spec);
}

#define INSTANTIATE_MACRO(T) \
    template IMPLOT_API void PlotLine<T> (const char* label_id, const T* values, int count, double xscale, double x0, const ImPlotSpec& spec); \
    template IMPLOT_API void PlotLine<T>(const char* label_id, const T* xs, const T* ys, int count, const ImPlotSpec& spec);
CALL_INSTANTIATE_FOR_NUMERIC_TYPES()
#undef INSTANTIATE_MACRO

// custom
void PlotLineG(const char* label_id, ImPlotGetter getter_func, void* data, int count, const ImPlotSpec& spec) {
    GetterFuncPtr getter(getter_func,data, count);
    PlotLineEx(label_id, getter, spec);
}

//-----------------------------------------------------------------------------
// [SECTION] PlotScatter
//-----------------------------------------------------------------------------

template <typename Getter>
void PlotScatterEx(const char* label_id, const Getter& getter, const ImPlotSpec& spec) {
    // force scatter to render a marker even if none
    ImPlotMarker marker = spec.Marker == ImPlotMarker_None ? ImPlotMarker_Auto: spec.Marker;
    if (BeginItemEx(label_id, Fitter1<Getter>(getter), spec, spec.LineColor, marker)) {
        if (getter.Count <= 0) {
            EndItem();
            return;
        }
        const ImPlotNextItemData& s = GetItemData();
        if (s.RenderMarkers) {
            if (ImHasFlag(spec.Flags,ImPlotScatterFlags_NoClip)) {
                PopPlotClipRect();
                PushPlotClipRect(s.Spec.MarkerSize);
            }
            const ImU32 col_line = ImGui::GetColorU32(s.Spec.MarkerLineColor);
            const ImU32 col_fill = ImGui::GetColorU32(s.Spec.MarkerFillColor);
            RenderMarkers<Getter>(getter, s.Spec.Marker, s.Spec.MarkerSize, s.RenderMarkerFill, col_fill, s.RenderMarkerLine, col_line, s.Spec.LineWeight);
        }
        EndItem();
    }
}

template <typename T>
void PlotScatter(const char* label_id, const T* values, int count, double xscale, double x0, const ImPlotSpec& spec) {
    GetterXY<IndexerLin,IndexerIdx<T>> getter(IndexerLin(xscale,x0),IndexerIdx<T>(values,count,spec.Offset, Stride<T>(spec)),count);
    PlotScatterEx(label_id, getter, spec);
}

template <typename T>
void PlotScatter(const char* label_id, const T* xs, const T* ys, int count, const ImPlotSpec& spec) {
    GetterXY<IndexerIdx<T>,IndexerIdx<T>> getter(IndexerIdx<T>(xs,count,spec.Offset, Stride<T>(spec)),IndexerIdx<T>(ys,count,spec.Offset, Stride<T>(spec)),count);
    return PlotScatterEx(label_id, getter, spec);
}

#define INSTANTIATE_MACRO(T) \
    template IMPLOT_API void PlotScatter<T>(const char* label_id, const T* values, int count, double xscale, double x0, const ImPlotSpec& spec); \
    template IMPLOT_API void PlotScatter<T>(const char* label_id, const T* xs, const T* ys, int count, const ImPlotSpec& spec);
CALL_INSTANTIATE_FOR_NUMERIC_TYPES()
#undef INSTANTIATE_MACRO

// custom
void PlotScatterG(const char* label_id, ImPlotGetter getter_func, void* data, int count, const ImPlotSpec& spec) {
    GetterFuncPtr getter(getter_func,data, count);
    return PlotScatterEx(label_id, getter, spec);
}

//-----------------------------------------------------------------------------
// [SECTION] PlotBubbles
//-----------------------------------------------------------------------------

template <typename Getter>
void PlotBubblesEx(const char* label_id, const Getter& getter, const ImPlotSpec& spec) {
    if (BeginItemEx(label_id, FitterBubbles1<Getter>(getter), spec, spec.FillColor, spec.Marker)) {
        if (getter.Count <= 0) {
            EndItem();
            return;
        }
        const ImPlotNextItemData& s = GetItemData();

        if (s.RenderFill) {
            const ImU32 col_fill = ImGui::GetColorU32(s.Spec.FillColor);
            RenderPrimitives1<RendererCircleFill>(getter, col_fill);
        }
        if (s.RenderLine) {
            const ImU32 col_line = ImGui::GetColorU32(s.Spec.LineColor);
            RenderPrimitives1<RendererCircleLine>(getter, s.Spec.LineWeight, col_line);
        }

        EndItem();
    }
}

template <typename T>
void PlotBubbles(const char* label_id, const T* values, const T* szs, int count, double xscale, double x0, const ImPlotSpec& spec) {
  GetterXYZ<IndexerLin,IndexerIdx<T>,IndexerIdx<T>> getter(IndexerLin(xscale,x0), IndexerIdx<T>(values,count,spec.Offset,Stride<T>(spec)), IndexerIdx<T>(szs,count,spec.Offset,Stride<T>(spec)),count);
  PlotBubblesEx(label_id, getter, spec);
}

template <typename T>
void PlotBubbles(const char* label_id, const T* xs, const T* ys, const T* szs, int count, const ImPlotSpec& spec) {
  GetterXYZ<IndexerIdx<T>,IndexerIdx<T>,IndexerIdx<T>> getter(IndexerIdx<T>(xs,count,spec.Offset,Stride<T>(spec)),IndexerIdx<T>(ys,count,spec.Offset,Stride<T>(spec)), IndexerIdx<T>(szs,count,spec.Offset,Stride<T>(spec)),count);
  return PlotBubblesEx(label_id, getter, spec);
}

#define INSTANTIATE_MACRO(T) \
    template IMPLOT_API void PlotBubbles<T>(const char* label_id, const T* values, const T* szs, int count, double xscale, double x0, const ImPlotSpec& spec); \
    template IMPLOT_API void PlotBubbles<T>(const char* label_id, const T* xs, const T* ys, const T* szs, int count, const ImPlotSpec& spec);
CALL_INSTANTIATE_FOR_NUMERIC_TYPES()
#undef INSTANTIATE_MACRO

//-----------------------------------------------------------------------------
// [SECTION] PlotStairs
//-----------------------------------------------------------------------------

template <typename Getter>
void PlotStairsEx(const char* label_id, const Getter& getter, const ImPlotSpec& spec) {
    if (BeginItemEx(label_id, Fitter1<Getter>(getter), spec, spec.LineColor, spec.Marker)) {
        if (getter.Count <= 0) {
            EndItem();
            return;
        }
        const ImPlotNextItemData& s = GetItemData();
        if (getter.Count > 1) {
            if (s.RenderFill && ImHasFlag(spec.Flags,ImPlotStairsFlags_Shaded)) {
                const ImU32 col_fill = ImGui::GetColorU32(s.Spec.FillColor);
                if (ImHasFlag(spec.Flags, ImPlotStairsFlags_PreStep))
                    RenderPrimitives1<RendererStairsPreShaded>(getter,col_fill);
                else
                    RenderPrimitives1<RendererStairsPostShaded>(getter,col_fill);
            }
            if (s.RenderLine) {
                const ImU32 col_line = ImGui::GetColorU32(s.Spec.LineColor);
                if (ImHasFlag(spec.Flags, ImPlotStairsFlags_PreStep))
                    RenderPrimitives1<RendererStairsPre>(getter,col_line,s.Spec.LineWeight);
                else
                    RenderPrimitives1<RendererStairsPost>(getter,col_line,s.Spec.LineWeight);
            }
        }
        // render markers
        if (s.RenderMarkers) {
            PopPlotClipRect();
            PushPlotClipRect(s.Spec.MarkerSize);
            const ImU32 col_line = ImGui::GetColorU32(s.Spec.MarkerLineColor);
            const ImU32 col_fill = ImGui::GetColorU32(s.Spec.MarkerFillColor);
            RenderMarkers<Getter>(getter, s.Spec.Marker, s.Spec.MarkerSize, s.RenderMarkerFill, col_fill, s.RenderMarkerLine, col_line, s.Spec.LineWeight);
        }
        EndItem();
    }
}

template <typename T>
void PlotStairs(const char* label_id, const T* values, int count, double xscale, double x0, const ImPlotSpec& spec) {
    GetterXY<IndexerLin,IndexerIdx<T>> getter(IndexerLin(xscale,x0),IndexerIdx<T>(values,count,spec.Offset,Stride<T>(spec)),count);
    PlotStairsEx(label_id, getter, spec);
}

template <typename T>
void PlotStairs(const char* label_id, const T* xs, const T* ys, int count, const ImPlotSpec& spec) {
    GetterXY<IndexerIdx<T>,IndexerIdx<T>> getter(IndexerIdx<T>(xs,count,spec.Offset,Stride<T>(spec)),IndexerIdx<T>(ys,count,spec.Offset,Stride<T>(spec)),count);
    return PlotStairsEx(label_id, getter, spec);
}

#define INSTANTIATE_MACRO(T) \
    template IMPLOT_API void PlotStairs<T> (const char* label_id, const T* values, int count, double xscale, double x0, const ImPlotSpec& spec); \
    template IMPLOT_API void PlotStairs<T>(const char* label_id, const T* xs, const T* ys, int count, const ImPlotSpec& spec);
CALL_INSTANTIATE_FOR_NUMERIC_TYPES()
#undef INSTANTIATE_MACRO

// custom
void PlotStairsG(const char* label_id, ImPlotGetter getter_func, void* data, int count, const ImPlotSpec& spec) {
    GetterFuncPtr getter(getter_func,data, count);
    return PlotStairsEx(label_id, getter, spec);
}

//-----------------------------------------------------------------------------
// [SECTION] PlotShaded
//-----------------------------------------------------------------------------

template <typename Getter1, typename Getter2>
void PlotShadedEx(const char* label_id, const Getter1& getter1, const Getter2& getter2, const ImPlotSpec& spec) {
    if (BeginItemEx(label_id, Fitter2<Getter1,Getter2>(getter1,getter2), spec, spec.FillColor)) {
        if (getter1.Count <= 0 || getter2.Count <= 0) {
            EndItem();
            return;
        }
        const ImPlotNextItemData& s = GetItemData();
        if (s.RenderFill) {
            const ImU32 col = ImGui::GetColorU32(s.Spec.FillColor);
            RenderPrimitives2<RendererShaded>(getter1,getter2,col);
        }
        EndItem();
    }
}

template <typename T>
void PlotShaded(const char* label_id, const T* values, int count, double y_ref, double xscale, double x0, const ImPlotSpec& spec) {
    if (!(y_ref > -DBL_MAX))
        y_ref = GetPlotLimits(IMPLOT_AUTO,IMPLOT_AUTO).Y.Min;
    if (!(y_ref < DBL_MAX))
        y_ref = GetPlotLimits(IMPLOT_AUTO,IMPLOT_AUTO).Y.Max;
    GetterXY<IndexerLin,IndexerIdx<T>> getter1(IndexerLin(xscale,x0),IndexerIdx<T>(values,count,spec.Offset,Stride<T>(spec)),count);
    GetterXY<IndexerLin,IndexerConst>  getter2(IndexerLin(xscale,x0),IndexerConst(y_ref),count);
    PlotShadedEx(label_id, getter1, getter2, spec);
}

template <typename T>
void PlotShaded(const char* label_id, const T* xs, const T* ys, int count, double y_ref, const ImPlotSpec& spec) {
    if (y_ref == -HUGE_VAL)
        y_ref = GetPlotLimits(IMPLOT_AUTO,IMPLOT_AUTO).Y.Min;
    if (y_ref == HUGE_VAL)
        y_ref = GetPlotLimits(IMPLOT_AUTO,IMPLOT_AUTO).Y.Max;
    GetterXY<IndexerIdx<T>,IndexerIdx<T>> getter1(IndexerIdx<T>(xs,count,spec.Offset,Stride<T>(spec)),IndexerIdx<T>(ys,count,spec.Offset,Stride<T>(spec)),count);
    GetterXY<IndexerIdx<T>,IndexerConst>  getter2(IndexerIdx<T>(xs,count,spec.Offset,Stride<T>(spec)),IndexerConst(y_ref),count);
    PlotShadedEx(label_id, getter1, getter2, spec);
}


template <typename T>
void PlotShaded(const char* label_id, const T* xs, const T* ys1, const T* ys2, int count, const ImPlotSpec& spec) {
    GetterXY<IndexerIdx<T>,IndexerIdx<T>> getter1(IndexerIdx<T>(xs,count,spec.Offset,Stride<T>(spec)),IndexerIdx<T>(ys1,count,spec.Offset,Stride<T>(spec)),count);
    GetterXY<IndexerIdx<T>,IndexerIdx<T>> getter2(IndexerIdx<T>(xs,count,spec.Offset,Stride<T>(spec)),IndexerIdx<T>(ys2,count,spec.Offset,Stride<T>(spec)),count);
    PlotShadedEx(label_id, getter1, getter2, spec);
}

#define INSTANTIATE_MACRO(T) \
    template IMPLOT_API void PlotShaded<T>(const char* label_id, const T* values, int count, double y_ref, double xscale, double x0, const ImPlotSpec& spec); \
    template IMPLOT_API void PlotShaded<T>(const char* label_id, const T* xs, const T* ys, int count, double y_ref, const ImPlotSpec& spec); \
    template IMPLOT_API void PlotShaded<T>(const char* label_id, const T* xs, const T* ys1, const T* ys2, int count, const ImPlotSpec& spec);
CALL_INSTANTIATE_FOR_NUMERIC_TYPES()
#undef INSTANTIATE_MACRO

// custom
void PlotShadedG(const char* label_id, ImPlotGetter getter_func1, void* data1, ImPlotGetter getter_func2, void* data2, int count, const ImPlotSpec& spec) {
    GetterFuncPtr getter1(getter_func1, data1, count);
    GetterFuncPtr getter2(getter_func2, data2, count);
    PlotShadedEx(label_id, getter1, getter2, spec);
}

//-----------------------------------------------------------------------------
// [SECTION] PlotBars
//-----------------------------------------------------------------------------

template <typename Getter1, typename Getter2>
void PlotBarsVEx(const char* label_id, const Getter1& getter1, const Getter2 getter2, double width, const ImPlotSpec& spec) {
    if (BeginItemEx(label_id, FitterBarV<Getter1,Getter2>(getter1,getter2,width), spec, spec.FillColor)) {
        if (getter1.Count <= 0 || getter2.Count <= 0) {
            EndItem();
            return;
        }
        const ImPlotNextItemData& s = GetItemData();
        const ImU32 col_fill = ImGui::GetColorU32(s.Spec.FillColor);
        const ImU32 col_line = ImGui::GetColorU32(s.Spec.LineColor);
        bool rend_fill = s.RenderFill;
        bool rend_line = s.RenderLine;
        if (rend_fill) {
            RenderPrimitives2<RendererBarsFillV>(getter1,getter2,col_fill,width);
            if (rend_line && col_fill == col_line)
                rend_line = false;
        }
        if (rend_line) {
            RenderPrimitives2<RendererBarsLineV>(getter1,getter2,col_line,width,s.Spec.LineWeight);
        }
        EndItem();
    }
}

template <typename Getter1, typename Getter2>
void PlotBarsHEx(const char* label_id, const Getter1& getter1, const Getter2& getter2, double height, const ImPlotSpec& spec) {
    if (BeginItemEx(label_id, FitterBarH<Getter1,Getter2>(getter1,getter2,height), spec, spec.FillColor)) {
        if (getter1.Count <= 0 || getter2.Count <= 0) {
            EndItem();
            return;
        }
        const ImPlotNextItemData& s = GetItemData();
        const ImU32 col_fill = ImGui::GetColorU32(s.Spec.FillColor);
        const ImU32 col_line = ImGui::GetColorU32(s.Spec.LineColor);
        bool rend_fill = s.RenderFill;
        bool rend_line = s.RenderLine;
        if (rend_fill) {
            RenderPrimitives2<RendererBarsFillH>(getter1,getter2,col_fill,height);
            if (rend_line && col_fill == col_line)
                rend_line = false;
        }
        if (rend_line) {
            RenderPrimitives2<RendererBarsLineH>(getter1,getter2,col_line,height,s.Spec.LineWeight);
        }
        EndItem();
    }
}

template <typename T>
void PlotBars(const char* label_id, const T* values, int count, double bar_size, double shift, const ImPlotSpec& spec) {
    if (ImHasFlag(spec.Flags, ImPlotBarsFlags_Horizontal)) {
        GetterXY<IndexerIdx<T>,IndexerLin> getter1(IndexerIdx<T>(values,count,spec.Offset,Stride<T>(spec)),IndexerLin(1.0,shift),count);
        GetterXY<IndexerConst,IndexerLin>  getter2(IndexerConst(0),IndexerLin(1.0,shift),count);
        PlotBarsHEx(label_id, getter1, getter2, bar_size, spec);
    }
    else {
        GetterXY<IndexerLin,IndexerIdx<T>> getter1(IndexerLin(1.0,shift),IndexerIdx<T>(values,count,spec.Offset,Stride<T>(spec)),count);
        GetterXY<IndexerLin,IndexerConst>  getter2(IndexerLin(1.0,shift),IndexerConst(0),count);
        PlotBarsVEx(label_id, getter1, getter2, bar_size, spec);
    }
}

template <typename T>
void PlotBars(const char* label_id, const T* xs, const T* ys, int count, double bar_size, const ImPlotSpec& spec) {
    if (ImHasFlag(spec.Flags, ImPlotBarsFlags_Horizontal)) {
        GetterXY<IndexerIdx<T>,IndexerIdx<T>> getter1(IndexerIdx<T>(xs,count,spec.Offset,Stride<T>(spec)),IndexerIdx<T>(ys,count,spec.Offset,Stride<T>(spec)),count);
        GetterXY<IndexerConst, IndexerIdx<T>> getter2(IndexerConst(0),IndexerIdx<T>(ys,count,spec.Offset,Stride<T>(spec)),count);
        PlotBarsHEx(label_id, getter1, getter2, bar_size, spec);
    }
    else {
        GetterXY<IndexerIdx<T>,IndexerIdx<T>> getter1(IndexerIdx<T>(xs,count,spec.Offset,Stride<T>(spec)),IndexerIdx<T>(ys,count,spec.Offset,Stride<T>(spec)),count);
        GetterXY<IndexerIdx<T>,IndexerConst>  getter2(IndexerIdx<T>(xs,count,spec.Offset,Stride<T>(spec)),IndexerConst(0),count);
        PlotBarsVEx(label_id, getter1, getter2, bar_size, spec);
    }
}

#define INSTANTIATE_MACRO(T) \
    template IMPLOT_API void PlotBars<T>(const char* label_id, const T* values, int count, double bar_size, double shift, const ImPlotSpec& spec); \
    template IMPLOT_API void PlotBars<T>(const char* label_id, const T* xs, const T* ys, int count, double bar_size, const ImPlotSpec& spec);
CALL_INSTANTIATE_FOR_NUMERIC_TYPES()
#undef INSTANTIATE_MACRO

void PlotBarsG(const char* label_id, ImPlotGetter getter_func, void* data, int count, double bar_size, const ImPlotSpec& spec) {
    if (ImHasFlag(spec.Flags, ImPlotBarsFlags_Horizontal)) {
        GetterFuncPtr getter1(getter_func, data, count);
        GetterOverrideX<GetterFuncPtr> getter2(getter1,0);
        PlotBarsHEx(label_id, getter1, getter2, bar_size, spec);
    }
    else {
        GetterFuncPtr getter1(getter_func, data, count);
        GetterOverrideY<GetterFuncPtr> getter2(getter1,0);
        PlotBarsVEx(label_id, getter1, getter2, bar_size, spec);
    }
}

//-----------------------------------------------------------------------------
// [SECTION] PlotBarGroups
//-----------------------------------------------------------------------------

template <typename T>
void PlotBarGroups(const char* const label_ids[], const T* values, int item_count, int group_count, double group_size, double shift, const ImPlotSpec& spec) {
    IndexerIdx<T> indexer(values,item_count*group_count,spec.Offset,Stride<T>(spec));
    const bool horz = ImHasFlag(spec.Flags, ImPlotBarGroupsFlags_Horizontal);
    const bool stack = ImHasFlag(spec.Flags, ImPlotBarGroupsFlags_Stacked);
    ImPlotSpec spec_bars = spec;
    spec_bars.Flags = 0;
    if (stack) {
        SetupLock();
        ImPlotContext& gp = *GImPlot;
        gp.TempDouble1.resize(4*group_count);
        double* temp = gp.TempDouble1.Data;
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
                        double v = indexer[i*group_count+g];
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
                GetterXY<IndexerIdx<double>,IndexerLin> getter1(IndexerIdx<double>(curr_min,group_count),IndexerLin(1.0,shift),group_count);
                GetterXY<IndexerIdx<double>,IndexerLin> getter2(IndexerIdx<double>(curr_max,group_count),IndexerLin(1.0,shift),group_count);
                PlotBarsHEx(label_ids[i],getter1,getter2,group_size,spec_bars);
            }
        }
        else {
            for (int i = 0; i < item_count; ++i) {
                if (!IsItemHidden(label_ids[i])) {
                    for (int g = 0; g < group_count; ++g) {
                        double v = indexer[i*group_count+g];
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
                GetterXY<IndexerLin,IndexerIdx<double>> getter1(IndexerLin(1.0,shift),IndexerIdx<double>(curr_min,group_count),group_count);
                GetterXY<IndexerLin,IndexerIdx<double>> getter2(IndexerLin(1.0,shift),IndexerIdx<double>(curr_max,group_count),group_count);
                PlotBarsVEx(label_ids[i],getter1,getter2,group_size,spec_bars);
            }
        }
    }
    else {
        const double subsize = group_size / item_count;
        if (horz) {
            spec_bars.Flags = ImPlotBarsFlags_Horizontal;
            for (int i = 0; i < item_count; ++i) {
                const double subshift = (i+0.5)*subsize - group_size/2;
                PlotBars(label_ids[i],&values[i*group_count],group_count,subsize,subshift+shift,spec_bars);
            }
        }
        else {
            for (int i = 0; i < item_count; ++i) {
                const double subshift = (i+0.5)*subsize - group_size/2;
                PlotBars(label_ids[i],&values[i*group_count],group_count,subsize,subshift+shift,spec_bars);
            }
        }
    }
}

#define INSTANTIATE_MACRO(T) template IMPLOT_API void PlotBarGroups<T>(const char* const label_ids[], const T* values, int items, int groups, double width, double shift, const ImPlotSpec& spec);
CALL_INSTANTIATE_FOR_NUMERIC_TYPES()
#undef INSTANTIATE_MACRO

//-----------------------------------------------------------------------------
// [SECTION] PlotErrorBars
//-----------------------------------------------------------------------------

template <typename _GetterPos, typename _GetterNeg>
void PlotErrorBarsVEx(const char* label_id, const _GetterPos& getter_pos, const _GetterNeg& getter_neg, const ImPlotSpec& spec) {
    if (BeginItemEx(label_id, Fitter2<_GetterPos,_GetterNeg>(getter_pos, getter_neg), spec, IMPLOT_AUTO_COL)) {
        if (getter_pos.Count <= 0 || getter_neg.Count <= 0) {
            EndItem();
            return;
        }
        const ImPlotNextItemData& s = GetItemData();
        ImDrawList& draw_list = *GetPlotDrawList();
        const ImU32 col = ImGui::GetColorU32( IsColorAuto(spec.LineColor) ? ImGui::GetStyleColorVec4(ImGuiCol_Text) : s.Spec.LineColor );
        const bool rend_whisker  = s.Spec.Size > 0;
        const float half_whisker = s.Spec.Size * 0.5f;
        for (int i = 0; i < getter_pos.Count; ++i) {
            ImVec2 p1 = PlotToPixels(getter_neg[i],IMPLOT_AUTO,IMPLOT_AUTO);
            ImVec2 p2 = PlotToPixels(getter_pos[i],IMPLOT_AUTO,IMPLOT_AUTO);
            draw_list.AddLine(p1,p2,col, s.Spec.LineWeight);
            if (rend_whisker) {
                draw_list.AddLine(p1 - ImVec2(half_whisker, 0), p1 + ImVec2(half_whisker, 0), col, s.Spec.LineWeight);
                draw_list.AddLine(p2 - ImVec2(half_whisker, 0), p2 + ImVec2(half_whisker, 0), col, s.Spec.LineWeight);
            }
        }
        EndItem();
    }
}

template <typename _GetterPos, typename _GetterNeg>
void PlotErrorBarsHEx(const char* label_id, const _GetterPos& getter_pos, const _GetterNeg& getter_neg, const ImPlotSpec& spec) {
    if (BeginItemEx(label_id, Fitter2<_GetterPos,_GetterNeg>(getter_pos, getter_neg), spec, IMPLOT_AUTO_COL)) {
        if (getter_pos.Count <= 0 || getter_neg.Count <= 0) {
            EndItem();
            return;
        }
        const ImPlotNextItemData& s = GetItemData();
        ImDrawList& draw_list = *GetPlotDrawList();
        const ImU32 col = ImGui::GetColorU32( IsColorAuto(spec.LineColor) ? ImGui::GetStyleColorVec4(ImGuiCol_Text) : s.Spec.LineColor );
        const bool rend_whisker  = s.Spec.Size > 0;
        const float half_whisker = s.Spec.Size * 0.5f;
        for (int i = 0; i < getter_pos.Count; ++i) {
            ImVec2 p1 = PlotToPixels(getter_neg[i],IMPLOT_AUTO,IMPLOT_AUTO);
            ImVec2 p2 = PlotToPixels(getter_pos[i],IMPLOT_AUTO,IMPLOT_AUTO);
            draw_list.AddLine(p1, p2, col, s.Spec.LineWeight);
            if (rend_whisker) {
                draw_list.AddLine(p1 - ImVec2(0, half_whisker), p1 + ImVec2(0, half_whisker), col, s.Spec.LineWeight);
                draw_list.AddLine(p2 - ImVec2(0, half_whisker), p2 + ImVec2(0, half_whisker), col, s.Spec.LineWeight);
            }
        }
        EndItem();
    }
}

template <typename T>
void PlotErrorBars(const char* label_id, const T* xs, const T* ys, const T* err, int count, const ImPlotSpec& spec) {
    PlotErrorBars(label_id, xs, ys, err, err, count, spec);
}

template <typename T>
void PlotErrorBars(const char* label_id, const T* xs, const T* ys, const T* neg, const T* pos, int count, const ImPlotSpec& spec) {
    IndexerIdx<T> indexer_x(xs, count,spec.Offset,Stride<T>(spec));
    IndexerIdx<T> indexer_y(ys, count,spec.Offset,Stride<T>(spec));
    IndexerIdx<T> indexer_n(neg,count,spec.Offset,Stride<T>(spec));
    IndexerIdx<T> indexer_p(pos,count,spec.Offset,Stride<T>(spec));
    GetterError<T> getter(xs, ys, neg, pos, count, spec.Offset, Stride<T>(spec));
    if (ImHasFlag(spec.Flags, ImPlotErrorBarsFlags_Horizontal)) {
        IndexerAdd<IndexerIdx<T>,IndexerIdx<T>> indexer_xp(indexer_x, indexer_p, 1,  1);
        IndexerAdd<IndexerIdx<T>,IndexerIdx<T>> indexer_xn(indexer_x, indexer_n, 1, -1);
        GetterXY<IndexerAdd<IndexerIdx<T>,IndexerIdx<T>>,IndexerIdx<T>> getter_p(indexer_xp, indexer_y, count);
        GetterXY<IndexerAdd<IndexerIdx<T>,IndexerIdx<T>>,IndexerIdx<T>> getter_n(indexer_xn, indexer_y, count);
        PlotErrorBarsHEx(label_id, getter_p, getter_n, spec);
    }
    else {
        IndexerAdd<IndexerIdx<T>,IndexerIdx<T>> indexer_yp(indexer_y, indexer_p, 1,  1);
        IndexerAdd<IndexerIdx<T>,IndexerIdx<T>> indexer_yn(indexer_y, indexer_n, 1, -1);
        GetterXY<IndexerIdx<T>,IndexerAdd<IndexerIdx<T>,IndexerIdx<T>>> getter_p(indexer_x, indexer_yp, count);
        GetterXY<IndexerIdx<T>,IndexerAdd<IndexerIdx<T>,IndexerIdx<T>>> getter_n(indexer_x, indexer_yn, count);
        PlotErrorBarsVEx(label_id, getter_p, getter_n, spec);
    }
}

#define INSTANTIATE_MACRO(T) \
    template IMPLOT_API void PlotErrorBars<T>(const char* label_id, const T* xs, const T* ys, const T* err, int count, const ImPlotSpec& spec); \
    template IMPLOT_API void PlotErrorBars<T>(const char* label_id, const T* xs, const T* ys, const T* neg, const T* pos, int count, const ImPlotSpec& spec);
CALL_INSTANTIATE_FOR_NUMERIC_TYPES()
#undef INSTANTIATE_MACRO

//-----------------------------------------------------------------------------
// [SECTION] PlotStems
//-----------------------------------------------------------------------------

template <typename _GetterM, typename _GetterB>
void PlotStemsEx(const char* label_id, const _GetterM& getter_mark, const _GetterB& getter_base, const ImPlotSpec& spec) {
    if (BeginItemEx(label_id, Fitter2<_GetterM,_GetterB>(getter_mark,getter_base), spec, spec.LineColor, spec.Marker)) {
        if (getter_mark.Count <= 0 || getter_base.Count <= 0) {
            EndItem();
            return;
        }
        const ImPlotNextItemData& s = GetItemData();
        // render stems
        if (s.RenderLine) {
            const ImU32 col_line = ImGui::GetColorU32(s.Spec.LineColor);
            RenderPrimitives2<RendererLineSegments2>(getter_mark, getter_base, col_line, s.Spec.LineWeight);
        }
        // render markers
        if (s.RenderMarkers) {
            PopPlotClipRect();
            PushPlotClipRect(s.Spec.MarkerSize);
            const ImU32 col_line = ImGui::GetColorU32(s.Spec.MarkerLineColor);
            const ImU32 col_fill = ImGui::GetColorU32(s.Spec.MarkerFillColor);
            RenderMarkers<_GetterM>(getter_mark, s.Spec.Marker, s.Spec.MarkerSize, s.RenderMarkerFill, col_fill, s.RenderMarkerLine, col_line, s.Spec.LineWeight);
        }
        EndItem();
    }
}

template <typename T>
void PlotStems(const char* label_id, const T* values, int count, double ref, double scale, double start, const ImPlotSpec& spec) {
    if (ImHasFlag(spec.Flags, ImPlotStemsFlags_Horizontal)) {
        GetterXY<IndexerIdx<T>,IndexerLin> get_mark(IndexerIdx<T>(values,count,spec.Offset,Stride<T>(spec)),IndexerLin(scale,start),count);
        GetterXY<IndexerConst,IndexerLin>  get_base(IndexerConst(ref),IndexerLin(scale,start),count);
        PlotStemsEx(label_id, get_mark, get_base, spec);
    }
    else {
        GetterXY<IndexerLin,IndexerIdx<T>> get_mark(IndexerLin(scale,start),IndexerIdx<T>(values,count,spec.Offset,Stride<T>(spec)),count);
        GetterXY<IndexerLin,IndexerConst>  get_base(IndexerLin(scale,start),IndexerConst(ref),count);
        PlotStemsEx(label_id, get_mark, get_base, spec);
    }
}

template <typename T>
void PlotStems(const char* label_id, const T* xs, const T* ys, int count, double ref, const ImPlotSpec& spec) {
    if (ImHasFlag(spec.Flags, ImPlotStemsFlags_Horizontal)) {
        GetterXY<IndexerIdx<T>,IndexerIdx<T>> get_mark(IndexerIdx<T>(xs,count,spec.Offset,Stride<T>(spec)),IndexerIdx<T>(ys,count,spec.Offset,Stride<T>(spec)),count);
        GetterXY<IndexerConst,IndexerIdx<T>>  get_base(IndexerConst(ref),IndexerIdx<T>(ys,count,spec.Offset,Stride<T>(spec)),count);
        PlotStemsEx(label_id, get_mark, get_base, spec);
    }
    else {
        GetterXY<IndexerIdx<T>,IndexerIdx<T>> get_mark(IndexerIdx<T>(xs,count,spec.Offset,Stride<T>(spec)),IndexerIdx<T>(ys,count,spec.Offset,Stride<T>(spec)),count);
        GetterXY<IndexerIdx<T>,IndexerConst>  get_base(IndexerIdx<T>(xs,count,spec.Offset,Stride<T>(spec)),IndexerConst(ref),count);
        PlotStemsEx(label_id, get_mark, get_base, spec);
    }
}

#define INSTANTIATE_MACRO(T) \
    template IMPLOT_API void PlotStems<T>(const char* label_id, const T* values, int count, double ref, double scale, double start, const ImPlotSpec& spec); \
    template IMPLOT_API void PlotStems<T>(const char* label_id, const T* xs, const T* ys, int count, double ref, const ImPlotSpec& spec);
CALL_INSTANTIATE_FOR_NUMERIC_TYPES()
#undef INSTANTIATE_MACRO


//-----------------------------------------------------------------------------
// [SECTION] PlotInfLines
//-----------------------------------------------------------------------------

template <typename T>
void PlotInfLines(const char* label_id, const T* values, int count, const ImPlotSpec& spec) {
    const ImPlotRect lims = GetPlotLimits(IMPLOT_AUTO,IMPLOT_AUTO);
    if (ImHasFlag(spec.Flags, ImPlotInfLinesFlags_Horizontal)) {
        GetterXY<IndexerConst,IndexerIdx<T>> getter_min(IndexerConst(lims.X.Min),IndexerIdx<T>(values,count,spec.Offset,Stride<T>(spec)),count);
        GetterXY<IndexerConst,IndexerIdx<T>> getter_max(IndexerConst(lims.X.Max),IndexerIdx<T>(values,count,spec.Offset,Stride<T>(spec)),count);
        if (BeginItemEx(label_id, FitterY<GetterXY<IndexerConst,IndexerIdx<T>>>(getter_min), spec, spec.LineColor)) {
            if (count <= 0) {
                EndItem();
                return;
            }
            const ImPlotNextItemData& s = GetItemData();
            const ImU32 col_line = ImGui::GetColorU32(s.Spec.LineColor);
            if (s.RenderLine)
                RenderPrimitives2<RendererLineSegments2>(getter_min, getter_max, col_line, s.Spec.LineWeight);
            EndItem();
        }
    }
    else {
        GetterXY<IndexerIdx<T>,IndexerConst> get_min(IndexerIdx<T>(values,count,spec.Offset,Stride<T>(spec)),IndexerConst(lims.Y.Min),count);
        GetterXY<IndexerIdx<T>,IndexerConst> get_max(IndexerIdx<T>(values,count,spec.Offset,Stride<T>(spec)),IndexerConst(lims.Y.Max),count);
        if (BeginItemEx(label_id, FitterX<GetterXY<IndexerIdx<T>,IndexerConst>>(get_min), spec, spec.LineColor)) {
            if (count <= 0) {
                EndItem();
                return;
            }
            const ImPlotNextItemData& s = GetItemData();
            const ImU32 col_line = ImGui::GetColorU32(s.Spec.LineColor);
            if (s.RenderLine)
                RenderPrimitives2<RendererLineSegments2>(get_min, get_max, col_line, s.Spec.LineWeight);
            EndItem();
        }
    }
}
#define INSTANTIATE_MACRO(T) template IMPLOT_API void PlotInfLines<T>(const char* label_id, const T* xs, int count, const ImPlotSpec& spec);
CALL_INSTANTIATE_FOR_NUMERIC_TYPES()
#undef INSTANTIATE_MACRO

//-----------------------------------------------------------------------------
// [SECTION] PlotPieChart
//-----------------------------------------------------------------------------

IMPLOT_INLINE void RenderPieSlice(ImDrawList& draw_list, const ImPlotPoint& center, double radius, double a0, double a1, ImU32 col, bool detached = false) {
    const float resolution = 50 / (2 * IM_PI);
    ImVec2 buffer[52];

    int n = ImMax(3, (int)((a1 - a0) * resolution));
    double da = (a1 - a0) / (n - 1);
    int i = 0;

    if (detached) {
        const double offset = 0.08; // Offset of the detached slice
        const double width_scale = 0.95; // Scale factor for the width of the detached slice

        double a_mid = (a0 + a1) / 2;
        double new_a0 = a_mid - (a1 - a0) * width_scale / 2;
        double new_a1 = a_mid + (a1 - a0) * width_scale / 2;
        double new_da = (new_a1 - new_a0) / (n - 1);

        ImPlotPoint offsetCenter(center.x + offset * cos(a_mid), center.y + offset * sin(a_mid));

        // Start point (center of the offset)
        buffer[0] = PlotToPixels(offsetCenter, IMPLOT_AUTO, IMPLOT_AUTO);

        for (; i < n; ++i) {
            double a = new_a0 + i * new_da;
            buffer[i + 1] = PlotToPixels(
                offsetCenter.x + (radius + offset/2) * cos(a),
                offsetCenter.y + (radius + offset/2) * sin(a),
                IMPLOT_AUTO, IMPLOT_AUTO
            );
        }

    } else {
        buffer[0] = PlotToPixels(center, IMPLOT_AUTO, IMPLOT_AUTO);
        for (; i < n; ++i) {
            double a = a0 + i * da;
            buffer[i + 1] = PlotToPixels(
                center.x + radius * cos(a),
                center.y + radius * sin(a),
                IMPLOT_AUTO, IMPLOT_AUTO);
        }
    }
    // Close the shape
    buffer[i + 1] = buffer[0];

    // fill
    draw_list.AddConvexPolyFilled(buffer, n + 2, col);

    // border (for AA)
    draw_list.AddPolyline(buffer, n + 2, col, 0, 2.0f);
}

template <typename T>
double PieChartSum(IndexerIdx<T> indexer, bool ignore_hidden) {
    double sum = 0;
    if (ignore_hidden) {
        ImPlotContext& gp = *GImPlot;
        ImPlotItemGroup& Items = *gp.CurrentItems;
        for (int i = 0; i < indexer.Count; ++i) {
            if (i >= Items.GetItemCount())
                break;

            ImPlotItem* item = Items.GetItemByIndex(i);
            IM_ASSERT(item != nullptr);
            if (item->Show) {
                sum += (double)indexer[i];
            }
        }
    }
    else {
        for (int i = 0; i < indexer.Count; ++i) {
            sum += (double)indexer[i];
        }
    }
    return sum;
}

template <typename T>
void PlotPieChartEx(const char* const label_ids[], IndexerIdx<T> indexer, ImPlotPoint center, double radius, double angle0, const ImPlotSpec& spec) {
    ImDrawList& draw_list  = *GetPlotDrawList();

    const bool ignore_hidden = ImHasFlag(spec.Flags, ImPlotPieChartFlags_IgnoreHidden);
    const double sum         = PieChartSum(indexer, ignore_hidden);
    const bool normalize     = ImHasFlag(spec.Flags, ImPlotPieChartFlags_Normalize) || sum > 1.0;

    double a0 = angle0 * 2 * IM_PI / 360.0;
    double a1 = angle0 * 2 * IM_PI / 360.0;
    ImPlotPoint Pmin = ImPlotPoint(center.x - radius, center.y - radius);
    ImPlotPoint Pmax = ImPlotPoint(center.x + radius, center.y + radius);
    for (int i = 0; i < indexer.Count; ++i) {
        ImPlotItem* item = GetItem(label_ids[i]);

        const double percent = normalize ? (double)indexer[i] / sum : (double)indexer[i];
        const bool skip      = sum <= 0.0 || (ignore_hidden && item != nullptr && !item->Show);
        if (!skip)
            a1 = a0 + 2 * IM_PI * percent;

        if (BeginItemEx(label_ids[i], FitterRect(Pmin, Pmax), spec)) {
            const bool hovered = ImPlot::IsLegendEntryHovered(label_ids[i]) && ImHasFlag(spec.Flags, ImPlotPieChartFlags_Exploding);
            if (sum > 0.0) {
                ImU32 col = GetCurrentItem()->Color;
                if (percent < 0.5) {
                    RenderPieSlice(draw_list, center, radius, a0, a1, col, hovered);
                }
                else {
                    RenderPieSlice(draw_list, center, radius, a0, a0 + (a1 - a0) * 0.5, col, hovered);
                    RenderPieSlice(draw_list, center, radius, a0 + (a1 - a0) * 0.5, a1, col, hovered);
                }
            }
            EndItem();
        }
        if (!skip)
            a0 = a1;
    }
}

int PieChartFormatter(double value, char* buff, int size, void* data) {
    const char* fmt = (const char*)data;
    return snprintf(buff, size, fmt, value);
}

template <typename T>
void PlotPieChart(const char* const label_ids[], const T* values, int count, double x, double y, double radius, const char* fmt, double angle0, const ImPlotSpec& spec) {
    PlotPieChart<T>(label_ids, values, count, x, y, radius, PieChartFormatter, (void*)fmt, angle0, spec);
}
#define INSTANTIATE_MACRO(T) template IMPLOT_API void PlotPieChart<T>(const char* const label_ids[], const T* values, int count, double x, double y, double radius, const char* fmt, double angle0, const ImPlotSpec& spec);
CALL_INSTANTIATE_FOR_NUMERIC_TYPES()
#undef INSTANTIATE_MACRO

template <typename T>
void PlotPieChart(const char* const label_ids[], const T* values, int count, double x, double y, double radius, ImPlotFormatter fmt, void* fmt_data, double angle0, const ImPlotSpec& spec) {
    IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot != nullptr, "PlotPieChart() needs to be called between BeginPlot() and EndPlot()!");
    ImDrawList& draw_list = *GetPlotDrawList();

    IndexerIdx<T> indexer(values,count,spec.Offset,Stride<T>(spec));

    const bool ignore_hidden = ImHasFlag(spec.Flags, ImPlotPieChartFlags_IgnoreHidden);
    const double sum = PieChartSum(indexer, ignore_hidden);
    const bool normalize = ImHasFlag(spec.Flags, ImPlotPieChartFlags_Normalize) || sum > 1.0;
    ImPlotPoint center(x, y);

    PushPlotClipRect();
    PlotPieChartEx(label_ids, indexer, center, radius, angle0, spec);
    if (fmt != nullptr) {
        double a0 = angle0 * 2 * IM_PI / 360.0;
        double a1 = angle0 * 2 * IM_PI / 360.0;
        char buffer[32];
        for (int i = 0; i < count; ++i) {
            ImPlotItem* item = GetItem(label_ids[i]);
            IM_ASSERT(item != nullptr);

            const double percent = normalize ? (double)indexer[i] / sum : (double)indexer[i];
            const bool skip = ignore_hidden && item != nullptr && !item->Show;

            if (!skip) {
                a1 = a0 + 2 * IM_PI * percent;
                if (item->Show) {
                    fmt((double)indexer[i], buffer, 32, fmt_data);
                    ImVec2 size = ImGui::CalcTextSize(buffer);
                    double angle = a0 + (a1 - a0) * 0.5;
                    const bool hovered = ImPlot::IsLegendEntryHovered(label_ids[i]) && ImHasFlag(spec.Flags, ImPlotPieChartFlags_Exploding);
                    const double offset = (hovered ? 0.6 : 0.5) * radius;
                    ImVec2 pos = PlotToPixels(center.x + offset * cos(angle), center.y + offset * sin(angle), IMPLOT_AUTO, IMPLOT_AUTO);
                    ImU32 col = CalcTextColor(ImGui::ColorConvertU32ToFloat4(item->Color));
                    draw_list.AddText(pos - size * 0.5f, col, buffer);
                }
                a0 = a1;
            }
        }
    }
    PopPlotClipRect();
}
#define INSTANTIATE_MACRO(T) template IMPLOT_API void PlotPieChart(const char* const label_ids[], const T* values, int count, double x, double y, double radius, ImPlotFormatter fmt, void* fmt_data, double angle0, const ImPlotSpec& spec);
CALL_INSTANTIATE_FOR_NUMERIC_TYPES()
#undef INSTANTIATE_MACRO

//-----------------------------------------------------------------------------
// [SECTION] PlotHeatmap
//-----------------------------------------------------------------------------

template <typename _Indexer>
struct GetterHeatmapRowMaj {
    GetterHeatmapRowMaj(_Indexer indexer, int rows, int cols, double scale_min, double scale_max, double width, double height, double xref, double yref, double ydir) :
        Indexer(indexer),
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
    template <typename I> IMPLOT_INLINE RectC operator[](I idx) const {
        double val = (double)Indexer[idx];
        const int r = idx / Cols;
        const int c = idx % Cols;
        const ImPlotPoint p(XRef + HalfSize.x + c*Width, YRef + YDir * (HalfSize.y + r*Height));
        RectC rect;
        rect.Pos = p;
        rect.HalfSize = HalfSize;
        const float t = ImClamp((float)ImRemap01(val, ScaleMin, ScaleMax),0.0f,1.0f);
        ImPlotContext& gp = *GImPlot;
        rect.Color = gp.ColormapData.LerpTable(gp.Style.Colormap, t);
        return rect;
    }
    const _Indexer Indexer;
    const int Count, Rows, Cols;
    const double ScaleMin, ScaleMax, Width, Height, XRef, YRef, YDir;
    const ImPlotPoint HalfSize;
    typedef RectC value_type;
};

template <typename _Indexer>
struct GetterHeatmapColMaj {
    GetterHeatmapColMaj(_Indexer indexer, int rows, int cols, double scale_min, double scale_max, double width, double height, double xref, double yref, double ydir) :
        Indexer(indexer),
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
    template <typename I> IMPLOT_INLINE RectC operator[](I idx) const {
        double val = (double)Indexer[idx];
        const int r = idx % Rows;
        const int c = idx / Rows;
        const ImPlotPoint p(XRef + HalfSize.x + c*Width, YRef + YDir * (HalfSize.y + r*Height));
        RectC rect;
        rect.Pos = p;
        rect.HalfSize = HalfSize;
        const float t = ImClamp((float)ImRemap01(val, ScaleMin, ScaleMax),0.0f,1.0f);
        ImPlotContext& gp = *GImPlot;
        rect.Color = gp.ColormapData.LerpTable(gp.Style.Colormap, t);
        return rect;
    }
    // const T* const Values;
    const _Indexer Indexer;
    const int Count, Rows, Cols;
    const double ScaleMin, ScaleMax, Width, Height, XRef, YRef, YDir;
    const ImPlotPoint HalfSize;
    typedef RectC value_type;
};

template <typename T>
void RenderHeatmap(ImDrawList& draw_list, IndexerIdx<T> indexer, int rows, int cols, double scale_min, double scale_max, const char* fmt, const ImPlotPoint& bounds_min, const ImPlotPoint& bounds_max, bool reverse_y, bool col_maj) {
    ImPlotContext& gp = *GImPlot;
    Transformer2 transformer;
    if (scale_min == 0 && scale_max == 0) {
        ImMinMaxIndexer(indexer,rows*cols,&scale_min,&scale_max);
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
        GetterHeatmapColMaj<IndexerIdx<T>> getter(indexer, rows, cols, scale_min, scale_max, (bounds_max.x - bounds_min.x) / cols, (bounds_max.y - bounds_min.y) / rows, bounds_min.x, yref, ydir);
        RenderPrimitives1<RendererRectC>(getter);
    }
    else {
        GetterHeatmapRowMaj<IndexerIdx<T>> getter(indexer, rows, cols, scale_min, scale_max, (bounds_max.x - bounds_min.x) / cols, (bounds_max.y - bounds_min.y) / rows, bounds_min.x, yref, ydir);
        RenderPrimitives1<RendererRectC>(getter);
    }
    // labels
    if (fmt != nullptr) {
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
                    ImFormatString(buff, 32, fmt, indexer[i]);
                    ImVec2 size = ImGui::CalcTextSize(buff);
                    double t = ImClamp(ImRemap01((double)indexer[i], scale_min, scale_max),0.0,1.0);
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
                    ImFormatString(buff, 32, fmt, indexer[i]);
                    ImVec2 size = ImGui::CalcTextSize(buff);
                    double t = ImClamp(ImRemap01((double)indexer[i], scale_min, scale_max),0.0,1.0);
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
void PlotHeatmap(const char* label_id, const T* values, int rows, int cols, double scale_min, double scale_max, const char* fmt, const ImPlotPoint& bounds_min, const ImPlotPoint& bounds_max, const ImPlotSpec& spec) {
    if (BeginItemEx(label_id, FitterRect(bounds_min, bounds_max), spec)) {
        if (rows <= 0 || cols <= 0) {
            EndItem();
            return;
        }
        ImDrawList& draw_list = *GetPlotDrawList();
        const bool col_maj = ImHasFlag(spec.Flags, ImPlotHeatmapFlags_ColMajor);
        IndexerIdx<T> indexer(values,rows*cols,spec.Offset,Stride<T>(spec));
        RenderHeatmap(draw_list, indexer, rows, cols, scale_min, scale_max, fmt, bounds_min, bounds_max, true, col_maj);
        EndItem();
    }
}
#define INSTANTIATE_MACRO(T) template IMPLOT_API void PlotHeatmap<T>(const char* label_id, const T* values, int rows, int cols, double scale_min, double scale_max, const char* fmt, const ImPlotPoint& bounds_min, const ImPlotPoint& bounds_max, const ImPlotSpec& spec);
CALL_INSTANTIATE_FOR_NUMERIC_TYPES()
#undef INSTANTIATE_MACRO

//-----------------------------------------------------------------------------
// [SECTION] PlotHistogram
//-----------------------------------------------------------------------------

template <typename T>
double PlotHistogram(const char* label_id, const T* values, int count, int bins, double bar_scale, ImPlotRange range, const ImPlotSpec& spec) {

    const bool cumulative = ImHasFlag(spec.Flags, ImPlotHistogramFlags_Cumulative);
    const bool density    = ImHasFlag(spec.Flags, ImPlotHistogramFlags_Density);
    const bool outliers   = !ImHasFlag(spec.Flags, ImPlotHistogramFlags_NoOutliers);

    IndexerIdx<T> indexer(values,count,spec.Offset,Stride<T>(spec));

    if (count <= 0 || bins == 0)
        return 0;

    if (range.Min == 0 && range.Max == 0) {
        ImMinMaxIndexer(indexer, count, &range.Min, &range.Max);
    }

    double width;
    if (bins < 0)
        CalculateBins(indexer, count, bins, range, bins, width);
    else
        width = range.Size() / bins;

    ImPlotContext& gp = *GImPlot;
    ImVector<double>& bin_centers = gp.TempDouble1;
    ImVector<double>& bin_counts  = gp.TempDouble2;
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
        double val = indexer[i];
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
    ImPlotSpec spec_bars = spec;
    if (ImHasFlag(spec.Flags, ImPlotHistogramFlags_Horizontal)) {
        spec_bars.Flags = ImPlotBarsFlags_Horizontal;
        PlotBars(label_id, &bin_counts.Data[0], &bin_centers.Data[0], bins, bar_scale*width, spec_bars);
    }
    else {
        spec_bars.Flags = 0;
        PlotBars(label_id, &bin_centers.Data[0], &bin_counts.Data[0], bins, bar_scale*width, spec_bars);
    }
    return max_count;
}
#define INSTANTIATE_MACRO(T) template IMPLOT_API double PlotHistogram<T>(const char* label_id, const T* values, int count, int bins, double bar_scale, ImPlotRange range, const ImPlotSpec& spec);
CALL_INSTANTIATE_FOR_NUMERIC_TYPES()
#undef INSTANTIATE_MACRO

//-----------------------------------------------------------------------------
// [SECTION] PlotHistogram2D
//-----------------------------------------------------------------------------

template <typename T>
double PlotHistogram2D(const char* label_id, const T* xs, const T* ys, int count, int x_bins, int y_bins, ImPlotRect range, const ImPlotSpec& spec) {

    // const bool cumulative = ImHasFlag(flags, ImPlotHistogramFlags_Cumulative); NOT SUPPORTED
    const bool density  = ImHasFlag(spec.Flags, ImPlotHistogramFlags_Density);
    const bool outliers = !ImHasFlag(spec.Flags, ImPlotHistogramFlags_NoOutliers);
    const bool col_maj  = ImHasFlag(spec.Flags, ImPlotHistogramFlags_ColMajor);

    IndexerIdx<T> indexer_x(xs,count,spec.Offset,Stride<T>(spec));
    IndexerIdx<T> indexer_y(ys,count,spec.Offset,Stride<T>(spec));

    if (count <= 0 || x_bins == 0 || y_bins == 0)
        return 0;

    if (range.X.Min == 0 && range.X.Max == 0) {
        ImMinMaxIndexer(indexer_x, count, &range.X.Min, &range.X.Min);
    }
    if (range.Y.Min == 0 && range.Y.Max == 0) {
        ImMinMaxIndexer(indexer_y, count, &range.Y.Min, &range.Y.Max);
    }

    double width, height;
    if (x_bins < 0)
        CalculateBins(indexer_x, count, x_bins, range.X, x_bins, width);
    else
        width = range.X.Size() / x_bins;
    if (y_bins < 0)
        CalculateBins(indexer_y, count, y_bins, range.Y, y_bins, height);
    else
        height = range.Y.Size() / y_bins;

    const int bins = x_bins * y_bins;

    ImPlotContext& gp = *GImPlot;
    ImVector<double>& bin_counts = gp.TempDouble1;
    bin_counts.resize(bins);

    for (int b = 0; b < bins; ++b)
        bin_counts[b] = 0;

    int counted = 0;
    double max_count = 0;
    for (int i = 0; i < count; ++i) {
        if (range.Contains(indexer_x[i], indexer_y[i])) {
            const int xb = ImClamp( (int)((indexer_x[i] - range.X.Min) / width)  , 0, x_bins - 1);
            const int yb = ImClamp( (int)((indexer_y[i] - range.Y.Min) / height) , 0, y_bins - 1);
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

    if (BeginItemEx(label_id, FitterRect(range), spec)) {
        if (y_bins <= 0 || x_bins <= 0) {
            EndItem();
            return max_count;
        }
        ImDrawList& draw_list = *GetPlotDrawList();
        IndexerIdx<double> indexer_bin(bin_counts.begin(), y_bins*x_bins, 0, sizeof(double));
        RenderHeatmap(draw_list, indexer_bin, y_bins, x_bins, 0, max_count, nullptr, range.Min(), range.Max(), false, col_maj);
        EndItem();
    }
    return max_count;
}
#define INSTANTIATE_MACRO(T) template IMPLOT_API double PlotHistogram2D<T>(const char* label_id,   const T*   xs, const T*   ys, int count, int x_bins, int y_bins, ImPlotRect range, const ImPlotSpec& spec);
CALL_INSTANTIATE_FOR_NUMERIC_TYPES()
#undef INSTANTIATE_MACRO

//-----------------------------------------------------------------------------
// [SECTION] PlotDigital
//-----------------------------------------------------------------------------

// TODO: Make this behave like all the other plot types (.e. not fixed in y axis)
// TODO: Currently broken if x or y axis is inverted! (what should happen in this case, anyway?)

template <typename Getter>
void PlotDigitalEx(const char* label_id, Getter getter, const ImPlotSpec& spec) {
    if (BeginItem(label_id, spec, spec.FillColor)) {
        ImPlotContext& gp = *GImPlot;
        ImDrawList& draw_list = *GetPlotDrawList();
        const ImPlotNextItemData& s = GetItemData();
        if (getter.Count > 1 && s.RenderFill) {
            ImPlotPlot& plot   = *gp.CurrentPlot;
            ImPlotAxis& x_axis = plot.Axes[plot.CurrentX];
            ImPlotAxis& y_axis = plot.Axes[plot.CurrentY];

            int pixYMax = 0;
            ImPlotPoint itemData1 = getter[0];
            for (int i = 0; i < getter.Count; ++i) {
                ImPlotPoint itemData2 = getter[i];
                if (ImNanOrInf(itemData1.y)) {
                    itemData1 = itemData2;
                    continue;
                }
                if (ImNanOrInf(itemData2.y)) {
                    itemData2.y = ImConstrainNan(ImConstrainInf(itemData2.y));
                }
                int pixY_0 = (int)(s.Spec.LineWeight);
                itemData1.y = ImMax(0.0, itemData1.y);
                const float pixY_1 = s.Spec.Size * (float)itemData1.y;
                const int pixY_chPosOffset = (int)(ImMax(s.Spec.Size, pixY_1) + gp.Style.DigitalSpacing);
                pixYMax = ImMax(pixYMax, pixY_chPosOffset);
                ImVec2 pMin = PlotToPixels(itemData1,IMPLOT_AUTO,IMPLOT_AUTO);
                ImVec2 pMax = PlotToPixels(itemData2,IMPLOT_AUTO,IMPLOT_AUTO);
                const int pixY_Offset = (int)gp.Style.DigitalPadding;
                const float y_ref = y_axis.IsInverted() ? y_axis.PixelMax : y_axis.PixelMin;
                pMin.y = y_ref - (gp.DigitalPlotOffset + pixY_Offset);
                pMax.y = y_ref - (gp.DigitalPlotOffset + pixY_0 + (int)pixY_1 + pixY_Offset);
                //plot only one rectangle for same digital state
                while (((i+2) < getter.Count) && (itemData1.y == itemData2.y)) {
                    const int in = (i + 1);
                    itemData2 = getter[in];
                    if (ImNanOrInf(itemData2.y)) break;
                    pMax.x = PlotToPixels(itemData2,IMPLOT_AUTO,IMPLOT_AUTO).x;
                    i++;
                }
                // do not extend plot outside plot range
                pMin.x = ImClamp(pMin.x, !x_axis.IsInverted() ? x_axis.PixelMin : x_axis.PixelMax, !x_axis.IsInverted() ? x_axis.PixelMax - 1 : x_axis.PixelMin - 1);
                pMax.x = ImClamp(pMax.x, !x_axis.IsInverted() ? x_axis.PixelMin : x_axis.PixelMax, !x_axis.IsInverted() ? x_axis.PixelMax - 1 : x_axis.PixelMin - 1);
                //plot a rectangle that extends up to x2 with y1 height
                if ((gp.CurrentPlot->PlotRect.Contains(pMin) || gp.CurrentPlot->PlotRect.Contains(pMax))) {
                    // ImVec4 colAlpha = item->Color;
                    // colAlpha.w = item->Highlight ? 1.0f : 0.9f;
                    draw_list.AddRectFilled(pMin, pMax, ImGui::GetColorU32(s.Spec.FillColor));
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
void PlotDigital(const char* label_id, const T* xs, const T* ys, int count, const ImPlotSpec& spec) {
    GetterXY<IndexerIdx<T>,IndexerIdx<T>> getter(IndexerIdx<T>(xs,count,spec.Offset,Stride<T>(spec)),IndexerIdx<T>(ys,count,spec.Offset,Stride<T>(spec)),count);
    return PlotDigitalEx(label_id, getter, spec);
}
#define INSTANTIATE_MACRO(T) template IMPLOT_API void PlotDigital<T>(const char* label_id, const T* xs, const T* ys, int count, const ImPlotSpec& spec);
CALL_INSTANTIATE_FOR_NUMERIC_TYPES()
#undef INSTANTIATE_MACRO

// custom
void PlotDigitalG(const char* label_id, ImPlotGetter getter_func, void* data, int count, const ImPlotSpec& spec) {
    GetterFuncPtr getter(getter_func,data,count);
    return PlotDigitalEx(label_id, getter, spec);
}

//-----------------------------------------------------------------------------
// [SECTION] PlotImage
//-----------------------------------------------------------------------------

#ifdef IMGUI_HAS_TEXTURES
void PlotImage(const char* label_id, ImTextureRef tex_ref, const ImPlotPoint& bmin, const ImPlotPoint& bmax, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImPlotSpec& spec) {
#else
void PlotImage(const char* label_id, ImTextureID tex_ref, const ImPlotPoint& bmin, const ImPlotPoint& bmax, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImPlotSpec& spec) {
#endif
    if (BeginItemEx(label_id, FitterRect(bmin,bmax), spec)) {
        ImU32 tint_col32 = ImGui::ColorConvertFloat4ToU32(tint_col);
        GetCurrentItem()->Color = tint_col32;
        ImDrawList& draw_list = *GetPlotDrawList();
        ImVec2 p1 = PlotToPixels(bmin.x, bmax.y,IMPLOT_AUTO,IMPLOT_AUTO);
        ImVec2 p2 = PlotToPixels(bmax.x, bmin.y,IMPLOT_AUTO,IMPLOT_AUTO);
        PushPlotClipRect();
        draw_list.AddImage(tex_ref, p1, p2, uv0, uv1, tint_col32);
        PopPlotClipRect();
        EndItem();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] PlotText
//-----------------------------------------------------------------------------

void PlotText(const char* text, double x, double y, const ImVec2& pixel_offset, const ImPlotSpec& spec) {
    IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot != nullptr, "PlotText() needs to be called between BeginPlot() and EndPlot()!");
    SetupLock();
    ImDrawList & draw_list = *GetPlotDrawList();
    PushPlotClipRect();
    ImU32 colTxt = GetStyleColorU32(ImPlotCol_InlayText);
    if (ImHasFlag(spec.Flags,ImPlotTextFlags_Vertical)) {
        ImVec2 siz = CalcTextSizeVertical(text) * 0.5f;
        ImVec2 ctr = siz * 0.5f;
        ImVec2 pos = PlotToPixels(ImPlotPoint(x,y),IMPLOT_AUTO,IMPLOT_AUTO) + ImVec2(-ctr.x, ctr.y) + pixel_offset;
        if (FitThisFrame() && !ImHasFlag(spec.Flags, ImPlotItemFlags_NoFit)) {
            FitPoint(PixelsToPlot(pos));
            FitPoint(PixelsToPlot(pos.x + siz.x, pos.y - siz.y));
        }
        AddTextVertical(&draw_list, pos, colTxt, text);
    }
    else {
        ImVec2 siz = ImGui::CalcTextSize(text);
        ImVec2 pos = PlotToPixels(ImPlotPoint(x,y),IMPLOT_AUTO,IMPLOT_AUTO) - siz * 0.5f + pixel_offset;
        if (FitThisFrame() && !ImHasFlag(spec.Flags, ImPlotItemFlags_NoFit)) {
            FitPoint(PixelsToPlot(pos));
            FitPoint(PixelsToPlot(pos+siz));
        }
        draw_list.AddText(pos, colTxt, text);
    }
    PopPlotClipRect();
}

//-----------------------------------------------------------------------------
// [SECTION] PlotDummy
//-----------------------------------------------------------------------------

void PlotDummy(const char* label_id, const ImPlotSpec& spec) {
    if (BeginItem(label_id, spec))
        EndItem();
}

} // namespace ImPlot

#endif // #ifndef IMGUI_DISABLE
