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
        return ImPlotPoint((T)idx, OffsetAndStride(Ys, idx, Count, Offset, Stride));
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
        return ImPlotPoint(OffsetAndStride(Xs, idx, Count, Offset, Stride), OffsetAndStride(Ys, idx, Count, Offset, Stride));
    }
};

// Always returns a constant Y reference value where the X value is the index
template <typename T>
struct GetterYRef {
    GetterYRef(T y_ref, int count) { YRef = y_ref; Count = count; }
    inline ImPlotPoint operator()(int idx) {
        return ImPlotPoint((T)idx, YRef);
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
        return ImPlotPoint(OffsetAndStride(Xs, idx, Count, Offset, Stride), YRef);
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
        return ImPlotPoint(Data[idx].x, Data[idx].y);
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
    inline ImPlotPoint operator()(int idx) { return ImPlotPoint((T)idx + XShift, OffsetAndStride(Ys, idx, Count, Offset, Stride)); }
};

template <typename T>
struct GetterBarH {
    const T* Xs; T YShift; int Count; int Offset; int Stride;
    GetterBarH(const T* xs, T yshift, int count, int offset, int stride) { Xs = xs; YShift = yshift; Count = count; Offset = offset; Stride = stride; }
    inline ImPlotPoint operator()(int idx) { return ImPlotPoint(OffsetAndStride(Xs, idx, Count, Offset, Stride), (T)idx + YShift); }
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

// Transforms convert points in plot space (i.e. ImPlotPoint) to pixel space (i.e. ImVec2)

// Transforms points for linear x and linear y space
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

// Transforms points for log x and linear y space
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

// Transforms points for linear x and log y space
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

// Transforms points for log x and log y space
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
// PRIMITIVE RENDERERS
//-----------------------------------------------------------------------------

template <typename TGetter, typename TTransformer>
struct LineRenderer {
    inline LineRenderer(TGetter getter, TTransformer transformer, ImU32 col, float weight) :
        Getter(getter),
        Transformer(transformer)
    {
        Prims = Getter.Count - 1;
        Col = col;
        Weight = weight;
        P1 = Transformer(Getter(0));
    }
    inline bool operator()(ImDrawList& DrawList, ImVec2 uv, int prim) {
        ImPlotContext& gp = *GImPlot;
        ImVec2 P2 = Transformer(Getter(prim + 1));
        if (!gp.BB_Plot.Overlaps(ImRect(ImMin(P1, P2), ImMax(P1, P2)))) {
            P1 = P2;
            return false;
        }
        float dx = P2.x - P1.x;
        float dy = P2.y - P1.y;
        IM_NORMALIZE2F_OVER_ZERO(dx, dy);
        dx *= (Weight * 0.5f);
        dy *= (Weight * 0.5f);
        DrawList._VtxWritePtr[0].pos.x = P1.x + dy;
        DrawList._VtxWritePtr[0].pos.y = P1.y - dx;
        DrawList._VtxWritePtr[0].uv    = uv;
        DrawList._VtxWritePtr[0].col   = Col;
        DrawList._VtxWritePtr[1].pos.x = P2.x + dy;
        DrawList._VtxWritePtr[1].pos.y = P2.y - dx;
        DrawList._VtxWritePtr[1].uv    = uv;
        DrawList._VtxWritePtr[1].col   = Col;
        DrawList._VtxWritePtr[2].pos.x = P2.x - dy;
        DrawList._VtxWritePtr[2].pos.y = P2.y + dx;
        DrawList._VtxWritePtr[2].uv    = uv;
        DrawList._VtxWritePtr[2].col   = Col;
        DrawList._VtxWritePtr[3].pos.x = P1.x - dy;
        DrawList._VtxWritePtr[3].pos.y = P1.y + dx;
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

    inline bool operator()(ImDrawList& DrawList, ImVec2 uv, int prim) {
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
    inline bool operator()(ImDrawList& DrawList, ImVec2 uv, int prim) {
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
inline void RenderPrimitives(Renderer renderer, ImDrawList& DrawList) {
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
            if (!renderer(DrawList, uv, idx))
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

//-----------------------------------------------------------------------------
// MARKER RENDERERS
//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------
// PLOT LINES / MARKERS
//-----------------------------------------------------------------------------

template <typename Getter>
inline void PlotEx(const char* label_id, Getter getter)
{
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotEx() needs to be called between BeginPlot() and EndPlot()!");

    ImPlotItem* item = RegisterOrGetItem(label_id);
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

    ImPlotItem* item = RegisterOrGetItem(label_id);
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

template <typename Getter, typename TWidth>
void PlotBarsEx(const char* label_id, Getter getter, TWidth width) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotBars() needs to be called between BeginPlot() and EndPlot()!");

    ImPlotItem* item = RegisterOrGetItem(label_id);
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
            DrawList.AddRect(a, b, col_line, 0, ImDrawCornerFlags_All, gp.Style.LineWeight);
    }
    PopPlotClipRect();
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
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotBarsH() needs to be called between BeginPlot() and EndPlot()!");

    ImPlotItem* item = RegisterOrGetItem(label_id);
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
            DrawList.AddRect(a, b, col_line, 0, ImDrawCornerFlags_All, gp.Style.LineWeight);
    }
    PopPlotClipRect();
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
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotErrorBars() needs to be called between BeginPlot() and EndPlot()!");

    ImPlotItem* item = RegisterOrGetItem(label_id);
    if (!item->Show)
        return;

    // find data extents
    if (gp.FitThisFrame) {
        for (int i = 0; i < getter.Count; ++i) {
            ImPlotPointError e = getter(i);
            FitPoint(ImPlotPoint(e.X , e.Y - e.Neg));
            FitPoint(ImPlotPoint(e.X , e.Y + e.Pos ));
        }
    }

    const ImU32 col = ImGui::GetColorU32(GetErrorBarColor());
    const bool rend_whisker = gp.Style.ErrorBarSize > 0;
    const float half_whisker = gp.Style.ErrorBarSize * 0.5f;

    ImDrawList & DrawList = *ImGui::GetWindowDrawList();

    PushPlotClipRect();
    for (int i = 0; i < getter.Count; ++i) {
        ImPlotPointError e = getter(i);
        ImVec2 p1 = PlotToPixels(e.X, e.Y - e.Neg);
        ImVec2 p2 = PlotToPixels(e.X, e.Y + e.Pos);
        DrawList.AddLine(p1,p2,col, gp.Style.ErrorBarWeight);
        if (rend_whisker) {
            DrawList.AddLine(p1 - ImVec2(half_whisker, 0), p1 + ImVec2(half_whisker, 0), col, gp.Style.ErrorBarWeight);
            DrawList.AddLine(p2 - ImVec2(half_whisker, 0), p2 + ImVec2(half_whisker, 0), col, gp.Style.ErrorBarWeight);
        }
    }
    PopPlotClipRect();
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
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotErrorBarsH() needs to be called between BeginPlot() and EndPlot()!");

    ImPlotItem* item = RegisterOrGetItem(label_id);
    if (!item->Show)
        return;

    // find data extents
    if (gp.FitThisFrame) {
        for (int i = 0; i < getter.Count; ++i) {
            ImPlotPointError e = getter(i);
            FitPoint(ImPlotPoint(e.X - e.Neg, e.Y));
            FitPoint(ImPlotPoint(e.X + e.Pos, e.Y));
        }
    }

    const ImU32 col = ImGui::GetColorU32(GetErrorBarColor());
    const bool rend_whisker = gp.Style.ErrorBarSize > 0;
    const float half_whisker = gp.Style.ErrorBarSize * 0.5f;

    ImDrawList& DrawList = *ImGui::GetWindowDrawList();

    PushPlotClipRect();
    for (int i = 0; i < getter.Count; ++i) {
        ImPlotPointError e = getter(i);
        ImVec2 p1 = PlotToPixels(e.X - e.Neg, e.Y);
        ImVec2 p2 = PlotToPixels(e.X + e.Pos, e.Y);
        DrawList.AddLine(p1, p2, col, gp.Style.ErrorBarWeight);
        if (rend_whisker) {
            DrawList.AddLine(p1 - ImVec2(0, half_whisker), p1 + ImVec2(0, half_whisker), col, gp.Style.ErrorBarWeight);
            DrawList.AddLine(p2 - ImVec2(0, half_whisker), p2 + ImVec2(0, half_whisker), col, gp.Style.ErrorBarWeight);
        }
    }
    PopPlotClipRect();
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
        ImPlotItem* item = RegisterOrGetItem(label_ids[i]);
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
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotHeatmap() needs to be called between BeginPlot() and EndPlot()!");
    IM_ASSERT_USER_ERROR(scale_min != scale_max, "Scale values must be different!");
    ImPlotItem* item = RegisterOrGetItem(label_id);
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

template <typename Getter>
inline void PlotDigitalEx(const char* label_id, Getter getter)
{
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotDigital() needs to be called between BeginPlot() and EndPlot()!");

    ImPlotItem* item = RegisterOrGetItem(label_id);
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
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotRects() needs to be called between BeginPlot() and EndPlot()!");

    ImPlotItem* item = RegisterOrGetItem(label_id);
    if (!item->Show)
        return;
    TryRecolorItem(item, ImPlotCol_Fill);

    if (!WillFillRender())
        return;

    if (gp.FitThisFrame) {
        for (int i = 0; i < getter.Count; ++i) {
            ImPlotPoint p = getter(i);
            FitPoint(p);
        }
    }

    ImDrawList & DrawList = *ImGui::GetWindowDrawList();
    ImPlotState* plot = gp.CurrentPlot;
    const int y_axis = plot->CurrentYAxis;
    ImU32 col = ImGui::GetColorU32(GetItemFillColor(item));

    PushPlotClipRect();
    if (ImHasFlag(plot->XAxis.Flags, ImPlotAxisFlags_LogScale) && ImHasFlag(plot->YAxis[y_axis].Flags, ImPlotAxisFlags_LogScale))
        RenderPrimitives(RectRenderer<Getter,TransformerLogLog>(getter, TransformerLogLog(y_axis), col), DrawList);
    else if (ImHasFlag(plot->XAxis.Flags, ImPlotAxisFlags_LogScale))
        RenderPrimitives(RectRenderer<Getter,TransformerLogLin>(getter, TransformerLogLin(y_axis), col), DrawList);
    else if (ImHasFlag(plot->YAxis[y_axis].Flags, ImPlotAxisFlags_LogScale))
        RenderPrimitives(RectRenderer<Getter,TransformerLinLog>(getter, TransformerLinLog(y_axis), col), DrawList);
    else
        RenderPrimitives(RectRenderer<Getter,TransformerLinLin>(getter, TransformerLinLin(y_axis), col), DrawList);
    PopPlotClipRect();
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
    ImDrawList & DrawList = *ImGui::GetWindowDrawList();
    PushPlotClipRect();
    ImU32 colTxt = ImGui::GetColorU32(ImGuiCol_Text);
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