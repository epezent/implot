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
// GETTERS
//-----------------------------------------------------------------------------

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
        return ImPlotPoint((T)idx, OffsetAndStride(Ys, idx, Count, Offset, Stride));
    }
};

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
        return ImPlotPoint(OffsetAndStride(Xs, idx, Count, Offset, Stride), OffsetAndStride(Ys, idx, Count, Offset, Stride));
    }
};

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
        return ImPlotPoint(OffsetAndStride(Xs, idx, Count, Offset, Stride), YRef);
    }
};

struct GetterImVec2 {
    GetterImVec2(const ImVec2* data, int count, int offset) {
        Data = data;
        Count = count;
        Offset = count ? ImPosMod(offset, count) : 0;
    }
    inline ImPlotPoint operator()(int idx) { return ImPlotPoint(Data[idx].x, Data[idx].y); }
    const ImVec2* Data;
    int Count;
    int Offset;
};

struct GetterImPlotPoint {
    GetterImPlotPoint(const ImPlotPoint* data, int count, int offset) {
        Data = data;
        Count = count;
        Offset = count ? ImPosMod(offset, count) : 0;
    }
    inline ImPlotPoint operator()(int idx) { return Data[idx]; }
    const ImPlotPoint* Data;
    int Count;
    int Offset;
};

struct GetterFuncPtrImPlotPoint {
    GetterFuncPtrImPlotPoint(ImPlotPoint (*g)(void* data, int idx), void* d, int count, int offset) {
        getter = g;
        Data = d;
        Count = count;
        Offset = count ? ImPosMod(offset, count) : 0;
    }
    inline ImPlotPoint operator()(int idx) { return getter(Data, idx); }
    ImPlotPoint (*getter)(void* data, int idx);
    void* Data;
    int Count;
    int Offset;
};

template <typename T>
struct GetterBarV {
    const T* Ys; T XShift; int Count; int Offset; int Stride;
    GetterBarV(const T* ys, T xshift, int count, int offset, int stride) { Ys = ys; XShift = xshift; Count = count; Offset = offset; Stride = stride; }
    inline ImPlotPoint operator()(int idx) { return ImPlotPoint((T)idx + XShift, OffsetAndStride(Ys, idx, Count, Offset, Stride)); }
};

template <typename T>
struct GetterBarH {
    const T* Xs; T YShift; int Count; int Offset; int Stride;
    GetterBarH(const T* xs, T yshift, int count, int offset, int stride) { Xs = xs; YShift = yshift; Count = count; Offset = offset; Stride = stride; }
    inline ImPlotPoint operator()(int idx) { return ImPlotPoint(OffsetAndStride(Xs, idx, Count, Offset, Stride), (T)idx + YShift); }
};

struct ImPlotPointError {
    ImPlotPointError(double _x, double _y, double _neg, double _pos) {
        x = _x; y = _y; neg = _neg; pos = _pos;
    }
    double x, y, neg, pos;
};

template <typename T>
struct GetterError {
    const T* Xs; const T* Ys; const T* Neg; const T* Pos; int Count; int Offset; int Stride;
    GetterError(const T* xs, const T* ys, const T* neg, const T* pos, int count, int offset, int stride) {
        Xs = xs; Ys = ys; Neg = neg; Pos = pos; Count = count; Offset = offset; Stride = stride;
    }
    ImPlotPointError operator()(int idx) {
        return ImPlotPointError(OffsetAndStride(Xs,  idx, Count, Offset, Stride),
                                OffsetAndStride(Ys,  idx, Count, Offset, Stride),
                                OffsetAndStride(Neg, idx, Count, Offset, Stride),
                                OffsetAndStride(Pos, idx, Count, Offset, Stride));
    }
};

//-----------------------------------------------------------------------------
// TRANSFORMERS
//-----------------------------------------------------------------------------

struct TransformerLinLin {
    TransformerLinLin(int y_axis) : YAxis(y_axis) {}

    inline ImVec2 operator()(const ImPlotPoint& plt) { return (*this)(plt.x, plt.y); }
    inline ImVec2 operator()(double x, double y) {
        ImPlotContext& gp = *GImPlot;
        return ImVec2( (float)(gp.PixelRange[YAxis].Min.x + gp.Mx * (x - gp.CurrentPlot->XAxis.Range.Min)),
                       (float)(gp.PixelRange[YAxis].Min.y + gp.My[YAxis] * (y - gp.CurrentPlot->YAxis[YAxis].Range.Min)) );
    }

    int YAxis;
};

struct TransformerLogLin {
    TransformerLogLin(int y_axis) : YAxis(y_axis) {}

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

struct TransformerLinLog {
    TransformerLinLog(int y_axis) : YAxis(y_axis) {}

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

struct TransformerLogLog {
    TransformerLogLog(int y_axis) : YAxis(y_axis) {}

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
// RENDERERS
//-----------------------------------------------------------------------------

/// Renders primitive shapes in bulk as efficiently as possible.
template <typename Renderer>
inline void RenderPrimitives(Renderer renderer, ImDrawList& DrawList) {
    unsigned int prims = renderer.Prims;
    unsigned int prims_culled = 0;
    unsigned int idx = 0;
    static const unsigned int max_idx = (unsigned int)(ImPow(2.0f, (float)(sizeof(ImDrawIdx) * 8)) - 1);
    const ImVec2 uv = DrawList._Data->TexUvWhitePixel;
    while (prims) {
        // find how many can be reserved up to end of current draw command's limit
        unsigned int cnt = (unsigned int)ImMin(prims, (max_idx - DrawList._VtxCurrentIdx) / Renderer::VtxConsumed);
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
            cnt = (unsigned int)ImMin(prims, (max_idx - 0/*DrawList._VtxCurrentIdx*/) / Renderer::VtxConsumed);
            DrawList.PrimReserve(cnt * Renderer::IdxConsumed, cnt * Renderer::VtxConsumed); // reserve new draw command
        }
        prims -= cnt;
        for (unsigned int ie = idx + cnt; idx != ie; ++idx) {
            if (!renderer(DrawList, uv, idx))
                prims_culled++;
        }
    }
    if (prims_culled > 0)
        DrawList.PrimUnreserve(prims_culled * Renderer::IdxConsumed, prims_culled * Renderer::VtxConsumed);
}

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

inline void MarkerCircle(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
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
    MarkerGeneral(DrawList, marker, 10, c, s, outline, col_outline, fill, col_fill, weight);
}

inline void MarkerDiamond(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[4] = {ImVec2(1, 0), ImVec2(0, -1), ImVec2(-1, 0), ImVec2(0, 1)};
    MarkerGeneral(DrawList, marker, 4, c, s, outline, col_outline, fill, col_fill, weight);
}

inline void MarkerSquare(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[4] = {ImVec2(SQRT_1_2,SQRT_1_2),ImVec2(SQRT_1_2,-SQRT_1_2),ImVec2(-SQRT_1_2,-SQRT_1_2),ImVec2(-SQRT_1_2,SQRT_1_2)};
    MarkerGeneral(DrawList, marker, 4, c, s, outline, col_outline, fill, col_fill, weight);
}

inline void MarkerUp(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[3] = {ImVec2(SQRT_3_2,0.5f),ImVec2(0,-1),ImVec2(-SQRT_3_2,0.5f)};
    MarkerGeneral(DrawList, marker, 3, c, s, outline, col_outline, fill, col_fill, weight);
}

inline void MarkerDown(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[3] = {ImVec2(SQRT_3_2,-0.5f),ImVec2(0,1),ImVec2(-SQRT_3_2,-0.5f)};
    MarkerGeneral(DrawList, marker, 3, c, s, outline, col_outline, fill, col_fill, weight);
}

inline void MarkerLeft(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[3] = {ImVec2(-1,0), ImVec2(0.5, SQRT_3_2), ImVec2(0.5, -SQRT_3_2)};
    MarkerGeneral(DrawList, marker, 3, c, s, outline, col_outline, fill, col_fill, weight);
}

inline void MarkerRight(ImDrawList& DrawList, const ImVec2& c, float s, bool outline, ImU32 col_outline, bool fill, ImU32 col_fill, float weight) {
    ImVec2 marker[3] = {ImVec2(1,0), ImVec2(-0.5, SQRT_3_2), ImVec2(-0.5, -SQRT_3_2)};
    MarkerGeneral(DrawList, marker, 3, c, s, outline, col_outline, fill, col_fill, weight);
}

inline void MarkerAsterisk(ImDrawList& DrawList, const ImVec2& c, float s, bool /*outline*/, ImU32 col_outline, bool /*fill*/, ImU32 /*col_fill*/, float weight) {
    ImVec2 marker[6] = {ImVec2(SQRT_3_2, 0.5f), ImVec2(0, -1), ImVec2(-SQRT_3_2, 0.5f), ImVec2(SQRT_3_2, -0.5f), ImVec2(0, 1),  ImVec2(-SQRT_3_2, -0.5f)};
    TransformMarker(marker, 6, c, s);
    DrawList.AddLine(marker[0], marker[5], col_outline, weight);
    DrawList.AddLine(marker[1], marker[4], col_outline, weight);
    DrawList.AddLine(marker[2], marker[3], col_outline, weight);
}

inline void MarkerPlus(ImDrawList& DrawList, const ImVec2& c, float s, bool /*outline*/, ImU32 col_outline, bool /*fill*/, ImU32 /*col_fill*/, float weight) {
    ImVec2 marker[4] = {ImVec2(1, 0), ImVec2(0, -1), ImVec2(-1, 0), ImVec2(0, 1)};
    TransformMarker(marker, 4, c, s);
    DrawList.AddLine(marker[0], marker[2], col_outline, weight);
    DrawList.AddLine(marker[1], marker[3], col_outline, weight);
}

inline void MarkerCross(ImDrawList& DrawList, const ImVec2& c, float s, bool /*outline*/, ImU32 col_outline, bool /*fill*/, ImU32 /*col_fill*/, float weight) {
    ImVec2 marker[4] = {ImVec2(SQRT_1_2,SQRT_1_2),ImVec2(SQRT_1_2,-SQRT_1_2),ImVec2(-SQRT_1_2,-SQRT_1_2),ImVec2(-SQRT_1_2,SQRT_1_2)};
    TransformMarker(marker, 4, c, s);
    DrawList.AddLine(marker[0], marker[2], col_outline, weight);
    DrawList.AddLine(marker[1], marker[3], col_outline, weight);
}

template <typename Transformer, typename Getter>
inline void RenderMarkers(Getter getter, Transformer transformer, ImDrawList& DrawList, bool rend_mk_line, ImU32 col_mk_line, bool rend_mk_fill, ImU32 col_mk_fill) {
    ImPlotContext& gp = *GImPlot;
    for (int i = 0; i < getter.Count; ++i) {
        ImVec2 c = transformer(getter(i));
        if (gp.BB_Plot.Contains(c)) {
            // TODO: Optimize the loop and if statements, this is atrocious
            if (ImHasFlag(gp.Style.Marker, ImPlotMarker_Circle))
                MarkerCircle(DrawList, c, gp.Style.MarkerSize, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, gp.Style.MarkerWeight);
            if (ImHasFlag(gp.Style.Marker, ImPlotMarker_Square))
                MarkerSquare(DrawList, c, gp.Style.MarkerSize, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, gp.Style.MarkerWeight);
            if (ImHasFlag(gp.Style.Marker, ImPlotMarker_Diamond))
                MarkerDiamond(DrawList, c, gp.Style.MarkerSize, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, gp.Style.MarkerWeight);
            if (ImHasFlag(gp.Style.Marker, ImPlotMarker_Up))
                MarkerUp(DrawList, c, gp.Style.MarkerSize, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, gp.Style.MarkerWeight);
            if (ImHasFlag(gp.Style.Marker, ImPlotMarker_Down))
                MarkerDown(DrawList, c, gp.Style.MarkerSize, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, gp.Style.MarkerWeight);
            if (ImHasFlag(gp.Style.Marker, ImPlotMarker_Left))
                MarkerLeft(DrawList, c, gp.Style.MarkerSize, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, gp.Style.MarkerWeight);
            if (ImHasFlag(gp.Style.Marker, ImPlotMarker_Right))
                MarkerRight(DrawList, c, gp.Style.MarkerSize, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, gp.Style.MarkerWeight);
            if (ImHasFlag(gp.Style.Marker, ImPlotMarker_Cross))
                MarkerCross(DrawList, c, gp.Style.MarkerSize, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, gp.Style.MarkerWeight);
            if (ImHasFlag(gp.Style.Marker, ImPlotMarker_Plus))
                MarkerPlus(DrawList, c, gp.Style.MarkerSize, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, gp.Style.MarkerWeight);
            if (ImHasFlag(gp.Style.Marker, ImPlotMarker_Asterisk))
                MarkerAsterisk(DrawList, c, gp.Style.MarkerSize, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill, gp.Style.MarkerWeight);
        }
    }
}

template <typename Getter, typename Transformer>
struct LineRenderer {
    inline LineRenderer(Getter _getter, Transformer _transformer, ImU32 col, float weight) :
        getter(_getter),
        transformer(_transformer)
    {
        Prims = getter.Count - 1;
        Col = col;
        Weight = weight;
        p1 = transformer(getter(0));
    }
    inline bool operator()(ImDrawList& DrawList, ImVec2 uv, int prim) {
        ImPlotContext& gp = *GImPlot;
        ImVec2 p2 = transformer(getter(prim + 1));
        if (!gp.BB_Plot.Overlaps(ImRect(ImMin(p1, p2), ImMax(p1, p2)))) {
            p1 = p2;
            return false;
        }
        float dx = p2.x - p1.x;
        float dy = p2.y - p1.y;
        IM_NORMALIZE2F_OVER_ZERO(dx, dy);
        dx *= (Weight * 0.5f);
        dy *= (Weight * 0.5f);
        DrawList._VtxWritePtr[0].pos.x = p1.x + dy;
        DrawList._VtxWritePtr[0].pos.y = p1.y - dx;
        DrawList._VtxWritePtr[0].uv    = uv;
        DrawList._VtxWritePtr[0].col   = Col;
        DrawList._VtxWritePtr[1].pos.x = p2.x + dy;
        DrawList._VtxWritePtr[1].pos.y = p2.y - dx;
        DrawList._VtxWritePtr[1].uv    = uv;
        DrawList._VtxWritePtr[1].col   = Col;
        DrawList._VtxWritePtr[2].pos.x = p2.x - dy;
        DrawList._VtxWritePtr[2].pos.y = p2.y + dx;
        DrawList._VtxWritePtr[2].uv    = uv;
        DrawList._VtxWritePtr[2].col   = Col;
        DrawList._VtxWritePtr[3].pos.x = p1.x - dy;
        DrawList._VtxWritePtr[3].pos.y = p1.y + dx;
        DrawList._VtxWritePtr[3].uv    = uv;
        DrawList._VtxWritePtr[3].col   = Col;
        DrawList._VtxWritePtr += 4;
        DrawList._IdxWritePtr[0] = (ImDrawIdx)(DrawList._VtxCurrentIdx);
        DrawList._IdxWritePtr[1] = (ImDrawIdx)(DrawList._VtxCurrentIdx + 1);
        DrawList._IdxWritePtr[2] = (ImDrawIdx)(DrawList._VtxCurrentIdx + 2);
        DrawList._IdxWritePtr[3] = (ImDrawIdx)(DrawList._VtxCurrentIdx);
        DrawList._IdxWritePtr[4] = (ImDrawIdx)(DrawList._VtxCurrentIdx + 2);
        DrawList._IdxWritePtr[5] = (ImDrawIdx)(DrawList._VtxCurrentIdx + 3);
        DrawList._IdxWritePtr += 6;
        DrawList._VtxCurrentIdx += 4;
        p1 = p2;
        return true;
    }
    Getter getter;
    Transformer transformer;
    int Prims;
    ImU32 Col;
    float Weight;
    ImVec2 p1;
    static const int IdxConsumed = 6;
    static const int VtxConsumed = 4;
};

template <typename Getter, typename Transformer>
inline void RenderLineStrip(Getter getter, Transformer transformer, ImDrawList& DrawList, float line_weight, ImU32 col) {
    ImPlotContext& gp = *GImPlot;
    if (ImHasFlag(gp.CurrentPlot->Flags, ImPlotFlags_AntiAliased)) {
        ImVec2 p1 = transformer(getter(0));
        for (int i = 0; i < getter.Count; ++i) {
            ImVec2 p2 = transformer(getter(i));
            if (gp.BB_Plot.Overlaps(ImRect(ImMin(p1, p2), ImMax(p1, p2))))
                DrawList.AddLine(p1, p2, col, line_weight);
            p1 = p2;
        }
    }
    else {
        RenderPrimitives(LineRenderer<Getter,Transformer>(getter, transformer, col, line_weight), DrawList);
    }
}

template <typename Getter1, typename Getter2, typename Transformer>
struct ShadedRenderer {
    ShadedRenderer(Getter1 _getter1, Getter2 _getter2, Transformer _transformer, ImU32 col) :
        getter1(_getter1),
        getter2(_getter2),
        transformer(_transformer),
        Col(col)
    {
        Prims = ImMin(getter1.Count, getter2.Count) - 1;
        p11 = transformer(getter1(0));
        p12 = transformer(getter2(0));
    }

    inline bool operator()(ImDrawList& DrawList, ImVec2 uv, int prim) {
        ImVec2 p21 = transformer(getter1(prim+1));
        ImVec2 p22 = transformer(getter2(prim+1));
        const int intersect = (p11.y > p12.y && p22.y > p21.y) || (p12.y > p11.y && p21.y > p22.y);
        ImVec2 intersection = Intersection(p11,p21,p12,p22);
        DrawList._VtxWritePtr[0].pos = p11;
        DrawList._VtxWritePtr[0].uv  = uv;
        DrawList._VtxWritePtr[0].col = Col;
        DrawList._VtxWritePtr[1].pos = p21;
        DrawList._VtxWritePtr[1].uv  = uv;
        DrawList._VtxWritePtr[1].col = Col;
        DrawList._VtxWritePtr[2].pos = intersection;
        DrawList._VtxWritePtr[2].uv  = uv;
        DrawList._VtxWritePtr[2].col = Col;
        DrawList._VtxWritePtr[3].pos = p12;
        DrawList._VtxWritePtr[3].uv  = uv;
        DrawList._VtxWritePtr[3].col = Col;
        DrawList._VtxWritePtr[4].pos = p22;
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
        p11 = p21;
        p12 = p22;
        return true;
    }
    Getter1 getter1;
    Getter2 getter2;
    Transformer transformer;
    int Prims;
    ImU32 Col;
    ImVec2 p11, p12;
    static const int IdxConsumed = 6;
    static const int VtxConsumed = 5;
};

//-----------------------------------------------------------------------------
// RENDERING UTILS
//-----------------------------------------------------------------------------

// Returns true if a style color is set to be automaticaly determined
inline bool ColorIsAuto(ImPlotCol idx) {
    ImPlotContext& gp = *GImPlot;
    return gp.Style.Colors[idx].w == -1;
}

// Recolors an item from an the current ImPlotCol if it is not automatic (i.e. alpha != -1)
inline void TryRecolorItem(ImPlotItem* item, ImPlotCol idx) {
    ImPlotContext& gp = *GImPlot;
    if (gp.Style.Colors[idx].w != -1)
        item->Color = gp.Style.Colors[idx];
}

// Returns true if lines will render (e.g. basic lines, bar outlines)
inline bool WillLineRender() {
    ImPlotContext& gp = *GImPlot;
    return gp.Style.Colors[ImPlotCol_Line].w != 0 && gp.Style.LineWeight > 0;
}

// Returns true if fills will render (e.g. shaded plots, bar fills)
inline bool WillFillRender() {
    ImPlotContext& gp = *GImPlot;
    return gp.Style.Colors[ImPlotCol_Fill].w != 0 && gp.Style.FillAlpha > 0;
}

// Returns true if marker outlines will render
inline bool WillMarkerOutlineRender() {
    ImPlotContext& gp = *GImPlot;
    return gp.Style.Colors[ImPlotCol_MarkerOutline].w != 0 && gp.Style.MarkerWeight > 0;
}

// Returns true if mark fill will render
inline bool WillMarkerFillRender() {
    ImPlotContext& gp = *GImPlot;
    return gp.Style.Colors[ImPlotCol_MarkerFill].w != 0 && gp.Style.FillAlpha > 0;
}

// Gets the line color for an item
inline ImVec4 GetLineColor(ImPlotItem* item) {
    ImPlotContext& gp = *GImPlot;
    return ColorIsAuto(ImPlotCol_Line) ? item->Color : gp.Style.Colors[ImPlotCol_Line];
}

// Gets the fill color for an item
inline ImVec4 GetItemFillColor(ImPlotItem* item) {
    ImPlotContext& gp = *GImPlot;
    ImVec4 col = ColorIsAuto(ImPlotCol_Fill) ? item->Color : gp.Style.Colors[ImPlotCol_Fill];
    col.w *= gp.Style.FillAlpha;
    return col;
}

// Gets the marker outline color for an item
inline ImVec4 GetMarkerOutlineColor(ImPlotItem* item) {
    ImPlotContext& gp = *GImPlot;
    return ColorIsAuto(ImPlotCol_MarkerOutline) ? GetLineColor(item) : gp.Style.Colors[ImPlotCol_MarkerOutline];
}

// Gets the marker fill color for an item
inline ImVec4 GetMarkerFillColor(ImPlotItem* item) {
    ImPlotContext& gp = *GImPlot;
    ImVec4 col = ColorIsAuto(ImPlotCol_MarkerFill) ?  GetLineColor(item) :gp.Style.Colors[ImPlotCol_MarkerFill];
    col.w *= gp.Style.FillAlpha;
    return col;
}

// Gets the error bar color
inline ImVec4 GetErrorBarColor() {
    ImPlotContext& gp = *GImPlot;
    return ColorIsAuto(ImPlotCol_ErrorBar) ? ImGui::GetStyleColorVec4(ImGuiCol_Text) : gp.Style.Colors[ImPlotCol_ErrorBar];
}

//-----------------------------------------------------------------------------
// PLOT LINES / MARKERS
//-----------------------------------------------------------------------------

template <typename Getter>
inline void PlotEx(const char* label_id, Getter getter)
{
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotEx() needs to be called between BeginPlot() and EndPlot()!");

    ImPlotItem* item = RegisterItem(label_id);
    if (!item->Show)
        return;
    TryRecolorItem(item, ImPlotCol_Line);

    // find data extents
    if (gp.FitThisFrame) {
        for (int i = 0; i < getter.Count; ++i) {
            ImPlotPoint p = getter(i);
            FitPoint(p);
        }
    }

    ImDrawList& DrawList = *ImGui::GetWindowDrawList();
    ImPlotState* plot = gp.CurrentPlot;
    const int y_axis = plot->CurrentYAxis;

    PushPlotClipRect();
    // render line
    if (getter.Count > 1 && WillLineRender()) {
        ImU32 col_line = ImGui::GetColorU32(GetLineColor(item));
        const float line_weight = item->Highlight ? gp.Style.LineWeight * 2 : gp.Style.LineWeight;
        if (ImHasFlag(plot->XAxis.Flags, ImPlotAxisFlags_LogScale) && ImHasFlag(plot->YAxis[y_axis].Flags, ImPlotAxisFlags_LogScale))
            RenderLineStrip(getter, TransformerLogLog(y_axis), DrawList, line_weight, col_line);
        else if (ImHasFlag(plot->XAxis.Flags, ImPlotAxisFlags_LogScale))
            RenderLineStrip(getter, TransformerLogLin(y_axis), DrawList, line_weight, col_line);
        else if (ImHasFlag(plot->YAxis[y_axis].Flags, ImPlotAxisFlags_LogScale))
            RenderLineStrip(getter, TransformerLinLog(y_axis), DrawList, line_weight, col_line);
        else
            RenderLineStrip(getter, TransformerLinLin(y_axis), DrawList, line_weight, col_line);
    }
    // render markers
    if (gp.Style.Marker != ImPlotMarker_None) {
        const bool rend_mk_line = WillMarkerOutlineRender();
        const bool rend_mk_fill = WillMarkerFillRender();
        const ImU32 col_mk_line = ImGui::GetColorU32(GetMarkerOutlineColor(item));
        const ImU32 col_mk_fill = ImGui::GetColorU32(GetMarkerFillColor(item));
        if (ImHasFlag(plot->XAxis.Flags, ImPlotAxisFlags_LogScale) && ImHasFlag(plot->YAxis[y_axis].Flags, ImPlotAxisFlags_LogScale))
            RenderMarkers(getter, TransformerLogLog(y_axis), DrawList, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill);
        else if (ImHasFlag(plot->XAxis.Flags, ImPlotAxisFlags_LogScale))
            RenderMarkers(getter, TransformerLogLin(y_axis), DrawList, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill);
        else if (ImHasFlag(plot->YAxis[y_axis].Flags, ImPlotAxisFlags_LogScale))
            RenderMarkers(getter, TransformerLinLog(y_axis), DrawList, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill);
        else
            RenderMarkers(getter, TransformerLinLin(y_axis), DrawList, rend_mk_line, col_mk_line, rend_mk_fill, col_mk_fill);
    }
    PopPlotClipRect();
}

//-----------------------------------------------------------------------------
// float

void PlotLine(const char* label_id, const float* values, int count, int offset, int stride) {
    GetterYs<float> getter(values,count,offset,stride);
    PlotEx(label_id, getter);
}

void PlotLine(const char* label_id, const float* xs, const float* ys, int count, int offset, int stride) {
    GetterXsYs<float> getter(xs,ys,count,offset,stride);
    return PlotEx(label_id, getter);
}

void PlotLine(const char* label_id, const ImVec2* data, int count, int offset) {
    GetterImVec2 getter(data, count, offset);
    return PlotEx(label_id, getter);
}

//-----------------------------------------------------------------------------
// double

void PlotLine(const char* label_id, const double* values, int count, int offset, int stride) {
    GetterYs<double> getter(values,count,offset,stride);
    PlotEx(label_id, getter);
}

void PlotLine(const char* label_id, const double* xs, const double* ys, int count, int offset, int stride) {
    GetterXsYs<double> getter(xs,ys,count,offset,stride);
    return PlotEx(label_id, getter);
}

void PlotLine(const char* label_id, const ImPlotPoint* data, int count, int offset) {
    GetterImPlotPoint getter(data, count, offset);
    return PlotEx(label_id, getter);
}

//-----------------------------------------------------------------------------
// custom

void PlotLine(const char* label_id, ImPlotPoint (*getter_func)(void* data, int idx), void* data, int count, int offset) {
    GetterFuncPtrImPlotPoint getter(getter_func,data, count, offset);
    return PlotEx(label_id, getter);
}

//-----------------------------------------------------------------------------
// PLOT SCATTER
//-----------------------------------------------------------------------------

inline int PushScatterStyle() {
    int vars = 1;
    PushStyleVar(ImPlotStyleVar_LineWeight, 0);
    if (GetStyle().Marker == ImPlotMarker_None) {
        PushStyleVar(ImPlotStyleVar_Marker, ImPlotMarker_Circle);
        vars++;
    }
    return vars;
}

//-----------------------------------------------------------------------------
// float

void PlotScatter(const char* label_id, const float* values, int count, int offset, int stride) {
    int vars = PushScatterStyle();
    PlotLine(label_id, values, count, offset, stride);
    PopStyleVar(vars);
}

void PlotScatter(const char* label_id, const float* xs, const float* ys, int count, int offset, int stride) {
    int vars = PushScatterStyle();
    PlotLine(label_id, xs, ys, count, offset, stride);
    PopStyleVar(vars);
}

void PlotScatter(const char* label_id, const ImVec2* data, int count, int offset) {
    int vars = PushScatterStyle();
    PlotLine(label_id, data, count, offset);
    PopStyleVar(vars);
}

//-----------------------------------------------------------------------------
// double

void PlotScatter(const char* label_id, const double* values, int count, int offset, int stride) {
    int vars = PushScatterStyle();
    PlotLine(label_id, values, count, offset, stride);
    PopStyleVar(vars);
}

void PlotScatter(const char* label_id, const double* xs, const double* ys, int count, int offset, int stride) {
    int vars = PushScatterStyle();
    PlotLine(label_id, xs, ys, count, offset, stride);
    PopStyleVar(vars);
}

void PlotScatter(const char* label_id, const ImPlotPoint* data, int count, int offset) {
    int vars = PushScatterStyle();
    PlotLine(label_id, data, count, offset);
    PopStyleVar(vars);
}

//-----------------------------------------------------------------------------
// custom

void PlotScatter(const char* label_id, ImPlotPoint (*getter)(void* data, int idx), void* data, int count, int offset) {
    int vars = PushScatterStyle();
    PlotLine(label_id, getter, data, count, offset);
    PopStyleVar(vars);
}

//-----------------------------------------------------------------------------
// PLOT SHADED
//-----------------------------------------------------------------------------

template <typename Getter1, typename Getter2>
inline void PlotShadedEx(const char* label_id, Getter1 getter1, Getter2 getter2) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotShaded() needs to be called between BeginPlot() and EndPlot()!");

    ImPlotItem* item = RegisterItem(label_id);
    if (!item->Show)
        return;
    TryRecolorItem(item, ImPlotCol_Fill);

    if (!WillFillRender())
        return;

    // find data extents
    if (gp.FitThisFrame) {
        for (int i = 0; i < ImMin(getter1.Count, getter2.Count); ++i) {
            ImPlotPoint p1 = getter1(i);
            ImPlotPoint p2 = getter2(i);
            FitPoint(p1);
            FitPoint(p2);
        }
    }

    ImDrawList & DrawList = *ImGui::GetWindowDrawList();
    ImPlotState* plot = gp.CurrentPlot;
    const int y_axis = plot->CurrentYAxis;

    ImU32 col = ImGui::GetColorU32(GetItemFillColor(item));

    PushPlotClipRect();
    if (ImHasFlag(plot->XAxis.Flags, ImPlotAxisFlags_LogScale) && ImHasFlag(plot->YAxis[y_axis].Flags, ImPlotAxisFlags_LogScale))
        RenderPrimitives(ShadedRenderer<Getter1,Getter2,TransformerLogLog>(getter1,getter2,TransformerLogLog(y_axis), col), DrawList);
    else if (ImHasFlag(plot->XAxis.Flags, ImPlotAxisFlags_LogScale))
        RenderPrimitives(ShadedRenderer<Getter1,Getter2,TransformerLogLin>(getter1,getter2,TransformerLogLin(y_axis), col), DrawList);
    else if (ImHasFlag(plot->YAxis[y_axis].Flags, ImPlotAxisFlags_LogScale))
        RenderPrimitives(ShadedRenderer<Getter1,Getter2,TransformerLinLog>(getter1,getter2,TransformerLinLog(y_axis), col), DrawList);
    else
        RenderPrimitives(ShadedRenderer<Getter1,Getter2,TransformerLinLin>(getter1,getter2,TransformerLinLin(y_axis), col), DrawList);
    PopPlotClipRect();
}

//-----------------------------------------------------------------------------
// float

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

//-----------------------------------------------------------------------------
// double

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

//-----------------------------------------------------------------------------
// PLOT BAR V
//-----------------------------------------------------------------------------

template <typename Getter, typename TWidth>
void PlotBarsEx(const char* label_id, Getter getter, TWidth width) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotBars() needs to be called between BeginPlot() and EndPlot()!");

    ImPlotItem* item = RegisterItem(label_id);
    if (!item->Show)
        return;
    TryRecolorItem(item, ImPlotCol_Fill);

    const TWidth half_width = width / 2;
    // find data extents
    if (gp.FitThisFrame) {
        for (int i = 0; i < getter.Count; ++i) {
            ImPlotPoint p = getter(i);
            FitPoint(ImPlotPoint(p.x - half_width, p.y));
            FitPoint(ImPlotPoint(p.x + half_width, 0));
        }
    }

    ImU32 col_line = ImGui::GetColorU32(GetLineColor(item));
    ImU32 col_fill = ImGui::GetColorU32(GetItemFillColor(item));
    const bool rend_fill = WillFillRender();
    bool rend_line       = WillLineRender();
    if (rend_fill && col_line == col_fill)
        rend_line = false;

    ImDrawList & DrawList = *ImGui::GetWindowDrawList();
    PushPlotClipRect();
    for (int i = 0; i < getter.Count; ++i) {
        ImPlotPoint p = getter(i);
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

//-----------------------------------------------------------------------------
// float

void PlotBars(const char* label_id, const float* values, int count, float width, float shift, int offset, int stride) {
    GetterBarV<float> getter(values,shift,count,offset,stride);
    PlotBarsEx(label_id, getter, width);
}

void PlotBars(const char* label_id, const float* xs, const float* ys, int count, float width, int offset, int stride) {
    GetterXsYs<float> getter(xs,ys,count,offset,stride);
    PlotBarsEx(label_id, getter, width);
}

//-----------------------------------------------------------------------------
// double

void PlotBars(const char* label_id, const double* values, int count, double width, double shift, int offset, int stride) {
    GetterBarV<double> getter(values,shift,count,offset,stride);
    PlotBarsEx(label_id, getter, width);
}

void PlotBars(const char* label_id, const double* xs, const double* ys, int count, double width, int offset, int stride) {
    GetterXsYs<double> getter(xs,ys,count,offset,stride);
    PlotBarsEx(label_id, getter, width);
}

//-----------------------------------------------------------------------------
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
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotBarsH() needs to be called between BeginPlot() and EndPlot()!");

    ImPlotItem* item = RegisterItem(label_id);
    if (!item->Show)
        return;
    TryRecolorItem(item, ImPlotCol_Fill);

    const THeight half_height = height / 2;
    // find data extents
    if (gp.FitThisFrame) {
        for (int i = 0; i < getter.Count; ++i) {
            ImPlotPoint p = getter(i);
            FitPoint(ImPlotPoint(0, p.y - half_height));
            FitPoint(ImPlotPoint(p.x, p.y + half_height));
        }
    }

    ImU32 col_line = ImGui::GetColorU32(GetLineColor(item));
    ImU32 col_fill = ImGui::GetColorU32(GetItemFillColor(item));
    const bool rend_fill = WillFillRender();
    bool rend_line       = WillLineRender();
    if (rend_fill && col_line == col_fill)
        rend_line = false;

    PushPlotClipRect();
    ImDrawList & DrawList = *ImGui::GetWindowDrawList();
    for (int i = 0; i < getter.Count; ++i) {
        ImPlotPoint p = getter(i);
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

//-----------------------------------------------------------------------------
// float

void PlotBarsH(const char* label_id, const float* values, int count, float height, float shift, int offset, int stride) {
    GetterBarH<float> getter(values,shift,count,offset,stride);
    PlotBarsHEx(label_id, getter, height);
}

void PlotBarsH(const char* label_id, const float* xs, const float* ys, int count, float height,  int offset, int stride) {
    GetterXsYs<float> getter(xs,ys,count,offset,stride);
    PlotBarsHEx(label_id, getter, height);
}

//-----------------------------------------------------------------------------
// double

void PlotBarsH(const char* label_id, const double* values, int count, double height, double shift, int offset, int stride) {
    GetterBarH<double> getter(values,shift,count,offset,stride);
    PlotBarsHEx(label_id, getter, height);
}

void PlotBarsH(const char* label_id, const double* xs, const double* ys, int count, double height,  int offset, int stride) {
    GetterXsYs<double> getter(xs,ys,count,offset,stride);
    PlotBarsHEx(label_id, getter, height);
}

//-----------------------------------------------------------------------------
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
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotErrorBars() needs to be called between BeginPlot() and EndPlot()!");

    ImPlotItem* item = RegisterItem(label_id);
    if (!item->Show)
        return;

    // find data extents
    if (gp.FitThisFrame) {
        for (int i = 0; i < getter.Count; ++i) {
            ImPlotPointError e = getter(i);
            FitPoint(ImPlotPoint(e.x , e.y - e.neg));
            FitPoint(ImPlotPoint(e.x , e.y + e.pos ));
        }
    }

    const ImU32 col = ImGui::GetColorU32(GetErrorBarColor());
    const bool rend_whisker = gp.Style.ErrorBarSize > 0;
    const float half_whisker = gp.Style.ErrorBarSize * 0.5f;

    ImDrawList & DrawList = *ImGui::GetWindowDrawList();

    PushPlotClipRect();
    for (int i = 0; i < getter.Count; ++i) {
        ImPlotPointError e = getter(i);
        ImVec2 p1 = PlotToPixels(e.x, e.y - e.neg);
        ImVec2 p2 = PlotToPixels(e.x, e.y + e.pos);
        DrawList.AddLine(p1,p2,col, gp.Style.ErrorBarWeight);
        if (rend_whisker) {
            DrawList.AddLine(p1 - ImVec2(half_whisker, 0), p1 + ImVec2(half_whisker, 0), col, gp.Style.ErrorBarWeight);
            DrawList.AddLine(p2 - ImVec2(half_whisker, 0), p2 + ImVec2(half_whisker, 0), col, gp.Style.ErrorBarWeight);
        }
    }
    PopPlotClipRect();
}

//-----------------------------------------------------------------------------
// float

void PlotErrorBars(const char* label_id, const float* xs, const float* ys, const float* err, int count, int offset, int stride) {
    GetterError<float> getter(xs, ys, err, err, count, offset, stride);
    PlotErrorBarsEx(label_id, getter);
}

void PlotErrorBars(const char* label_id, const float* xs, const float* ys, const float* neg, const float* pos, int count, int offset, int stride) {
    GetterError<float> getter(xs, ys, neg, pos, count, offset, stride);
    PlotErrorBarsEx(label_id, getter);
}

//-----------------------------------------------------------------------------
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
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotErrorBarsH() needs to be called between BeginPlot() and EndPlot()!");

    ImPlotItem* item = RegisterItem(label_id);
    if (!item->Show)
        return;

    // find data extents
    if (gp.FitThisFrame) {
        for (int i = 0; i < getter.Count; ++i) {
            ImPlotPointError e = getter(i);
            FitPoint(ImPlotPoint(e.x - e.neg, e.y));
            FitPoint(ImPlotPoint(e.x + e.pos, e.y));
        }
    }

    const ImU32 col = ImGui::GetColorU32(GetErrorBarColor());
    const bool rend_whisker = gp.Style.ErrorBarSize > 0;
    const float half_whisker = gp.Style.ErrorBarSize * 0.5f;

    ImDrawList& DrawList = *ImGui::GetWindowDrawList();

    PushPlotClipRect();
    for (int i = 0; i < getter.Count; ++i) {
        ImPlotPointError e = getter(i);
        ImVec2 p1 = PlotToPixels(e.x - e.neg, e.y);
        ImVec2 p2 = PlotToPixels(e.x + e.pos, e.y);
        DrawList.AddLine(p1, p2, col, gp.Style.ErrorBarWeight);
        if (rend_whisker) {
            DrawList.AddLine(p1 - ImVec2(0, half_whisker), p1 + ImVec2(0, half_whisker), col, gp.Style.ErrorBarWeight);
            DrawList.AddLine(p2 - ImVec2(0, half_whisker), p2 + ImVec2(0, half_whisker), col, gp.Style.ErrorBarWeight);
        }
    }
    PopPlotClipRect();
}

//-----------------------------------------------------------------------------
// float

void PlotErrorBarsH(const char* label_id, const float* xs, const float* ys, const float* err, int count, int offset, int stride) {
    GetterError<float> getter(xs, ys, err, err, count, offset, stride);
    PlotErrorBarsHEx(label_id, getter);
}

void PlotErrorBarsH(const char* label_id, const float* xs, const float* ys, const float* neg, const float* pos, int count, int offset, int stride) {
    GetterError<float> getter(xs, ys, neg, pos, count, offset, stride);
    PlotErrorBarsHEx(label_id, getter);
}

//-----------------------------------------------------------------------------
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
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotPieChart() needs to be called between BeginPlot() and EndPlot()!");
    ImDrawList & DrawList = *ImGui::GetWindowDrawList();

    T sum = 0;
    for (int i = 0; i < count; ++i)
        sum += values[i];

    normalize = normalize || sum > 1.0f;

    ImPlotPoint center(x,y);

    PushPlotClipRect();
    T a0 = angle0 * 2 * IM_PI / 360.0f;
    T a1 = angle0 * 2 * IM_PI / 360.0f;
    for (int i = 0; i < count; ++i) {
        ImPlotItem* item = RegisterItem(label_ids[i]);
        ImU32 col = ImGui::GetColorU32(GetItemFillColor(item));
        T percent = normalize ? values[i] / sum : values[i];
        a1 = a0 + 2 * IM_PI * percent;
        if (item->Show) {
            if (percent < 0.5) {
                RenderPieSlice(DrawList, center, radius, a0, a1, col);
            }
            else  {
                RenderPieSlice(DrawList, center, radius, a0, a0 + (a1 - a0) * 0.5f, col);
                RenderPieSlice(DrawList, center, radius, a0 + (a1 - a0) * 0.5f, a1, col);
            }
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
                ImU32 col = CalcTextColor(GetItemFillColor(item));
                DrawList.AddText(pos - size * 0.5f, col, buffer);
            }
            a0 = a1;
        }
    }
    PopPlotClipRect();
}

//-----------------------------------------------------------------------------
// float

void PlotPieChart(const char** label_ids, const float* values, int count, float x, float y, float radius, bool normalize, const char* fmt, float angle0) {
    return PlotPieChartEx(label_ids, values, count, x, y, radius, normalize, fmt, angle0);
}

//-----------------------------------------------------------------------------
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
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotHeatmap() needs to be called between BeginPlot() and EndPlot()!");
    IM_ASSERT_USER_ERROR(scale_min != scale_max, "Scale values must be different!");
    ImPlotItem* item = RegisterItem(label_id);
    if (!item->Show)
        return;
    if (gp.FitThisFrame) {
        FitPoint(bounds_min);
        FitPoint(bounds_max);
    }
    ImDrawList& DrawList = *ImGui::GetWindowDrawList();
    ImGui::PushClipRect(gp.BB_Plot.Min, gp.BB_Plot.Max, true);
    ImPlotState* plot = gp.CurrentPlot;
    int y_axis = plot->CurrentYAxis;
    if (ImHasFlag(plot->XAxis.Flags, ImPlotAxisFlags_LogScale) && ImHasFlag(plot->YAxis[y_axis].Flags, ImPlotAxisFlags_LogScale))
        RenderHeatmap(TransformerLogLog(y_axis), DrawList, values, rows, cols, scale_min, scale_max, fmt, bounds_min, bounds_max);
    else if (ImHasFlag(plot->XAxis.Flags, ImPlotAxisFlags_LogScale))
        RenderHeatmap(TransformerLogLin(y_axis), DrawList, values, rows, cols, scale_min, scale_max, fmt, bounds_min, bounds_max);
    else if (ImHasFlag(plot->YAxis[y_axis].Flags, ImPlotAxisFlags_LogScale))
        RenderHeatmap(TransformerLinLog(y_axis), DrawList, values, rows, cols, scale_min, scale_max, fmt, bounds_min, bounds_max);
    else
        RenderHeatmap(TransformerLinLin(y_axis), DrawList, values, rows, cols, scale_min, scale_max, fmt, bounds_min, bounds_max);
    ImGui::PopClipRect();
}

//-----------------------------------------------------------------------------
// float

void PlotHeatmap(const char* label_id, const float* values, int rows, int cols, float scale_min, float scale_max, const char* fmt, const ImPlotPoint& bounds_min, const ImPlotPoint& bounds_max) {
    return PlotHeatmapEx(label_id, values, rows, cols, scale_min, scale_max, fmt, bounds_min, bounds_max);
}

//-----------------------------------------------------------------------------
// double

void PlotHeatmap(const char* label_id, const double* values, int rows, int cols, double scale_min, double scale_max, const char* fmt, const ImPlotPoint& bounds_min, const ImPlotPoint& bounds_max) {
    return PlotHeatmapEx(label_id, values, rows, cols, scale_min, scale_max, fmt, bounds_min, bounds_max);
}

//-----------------------------------------------------------------------------
// PLOT DIGITAL
//-----------------------------------------------------------------------------

template <typename Getter>
inline void PlotDigitalEx(const char* label_id, Getter getter)
{
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotDigital() needs to be called between BeginPlot() and EndPlot()!");

    ImPlotItem* item = RegisterItem(label_id);
    if (!item->Show)
        return;
    TryRecolorItem(item, ImPlotCol_Line);

    // render digital signals as "pixel bases" rectangles
    PushPlotClipRect();
    if (getter.Count > 1 && WillLineRender()) {
        ImDrawList & DrawList = *ImGui::GetWindowDrawList();
        const float line_weight = item->Highlight ? gp.Style.LineWeight * 2 : gp.Style.LineWeight;
        const int y_axis = gp.CurrentPlot->CurrentYAxis;
        int pixYMax = 0;
        ImPlotPoint itemData1 = getter(0);
        for (int i = 0; i < getter.Count; ++i) {
            ImPlotPoint itemData2 = getter(i);
            if (NanOrInf(itemData1.y)) {
                itemData1 = itemData2;
                continue;
            }
            if (NanOrInf(itemData2.y)) itemData2.y = ConstrainNan(ConstrainInf(itemData2.y));
            int pixY_0 = (int)(line_weight);
            itemData1.y = ImMax(0.0, itemData1.y);
            float pixY_1_float = gp.Style.DigitalBitHeight * (float)itemData1.y;
            int pixY_1 = (int)(pixY_1_float); //allow only positive values
            int pixY_chPosOffset = (int)(ImMax(gp.Style.DigitalBitHeight, pixY_1_float) + gp.Style.DigitalBitGap);
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
                if (NanOrInf(itemData2.y)) break;
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
                ImVec4 colAlpha = item->Color;
                colAlpha.w = item->Highlight ? 1.0f : 0.9f;
                DrawList.AddRectFilled(pMin, pMax, ImGui::GetColorU32(colAlpha));
            }
            itemData1 = itemData2;
        }
        gp.DigitalPlotItemCnt++;
        gp.DigitalPlotOffset += pixYMax;
    }
    PopPlotClipRect();
}

//-----------------------------------------------------------------------------
// float

void PlotDigital(const char* label_id, const float* xs, const float* ys, int count, int offset, int stride) {
    GetterXsYs<float> getter(xs,ys,count,offset,stride);
    return PlotDigitalEx(label_id, getter);
}

//-----------------------------------------------------------------------------
// double

void PlotDigital(const char* label_id, const double* xs, const double* ys, int count, int offset, int stride) {
    GetterXsYs<double> getter(xs,ys,count,offset,stride);
    return PlotDigitalEx(label_id, getter);
}

//-----------------------------------------------------------------------------
// custom

void PlotDigital(const char* label_id, ImPlotPoint (*getter_func)(void* data, int idx), void* data, int count, int offset) {
    GetterFuncPtrImPlotPoint getter(getter_func,data,count,offset);
    return PlotDigitalEx(label_id, getter);
}

//-----------------------------------------------------------------------------
// PLOT TEXT
//-----------------------------------------------------------------------------
// float

void PlotText(const char* text, float x, float y, bool vertical, const ImVec2& pixel_offset) {
    return PlotText(text, (double)x, (double)y, vertical, pixel_offset);
}

//-----------------------------------------------------------------------------
// double
void PlotText(const char* text, double x, double y, bool vertical, const ImVec2& pixel_offset) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotText() needs to be called between BeginPlot() and EndPlot()!");
    ImDrawList & DrawList = *ImGui::GetWindowDrawList();
    PushPlotClipRect();
    ImVec2 pos = PlotToPixels(ImPlotPoint(x,y)) + pixel_offset;
    ImU32 colTxt = ImGui::GetColorU32(ImGuiCol_Text);
    if (vertical)
        AddTextVertical(&DrawList, text, pos, colTxt);
    else
        DrawList.AddText(pos, colTxt, text);
    PopPlotClipRect();
}

} // namespace ImPlot