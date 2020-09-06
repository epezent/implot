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

// ImPlot v0.7 WIP

#include "implot.h"
#include "implot_internal.h"

#ifdef _MSC_VER
#define sprintf sprintf_s
#endif

#define SQRT_1_2 0.70710678118f
#define SQRT_3_2 0.86602540378f

#define IM_NORMALIZE2F_OVER_ZERO(VX, VY)                                                           \
    {                                                                                              \
        float d2 = VX * VX + VY * VY;                                                              \
        if (d2 > 0.0f) {                                                                           \
            float inv_len = 1.0f / ImSqrt(d2);                                                     \
            VX *= inv_len;                                                                         \
            VY *= inv_len;                                                                         \
        }                                                                                          \
    }

namespace ImPlot {

//-----------------------------------------------------------------------------
// Item Utils
//-----------------------------------------------------------------------------

ImPlotItem* RegisterOrGetItem(const char* label_id) {
    ImPlotContext& gp = *GImPlot;
    ImGuiID id = ImGui::GetID(label_id);
    ImPlotItem* item = gp.CurrentPlot->Items.GetOrAddByKey(id);
    if (item->SeenThisFrame)
        return item;
    item->SeenThisFrame = true;
    int idx = gp.CurrentPlot->Items.GetIndex(item);
    item->ID = id;
    if (ImGui::FindRenderedTextEnd(label_id, NULL) != label_id) {
        gp.LegendIndices.push_back(idx);
        item->NameOffset = gp.LegendLabels.size();
        gp.LegendLabels.append(label_id, label_id + strlen(label_id) + 1);
    }
    else {
        item->Show = true;
    }
    if (item->Show)
        gp.VisibleItemCount++;
    return item;
}

ImPlotItem* GetItem(int i) {
    ImPlotContext& gp = *GImPlot;
    return gp.CurrentPlot->Items.GetByIndex(gp.LegendIndices[i]);
}

ImPlotItem* GetItem(const char* label_id) {
    ImPlotContext& gp = *GImPlot;
    ImGuiID id = ImGui::GetID(label_id);
    return gp.CurrentPlot->Items.GetByKey(id);
}

ImPlotItem* GetItem(const char* plot_title, const char* item_label_id) {
    ImPlotState* plot = GetPlot(plot_title);
    if (plot) {
        ImGuiID id = ImGui::GetID(item_label_id);
        return plot->Items.GetByKey(id);
    }
    return NULL;
}

ImPlotItem* GetCurrentItem() {
    ImPlotContext& gp = *GImPlot;
    return gp.CurrentItem;
}

void BustItemCache() {
    ImPlotContext& gp = *GImPlot;
    for (int p = 0; p < gp.Plots.GetSize(); ++p) {
        ImPlotState& plot = *gp.Plots.GetByIndex(p);
        plot.ColormapIdx = 0;
        plot.Items.Clear();
    }
}

// Begins a new item. Returns false if the item should not be plotted.
bool BeginItem(const char* label_id, ImPlotCol recolor_from) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotX() needs to be called between BeginPlot() and EndPlot()!");
    ImPlotItem* item = RegisterOrGetItem(label_id);
    if (!item->Show) {
        // reset next item data
        gp.NextItemStyle = ImPlotItemStyle();
        return false;
    }
    else {
        // set current item
        gp.CurrentItem = item;
        ImPlotItemStyle& s = gp.NextItemStyle;
        // override item color
        if (recolor_from != -1) {
            if (!IsColorAuto(s.Colors[recolor_from]))
                item->Color = s.Colors[recolor_from];
            else if (!IsColorAuto(gp.Style.Colors[recolor_from]))
                item->Color = gp.Style.Colors[recolor_from];
        }
        // stage next item colors
        s.Colors[ImPlotCol_Line]           = IsColorAuto(s.Colors[ImPlotCol_Line])          ? ( IsColorAuto(ImPlotCol_Line)           ? item->Color                : gp.Style.Colors[ImPlotCol_Line]          ) : s.Colors[ImPlotCol_Line];
        s.Colors[ImPlotCol_Fill]           = IsColorAuto(s.Colors[ImPlotCol_Fill])          ? ( IsColorAuto(ImPlotCol_Fill)           ? item->Color                : gp.Style.Colors[ImPlotCol_Fill]          ) : s.Colors[ImPlotCol_Fill];
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
        // s.Colors[ImPlotCol_MarkerFill].w *= s.FillAlpha; // TODO: this should be separate, if it at all
        // apply highlight mods
        if (item->LegendHovered && !ImHasFlag(gp.CurrentPlot->Flags, ImPlotFlags_NoHighlight)) {
            s.LineWeight   *= 2;
            s.MarkerWeight *= 2;
            // TODO: highlight fills?
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
    gp.NextItemStyle = ImPlotItemStyle();
    // set current item
    gp.CurrentItem = NULL;
}

void SetNextLineStyle(const ImVec4& col, float weight) {
    ImPlotContext& gp = *GImPlot;
    gp.NextItemStyle.Colors[ImPlotCol_Line] = col;
    gp.NextItemStyle.LineWeight             = weight;
}

void SetNextFillStyle(const ImVec4& col, float alpha) {
    ImPlotContext& gp = *GImPlot;
    gp.NextItemStyle.Colors[ImPlotCol_Fill] = col;
    gp.NextItemStyle.FillAlpha              = alpha;
}

void SetNextMarkerStyle(ImPlotMarker marker, float size, const ImVec4& fill, float weight, const ImVec4& outline) {
    ImPlotContext& gp = *GImPlot;
    gp.NextItemStyle.Marker                          = marker;
    gp.NextItemStyle.Colors[ImPlotCol_MarkerFill]    = fill;
    gp.NextItemStyle.MarkerSize                      = size;
    gp.NextItemStyle.Colors[ImPlotCol_MarkerOutline] = outline;
    gp.NextItemStyle.MarkerWeight                    = weight;
}

void SetNextErrorBarStyle(const ImVec4& col, float size, float weight) {
    ImPlotContext& gp = *GImPlot;
    gp.NextItemStyle.Colors[ImPlotCol_ErrorBar] = col;
    gp.NextItemStyle.ErrorBarSize               = size;
    gp.NextItemStyle.ErrorBarWeight             = weight;
}


//-----------------------------------------------------------------------------
// GETTERS
//-----------------------------------------------------------------------------

// Getters can be thought of as iterators that convert user data (e.g. raw arrays)
// to ImPlotPoints

// Interprets an array of Y points as ImPlotPoints where the X value is the index
template <typename T>
struct GetterYs {
    GetterYs(const T* ys, int count, int offset, int stride) {
        Ys = ys;
        Count = count;
        Offset = count ? ImPosMod(offset, count) : 0;
        Stride = stride;
    }
    const T* Ys;
    int Count;
    int Offset;
    int Stride;
    inline ImPlotPoint operator()(int idx) {
        return ImPlotPoint((double)idx, (double)OffsetAndStride(Ys, idx, Count, Offset, Stride));
    }
};

// Interprets separate arrays for X and Y points as ImPlotPoints
template <typename T>
struct GetterXsYs {
    GetterXsYs(const T* xs, const T* ys, int count, int offset, int stride) {
        Xs = xs; Ys = ys;
        Count = count;
        Offset = count ? ImPosMod(offset, count) : 0;
        Stride = stride;
    }
    const T* Xs;
    const T* Ys;
    int Count;
    int Offset;
    int Stride;
    inline ImPlotPoint operator()(int idx) {
        return ImPlotPoint((double)OffsetAndStride(Xs, idx, Count, Offset, Stride), (double)OffsetAndStride(Ys, idx, Count, Offset, Stride));
    }
};

// Always returns a constant Y reference value where the X value is the index
template <typename T>
struct GetterYRef {
    GetterYRef(T y_ref, int count) { YRef = y_ref; Count = count; }
    inline ImPlotPoint operator()(int idx) {
        return ImPlotPoint((double)idx, (double)YRef);
    }
    T YRef;
    int Count;
};

// Interprets an array of X points as ImPlotPoints where the Y value is a constant reference value
template <typename T>
struct GetterXsYRef {
    GetterXsYRef(const T* xs, T y_ref, int count, int offset, int stride) {
        Xs = xs;
        YRef = y_ref;
        Count = count;
        Offset = count ? ImPosMod(offset, count) : 0;
        Stride = stride;
    }
    const T* Xs;
    T YRef;
    int Count;
    int Offset;
    int Stride;
    inline ImPlotPoint operator()(int idx) {
        return ImPlotPoint((double)OffsetAndStride(Xs, idx, Count, Offset, Stride), (double)YRef);
    }
};

// Interprets an array of ImVec2 points as ImPlotPoints
struct GetterImVec2 {
    GetterImVec2(const ImVec2* data, int count, int offset) {
        Data = data;
        Count = count;
        Offset = count ? ImPosMod(offset, count) : 0;
    }
    inline ImPlotPoint operator()(int idx) {
        idx = ImPosMod(Offset + idx, Count);
        return ImPlotPoint((double)Data[idx].x, (double)Data[idx].y);
    }
    const ImVec2* Data;
    int Count;
    int Offset;
};

// Interprets an array of ImPlotPoints as ImPlotPoints (essentially a pass through)
struct GetterImPlotPoint {
    GetterImPlotPoint(const ImPlotPoint* data, int count, int offset) {
        Data = data;
        Count = count;
        Offset = count ? ImPosMod(offset, count) : 0;
    }
    inline ImPlotPoint operator()(int idx) {
        idx = ImPosMod(Offset + idx, Count);
        return Data[idx];
    }
    const ImPlotPoint* Data;
    int Count;
    int Offset;
};

/// Interprets a user's function pointer as ImPlotPoints
struct GetterFuncPtrImPlotPoint {
    GetterFuncPtrImPlotPoint(ImPlotPoint (*g)(void* data, int idx), void* d, int count, int offset) {
        getter = g;
        Data = d;
        Count = count;
        Offset = count ? ImPosMod(offset, count) : 0;
    }
    inline ImPlotPoint operator()(int idx) {
        idx = ImPosMod(Offset + idx, Count);
        return getter(Data, idx);
    }
    ImPlotPoint (*getter)(void* data, int idx);
    void* Data;
    int Count;
    int Offset;
};

template <typename T>
struct GetterBarV {
    const T* Ys; T XShift; int Count; int Offset; int Stride;
    GetterBarV(const T* ys, T xshift, int count, int offset, int stride) { Ys = ys; XShift = xshift; Count = count; Offset = offset; Stride = stride; }
    inline ImPlotPoint operator()(int idx) { return ImPlotPoint((double)idx + (double)XShift, (double)OffsetAndStride(Ys, idx, Count, Offset, Stride)); }
};

template <typename T>
struct GetterBarH {
    const T* Xs; T YShift; int Count; int Offset; int Stride;
    GetterBarH(const T* xs, T yshift, int count, int offset, int stride) { Xs = xs; YShift = yshift; Count = count; Offset = offset; Stride = stride; }
    inline ImPlotPoint operator()(int idx) { return ImPlotPoint((double)OffsetAndStride(Xs, idx, Count, Offset, Stride), (double)idx + (double)YShift); }
};

template <typename T>
struct GetterError {
    const T* Xs; const T* Ys; const T* Neg; const T* Pos; int Count; int Offset; int Stride;
    GetterError(const T* xs, const T* ys, const T* neg, const T* pos, int count, int offset, int stride) {
        Xs = xs; Ys = ys; Neg = neg; Pos = pos; Count = count; Offset = offset; Stride = stride;
    }
    ImPlotPointError operator()(int idx) {
        return ImPlotPointError((double)OffsetAndStride(Xs,  idx, Count, Offset, Stride),
                                (double)OffsetAndStride(Ys,  idx, Count, Offset, Stride),
                                (double)OffsetAndStride(Neg, idx, Count, Offset, Stride),
                                (double)OffsetAndStride(Pos, idx, Count, Offset, Stride));
    }
};

//-----------------------------------------------------------------------------
// TRANSFORMERS
//-----------------------------------------------------------------------------

// Transforms convert points in plot space (i.e. ImPlotPoint) to pixel space (i.e. ImVec2)
// TODO: Cache transformation variables

// Transforms points for linear x and linear y space
struct TransformerLinLin {
    TransformerLinLin() : YAxis(GetCurrentYAxis()) {}

    inline ImVec2 operator()(const ImPlotPoint& plt) { return (*this)(plt.x, plt.y); }
    inline ImVec2 operator()(double x, double y) {
        ImPlotContext& gp = *GImPlot;
        return ImVec2( (float)(gp.PixelRange[YAxis].Min.x + gp.Mx * (x - gp.CurrentPlot->XAxis.Range.Min)),
                       (float)(gp.PixelRange[YAxis].Min.y + gp.My[YAxis] * (y - gp.CurrentPlot->YAxis[YAxis].Range.Min)) );
    }

    int YAxis;
};

// Transforms points for log x and linear y space
struct TransformerLogLin {
    TransformerLogLin() : YAxis(GetCurrentYAxis()) {}

    inline ImVec2 operator()(const ImPlotPoint& plt) { return (*this)(plt.x, plt.y); }
    inline ImVec2 operator()(double x, double y) {
        ImPlotContext& gp = *GImPlot;
        double t = ImLog10(x / gp.CurrentPlot->XAxis.Range.Min) / gp.LogDenX;
        x        = ImLerp(gp.CurrentPlot->XAxis.Range.Min, gp.CurrentPlot->XAxis.Range.Max, (float)t);
        return ImVec2( (float)(gp.PixelRange[YAxis].Min.x + gp.Mx * (x - gp.CurrentPlot->XAxis.Range.Min)),
                       (float)(gp.PixelRange[YAxis].Min.y + gp.My[YAxis] * (y - gp.CurrentPlot->YAxis[YAxis].Range.Min)) );
    }

    int YAxis;
};

// Transforms points for linear x and log y space
struct TransformerLinLog {
    TransformerLinLog() : YAxis(GetCurrentYAxis()) {}

    inline ImVec2 operator()(const ImPlotPoint& plt) { return (*this)(plt.x, plt.y); }
    inline ImVec2 operator()(double x, double y) {
        ImPlotContext& gp = *GImPlot;
        double t = ImLog10(y / gp.CurrentPlot->YAxis[YAxis].Range.Min) / gp.LogDenY[YAxis];
        y        = ImLerp(gp.CurrentPlot->YAxis[YAxis].Range.Min, gp.CurrentPlot->YAxis[YAxis].Range.Max, (float)t);
        return ImVec2( (float)(gp.PixelRange[YAxis].Min.x + gp.Mx * (x - gp.CurrentPlot->XAxis.Range.Min)),
                       (float)(gp.PixelRange[YAxis].Min.y + gp.My[YAxis] * (y - gp.CurrentPlot->YAxis[YAxis].Range.Min)) );
    }
    int YAxis;
};

// Transforms points for log x and log y space
struct TransformerLogLog {
    TransformerLogLog() : YAxis(GetCurrentYAxis()) {}

    inline ImVec2 operator()(const ImPlotPoint& plt) { return (*this)(plt.x, plt.y); }
    inline ImVec2 operator()(double x, double y) {
        ImPlotContext& gp = *GImPlot;
        double t = ImLog10(x / gp.CurrentPlot->XAxis.Range.Min) / gp.LogDenX;
        x        = ImLerp(gp.CurrentPlot->XAxis.Range.Min, gp.CurrentPlot->XAxis.Range.Max, (float)t);
        t        = ImLog10(y / gp.CurrentPlot->YAxis[YAxis].Range.Min) / gp.LogDenY[YAxis];
        y        = ImLerp(gp.CurrentPlot->YAxis[YAxis].Range.Min, gp.CurrentPlot->YAxis[YAxis].Range.Max, (float)t);
        return ImVec2( (float)(gp.PixelRange[YAxis].Min.x + gp.Mx * (x - gp.CurrentPlot->XAxis.Range.Min)),
                       (float)(gp.PixelRange[YAxis].Min.y + gp.My[YAxis] * (y - gp.CurrentPlot->YAxis[YAxis].Range.Min)) );
    }

    int YAxis;
};

//-----------------------------------------------------------------------------
// PRIMITIVE RENDERERS
//-----------------------------------------------------------------------------

inline void AddLine(const ImVec2& P1, const ImVec2& P2, float weight, ImU32 col, ImDrawList& DrawList, ImVec2 uv) {
    float dx = P2.x - P1.x;
    float dy = P2.y - P1.y;
    IM_NORMALIZE2F_OVER_ZERO(dx, dy);
    dx *= (weight * 0.5f);
    dy *= (weight * 0.5f);
    DrawList._VtxWritePtr[0].pos.x = P1.x + dy;
    DrawList._VtxWritePtr[0].pos.y = P1.y - dx;
    DrawList._VtxWritePtr[0].uv    = uv;
    DrawList._VtxWritePtr[0].col   = col;
    DrawList._VtxWritePtr[1].pos.x = P2.x + dy;
    DrawList._VtxWritePtr[1].pos.y = P2.y - dx;
    DrawList._VtxWritePtr[1].uv    = uv;
    DrawList._VtxWritePtr[1].col   = col;
    DrawList._VtxWritePtr[2].pos.x = P2.x - dy;
    DrawList._VtxWritePtr[2].pos.y = P2.y + dx;
    DrawList._VtxWritePtr[2].uv    = uv;
    DrawList._VtxWritePtr[2].col   = col;
    DrawList._VtxWritePtr[3].pos.x = P1.x - dy;
    DrawList._VtxWritePtr[3].pos.y = P1.y + dx;
    DrawList._VtxWritePtr[3].uv    = uv;
    DrawList._VtxWritePtr[3].col   = col;
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

template <typename TGetter, typename TTransformer>
struct LineStripRenderer {
    inline LineStripRenderer(TGetter getter, TTransformer transformer, ImU32 col, float weight) :
        Getter(getter),
        Transformer(transformer)
    {
        Prims = Getter.Count - 1;
        Col = col;
        Weight = weight;
        P1 = Transformer(Getter(0));
    }
    inline bool operator()(ImDrawList& DrawList, const ImRect& cull_rect, const ImVec2& uv, int prim) {
        ImVec2 P2 = Transformer(Getter(prim + 1));
        if (!cull_rect.Overlaps(ImRect(ImMin(P1, P2), ImMax(P1, P2)))) {
            P1 = P2;
            return false;
        }
        AddLine(P1,P2,Weight,Col,DrawList,uv);
        P1 = P2;
        return true;
    }
    TGetter Getter;
    TTransformer Transformer;
    int Prims;
    ImU32 Col;
    float Weight;
    ImVec2 P1;
    static const int IdxConsumed = 6;
    static const int VtxConsumed = 4;
};

template <typename TGetter1, typename TGetter2, typename TTransformer>
struct LineSegmentsRenderer {
    inline LineSegmentsRenderer(TGetter1 getter1, TGetter2 getter2, TTransformer transformer, ImU32 col, float weight) :
        Getter1(getter1),
        Getter2(getter2),
        Transformer(transformer)
    {

        Prims = ImMin(Getter1.Count, Getter2.Count);
        Col = col;
        Weight = weight;
    }
    inline bool operator()(ImDrawList& DrawList, const ImRect& cull_rect, const ImVec2& uv, int prim) {
        ImVec2 P1 = Transformer(Getter1(prim));
        ImVec2 P2 = Transformer(Getter2(prim));
        if (!cull_rect.Overlaps(ImRect(ImMin(P1, P2), ImMax(P1, P2))))
            return false;
        AddLine(P1,P2,Weight,Col,DrawList,uv);
        return true;
    }
    TGetter1 Getter1;
    TGetter2 Getter2;
    TTransformer Transformer;
    int Prims;
    ImU32 Col;
    float Weight;
    static const int IdxConsumed = 6;
    static const int VtxConsumed = 4;
};

template <typename TGetter1, typename TGetter2, typename TTransformer>
struct ShadedRenderer {
    ShadedRenderer(TGetter1 getter1, TGetter2 getter2, TTransformer transformer, ImU32 col) :
        Getter1(getter1),
        Getter2(getter2),
        Transformer(transformer),
        Col(col)
    {
        Prims = ImMin(Getter1.Count, Getter2.Count) - 1;
        P11 = Transformer(Getter1(0));
        P12 = Transformer(Getter2(0));
    }

    inline bool operator()(ImDrawList& DrawList, const ImRect& /*cull_rect*/, const ImVec2& uv, int prim) {
        // TODO: Culling
        ImVec2 P21 = Transformer(Getter1(prim+1));
        ImVec2 P22 = Transformer(Getter2(prim+1));
        const int intersect = (P11.y > P12.y && P22.y > P21.y) || (P12.y > P11.y && P21.y > P22.y);
        ImVec2 intersection = Intersection(P11,P21,P12,P22);
        DrawList._VtxWritePtr[0].pos = P11;
        DrawList._VtxWritePtr[0].uv  = uv;
        DrawList._VtxWritePtr[0].col = Col;
        DrawList._VtxWritePtr[1].pos = P21;
        DrawList._VtxWritePtr[1].uv  = uv;
        DrawList._VtxWritePtr[1].col = Col;
        DrawList._VtxWritePtr[2].pos = intersection;
        DrawList._VtxWritePtr[2].uv  = uv;
        DrawList._VtxWritePtr[2].col = Col;
        DrawList._VtxWritePtr[3].pos = P12;
        DrawList._VtxWritePtr[3].uv  = uv;
        DrawList._VtxWritePtr[3].col = Col;
        DrawList._VtxWritePtr[4].pos = P22;
        DrawList._VtxWritePtr[4].uv  = uv;
        DrawList._VtxWritePtr[4].col = Col;
        DrawList._VtxWritePtr += 5;
        DrawList._IdxWritePtr[0] = (ImDrawIdx)(DrawList._VtxCurrentIdx);
        DrawList._IdxWritePtr[1] = (ImDrawIdx)(DrawList._VtxCurrentIdx + 1 + intersect);
        DrawList._IdxWritePtr[2] = (ImDrawIdx)(DrawList._VtxCurrentIdx + 3);
        DrawList._IdxWritePtr[3] = (ImDrawIdx)(DrawList._VtxCurrentIdx + 1);
        DrawList._IdxWritePtr[4] = (ImDrawIdx)(DrawList._VtxCurrentIdx + 3 - intersect);
        DrawList._IdxWritePtr[5] = (ImDrawIdx)(DrawList._VtxCurrentIdx + 4);
        DrawList._IdxWritePtr += 6;
        DrawList._VtxCurrentIdx += 5;
        P11 = P21;
        P12 = P22;
        return true;
    }
    TGetter1 Getter1;
    TGetter2 Getter2;
    TTransformer Transformer;
    int Prims;
    ImU32 Col;
    ImVec2 P11, P12;
    static const int IdxConsumed = 6;
    static const int VtxConsumed = 5;
};

template <typename TGetter, typename TTransformer>
struct RectRenderer {
    inline RectRenderer(TGetter getter, TTransformer transformer, ImU32 col) :
        Getter(getter),
        Transformer(transformer)
    {
        Prims = Getter.Count / 2;
        Col = col;
    }
    inline bool operator()(ImDrawList& DrawList, const ImRect& /*cull_rect*/, const ImVec2& uv, int prim) {
        // TODO: Culling
        ImVec2 P1 = Transformer(Getter(2*prim));
        ImVec2 P2 = Transformer(Getter(2*prim+1));
        DrawList._VtxWritePtr[0].pos   = P1;
        DrawList._VtxWritePtr[0].uv    = uv;
        DrawList._VtxWritePtr[0].col   = Col;
        DrawList._VtxWritePtr[1].pos.x = P1.x;
        DrawList._VtxWritePtr[1].pos.y = P2.y;
        DrawList._VtxWritePtr[1].uv    = uv;
        DrawList._VtxWritePtr[1].col   = Col;
        DrawList._VtxWritePtr[2].pos   = P2;
        DrawList._VtxWritePtr[2].uv    = uv;
        DrawList._VtxWritePtr[2].col   = Col;
        DrawList._VtxWritePtr[3].pos.x = P2.x;
        DrawList._VtxWritePtr[3].pos.y = P1.y;
        DrawList._VtxWritePtr[3].uv    = uv;
        DrawList._VtxWritePtr[3].col   = Col;
        DrawList._VtxWritePtr += 4;
        DrawList._IdxWritePtr[0] = (ImDrawIdx)(DrawList._VtxCurrentIdx);
        DrawList._IdxWritePtr[1] = (ImDrawIdx)(DrawList._VtxCurrentIdx + 1);
        DrawList._IdxWritePtr[2] = (ImDrawIdx)(DrawList._VtxCurrentIdx + 3);
        DrawList._IdxWritePtr[3] = (ImDrawIdx)(DrawList._VtxCurrentIdx + 1);
        DrawList._IdxWritePtr[4] = (ImDrawIdx)(DrawList._VtxCurrentIdx + 2);
        DrawList._IdxWritePtr[5] = (ImDrawIdx)(DrawList._VtxCurrentIdx + 3);
        DrawList._IdxWritePtr   += 6;
        DrawList._VtxCurrentIdx += 4;
        return true;
    }
    TGetter Getter;
    TTransformer Transformer;
    int Prims;
    ImU32 Col;
    static const int IdxConsumed = 6;
    static const int VtxConsumed = 4;
};

// Stupid way of calculating maximum index size of ImDrawIdx without integer overflow issues
template <typename T>
struct MaxIdx { static const unsigned int Value; };
template <> const unsigned int MaxIdx<unsigned short>::Value = 65535;
template <> const unsigned int MaxIdx<unsigned int>::Value   = 4294967295;

/// Renders primitive shapes in bulk as efficiently as possible.
template <typename Renderer>
inline void RenderPrimitives(Renderer renderer, ImDrawList& DrawList, const ImRect& cull_rect) {
    unsigned int prims        = renderer.Prims;
    unsigned int prims_culled = 0;
    unsigned int idx          = 0;
    const ImVec2 uv = DrawList._Data->TexUvWhitePixel;
    while (prims) {
        // find how many can be reserved up to end of current draw command's limit
        unsigned int cnt = ImMin(prims, (MaxIdx<ImDrawIdx>::Value - DrawList._VtxCurrentIdx) / Renderer::VtxConsumed);
        // make sure at least this many elements can be rendered to avoid situations where at the end of buffer this slow path is not taken all the time
        if (cnt >= ImMin(64u, prims)) {
            if (prims_culled >= cnt)
                prims_culled -= cnt; // reuse previous reservation
            else {
                DrawList.PrimReserve((cnt - prims_culled) * Renderer::IdxConsumed, (cnt - prims_culled) * Renderer::VtxConsumed); // add more elements to previous reservation
                prims_culled = 0;
            }
        }
        else
        {
            if (prims_culled > 0) {
                DrawList.PrimUnreserve(prims_culled * Renderer::IdxConsumed, prims_culled * Renderer::VtxConsumed);
                prims_culled = 0;
            }
            cnt = ImMin(prims, (MaxIdx<ImDrawIdx>::Value - 0/*DrawList._VtxCurrentIdx*/) / Renderer::VtxConsumed);
            DrawList.PrimReserve(cnt * Renderer::IdxConsumed, cnt * Renderer::VtxConsumed); // reserve new draw command
        }
        prims -= cnt;
        for (unsigned int ie = idx + cnt; idx != ie; ++idx) {
            if (!renderer(DrawList, cull_rect, uv, idx))
                prims_culled++;
        }
    }
    if (prims_culled > 0)
        DrawList.PrimUnreserve(prims_culled * Renderer::IdxConsumed, prims_culled * Renderer::VtxConsumed);
}

template <typename Getter, typename Transformer>
inline void RenderLineStrip(Getter getter, Transformer transformer, ImDrawList& DrawList, float line_weight, ImU32 col) {
    ImPlotContext& gp = *GImPlot;
    if (ImHasFlag(gp.CurrentPlot->Flags, ImPlotFlags_AntiAliased) || gp.Style.AntiAliasedLines) {
        ImVec2 p1 = transformer(getter(0));
        for (int i = 1; i < getter.Count; ++i) {
            ImVec2 p2 = transformer(getter(i));
            if (gp.BB_Plot.Overlaps(ImRect(ImMin(p1, p2), ImMax(p1, p2))))
                DrawList.AddLine(p1, p2, col, line_weight);
            p1 = p2;
        }
    }
    else {
        RenderPrimitives(LineStripRenderer<Getter,Transformer>(getter, transformer, col, line_weight), DrawList, gp.BB_Plot);
    }
}

template <typename Getter1, typename Getter2, typename Transformer>
inline void RenderLineSegments(Getter1 getter1, Getter2 getter2, Transformer transformer, ImDrawList& DrawList, float line_weight, ImU32 col) {
    ImPlotContext& gp = *GImPlot;
    if (ImHasFlag(gp.CurrentPlot->Flags, ImPlotFlags_AntiAliased) || gp.Style.AntiAliasedLines) {
        int I = ImMin(getter1.Count, getter2.Count);
        for (int i = 0; i < I; ++i) {
            ImVec2 p1 = transformer(getter1(i));
            ImVec2 p2 = transformer(getter2(i));
            if (gp.BB_Plot.Overlaps(ImRect(ImMin(p1, p2), ImMax(p1, p2))))
                DrawList.AddLine(p1, p2, col, line_weight);
        }
    }
    else {
        RenderPrimitives(LineSegmentsRenderer<Getter1,Getter2,Transformer>(getter1, getter2, transformer, col, line_weight), DrawList, gp.BB_Plot);
    }
}

//-----------------------------------------------------------------------------
// MARKER RENDERERS
//-----------------------------------------------------------------------------

inline void TransformMarker(ImVec2* points, int n, const ImVec2& c, float s) {
    for (int i = 0; i < n; ++i) {
        points[i].x = c.x + points[i].x * s;
        points[i].y = c.y + points[i].y * s;
    }
}

inline void RenderMarkerGeneral(ImDrawList& DrawList, ImVec2* points, int n, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    TransformMarker(points, n, c, s);
    if (fill)
        DrawList.AddConvexPolyFilled(points, n, col_fill);
    if (outline && !(fill && col_outline == col_fill)) {
        for (int i = 0; i < n; ++i)
            DrawList.AddLine(points[i], points[(i+1)%n], col_outline, weight);
    }
}

inline void RenderMarkerCircle(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
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
    RenderMarkerGeneral(DrawList, marker, 10, c, s, outline, col_outline, fill, col_fill, weight);
}

inline void RenderMarkerDiamond(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[4] = {ImVec2(1, 0), ImVec2(0, -1), ImVec2(-1, 0), ImVec2(0, 1)};
    RenderMarkerGeneral(DrawList, marker, 4, c, s, outline, col_outline, fill, col_fill, weight);
}

inline void RenderMarkerSquare(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[4] = {ImVec2(SQRT_1_2,SQRT_1_2),ImVec2(SQRT_1_2,-SQRT_1_2),ImVec2(-SQRT_1_2,-SQRT_1_2),ImVec2(-SQRT_1_2,SQRT_1_2)};
    RenderMarkerGeneral(DrawList, marker, 4, c, s, outline, col_outline, fill, col_fill, weight);
}

inline void RenderMarkerUp(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[3] = {ImVec2(SQRT_3_2,0.5f),ImVec2(0,-1),ImVec2(-SQRT_3_2,0.5f)};
    RenderMarkerGeneral(DrawList, marker, 3, c, s, outline, col_outline, fill, col_fill, weight);
}

inline void RenderMarkerDown(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[3] = {ImVec2(SQRT_3_2,-0.5f),ImVec2(0,1),ImVec2(-SQRT_3_2,-0.5f)};
    RenderMarkerGeneral(DrawList, marker, 3, c, s, outline, col_outline, fill, col_fill, weight);
}

inline void RenderMarkerLeft(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[3] = {ImVec2(-1,0), ImVec2(0.5, SQRT_3_2), ImVec2(0.5, -SQRT_3_2)};
    RenderMarkerGeneral(DrawList, marker, 3, c, s, outline, col_outline, fill, col_fill, weight);
}

inline void RenderMarkerRight(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[3] = {ImVec2(1,0), ImVec2(-0.5, SQRT_3_2), ImVec2(-0.5, -SQRT_3_2)};
    RenderMarkerGeneral(DrawList, marker, 3, c, s, outline, col_outline, fill, col_fill, weight);
}

inline void RenderMarkerAsterisk(ImDrawList& DrawList, const ImVec2& c, float s, bool /*outline*/, ImU32 col_outline, bool /*fill*/, ImU32 /*col_fill*/, float weight) {
    ImVec2 marker[6] = {ImVec2(SQRT_3_2, 0.5f), ImVec2(0, -1), ImVec2(-SQRT_3_2, 0.5f), ImVec2(SQRT_3_2, -0.5f), ImVec2(0, 1),  ImVec2(-SQRT_3_2, -0.5f)};
    TransformMarker(marker, 6, c, s);
    DrawList.AddLine(marker[0], marker[5], col_outline, weight);
    DrawList.AddLine(marker[1], marker[4], col_outline, weight);
    DrawList.AddLine(marker[2], marker[3], col_outline, weight);
}

inline void RenderMarkerPlus(ImDrawList& DrawList, const ImVec2& c, float s, bool /*outline*/, ImU32 col_outline, bool /*fill*/, ImU32 /*col_fill*/, float weight) {
    ImVec2 marker[4] = {ImVec2(1, 0), ImVec2(0, -1), ImVec2(-1, 0), ImVec2(0, 1)};
    TransformMarker(marker, 4, c, s);
    DrawList.AddLine(marker[0], marker[2], col_outline, weight);
    DrawList.AddLine(marker[1], marker[3], col_outline, weight);
}

inline void RenderMarkerCross(ImDrawList& DrawList, const ImVec2& c, float s, bool /*outline*/, ImU32 col_outline, bool /*fill*/, ImU32 /*col_fill*/, float weight) {
    ImVec2 marker[4] = {ImVec2(SQRT_1_2,SQRT_1_2),ImVec2(SQRT_1_2,-SQRT_1_2),ImVec2(-SQRT_1_2,-SQRT_1_2),ImVec2(-SQRT_1_2,SQRT_1_2)};
    TransformMarker(marker, 4, c, s);
    DrawList.AddLine(marker[0], marker[2], col_outline, weight);
    DrawList.AddLine(marker[1], marker[3], col_outline, weight);
}

template <typename Transformer, typename Getter>
inline void RenderMarkers(Getter getter, Transformer transformer, ImDrawList& DrawList, ImPlotMarker marker, float size, bool rend_mk_line, ImU32 col_mk_line, float weight, bool rend_mk_fill, ImU32 col_mk_fill) {
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
    for (int i = 0; i < getter.Count; ++i) {
        ImVec2 c = transformer(getter(i));
        if (gp.BB_Plot.Contains(c))
            marker_table[marker](DrawList, c, size, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, weight);
    }
}

//-----------------------------------------------------------------------------
// PLOT LINES / MARKERS
//-----------------------------------------------------------------------------

template <typename Getter>
inline void PlotLineEx(const char* label_id, Getter getter) {
    if (BeginItem(label_id, ImPlotCol_Line)) {
        if (FitThisFrame()) {
            for (int i = 0; i < getter.Count; ++i) {
                ImPlotPoint p = getter(i);
                FitPoint(p);
            }
        }
        const ImPlotItemStyle& s = GetItemStyle();
        ImDrawList& DrawList = *GetPlotDrawList();
        if (getter.Count > 1 && s.RenderLine) {
            const ImU32 col_line    = ImGui::GetColorU32(s.Colors[ImPlotCol_Line]);
            switch (GetCurrentScale()) {
                case ImPlotScale_LinLin: RenderLineStrip(getter, TransformerLinLin(), DrawList, s.LineWeight, col_line); break;
                case ImPlotScale_LogLin: RenderLineStrip(getter, TransformerLogLin(), DrawList, s.LineWeight, col_line); break;
                case ImPlotScale_LinLog: RenderLineStrip(getter, TransformerLinLog(), DrawList, s.LineWeight, col_line); break;
                case ImPlotScale_LogLog: RenderLineStrip(getter, TransformerLogLog(), DrawList, s.LineWeight, col_line); break;
            }
        }
        // render markers
        if (s.Marker != ImPlotMarker_None) {
            const ImU32 col_line = ImGui::GetColorU32(s.Colors[ImPlotCol_MarkerOutline]);
            const ImU32 col_fill = ImGui::GetColorU32(s.Colors[ImPlotCol_MarkerFill]);
            switch (GetCurrentScale()) {
                case ImPlotScale_LinLin: RenderMarkers(getter, TransformerLinLin(), DrawList, s.Marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
                case ImPlotScale_LogLin: RenderMarkers(getter, TransformerLogLin(), DrawList, s.Marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
                case ImPlotScale_LinLog: RenderMarkers(getter, TransformerLinLog(), DrawList, s.Marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
                case ImPlotScale_LogLog: RenderMarkers(getter, TransformerLogLog(), DrawList, s.Marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
            }
        }
        EndItem();
    }
}

// float
void PlotLine(const char* label_id, const float* values, int count, int offset, int stride) {
    GetterYs<float> getter(values,count,offset,stride);
    PlotLineEx(label_id, getter);
}

void PlotLine(const char* label_id, const float* xs, const float* ys, int count, int offset, int stride) {
    GetterXsYs<float> getter(xs,ys,count,offset,stride);
    return PlotLineEx(label_id, getter);
}

void PlotLine(const char* label_id, const ImVec2* data, int count, int offset) {
    GetterImVec2 getter(data, count, offset);
    return PlotLineEx(label_id, getter);
}

// double
void PlotLine(const char* label_id, const double* values, int count, int offset, int stride) {
    GetterYs<double> getter(values,count,offset,stride);
    PlotLineEx(label_id, getter);
}

void PlotLine(const char* label_id, const double* xs, const double* ys, int count, int offset, int stride) {
    GetterXsYs<double> getter(xs,ys,count,offset,stride);
    return PlotLineEx(label_id, getter);
}

void PlotLine(const char* label_id, const ImPlotPoint* data, int count, int offset) {
    GetterImPlotPoint getter(data, count, offset);
    return PlotLineEx(label_id, getter);
}

// custom
void PlotLine(const char* label_id, ImPlotPoint (*getter_func)(void* data, int idx), void* data, int count, int offset) {
    GetterFuncPtrImPlotPoint getter(getter_func,data, count, offset);
    return PlotLineEx(label_id, getter);
}

//-----------------------------------------------------------------------------
// PLOT SCATTER
//-----------------------------------------------------------------------------

template <typename Getter>
inline void PlotScatterEx(const char* label_id, Getter getter) {
    if (BeginItem(label_id, ImPlotCol_MarkerOutline)) {
        if (FitThisFrame()) {
            for (int i = 0; i < getter.Count; ++i) {
                ImPlotPoint p = getter(i);
                FitPoint(p);
            }
        }
        const ImPlotItemStyle& s = GetItemStyle();
        ImDrawList& DrawList = *GetPlotDrawList();
        // render markers
        ImPlotMarker marker = s.Marker == ImPlotMarker_None ? ImPlotMarker_Circle : s.Marker;
        if (marker != ImPlotMarker_None) {
            const ImU32 col_line = ImGui::GetColorU32(s.Colors[ImPlotCol_MarkerOutline]);
            const ImU32 col_fill = ImGui::GetColorU32(s.Colors[ImPlotCol_MarkerFill]);
            switch (GetCurrentScale()) {
                case ImPlotScale_LinLin: RenderMarkers(getter, TransformerLinLin(), DrawList, marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
                case ImPlotScale_LogLin: RenderMarkers(getter, TransformerLogLin(), DrawList, marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
                case ImPlotScale_LinLog: RenderMarkers(getter, TransformerLinLog(), DrawList, marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
                case ImPlotScale_LogLog: RenderMarkers(getter, TransformerLogLog(), DrawList, marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
            }
        }
        EndItem();
    }
}

// float
void PlotScatter(const char* label_id, const float* values, int count, int offset, int stride) {
    GetterYs<float> getter(values,count,offset,stride);
    PlotScatterEx(label_id, getter);
}

void PlotScatter(const char* label_id, const float* xs, const float* ys, int count, int offset, int stride) {
    GetterXsYs<float> getter(xs,ys,count,offset,stride);
    return PlotScatterEx(label_id, getter);
}

void PlotScatter(const char* label_id, const ImVec2* data, int count, int offset) {
    GetterImVec2 getter(data, count, offset);
    return PlotScatterEx(label_id, getter);
}

// double
void PlotScatter(const char* label_id, const double* values, int count, int offset, int stride) {
    GetterYs<double> getter(values,count,offset,stride);
    PlotScatterEx(label_id, getter);
}

void PlotScatter(const char* label_id, const double* xs, const double* ys, int count, int offset, int stride) {
    GetterXsYs<double> getter(xs,ys,count,offset,stride);
    return PlotScatterEx(label_id, getter);
}

void PlotScatter(const char* label_id, const ImPlotPoint* data, int count, int offset) {
    GetterImPlotPoint getter(data, count, offset);
    return PlotScatterEx(label_id, getter);
}

// custom
void PlotScatter(const char* label_id, ImPlotPoint (*getter_func)(void* data, int idx), void* data, int count, int offset) {
    GetterFuncPtrImPlotPoint getter(getter_func,data, count, offset);
    return PlotScatterEx(label_id, getter);
}

//-----------------------------------------------------------------------------
// PLOT SHADED
//-----------------------------------------------------------------------------

template <typename Getter1, typename Getter2>
inline void PlotShadedEx(const char* label_id, Getter1 getter1, Getter2 getter2) {
    if (BeginItem(label_id, ImPlotCol_Fill)) {
        if (FitThisFrame()) {
            for (int i = 0; i < ImMin(getter1.Count, getter2.Count); ++i) {
                ImPlotPoint p1 = getter1(i);
                ImPlotPoint p2 = getter2(i);
                FitPoint(p1);
                FitPoint(p2);
            }
        }
        const ImPlotItemStyle& s = GetItemStyle();
        ImDrawList & DrawList = *GetPlotDrawList();
        if (s.RenderFill) {
            ImU32 col = ImGui::GetColorU32(s.Colors[ImPlotCol_Fill]);
            switch (GetCurrentScale()) {
                case ImPlotScale_LinLin: RenderPrimitives(ShadedRenderer<Getter1,Getter2,TransformerLinLin>(getter1,getter2,TransformerLinLin(), col), DrawList, GImPlot->BB_Plot); break;
                case ImPlotScale_LogLin: RenderPrimitives(ShadedRenderer<Getter1,Getter2,TransformerLogLin>(getter1,getter2,TransformerLogLin(), col), DrawList, GImPlot->BB_Plot); break;
                case ImPlotScale_LinLog: RenderPrimitives(ShadedRenderer<Getter1,Getter2,TransformerLinLog>(getter1,getter2,TransformerLinLog(), col), DrawList, GImPlot->BB_Plot); break;
                case ImPlotScale_LogLog: RenderPrimitives(ShadedRenderer<Getter1,Getter2,TransformerLogLog>(getter1,getter2,TransformerLogLog(), col), DrawList, GImPlot->BB_Plot); break;
            }
        }
        EndItem();
    }
}

// float
void PlotShaded(const char* label_id, const float* values, int count, float y_ref, int offset, int stride) {
    GetterYs<float> getter1(values,count,offset,stride);
    GetterYRef<float> getter2(y_ref, count);
    PlotShadedEx(label_id, getter1, getter2);}

void PlotShaded(const char* label_id, const float* xs, const float* ys1, const float* ys2, int count, int offset, int stride) {
    GetterXsYs<float> getter1(xs, ys1, count, offset, stride);
    GetterXsYs<float> getter2(xs, ys2, count, offset, stride);
    PlotShadedEx(label_id, getter1, getter2);
}

void PlotShaded(const char* label_id, const float* xs, const float* ys, int count, float y_ref, int offset, int stride) {
    GetterXsYs<float> getter1(xs, ys, count, offset, stride);
    GetterXsYRef<float> getter2(xs, y_ref, count, offset, stride);
    PlotShadedEx(label_id, getter1, getter2);
}

// double
void PlotShaded(const char* label_id, const double* values, int count, double y_ref, int offset, int stride) {
    GetterYs<double> getter1(values,count,offset,stride);
    GetterYRef<double> getter2(y_ref, count);
    PlotShadedEx(label_id, getter1, getter2);
}

void PlotShaded(const char* label_id, const double* xs, const double* ys1, const double* ys2, int count, int offset, int stride) {
    GetterXsYs<double> getter1(xs, ys1, count, offset, stride);
    GetterXsYs<double> getter2(xs, ys2, count, offset, stride);
    PlotShadedEx(label_id, getter1, getter2);
}

void PlotShaded(const char* label_id, const double* xs, const double* ys, int count, double y_ref, int offset, int stride) {
    GetterXsYs<double> getter1(xs, ys, count, offset, stride);
    GetterXsYRef<double> getter2(xs, y_ref, count, offset, stride);
    PlotShadedEx(label_id, getter1, getter2);
}

// custom
void PlotShaded(const char* label_id, ImPlotPoint (*g1)(void* data, int idx), void* data1, ImPlotPoint (*g2)(void* data, int idx), void* data2, int count, int offset) {
    GetterFuncPtrImPlotPoint getter1(g1, data1, count, offset);
    GetterFuncPtrImPlotPoint getter2(g2, data2, count, offset);
    PlotShadedEx(label_id, getter1, getter2);
}

//-----------------------------------------------------------------------------
// PLOT BAR V
//-----------------------------------------------------------------------------

// TODO: Migrate to RenderPrimitives

template <typename Getter, typename TWidth>
void PlotBarsEx(const char* label_id, Getter getter, TWidth width) {
    if (BeginItem(label_id, ImPlotCol_Fill)) {
        const TWidth half_width = width / 2;
        if (FitThisFrame()) {
            for (int i = 0; i < getter.Count; ++i) {
                ImPlotPoint p = getter(i);
                FitPoint(ImPlotPoint(p.x - half_width, p.y));
                FitPoint(ImPlotPoint(p.x + half_width, 0));
            }
        }
        const ImPlotItemStyle& s = GetItemStyle();
        ImDrawList& DrawList = *GetPlotDrawList();
        ImU32 col_line  = ImGui::GetColorU32(s.Colors[ImPlotCol_Line]);
        ImU32 col_fill  = ImGui::GetColorU32(s.Colors[ImPlotCol_Fill]);
        bool  rend_line = s.RenderLine;
        if (s.RenderFill && col_line == col_fill)
            rend_line = false;
        for (int i = 0; i < getter.Count; ++i) {
            ImPlotPoint p = getter(i);
            if (p.y == 0)
                continue;
            ImVec2 a = PlotToPixels(p.x - half_width, p.y);
            ImVec2 b = PlotToPixels(p.x + half_width, 0);
            if (s.RenderFill)
                DrawList.AddRectFilled(a, b, col_fill);
            if (rend_line)
                DrawList.AddRect(a, b, col_line, 0, ImDrawCornerFlags_All, s.LineWeight);
        }
        EndItem();
    }
}

// float
void PlotBars(const char* label_id, const float* values, int count, float width, float shift, int offset, int stride) {
    GetterBarV<float> getter(values,shift,count,offset,stride);
    PlotBarsEx(label_id, getter, width);
}

void PlotBars(const char* label_id, const float* xs, const float* ys, int count, float width, int offset, int stride) {
    GetterXsYs<float> getter(xs,ys,count,offset,stride);
    PlotBarsEx(label_id, getter, width);
}

// double
void PlotBars(const char* label_id, const double* values, int count, double width, double shift, int offset, int stride) {
    GetterBarV<double> getter(values,shift,count,offset,stride);
    PlotBarsEx(label_id, getter, width);
}

void PlotBars(const char* label_id, const double* xs, const double* ys, int count, double width, int offset, int stride) {
    GetterXsYs<double> getter(xs,ys,count,offset,stride);
    PlotBarsEx(label_id, getter, width);
}

// custom
void PlotBars(const char* label_id, ImPlotPoint (*getter_func)(void* data, int idx), void* data, int count, double width, int offset) {
    GetterFuncPtrImPlotPoint getter(getter_func, data, count, offset);
    PlotBarsEx(label_id, getter, width);
}

//-----------------------------------------------------------------------------
// PLOT BAR H
//-----------------------------------------------------------------------------

// TODO: Migrate to RenderPrimitives

template <typename Getter, typename THeight>
void PlotBarsHEx(const char* label_id, Getter getter, THeight height) {
    if (BeginItem(label_id, ImPlotCol_Fill)) {
        const THeight half_height = height / 2;
        if (FitThisFrame()) {
            for (int i = 0; i < getter.Count; ++i) {
                ImPlotPoint p = getter(i);
                FitPoint(ImPlotPoint(0, p.y - half_height));
                FitPoint(ImPlotPoint(p.x, p.y + half_height));
            }
        }
        const ImPlotItemStyle& s = GetItemStyle();
        ImDrawList& DrawList = *GetPlotDrawList();
        ImU32 col_line  = ImGui::GetColorU32(s.Colors[ImPlotCol_Line]);
        ImU32 col_fill  = ImGui::GetColorU32(s.Colors[ImPlotCol_Fill]);
        bool  rend_line = s.RenderLine;
        if (s.RenderFill && col_line == col_fill)
            rend_line = false;
        for (int i = 0; i < getter.Count; ++i) {
            ImPlotPoint p = getter(i);
            if (p.x == 0)
                continue;
            ImVec2 a = PlotToPixels(0, p.y - half_height);
            ImVec2 b = PlotToPixels(p.x, p.y + half_height);
            if (s.RenderFill)
                DrawList.AddRectFilled(a, b, col_fill);
            if (rend_line)
                DrawList.AddRect(a, b, col_line, 0, ImDrawCornerFlags_All, s.LineWeight);
        }
        EndItem();
    }
}

// float
void PlotBarsH(const char* label_id, const float* values, int count, float height, float shift, int offset, int stride) {
    GetterBarH<float> getter(values,shift,count,offset,stride);
    PlotBarsHEx(label_id, getter, height);
}

void PlotBarsH(const char* label_id, const float* xs, const float* ys, int count, float height,  int offset, int stride) {
    GetterXsYs<float> getter(xs,ys,count,offset,stride);
    PlotBarsHEx(label_id, getter, height);
}

// double
void PlotBarsH(const char* label_id, const double* values, int count, double height, double shift, int offset, int stride) {
    GetterBarH<double> getter(values,shift,count,offset,stride);
    PlotBarsHEx(label_id, getter, height);
}

void PlotBarsH(const char* label_id, const double* xs, const double* ys, int count, double height,  int offset, int stride) {
    GetterXsYs<double> getter(xs,ys,count,offset,stride);
    PlotBarsHEx(label_id, getter, height);
}

// custom
void PlotBarsH(const char* label_id, ImPlotPoint (*getter_func)(void* data, int idx), void* data, int count, double height,  int offset) {
    GetterFuncPtrImPlotPoint getter(getter_func, data, count, offset);
    PlotBarsHEx(label_id, getter, height);
}

//-----------------------------------------------------------------------------
// PLOT ERROR BARS
//-----------------------------------------------------------------------------

template <typename Getter>
void PlotErrorBarsEx(const char* label_id, Getter getter) {
    if (BeginItem(label_id)) {
        if (FitThisFrame()) {
            for (int i = 0; i < getter.Count; ++i) {
                ImPlotPointError e = getter(i);
                FitPoint(ImPlotPoint(e.X , e.Y - e.Neg));
                FitPoint(ImPlotPoint(e.X , e.Y + e.Pos ));
            }
        }
        const ImPlotItemStyle& s = GetItemStyle();
        ImDrawList& DrawList = *GetPlotDrawList();
        const ImU32 col = ImGui::GetColorU32(s.Colors[ImPlotCol_ErrorBar]);
        const bool rend_whisker  = s.ErrorBarSize > 0;
        const float half_whisker = s.ErrorBarSize * 0.5f;
        for (int i = 0; i < getter.Count; ++i) {
            ImPlotPointError e = getter(i);
            ImVec2 p1 = PlotToPixels(e.X, e.Y - e.Neg);
            ImVec2 p2 = PlotToPixels(e.X, e.Y + e.Pos);
            DrawList.AddLine(p1,p2,col, s.ErrorBarWeight);
            if (rend_whisker) {
                DrawList.AddLine(p1 - ImVec2(half_whisker, 0), p1 + ImVec2(half_whisker, 0), col, s.ErrorBarWeight);
                DrawList.AddLine(p2 - ImVec2(half_whisker, 0), p2 + ImVec2(half_whisker, 0), col, s.ErrorBarWeight);
            }
        }
        EndItem();
    }
}

// float
void PlotErrorBars(const char* label_id, const float* xs, const float* ys, const float* err, int count, int offset, int stride) {
    GetterError<float> getter(xs, ys, err, err, count, offset, stride);
    PlotErrorBarsEx(label_id, getter);
}

void PlotErrorBars(const char* label_id, const float* xs, const float* ys, const float* neg, const float* pos, int count, int offset, int stride) {
    GetterError<float> getter(xs, ys, neg, pos, count, offset, stride);
    PlotErrorBarsEx(label_id, getter);
}

// double
void PlotErrorBars(const char* label_id, const double* xs, const double* ys, const double* err, int count, int offset, int stride) {
    GetterError<double> getter(xs, ys, err, err, count, offset, stride);
    PlotErrorBarsEx(label_id, getter);
}

void PlotErrorBars(const char* label_id, const double* xs, const double* ys, const double* neg, const double* pos, int count, int offset, int stride) {
    GetterError<double> getter(xs, ys, neg, pos, count, offset, stride);
    PlotErrorBarsEx(label_id, getter);
}

//-----------------------------------------------------------------------------
// PLOT ERROR BARS H
//-----------------------------------------------------------------------------

template <typename Getter>
void PlotErrorBarsHEx(const char* label_id, Getter getter) {
    if (BeginItem(label_id)) {
        if (FitThisFrame()) {
            for (int i = 0; i < getter.Count; ++i) {
                ImPlotPointError e = getter(i);
                FitPoint(ImPlotPoint(e.X - e.Neg, e.Y));
                FitPoint(ImPlotPoint(e.X + e.Pos, e.Y));
            }
        }
        const ImPlotItemStyle& s = GetItemStyle();
        ImDrawList& DrawList = *GetPlotDrawList();
        const ImU32 col = ImGui::GetColorU32(s.Colors[ImPlotCol_ErrorBar]);
        const bool rend_whisker  = s.ErrorBarSize > 0;
        const float half_whisker = s.ErrorBarSize * 0.5f;
        for (int i = 0; i < getter.Count; ++i) {
            ImPlotPointError e = getter(i);
            ImVec2 p1 = PlotToPixels(e.X - e.Neg, e.Y);
            ImVec2 p2 = PlotToPixels(e.X + e.Pos, e.Y);
            DrawList.AddLine(p1, p2, col, s.ErrorBarWeight);
            if (rend_whisker) {
                DrawList.AddLine(p1 - ImVec2(0, half_whisker), p1 + ImVec2(0, half_whisker), col, s.ErrorBarWeight);
                DrawList.AddLine(p2 - ImVec2(0, half_whisker), p2 + ImVec2(0, half_whisker), col, s.ErrorBarWeight);
            }
        }
        EndItem();
    }
}

// float
void PlotErrorBarsH(const char* label_id, const float* xs, const float* ys, const float* err, int count, int offset, int stride) {
    GetterError<float> getter(xs, ys, err, err, count, offset, stride);
    PlotErrorBarsHEx(label_id, getter);
}

void PlotErrorBarsH(const char* label_id, const float* xs, const float* ys, const float* neg, const float* pos, int count, int offset, int stride) {
    GetterError<float> getter(xs, ys, neg, pos, count, offset, stride);
    PlotErrorBarsHEx(label_id, getter);
}

// double
void PlotErrorBarsH(const char* label_id, const double* xs, const double* ys, const double* err, int count, int offset, int stride) {
    GetterError<double> getter(xs, ys, err, err, count, offset, stride);
    PlotErrorBarsHEx(label_id, getter);
}

void PlotErrorBarsH(const char* label_id, const double* xs, const double* ys, const double* neg, const double* pos, int count, int offset, int stride) {
    GetterError<double> getter(xs, ys, neg, pos, count, offset, stride);
    PlotErrorBarsHEx(label_id, getter);
}

//-----------------------------------------------------------------------------
// PLOT STEMS
//-----------------------------------------------------------------------------

template <typename GetterM, typename GetterB>
inline void PlotStemsEx(const char* label_id, GetterM get_mark, GetterB get_base) {
    if (BeginItem(label_id, ImPlotCol_Line)) {
        if (FitThisFrame()) {
            for (int i = 0; i < get_base.Count; ++i) {
                FitPoint(get_mark(i));
                FitPoint(get_base(i));
            }
        }
        const ImPlotItemStyle& s = GetItemStyle();
        ImDrawList& DrawList = *GetPlotDrawList();
        // render stems
        if (s.RenderLine) {
            const ImU32 col_line = ImGui::GetColorU32(s.Colors[ImPlotCol_Line]);
            switch (GetCurrentScale()) {
                case ImPlotScale_LinLin: RenderLineSegments(get_mark, get_base, TransformerLinLin(), DrawList, s.LineWeight, col_line); break;
                case ImPlotScale_LogLin: RenderLineSegments(get_mark, get_base, TransformerLogLin(), DrawList, s.LineWeight, col_line); break;
                case ImPlotScale_LinLog: RenderLineSegments(get_mark, get_base, TransformerLinLog(), DrawList, s.LineWeight, col_line); break;
                case ImPlotScale_LogLog: RenderLineSegments(get_mark, get_base, TransformerLogLog(), DrawList, s.LineWeight, col_line); break;
            }
        }
        // render markers
        ImPlotMarker marker = s.Marker == ImPlotMarker_None ? ImPlotMarker_Circle : s.Marker;
        if (marker != ImPlotMarker_None) {
            const ImU32 col_line = ImGui::GetColorU32(s.Colors[ImPlotCol_MarkerOutline]);
            const ImU32 col_fill = ImGui::GetColorU32(s.Colors[ImPlotCol_MarkerFill]);
            switch (GetCurrentScale()) {
                case ImPlotScale_LinLin: RenderMarkers(get_mark, TransformerLinLin(), DrawList, marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
                case ImPlotScale_LogLin: RenderMarkers(get_mark, TransformerLogLin(), DrawList, marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
                case ImPlotScale_LinLog: RenderMarkers(get_mark, TransformerLinLog(), DrawList, marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
                case ImPlotScale_LogLog: RenderMarkers(get_mark, TransformerLogLog(), DrawList, marker, s.MarkerSize, s.RenderMarkerLine, col_line, s.MarkerWeight, s.RenderMarkerFill, col_fill); break;
            }
        }
        EndItem();
    }
}

void PlotStems(const char* label_id, const float* values, int count, float y_ref, int offset, int stride) {
    GetterYs<float> get_mark(values,count,offset,stride);
    GetterYRef<float> get_base(y_ref,count);
    PlotStemsEx(label_id, get_mark, get_base);
}

void PlotStems(const char* label_id, const double* values, int count, double y_ref, int offset, int stride) {
    GetterYs<double> get_mark(values,count,offset,stride);
    GetterYRef<double> get_base(y_ref,count);
    PlotStemsEx(label_id, get_mark, get_base);
}

void PlotStems(const char* label_id, const float* xs, const float* ys, int count, float y_ref, int offset, int stride) {
    GetterXsYs<float> get_mark(xs,ys,count,offset,stride);
    GetterXsYRef<float> get_base(xs,y_ref,count,offset,stride);
    PlotStemsEx(label_id, get_mark, get_base);
}

void PlotStems(const char* label_id, const double* xs, const double* ys, int count, double y_ref, int offset, int stride) {
    GetterXsYs<double> get_mark(xs,ys,count,offset,stride);
    GetterXsYRef<double> get_base(xs,y_ref,count,offset,stride);
    PlotStemsEx(label_id, get_mark, get_base);
}

//-----------------------------------------------------------------------------
// PLOT PIE CHART
//-----------------------------------------------------------------------------

inline void RenderPieSlice(ImDrawList& DrawList, const ImPlotPoint& center, double radius, double a0, double a1, ImU32 col) {
    static const float resolution = 50 / (2 * IM_PI);
    static ImVec2 buffer[50];
    buffer[0] = PlotToPixels(center);
    int n = ImMax(3, (int)((a1 - a0) * resolution));
    double da = (a1 - a0) / (n - 1);
    for (int i = 0; i < n; ++i) {
        double a = a0 + i * da;
        buffer[i + 1] = PlotToPixels(center.x + radius * cos(a), center.y + radius * sin(a));
    }
    DrawList.AddConvexPolyFilled(buffer, n + 1, col);
}

template <typename T>
void PlotPieChartEx(const char** label_ids, const T* values, int count, T x, T y, T radius, bool normalize, const char* fmt, T angle0) {
    IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot != NULL, "PlotPieChart() needs to be called between BeginPlot() and EndPlot()!");
    ImDrawList & DrawList = *GetPlotDrawList();
    T sum = 0;
    for (int i = 0; i < count; ++i)
        sum += values[i];
    normalize = normalize || sum > 1.0f;
    ImPlotPoint center(x,y);
    PushPlotClipRect();
    T a0 = angle0 * 2 * IM_PI / 360.0f;
    T a1 = angle0 * 2 * IM_PI / 360.0f;
    for (int i = 0; i < count; ++i) {
        T percent = normalize ? values[i] / sum : values[i];
        a1 = a0 + 2 * IM_PI * percent;
        if (BeginItem(label_ids[i])) {
            ImU32 col = ImGui::GetColorU32(GetCurrentItem()->Color);
            if (percent < 0.5) {
                RenderPieSlice(DrawList, center, radius, a0, a1, col);
            }
            else  {
                RenderPieSlice(DrawList, center, radius, a0, a0 + (a1 - a0) * 0.5f, col);
                RenderPieSlice(DrawList, center, radius, a0 + (a1 - a0) * 0.5f, a1, col);
            }
            EndItem();
        }
        a0 = a1;
    }
    if (fmt != NULL) {
        a0 = angle0 * 2 * IM_PI / 360.0f;
        a1 = angle0 * 2 * IM_PI / 360.0f;
        char buffer[32];
        for (int i = 0; i < count; ++i) {
            ImPlotItem* item = GetItem(label_ids[i]);
            T percent = normalize ? values[i] / sum : values[i];
            a1 = a0 + 2 * IM_PI * percent;
            if (item->Show) {
                sprintf(buffer, fmt, values[i]);
                ImVec2 size = ImGui::CalcTextSize(buffer);
                T angle = a0 + (a1 - a0) * 0.5f;
                ImVec2 pos = PlotToPixels(center.x + 0.5f * radius * cos(angle), center.y + 0.5f * radius * sin(angle));
                ImU32 col = CalcTextColor(item->Color);
                DrawList.AddText(pos - size * 0.5f, col, buffer);
            }
            a0 = a1;
        }
    }
    PopPlotClipRect();
}

// float
void PlotPieChart(const char** label_ids, const float* values, int count, float x, float y, float radius, bool normalize, const char* fmt, float angle0) {
    return PlotPieChartEx(label_ids, values, count, x, y, radius, normalize, fmt, angle0);
}

// double
void PlotPieChart(const char** label_ids, const double* values, int count, double x, double y, double radius, bool normalize, const char* fmt, double angle0) {
    return PlotPieChartEx(label_ids, values, count, x, y, radius, normalize, fmt, angle0);
}

//-----------------------------------------------------------------------------
// PLOT HEATMAP
//-----------------------------------------------------------------------------

template <typename T, typename Transformer>
void RenderHeatmap(Transformer transformer, ImDrawList& DrawList, const T* values, int rows, int cols, T scale_min, T scale_max, const char* fmt, const ImPlotPoint& bounds_min, const ImPlotPoint& bounds_max) {
    ImPlotContext& gp = *GImPlot;
    const double w = (bounds_max.x - bounds_min.x) / cols;
    const double h = (bounds_max.y - bounds_min.y) / rows;
    const ImPlotPoint half_size(w*0.5,h*0.5);
    int i = 0;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            ImPlotPoint p;
            p.x = bounds_min.x + 0.5*w + c*w;
            p.y = bounds_max.y - (0.5*h + r*h);
            ImVec2 a  = transformer(p.x - half_size.x, p.y - half_size.y);
            ImVec2 b  = transformer(p.x + half_size.x, p.y + half_size.y);
            float t = (float)ImRemap(values[i], scale_min, scale_max, T(0), T(1));
            ImVec4 color = LerpColormap(t);
            color.w *= gp.Style.FillAlpha;
            ImU32 col = ImGui::GetColorU32(color);
            DrawList.AddRectFilled(a, b, col);
            i++;
        }
    }
    if (fmt != NULL) {
        i = 0;
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                ImPlotPoint p;
                p.x = bounds_min.x + 0.5*w + c*w;
                p.y = bounds_min.y + 1 - (0.5*h + r*h);
                ImVec2 px = transformer(p);
                char buff[32];
                sprintf(buff, fmt, values[i]);
                ImVec2 size = ImGui::CalcTextSize(buff);
                float t = (float)ImRemap(values[i], scale_min, scale_max, T(0), T(1));
                ImVec4 color = LerpColormap(t);
                ImU32 col = CalcTextColor(color);
                DrawList.AddText(px - size * 0.5f, col, buff);
                i++;
            }
        }
    }
}

template <typename T>
void PlotHeatmapEx(const char* label_id, const T* values, int rows, int cols, T scale_min, T scale_max, const char* fmt, const ImPlotPoint& bounds_min, const ImPlotPoint& bounds_max) {
    IM_ASSERT_USER_ERROR(scale_min != scale_max, "Scale values must be different!");
    if (BeginItem(label_id)) {
        if (FitThisFrame()) {
            FitPoint(bounds_min);
            FitPoint(bounds_max);
        }
        ImDrawList& DrawList = *GetPlotDrawList();
        switch (GetCurrentScale()) {
            case ImPlotScale_LinLin: RenderHeatmap(TransformerLinLin(), DrawList, values, rows, cols, scale_min, scale_max, fmt, bounds_min, bounds_max); break;
            case ImPlotScale_LogLin: RenderHeatmap(TransformerLogLin(), DrawList, values, rows, cols, scale_min, scale_max, fmt, bounds_min, bounds_max); break;
            case ImPlotScale_LinLog: RenderHeatmap(TransformerLinLog(), DrawList, values, rows, cols, scale_min, scale_max, fmt, bounds_min, bounds_max); break;
            case ImPlotScale_LogLog: RenderHeatmap(TransformerLogLog(), DrawList, values, rows, cols, scale_min, scale_max, fmt, bounds_min, bounds_max); break;
        }
        EndItem();
    }
}

// float
void PlotHeatmap(const char* label_id, const float* values, int rows, int cols, float scale_min, float scale_max, const char* fmt, const ImPlotPoint& bounds_min, const ImPlotPoint& bounds_max) {
    return PlotHeatmapEx(label_id, values, rows, cols, scale_min, scale_max, fmt, bounds_min, bounds_max);
}

// double
void PlotHeatmap(const char* label_id, const double* values, int rows, int cols, double scale_min, double scale_max, const char* fmt, const ImPlotPoint& bounds_min, const ImPlotPoint& bounds_max) {
    return PlotHeatmapEx(label_id, values, rows, cols, scale_min, scale_max, fmt, bounds_min, bounds_max);
}

//-----------------------------------------------------------------------------
// PLOT DIGITAL
//-----------------------------------------------------------------------------

// TODO: Make this behave like all the other plot types

template <typename Getter>
inline void PlotDigitalEx(const char* label_id, Getter getter) {
    if (BeginItem(label_id, ImPlotCol_Fill)) {
        ImPlotContext& gp = *GImPlot;
        ImDrawList& DrawList = *GetPlotDrawList();
        const ImPlotItemStyle& s = GetItemStyle();
        if (getter.Count > 1 && s.RenderFill) {
            const int y_axis = GetCurrentYAxis();
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
                ImVec2 pMin = PlotToPixels(itemData1);
                ImVec2 pMax = PlotToPixels(itemData2);
                int pixY_Offset = 20; //20 pixel from bottom due to mouse cursor label
                pMin.y = (gp.PixelRange[y_axis].Min.y) + ((-gp.DigitalPlotOffset)                   - pixY_Offset);
                pMax.y = (gp.PixelRange[y_axis].Min.y) + ((-gp.DigitalPlotOffset) - pixY_0 - pixY_1 - pixY_Offset);
                //plot only one rectangle for same digital state
                while (((i+2) < getter.Count) && (itemData1.y == itemData2.y)) {
                    const int in = (i + 1);
                    itemData2 = getter(in);
                    if (ImNanOrInf(itemData2.y)) break;
                    pMax.x = PlotToPixels(itemData2).x;
                    i++;
                }
                //do not extend plot outside plot range
                if (pMin.x < gp.PixelRange[y_axis].Min.x) pMin.x = gp.PixelRange[y_axis].Min.x;
                if (pMax.x < gp.PixelRange[y_axis].Min.x) pMax.x = gp.PixelRange[y_axis].Min.x;
                if (pMin.x > gp.PixelRange[y_axis].Max.x) pMin.x = gp.PixelRange[y_axis].Max.x;
                if (pMax.x > gp.PixelRange[y_axis].Max.x) pMax.x = gp.PixelRange[y_axis].Max.x;
                //plot a rectangle that extends up to x2 with y1 height
                if ((pMax.x > pMin.x) && (gp.BB_Plot.Contains(pMin) || gp.BB_Plot.Contains(pMax))) {
                    // ImVec4 colAlpha = item->Color;
                    // colAlpha.w = item->Highlight ? 1.0f : 0.9f;
                    DrawList.AddRectFilled(pMin, pMax, ImGui::GetColorU32(s.Colors[ImPlotCol_Fill]));
                }
                itemData1 = itemData2;
            }
            gp.DigitalPlotItemCnt++;
            gp.DigitalPlotOffset += pixYMax;
        }
        EndItem();
    }
}

// float
void PlotDigital(const char* label_id, const float* xs, const float* ys, int count, int offset, int stride) {
    GetterXsYs<float> getter(xs,ys,count,offset,stride);
    return PlotDigitalEx(label_id, getter);
}

// double
void PlotDigital(const char* label_id, const double* xs, const double* ys, int count, int offset, int stride) {
    GetterXsYs<double> getter(xs,ys,count,offset,stride);
    return PlotDigitalEx(label_id, getter);
}

// custom
void PlotDigital(const char* label_id, ImPlotPoint (*getter_func)(void* data, int idx), void* data, int count, int offset) {
    GetterFuncPtrImPlotPoint getter(getter_func,data,count,offset);
    return PlotDigitalEx(label_id, getter);
}

//-----------------------------------------------------------------------------
// PLOT RECTS
//-----------------------------------------------------------------------------
template <typename Getter>
void PlotRectsEx(const char* label_id, Getter getter) {
    if (BeginItem(label_id, ImPlotCol_Fill)) {
        if (FitThisFrame()) {
            for (int i = 0; i < getter.Count; ++i) {
                ImPlotPoint p = getter(i);
                FitPoint(p);
            }
        }
        const ImPlotItemStyle& s = GetItemStyle();
        if (s.RenderFill) {
            ImDrawList& DrawList = *GetPlotDrawList();
            ImU32 col = ImGui::GetColorU32(s.Colors[ImPlotCol_Fill]);
            switch (GetCurrentScale()) {
                case ImPlotScale_LinLin: RenderPrimitives(RectRenderer<Getter,TransformerLinLin>(getter, TransformerLinLin(), col), DrawList, GImPlot->BB_Plot); break;
                case ImPlotScale_LogLin: RenderPrimitives(RectRenderer<Getter,TransformerLogLin>(getter, TransformerLogLin(), col), DrawList, GImPlot->BB_Plot); break;
                case ImPlotScale_LinLog: RenderPrimitives(RectRenderer<Getter,TransformerLinLog>(getter, TransformerLinLog(), col), DrawList, GImPlot->BB_Plot); break;
                case ImPlotScale_LogLog: RenderPrimitives(RectRenderer<Getter,TransformerLogLog>(getter, TransformerLogLog(), col), DrawList, GImPlot->BB_Plot); break;
            }
        }
        EndItem();
    }
}

// float
void PlotRects(const char* label_id, const float* xs, const float* ys, int count, int offset, int stride) {
    GetterXsYs<float> getter(xs,ys,count,offset,stride);
    PlotRectsEx(label_id, getter);
}

// double
void PlotRects(const char* label_id, const double* xs, const double* ys, int count, int offset, int stride) {
    GetterXsYs<double> getter(xs,ys,count,offset,stride);
    PlotRectsEx(label_id, getter);
}

// custom
void PlotRects(const char* label_id, ImPlotPoint (*getter_func)(void* data, int idx), void* data, int count, int offset) {
    GetterFuncPtrImPlotPoint getter(getter_func,data,count,offset);
    return PlotRectsEx(label_id, getter);
}

//-----------------------------------------------------------------------------
// PLOT TEXT
//-----------------------------------------------------------------------------

// float
void PlotText(const char* text, float x, float y, bool vertical, const ImVec2& pixel_offset) {
    return PlotText(text, (double)x, (double)y, vertical, pixel_offset);
}

// double
void PlotText(const char* text, double x, double y, bool vertical, const ImVec2& pixel_offset) {
    IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot != NULL, "PlotText() needs to be called between BeginPlot() and EndPlot()!");
    ImDrawList & DrawList = *GetPlotDrawList();
    PushPlotClipRect();
    ImU32 colTxt = GetStyleColorU32(ImPlotCol_InlayText);
    if (vertical) {
        ImVec2 ctr = CalcTextSizeVertical(text) * 0.5f;
        ImVec2 pos = PlotToPixels(ImPlotPoint(x,y)) + ImVec2(-ctr.x, ctr.y) + pixel_offset;
        AddTextVertical(&DrawList, pos, colTxt, text);
    }
    else {
        ImVec2 pos = PlotToPixels(ImPlotPoint(x,y)) - ImGui::CalcTextSize(text) * 0.5f + pixel_offset;
        DrawList.AddText(pos, colTxt, text);
    }
    PopPlotClipRect();
}

} // namespace ImPlot