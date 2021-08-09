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

// ImPlot v0.13 WIP

/*

API BREAKING CHANGES
====================
Occasionally introducing changes that are breaking the API. We try to make the breakage minor and easy to fix.
Below is a change-log of API breaking changes only. If you are using one of the functions listed, expect to have to fix some code.
When you are not sure about a old symbol or function name, try using the Search/Find function of your IDE to look for comments or references in all implot files.
You can read releases logs https://github.com/epezent/implot/releases for more details.


- 2021/08/XX (0.13) MAJOR API OVERHAUL! See #xxx for more details!
                    - RENAMED:
                      - ImPlotPoint                               -> ImPoint
                      - ImPlotRange                               -> ImLimits
                      - ImPlotLimits                              -> ImLimitsXY
                      - ImPlotYAxis_                              -> ImAxis_
                      - SetPlotYAxis                              -> SetAxis
                      - IsPlotXAxisHovered/IsPlotXYAxisHovered    -> IsAxisHovered
                      - BeginDragDropTargetX/BeginDragDropTargetY -> BeginDragDropTargetAxis
                      - BeginDragDropSourceX/BeginDragDropSourceY -> BeginDragDropSourceAxis
                      - BeginDragDropTarget                       -> BeginDragDropTargetPlot
                      - BeginDragDropSource                       -> BeginDragDropSourcePlot
                      - SetLegendLocation                         -> SetupLegend
                      - SetMousePosLocation                       -> SetupMouseText
                      - ImPlotFlags_NoHighlight                   -> ImPlotLegendFlags_NoHighlight
                      - ImPlotFlags_NoMousePos                    -> ImPlotFlags_NoMouseText
                      - ImPlotCol_XAxis, ImPlotCol_YAxis1, etc.   -> ImPlotCol_AxisText
                      - ImPlotCol_XAxisGrid, ImPlotCol_Y1AxisGrid -> ImPlotCol_AxisGrid
                    - OBSOLETED:
                      - BeginPlot (original signature)
                      - SetNextPlotLimits, SetNextPlotLimitsX, SetNextPlotLimitsY, LinkNextPlotLimits, FitNextPlotAxes, 
                        SetNextPlotTicksX, SetNextPlotTicksY,  SetNextPlotFormatX, SetNextPlotFormatY
                    - REMOVED:
                      - ImPlotOrientation, 
                      - GetPlotQuery, SetPlotQuery, IsPlotQueried, ImPlotCol_Query (use DragRect tool)
                    - MODIFIED:
                      - PixelsToPlot, PlotToPixels, GetPlotMousePos, GetPlotLimits, GetPlotSelection
- 2021/07/30 (0.12) - The offset argument of `PlotXG` functions was been removed. Implement offsetting in your getter callback instead.
- 2021/03/08 (0.9)  - SetColormap and PushColormap(ImVec4*) were removed. Use AddColormap for custom colormap support. LerpColormap was changed to SampleColormap.
                      ShowColormapScale was changed to ColormapScale and requires additional arguments.
- 2021/03/07 (0.9)  - The signature of ShowColormapScale was modified to accept a ImVec2 size.
- 2021/02/28 (0.9)  - BeginLegendDragDropSource was changed to BeginDragDropSourceItem with a number of other drag and drop improvements.
- 2021/01/18 (0.9)  - The default behavior for opening context menus was change from double right-click to single right-click. ImPlotInputMap and related functions were moved
                      to implot_internal.h due to its immaturity.
- 2020/10/16 (0.8)  - ImPlotStyleVar_InfoPadding was changed to ImPlotStyleVar_MousePosPadding
- 2020/09/10 (0.8)  - The single array versions of PlotLine, PlotScatter, PlotStems, and PlotShaded were given additional arguments for x-scale and x0.
- 2020/09/07 (0.8)  - Plotting functions which accept a custom getter function pointer have been post-fixed with a G (e.g. PlotLineG)
- 2020/09/06 (0.7)  - Several flags under ImPlotFlags and ImPlotAxisFlags were inverted (e.g. ImPlotFlags_Legend -> ImPlotFlags_NoLegend) so that the default flagset
                      is simply 0. This more closely matches ImGui's style and makes it easier to enable non-default but commonly used flags (e.g. ImPlotAxisFlags_Time).
- 2020/08/28 (0.5)  - ImPlotMarker_ can no longer be combined with bitwise OR, |. This features caused unecessary slow-down, and almost no one used it.
- 2020/08/25 (0.5)  - ImPlotAxisFlags_Scientific was removed. Logarithmic axes automatically uses scientific notation.
- 2020/08/17 (0.5)  - PlotText was changed so that text is centered horizontally and vertically about the desired point.
- 2020/08/16 (0.5)  - An ImPlotContext must be explicitly created and destroyed now with `CreateContext` and `DestroyContext`. Previously, the context was statically initialized in this source file.
- 2020/06/13 (0.4)  - The flags `ImPlotAxisFlag_Adaptive` and `ImPlotFlags_Cull` were removed. Both are now done internally by default.
- 2020/06/03 (0.3)  - The signature and behavior of PlotPieChart was changed so that data with sum less than 1 can optionally be normalized. The label format can now be specified as well.
- 2020/06/01 (0.3)  - SetPalette was changed to `SetColormap` for consistency with other plotting libraries. `RestorePalette` was removed. Use `SetColormap(ImPlotColormap_Default)`.
- 2020/05/31 (0.3)  - Plot functions taking custom ImVec2* getters were removed. Use the ImPlotPoint* getter versions instead.
- 2020/05/29 (0.3)  - The signature of ImPlotLimits::Contains was changed to take two doubles instead of ImVec2
- 2020/05/16 (0.2)  - All plotting functions were reverted to being prefixed with "Plot" to maintain a consistent VerbNoun style. `Plot` was split into `PlotLine`
                      and `PlotScatter` (however, `PlotLine` can still be used to plot scatter points as `Plot` did before.). `Bar` is not `PlotBars`, to indicate
                      that multiple bars will be plotted.
- 2020/05/13 (0.2)  - `ImMarker` was change to `ImPlotMarker` and `ImAxisFlags` was changed to `ImPlotAxisFlags`.
- 2020/05/11 (0.2)  - `ImPlotFlags_Selection` was changed to `ImPlotFlags_BoxSelect`
- 2020/05/11 (0.2)  - The namespace ImGui:: was replaced with ImPlot::. As a result, the following additional changes were made:
                      - Functions that were prefixed or decorated with the word "Plot" have been truncated. E.g., `ImGui::PlotBars` is now just `ImPlot::Bar`.
                        It should be fairly obvious what was what.
                      - Some functions have been given names that would have otherwise collided with the ImGui namespace. This has been done to maintain a consistent
                        style with ImGui. E.g., 'ImGui::PushPlotStyleVar` is now 'ImPlot::PushStyleVar'.
- 2020/05/10 (0.2)  - The following function/struct names were changes:
                     - ImPlotRange       -> ImPlotLimits
                     - GetPlotRange()    -> GetPlotLimits()
                     - SetNextPlotRange  -> SetNextPlotLimits
                     - SetNextPlotRangeX -> SetNextPlotLimitsX
                     - SetNextPlotRangeY -> SetNextPlotLimitsY
- 2020/05/10 (0.2)  - Plot queries are pixel based by default. Query rects that maintain relative plot position have been removed. This was done to support multi-y-axis.

*/

#include "implot.h"
#include "implot_internal.h"

#ifdef _MSC_VER
#define sprintf sprintf_s
#endif

// Support for pre-1.82 versions. Users on 1.82+ can use 0 (default) flags to mean "all corners" but in order to support older versions we are more explicit.
#if (IMGUI_VERSION_NUM < 18102) && !defined(ImDrawFlags_RoundCornersAll)
#define ImDrawFlags_RoundCornersAll ImDrawCornerFlags_All
#endif

// Global plot context
ImPlotContext* GImPlot = NULL;

// TODO: remove, weird
#define IMPLOT_ID_PLT 10030910
#define IMPLOT_ID_LEG 10030911
#define IMPLOT_ID_AAX 10030912
#define IMPLOT_ID_XAX IMPLOT_ID_AAX
#define IMPLOT_ID_YAX (IMPLOT_ID_AAX+IMPLOT_MAX_AXES)
#define IMPLOT_ID_ITM (IMPLOT_ID_AAX+IMPLOT_MAX_AXES*2)

//-----------------------------------------------------------------------------
// Struct Implementations
//-----------------------------------------------------------------------------

ImPlotInputMap::ImPlotInputMap() {
    PanButton             = ImGuiMouseButton_Left;
    PanMod                = ImGuiKeyModFlags_None;
    FitButton             = ImGuiMouseButton_Left;
    ContextMenuButton     = ImGuiMouseButton_Right;
    BoxSelectButton       = ImGuiMouseButton_Right;
    BoxSelectMod          = ImGuiKeyModFlags_None;
    BoxSelectCancelButton = ImGuiMouseButton_Left;
    HorizontalMod         = ImGuiKeyModFlags_Alt;
    VerticalMod           = ImGuiKeyModFlags_Shift;
}

ImPlotStyle::ImPlotStyle() {

    LineWeight         = 1;
    Marker             = ImPlotMarker_None;
    MarkerSize         = 4;
    MarkerWeight       = 1;
    FillAlpha          = 1;
    ErrorBarSize       = 5;
    ErrorBarWeight     = 1.5f;
    DigitalBitHeight   = 8;
    DigitalBitGap      = 4;

    PlotBorderSize     = 1;
    MinorAlpha         = 0.25f;
    MajorTickLen       = ImVec2(10,10);
    MinorTickLen       = ImVec2(5,5);
    MajorTickSize      = ImVec2(1,1);
    MinorTickSize      = ImVec2(1,1);
    MajorGridSize      = ImVec2(1,1);
    MinorGridSize      = ImVec2(1,1);
    PlotPadding        = ImVec2(10,10);
    LabelPadding       = ImVec2(5,5);
    LegendPadding      = ImVec2(10,10);
    LegendInnerPadding = ImVec2(5,5);
    LegendSpacing      = ImVec2(5,0);
    MousePosPadding    = ImVec2(10,10);
    AnnotationPadding  = ImVec2(2,2);
    FitPadding         = ImVec2(0,0);
    PlotDefaultSize    = ImVec2(400,300);
    PlotMinSize        = ImVec2(200,150);

    ImPlot::StyleColorsAuto(this);

    Colormap = ImPlotColormap_Deep;

    AntiAliasedLines = false;
    UseLocalTime     = false;
    Use24HourClock   = false;
    UseISO8601       = false;
}

//-----------------------------------------------------------------------------
// Style
//-----------------------------------------------------------------------------

namespace ImPlot {

const char* GetStyleColorName(ImPlotCol col) {
    static const char* col_names[ImPlotCol_COUNT] = {
        "Line",
        "Fill",
        "MarkerOutline",
        "MarkerFill",
        "ErrorBar",
        "FrameBg",
        "PlotBg",
        "PlotBorder",
        "LegendBg",
        "LegendBorder",
        "LegendText",
        "TitleText",
        "InlayText",
        "AxisText",
        "AxisGrid",
        "AxisHovered",
        "AxisActive",
        "Selection",
        "Crosshairs"
    };
    return col_names[col];
}

const char* GetMarkerName(ImPlotMarker marker) {
    switch (marker) {
        case ImPlotMarker_None:     return "None";
        case ImPlotMarker_Circle:   return "Circle";
        case ImPlotMarker_Square:   return "Square";
        case ImPlotMarker_Diamond:  return "Diamond";
        case ImPlotMarker_Up:       return "Up";
        case ImPlotMarker_Down:     return "Down";
        case ImPlotMarker_Left:     return "Left";
        case ImPlotMarker_Right:    return "Right";
        case ImPlotMarker_Cross:    return "Cross";
        case ImPlotMarker_Plus:     return "Plus";
        case ImPlotMarker_Asterisk: return "Asterisk";
        default:                    return "";
    }
}

ImVec4 GetAutoColor(ImPlotCol idx) {
    ImVec4 col(0,0,0,1);
    switch(idx) {
        case ImPlotCol_Line:          return col; // these are plot dependent!
        case ImPlotCol_Fill:          return col; // these are plot dependent!
        case ImPlotCol_MarkerOutline: return col; // these are plot dependent!
        case ImPlotCol_MarkerFill:    return col; // these are plot dependent!
        case ImPlotCol_ErrorBar:      return ImGui::GetStyleColorVec4(ImGuiCol_Text);
        case ImPlotCol_FrameBg:       return ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
        case ImPlotCol_PlotBg:        return ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        case ImPlotCol_PlotBorder:    return ImGui::GetStyleColorVec4(ImGuiCol_Border);
        case ImPlotCol_LegendBg:      return ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
        case ImPlotCol_LegendBorder:  return GetStyleColorVec4(ImPlotCol_PlotBorder);
        case ImPlotCol_LegendText:    return GetStyleColorVec4(ImPlotCol_InlayText);
        case ImPlotCol_TitleText:     return ImGui::GetStyleColorVec4(ImGuiCol_Text);
        case ImPlotCol_InlayText:     return ImGui::GetStyleColorVec4(ImGuiCol_Text);
        case ImPlotCol_AxisText:      return ImGui::GetStyleColorVec4(ImGuiCol_Text);
        case ImPlotCol_AxisGrid:      return GetStyleColorVec4(ImPlotCol_AxisText) * ImVec4(1,1,1,0.25f);
        case ImPlotCol_AxisHovered:   return ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
        case ImPlotCol_AxisActive:    return ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
        case ImPlotCol_Selection:     return ImVec4(1,1,0,1);
        case ImPlotCol_Crosshairs:    return GetStyleColorVec4(ImPlotCol_PlotBorder);
        default: return col;
    }
}

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
    { ImGuiDataType_Float, 1, (ImU32)IM_OFFSETOF(ImPlotStyle, FillAlpha)          }, // ImPlotStyleVar_FillAlpha
    { ImGuiDataType_Float, 1, (ImU32)IM_OFFSETOF(ImPlotStyle, ErrorBarSize)       }, // ImPlotStyleVar_ErrorBarSize
    { ImGuiDataType_Float, 1, (ImU32)IM_OFFSETOF(ImPlotStyle, ErrorBarWeight)     }, // ImPlotStyleVar_ErrorBarWeight
    { ImGuiDataType_Float, 1, (ImU32)IM_OFFSETOF(ImPlotStyle, DigitalBitHeight)   }, // ImPlotStyleVar_DigitalBitHeight
    { ImGuiDataType_Float, 1, (ImU32)IM_OFFSETOF(ImPlotStyle, DigitalBitGap)      }, // ImPlotStyleVar_DigitalBitGap

    { ImGuiDataType_Float, 1, (ImU32)IM_OFFSETOF(ImPlotStyle, PlotBorderSize)     }, // ImPlotStyleVar_PlotBorderSize
    { ImGuiDataType_Float, 1, (ImU32)IM_OFFSETOF(ImPlotStyle, MinorAlpha)         }, // ImPlotStyleVar_MinorAlpha
    { ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlotStyle, MajorTickLen)       }, // ImPlotStyleVar_MajorTickLen
    { ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlotStyle, MinorTickLen)       }, // ImPlotStyleVar_MinorTickLen
    { ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlotStyle, MajorTickSize)      }, // ImPlotStyleVar_MajorTickSize
    { ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlotStyle, MinorTickSize)      }, // ImPlotStyleVar_MinorTickSize
    { ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlotStyle, MajorGridSize)      }, // ImPlotStyleVar_MajorGridSize
    { ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlotStyle, MinorGridSize)      }, // ImPlotStyleVar_MinorGridSize
    { ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlotStyle, PlotPadding)        }, // ImPlotStyleVar_PlotPadding
    { ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlotStyle, LabelPadding)       }, // ImPlotStyleVar_LabelPaddine
    { ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlotStyle, LegendPadding)      }, // ImPlotStyleVar_LegendPadding
    { ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlotStyle, LegendInnerPadding) }, // ImPlotStyleVar_LegendInnerPadding
    { ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlotStyle, LegendSpacing)      }, // ImPlotStyleVar_LegendSpacing

    { ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlotStyle, MousePosPadding)    }, // ImPlotStyleVar_MousePosPadding
    { ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlotStyle, AnnotationPadding)  }, // ImPlotStyleVar_AnnotationPadding
    { ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlotStyle, FitPadding)         }, // ImPlotStyleVar_FitPadding
    { ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlotStyle, PlotDefaultSize)    }, // ImPlotStyleVar_PlotDefaultSize
    { ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlotStyle, PlotMinSize)        }  // ImPlotStyleVar_PlotMinSize
};

static const ImPlotStyleVarInfo* GetPlotStyleVarInfo(ImPlotStyleVar idx) {
    IM_ASSERT(idx >= 0 && idx < ImPlotStyleVar_COUNT);
    IM_ASSERT(IM_ARRAYSIZE(GPlotStyleVarInfo) == ImPlotStyleVar_COUNT);
    return &GPlotStyleVarInfo[idx];
}

//-----------------------------------------------------------------------------
// Generic Helpers
//-----------------------------------------------------------------------------

void AddTextVertical(ImDrawList *DrawList, ImVec2 pos, ImU32 col, const char *text_begin, const char* text_end) {
    // the code below is based loosely on ImFont::RenderText
    if (!text_end)
        text_end = text_begin + strlen(text_begin);
    ImGuiContext& g = *GImGui;
    ImFont* font = g.Font;
    // Align to be pixel perfect
    pos.x = IM_FLOOR(pos.x);
    pos.y = IM_FLOOR(pos.y);
    const float scale = g.FontSize / font->FontSize;
    const char* s = text_begin;
    int chars_exp = (int)(text_end - s);
    int chars_rnd = 0;
    const int vtx_count_max = chars_exp * 4;
    const int idx_count_max = chars_exp * 6;
    DrawList->PrimReserve(idx_count_max, vtx_count_max);
    while (s < text_end) {
        unsigned int c = (unsigned int)*s;
        if (c < 0x80) {
            s += 1;
        }
        else {
            s += ImTextCharFromUtf8(&c, s, text_end);
            if (c == 0) // Malformed UTF-8?
                break;
        }
        const ImFontGlyph * glyph = font->FindGlyph((ImWchar)c);
        if (glyph == NULL) {
            continue;
        }
        DrawList->PrimQuadUV(pos + ImVec2(glyph->Y0, -glyph->X0) * scale, pos + ImVec2(glyph->Y0, -glyph->X1) * scale,
                             pos + ImVec2(glyph->Y1, -glyph->X1) * scale, pos + ImVec2(glyph->Y1, -glyph->X0) * scale,
                             ImVec2(glyph->U0, glyph->V0), ImVec2(glyph->U1, glyph->V0),
                             ImVec2(glyph->U1, glyph->V1), ImVec2(glyph->U0, glyph->V1),
                             col);
        pos.y -= glyph->AdvanceX * scale;
        chars_rnd++;
    }
    // Give back unused vertices
    int chars_skp = chars_exp-chars_rnd;
    DrawList->PrimUnreserve(chars_skp*6, chars_skp*4);
}

void AddTextCentered(ImDrawList* DrawList, ImVec2 top_center, ImU32 col, const char* text_begin, const char* text_end) {
    float txt_ht = ImGui::GetTextLineHeight();
    const char* title_end = ImGui::FindRenderedTextEnd(text_begin, text_end);
    ImVec2 text_size;
    float  y = 0;
    while (const char* tmp = (const char*)memchr(text_begin, '\n', title_end-text_begin)) {
        text_size = ImGui::CalcTextSize(text_begin,tmp,true);
        DrawList->AddText(ImVec2(top_center.x - text_size.x * 0.5f, top_center.y+y),col,text_begin,tmp);
        text_begin = tmp + 1;
        y += txt_ht;
    }
    text_size = ImGui::CalcTextSize(text_begin,title_end,true);
    DrawList->AddText(ImVec2(top_center.x - text_size.x * 0.5f, top_center.y+y),col,text_begin,title_end);
}

double NiceNum(double x, bool round) {
    double f;
    double nf;
    int expv = (int)floor(ImLog10(x));
    f = x / ImPow(10.0, (double)expv);
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

//-----------------------------------------------------------------------------
// Context Utils
//-----------------------------------------------------------------------------

void SetImGuiContext(ImGuiContext* ctx) {
    ImGui::SetCurrentContext(ctx);
}

ImPlotContext* CreateContext() {
    ImPlotContext* ctx = IM_NEW(ImPlotContext)();
    Initialize(ctx);
    if (GImPlot == NULL)
        SetCurrentContext(ctx);
    return ctx;
}

void DestroyContext(ImPlotContext* ctx) {
    if (ctx == NULL)
        ctx = GImPlot;
    if (GImPlot == ctx)
        SetCurrentContext(NULL);
    IM_DELETE(ctx);
}

ImPlotContext* GetCurrentContext() {
    return GImPlot;
}

void SetCurrentContext(ImPlotContext* ctx) {
    GImPlot = ctx;
}

#define IMPLOT_APPEND_CMAP(name, qual) ctx->ColormapData.Append(#name, name, sizeof(name)/sizeof(ImU32), qual)
#define IM_RGB(r,g,b) IM_COL32(r,g,b,255)

void Initialize(ImPlotContext* ctx) {
    ResetCtxForNextPlot(ctx);
    ResetCtxForNextAlignedPlots(ctx);
    ResetCtxForNextSubplot(ctx);

    const ImU32 Deep[]     = {4289753676, 4283598045, 4285048917, 4283584196, 4289950337, 4284512403, 4291005402, 4287401100, 4285839820, 4291671396                        };
    const ImU32 Dark[]     = {4280031972, 4290281015, 4283084621, 4288892568, 4278222847, 4281597951, 4280833702, 4290740727, 4288256409                                    };
    const ImU32 Pastel[]   = {4289639675, 4293119411, 4291161036, 4293184478, 4289124862, 4291624959, 4290631909, 4293712637, 4294111986                                    };
    const ImU32 Paired[]   = {4293119554, 4290017311, 4287291314, 4281114675, 4288256763, 4280031971, 4285513725, 4278222847, 4292260554, 4288298346, 4288282623, 4280834481};
    const ImU32 Viridis[]  = {4283695428, 4285867080, 4287054913, 4287455029, 4287526954, 4287402273, 4286883874, 4285579076, 4283552122, 4280737725, 4280674301            };
    const ImU32 Plasma[]   = {4287039501, 4288480321, 4289200234, 4288941455, 4287638193, 4286072780, 4284638433, 4283139314, 4281771772, 4280667900, 4280416752            };
    const ImU32 Hot[]      = {4278190144, 4278190208, 4278190271, 4278190335, 4278206719, 4278223103, 4278239231, 4278255615, 4283826175, 4289396735, 4294967295            };
    const ImU32 Cool[]     = {4294967040, 4294960666, 4294954035, 4294947661, 4294941030, 4294934656, 4294928025, 4294921651, 4294915020, 4294908646, 4294902015            };
    const ImU32 Pink[]     = {4278190154, 4282532475, 4284308894, 4285690554, 4286879686, 4287870160, 4288794330, 4289651940, 4291685869, 4293392118, 4294967295            };
    const ImU32 Jet[]      = {4289331200, 4294901760, 4294923520, 4294945280, 4294967040, 4289396565, 4283826090, 4278255615, 4278233855, 4278212095, 4278190335            };
    const ImU32 Twilight[] = {IM_RGB(226,217,226),IM_RGB(166,191,202),IM_RGB(109,144,192),IM_RGB(95,88,176),IM_RGB(83,30,124),IM_RGB(47,20,54),IM_RGB(100,25,75),IM_RGB(159,60,80),IM_RGB(192,117,94),IM_RGB(208,179,158),IM_RGB(226,217,226)};
    const ImU32 RdBu[]     = {IM_RGB(103,0,31),IM_RGB(178,24,43),IM_RGB(214,96,77),IM_RGB(244,165,130),IM_RGB(253,219,199),IM_RGB(247,247,247),IM_RGB(209,229,240),IM_RGB(146,197,222),IM_RGB(67,147,195),IM_RGB(33,102,172),IM_RGB(5,48,97)};
    const ImU32 BrBG[]     = {IM_RGB(84,48,5),IM_RGB(140,81,10),IM_RGB(191,129,45),IM_RGB(223,194,125),IM_RGB(246,232,195),IM_RGB(245,245,245),IM_RGB(199,234,229),IM_RGB(128,205,193),IM_RGB(53,151,143),IM_RGB(1,102,94),IM_RGB(0,60,48)};
    const ImU32 PiYG[]     = {IM_RGB(142,1,82),IM_RGB(197,27,125),IM_RGB(222,119,174),IM_RGB(241,182,218),IM_RGB(253,224,239),IM_RGB(247,247,247),IM_RGB(230,245,208),IM_RGB(184,225,134),IM_RGB(127,188,65),IM_RGB(77,146,33),IM_RGB(39,100,25)};
    const ImU32 Spectral[] = {IM_RGB(158,1,66),IM_RGB(213,62,79),IM_RGB(244,109,67),IM_RGB(253,174,97),IM_RGB(254,224,139),IM_RGB(255,255,191),IM_RGB(230,245,152),IM_RGB(171,221,164),IM_RGB(102,194,165),IM_RGB(50,136,189),IM_RGB(94,79,162)};
    const ImU32 Greys[]    = {IM_COL32_WHITE, IM_COL32_BLACK                                                                                                                };

    IMPLOT_APPEND_CMAP(Deep, true);
    IMPLOT_APPEND_CMAP(Dark, true);
    IMPLOT_APPEND_CMAP(Pastel, true);
    IMPLOT_APPEND_CMAP(Paired, true);
    IMPLOT_APPEND_CMAP(Viridis, false);
    IMPLOT_APPEND_CMAP(Plasma, false);
    IMPLOT_APPEND_CMAP(Hot, false);
    IMPLOT_APPEND_CMAP(Cool, false);
    IMPLOT_APPEND_CMAP(Pink, false);
    IMPLOT_APPEND_CMAP(Jet, false);
    IMPLOT_APPEND_CMAP(Twilight, false);
    IMPLOT_APPEND_CMAP(RdBu, false);
    IMPLOT_APPEND_CMAP(BrBG, false);
    IMPLOT_APPEND_CMAP(PiYG, false);
    IMPLOT_APPEND_CMAP(Spectral, false);
    IMPLOT_APPEND_CMAP(Greys, false);
}

void ResetCtxForNextPlot(ImPlotContext* ctx) {
    // end child window if it was made
    if (ctx->ChildWindowMade)
        ImGui::EndChild();
    ctx->ChildWindowMade = false;
    // reset the next plot/item data
    ctx->NextPlotData.Reset();
    ctx->NextItemData.Reset();
    // reset ticks/labels
    for (int i = 0; i < IMPLOT_MAX_AXES; ++i) {
        ctx->XTicks[i].Reset();
        ctx->YTicks[i].Reset();
    }
    // reset labels
    ctx->Annotations.Reset();
    // reset extents/fit
    ctx->FitThisFrame = false;
    for (int i = 0; i < IMPLOT_MAX_AXES; i++) {
        ctx->ExtentsX[i].Min = HUGE_VAL;
        ctx->ExtentsX[i].Max = -HUGE_VAL;
        ctx->FitX[i] = false;
        ctx->ExtentsY[i].Min = HUGE_VAL;
        ctx->ExtentsY[i].Max = -HUGE_VAL;
        ctx->FitY[i] = false;
    }
    // reset digital plot items count
    ctx->DigitalPlotItemCnt = 0;
    ctx->DigitalPlotOffset = 0;
    // nullify plot
    ctx->CurrentPlot  = NULL;
    ctx->CurrentItem  = NULL;
    ctx->PreviousItem = NULL;
}

void ResetCtxForNextAlignedPlots(ImPlotContext* ctx) {
    ctx->CurrentAlignmentH = NULL;
    ctx->CurrentAlignmentV = NULL;
}

void ResetCtxForNextSubplot(ImPlotContext* ctx) {
    ctx->CurrentSubplot      = NULL;
    ctx->CurrentAlignmentH   = NULL;
    ctx->CurrentAlignmentV   = NULL;
}

//-----------------------------------------------------------------------------
// Plot Utils
//-----------------------------------------------------------------------------

ImPlotPlot* GetPlot(const char* title) {
    ImGuiWindow*   Window = GImGui->CurrentWindow;
    const ImGuiID  ID     = Window->GetID(title);
    return GImPlot->Plots.GetByKey(ID);
}

ImPlotPlot* GetCurrentPlot() {
    return GImPlot->CurrentPlot;
}

void BustPlotCache() {
    GImPlot->Plots.Clear();
    GImPlot->Subplots.Clear();
}

void PushLinkedAxis(ImPlotAxis& axis) {
    if (axis.LinkedMin) { *axis.LinkedMin = axis.Range.Min; }
    if (axis.LinkedMax) { *axis.LinkedMax = axis.Range.Max; }
}

void PullLinkedAxis(ImPlotAxis& axis) {
    if (axis.LinkedMin) { axis.SetMin(*axis.LinkedMin,true); }
    if (axis.LinkedMax) { axis.SetMax(*axis.LinkedMax,true); }
}

//-----------------------------------------------------------------------------
// Coordinate Utils
//-----------------------------------------------------------------------------

void UpdateTransformCache() {
    ImPlotContext& gp = *GImPlot;
    ImPlotPlot& plot = *gp.CurrentPlot;
    // get pixels for transforms
    for (int i = 0; i < IMPLOT_MAX_AXES; i++) {
        gp.PixelRange[i] = ImRect(plot.XAxis[i].IsInverted() ? plot.PlotRect.Max.x : plot.PlotRect.Min.x,
                                  plot.YAxis[i].IsInverted() ? plot.PlotRect.Min.y : plot.PlotRect.Max.y,
                                  plot.XAxis[i].IsInverted() ? plot.PlotRect.Min.x : plot.PlotRect.Max.x,
                                  plot.YAxis[i].IsInverted() ? plot.PlotRect.Max.y : plot.PlotRect.Min.y);
        gp.Mx[i]      = (gp.PixelRange[i].Max.x - gp.PixelRange[i].Min.x) / plot.XAxis[i].Range.Size();
        gp.My[i]      = (gp.PixelRange[i].Max.y - gp.PixelRange[i].Min.y) / plot.YAxis[i].Range.Size();
        gp.LogDenX[i] = plot.XAxis[i].IsLog() ? ImLog10(plot.XAxis[i].Range.Max / plot.XAxis[i].Range.Min) : 0;
        gp.LogDenY[i] = plot.YAxis[i].IsLog() ? ImLog10(plot.YAxis[i].Range.Max / plot.YAxis[i].Range.Min) : 0;
    }
}

//-----------------------------------------------------------------------------
// Legend Utils
//-----------------------------------------------------------------------------

ImVec2 GetLocationPos(const ImRect& outer_rect, const ImVec2& inner_size, ImPlotLocation loc, const ImVec2& pad) {
    ImVec2 pos;
    if (ImHasFlag(loc, ImPlotLocation_West) && !ImHasFlag(loc, ImPlotLocation_East))
        pos.x = outer_rect.Min.x + pad.x;
    else if (!ImHasFlag(loc, ImPlotLocation_West) && ImHasFlag(loc, ImPlotLocation_East))
        pos.x = outer_rect.Max.x - pad.x - inner_size.x;
    else
        pos.x = outer_rect.GetCenter().x - inner_size.x * 0.5f;
    // legend reference point y
    if (ImHasFlag(loc, ImPlotLocation_North) && !ImHasFlag(loc, ImPlotLocation_South))
        pos.y = outer_rect.Min.y + pad.y;
    else if (!ImHasFlag(loc, ImPlotLocation_North) && ImHasFlag(loc, ImPlotLocation_South))
        pos.y = outer_rect.Max.y - pad.y - inner_size.y;
    else
        pos.y = outer_rect.GetCenter().y - inner_size.y * 0.5f;
    pos.x = IM_ROUND(pos.x);
    pos.y = IM_ROUND(pos.y);
    return pos;
}

ImVec2 CalcLegendSize(ImPlotItemGroup& items, const ImVec2& pad, const ImVec2& spacing, bool vertical) {
    // vars
    const int   nItems      = items.GetLegendCount();
    const float txt_ht      = ImGui::GetTextLineHeight();
    const float icon_size   = txt_ht;
    // get label max width
    float max_label_width = 0;
    float sum_label_width = 0;
    for (int i = 0; i < nItems; ++i) {
        const char* label       = items.GetLegendLabel(i);
        const float label_width = ImGui::CalcTextSize(label, NULL, true).x;
        max_label_width         = label_width > max_label_width ? label_width : max_label_width;
        sum_label_width        += label_width;
    }
    // calc legend size
    const ImVec2 legend_size = vertical ?
                               ImVec2(pad.x * 2 + icon_size + max_label_width, pad.y * 2 + nItems * txt_ht + (nItems - 1) * spacing.y) :
                               ImVec2(pad.x * 2 + icon_size * nItems + sum_label_width + (nItems - 1) * spacing.x, pad.y * 2 + txt_ht);
    return legend_size;
}

bool ShowLegendEntries(ImPlotItemGroup& items, const ImRect& legend_bb, bool hovered, const ImVec2& pad, const ImVec2& spacing, bool vertical, ImDrawList& DrawList) {
    ImGuiIO& IO = ImGui::GetIO();
    // vars
    const float txt_ht      = ImGui::GetTextLineHeight();
    const float icon_size   = txt_ht;
    const float icon_shrink = 2;
    ImU32 col_txt           = GetStyleColorU32(ImPlotCol_LegendText);
    ImU32  col_txt_dis      = ImAlphaU32(col_txt, 0.25f);
    // render each legend item
    float sum_label_width = 0;
    bool any_item_hovered = false;
    for (int i = 0; i < items.GetLegendCount(); ++i) {
        ImPlotItem* item        = items.GetLegendItem(i);
        const char* label       = items.GetLegendLabel(i);
        const float label_width = ImGui::CalcTextSize(label, NULL, true).x;
        const ImVec2 top_left   = vertical ?
                                  legend_bb.Min + pad + ImVec2(0, i * (txt_ht + spacing.y)) :
                                  legend_bb.Min + pad + ImVec2(i * (icon_size + spacing.x) + sum_label_width, 0);
        sum_label_width        += label_width;
        ImRect icon_bb;
        icon_bb.Min = top_left + ImVec2(icon_shrink,icon_shrink);
        icon_bb.Max = top_left + ImVec2(icon_size - icon_shrink, icon_size - icon_shrink);
        ImRect label_bb;
        label_bb.Min = top_left;
        label_bb.Max = top_left + ImVec2(label_width + icon_size, icon_size);
        ImU32 col_txt_hl;
        ImU32 col_item = ImAlphaU32(item->Color,1);

        bool icon_hov = false;
        bool icon_hld = false;
        bool icon_clk = ImHasFlag(items.Legend.Flags, ImPlotLegendFlags_NoButtons) 
                      ? false 
                      : ImGui::ButtonBehavior(icon_bb, item->ID, &icon_hov, &icon_hld);
        if (icon_clk)
            item->Show = !item->Show;

        const bool can_hover = (icon_hov || label_bb.Contains(IO.MousePos))
                             && (!ImHasFlag(items.Legend.Flags, ImPlotLegendFlags_NoHighlightItem)
                             || !ImHasFlag(items.Legend.Flags, ImPlotLegendFlags_NoHighlightAxis));

        if (can_hover) {
            item->LegendHovered = true;
            col_txt_hl = ImMixU32(col_txt, col_item, 64);
            any_item_hovered = true;
        }
        else {
            col_txt_hl = ImGui::GetColorU32(col_txt);
        }
        ImU32 col_icon;
        if (icon_hld)
            col_icon = item->Show ? ImAlphaU32(col_item,0.5f) : ImGui::GetColorU32(ImGuiCol_TextDisabled, 0.5f);
        else if (icon_hov)
            col_icon = item->Show ? ImAlphaU32(col_item,0.75f) : ImGui::GetColorU32(ImGuiCol_TextDisabled, 0.75f);
        else
            col_icon = item->Show ? col_item : col_txt_dis;

        DrawList.AddRectFilled(icon_bb.Min, icon_bb.Max, col_icon);
        const char* text_display_end = ImGui::FindRenderedTextEnd(label, NULL);
        if (label != text_display_end)
            DrawList.AddText(top_left + ImVec2(icon_size, 0), item->Show ? col_txt_hl  : col_txt_dis, label, text_display_end);
    }
    return hovered && !any_item_hovered;
}

//-----------------------------------------------------------------------------
// Tick Utils
//-----------------------------------------------------------------------------

static const float TICK_FILL_X = 0.8f;
static const float TICK_FILL_Y = 1.0f;

void AddTicksDefault(const ImLimits& range, float pix, bool vertical, ImPlotTickCollection& ticks, void (*formatter)(double, char*, int, void*), void* data) {
    const int idx0          = ticks.Size;
    const int nMinor        = 10;
    const int nMajor        = ImMax(2, (int)IM_ROUND(pix / (vertical ? 300.0f : 400.0f)));
    const double nice_range = NiceNum(range.Size() * 0.99, false);
    const double interval   = NiceNum(nice_range / (nMajor - 1), true);
    const double graphmin   = floor(range.Min / interval) * interval;
    const double graphmax   = ceil(range.Max / interval) * interval;
    bool first_major_set    = false;
    int  first_major_idx    = 0;
    ImVec2 total_size(0,0);
    for (double major = graphmin; major < graphmax + 0.5 * interval; major += interval) {
        // is this zero? combat zero formatting issues
        if (major-interval < 0 && major+interval > 0)
            major = 0;
        if (range.Contains(major)) {
            if (!first_major_set) {
                first_major_idx = ticks.Size;
                first_major_set = true;
            }
            total_size += ticks.Append(major, true, true, formatter, data).LabelSize;
        }
        for (int i = 1; i < nMinor; ++i) {
            double minor = major + i * interval / nMinor;
            if (range.Contains(minor)) {
                total_size += ticks.Append(minor, false, true, formatter, data).LabelSize;
            }
        }
    }
    // prune if necessary
    if ((!vertical && total_size.x > pix*TICK_FILL_X) || (vertical && total_size.y > pix*TICK_FILL_Y)) {
        for (int i = first_major_idx-1; i >= idx0; i -= 2)
            ticks.Ticks[i].ShowLabel = false;
        for (int i = first_major_idx+1; i < ticks.Size; i += 2)
            ticks.Ticks[i].ShowLabel = false;
    }
}

void AddTicksLogarithmic(const ImLimits& range, float pix, bool vertical, ImPlotTickCollection& ticks, void (*formatter)(double, char*, int, void*), void* data) {
    if (range.Min <= 0 || range.Max <= 0)
        return;
    const int nMajor = vertical ? ImMax(2, (int)IM_ROUND(pix * 0.02f)) : ImMax(2, (int)IM_ROUND(pix * 0.01f));
    double log_min = ImLog10(range.Min);
    double log_max = ImLog10(range.Max);
    int exp_step  = ImMax(1,(int)(log_max - log_min) / nMajor);
    int exp_min   = (int)log_min;
    int exp_max   = (int)log_max;
    if (exp_step != 1) {
        while(exp_step % 3 != 0)       exp_step++; // make step size multiple of three
        while(exp_min % exp_step != 0) exp_min--;  // decrease exp_min until exp_min + N * exp_step will be 0
    }
    for (int e = exp_min - exp_step; e < (exp_max + exp_step); e += exp_step) {
        double major1 = ImPow(10, (double)(e));
        double major2 = ImPow(10, (double)(e + 1));
        double interval = (major2 - major1) / 9;
        if (major1 >= (range.Min - DBL_EPSILON) && major1 <= (range.Max + DBL_EPSILON))
            ticks.Append(major1, true, true, formatter, data);
        for (int j = 0; j < exp_step; ++j) {
            major1 = ImPow(10, (double)(e+j));
            major2 = ImPow(10, (double)(e+j+1));
            interval = (major2 - major1) / 9;
            for (int i = 1; i < (9 + (int)(j < (exp_step - 1))); ++i) {
                double minor = major1 + i * interval;
                if (minor >= (range.Min - DBL_EPSILON) && minor <= (range.Max + DBL_EPSILON))
                    ticks.Append(minor, false, false, formatter, data);

            }
        }
    }
}

void AddTicksCustom(const double* values, const char* const labels[], int n, ImPlotTickCollection& ticks, void (*formatter)(double, char*, int, void*), void* data) {
    for (int i = 0; i < n; ++i) {
        if (labels != NULL) {
            ImPlotTick tick(values[i], false, true);
            tick.TextOffset = ticks.TextBuffer.size();
            ticks.TextBuffer.append(labels[i], labels[i] + strlen(labels[i]) + 1);
            tick.LabelSize = ImGui::CalcTextSize(labels[i]);
            ticks.Append(tick);
        }
        else {
            ticks.Append(values[i], false, true, formatter, data);
        }
    }
}

//-----------------------------------------------------------------------------
// Time Ticks and Utils
//-----------------------------------------------------------------------------

// this may not be thread safe?
static const double TimeUnitSpans[ImPlotTimeUnit_COUNT] = {
    0.000001,
    0.001,
    1,
    60,
    3600,
    86400,
    2629800,
    31557600
};

inline ImPlotTimeUnit GetUnitForRange(double range) {
    static double cutoffs[ImPlotTimeUnit_COUNT] = {0.001, 1, 60, 3600, 86400, 2629800, 31557600, IMPLOT_MAX_TIME};
    for (int i = 0; i < ImPlotTimeUnit_COUNT; ++i) {
        if (range <= cutoffs[i])
            return (ImPlotTimeUnit)i;
    }
    return ImPlotTimeUnit_Yr;
}

inline int LowerBoundStep(int max_divs, const int* divs, const int* step, int size) {
    if (max_divs < divs[0])
        return 0;
    for (int i = 1; i < size; ++i) {
        if (max_divs < divs[i])
            return step[i-1];
    }
    return step[size-1];
}

inline int GetTimeStep(int max_divs, ImPlotTimeUnit unit) {
    if (unit == ImPlotTimeUnit_Ms || unit == ImPlotTimeUnit_Us) {
        static const int step[] = {500,250,200,100,50,25,20,10,5,2,1};
        static const int divs[] = {2,4,5,10,20,40,50,100,200,500,1000};
        return LowerBoundStep(max_divs, divs, step, 11);
    }
    if (unit == ImPlotTimeUnit_S || unit == ImPlotTimeUnit_Min) {
        static const int step[] = {30,15,10,5,1};
        static const int divs[] = {2,4,6,12,60};
        return LowerBoundStep(max_divs, divs, step, 5);
    }
    else if (unit == ImPlotTimeUnit_Hr) {
        static const int step[] = {12,6,3,2,1};
        static const int divs[] = {2,4,8,12,24};
        return LowerBoundStep(max_divs, divs, step, 5);
    }
    else if (unit == ImPlotTimeUnit_Day) {
        static const int step[] = {14,7,2,1};
        static const int divs[] = {2,4,14,28};
        return LowerBoundStep(max_divs, divs, step, 4);
    }
    else if (unit == ImPlotTimeUnit_Mo) {
        static const int step[] = {6,3,2,1};
        static const int divs[] = {2,4,6,12};
        return LowerBoundStep(max_divs, divs, step, 4);
    }
    return 0;
}

ImPlotTime MkGmtTime(struct tm *ptm) {
    ImPlotTime t;
#ifdef _WIN32
    t.S = _mkgmtime(ptm);
#else
    t.S = timegm(ptm);
#endif
    if (t.S < 0)
        t.S = 0;
    return t;
}

tm* GetGmtTime(const ImPlotTime& t, tm* ptm)
{
#ifdef _WIN32
  if (gmtime_s(ptm, &t.S) == 0)
    return ptm;
  else
    return NULL;
#else
  return gmtime_r(&t.S, ptm);
#endif
}

ImPlotTime MkLocTime(struct tm *ptm) {
    ImPlotTime t;
    t.S = mktime(ptm);
    if (t.S < 0)
        t.S = 0;
    return t;
}

tm* GetLocTime(const ImPlotTime& t, tm* ptm) {
#ifdef _WIN32
  if (localtime_s(ptm, &t.S) == 0)
    return ptm;
  else
    return NULL;
#else
    return localtime_r(&t.S, ptm);
#endif
}

inline ImPlotTime MkTime(struct tm *ptm) {
    if (GetStyle().UseLocalTime)
        return MkLocTime(ptm);
    else
        return MkGmtTime(ptm);
}

inline tm* GetTime(const ImPlotTime& t, tm* ptm) {
    if (GetStyle().UseLocalTime)
        return GetLocTime(t,ptm);
    else
        return GetGmtTime(t,ptm);
}

ImPlotTime MakeTime(int year, int month, int day, int hour, int min, int sec, int us) {
    tm& Tm = GImPlot->Tm;

    int yr = year - 1900;
    if (yr < 0)
        yr = 0;

    sec  = sec + us / 1000000;
    us   = us % 1000000;

    Tm.tm_sec  = sec;
    Tm.tm_min  = min;
    Tm.tm_hour = hour;
    Tm.tm_mday = day;
    Tm.tm_mon  = month;
    Tm.tm_year = yr;

    ImPlotTime t = MkTime(&Tm);

    t.Us = us;
    return t;
}

int GetYear(const ImPlotTime& t) {
    tm& Tm = GImPlot->Tm;
    GetTime(t, &Tm);
    return Tm.tm_year + 1900;
}

ImPlotTime AddTime(const ImPlotTime& t, ImPlotTimeUnit unit, int count) {
    tm& Tm = GImPlot->Tm;
    ImPlotTime t_out = t;
    switch(unit) {
        case ImPlotTimeUnit_Us:  t_out.Us += count;         break;
        case ImPlotTimeUnit_Ms:  t_out.Us += count * 1000;  break;
        case ImPlotTimeUnit_S:   t_out.S  += count;         break;
        case ImPlotTimeUnit_Min: t_out.S  += count * 60;    break;
        case ImPlotTimeUnit_Hr:  t_out.S  += count * 3600;  break;
        case ImPlotTimeUnit_Day: t_out.S  += count * 86400; break;
        case ImPlotTimeUnit_Mo:  for (int i = 0; i < abs(count); ++i) {
                                     GetTime(t_out, &Tm);
                                     if (count > 0)
                                        t_out.S += 86400 * GetDaysInMonth(Tm.tm_year + 1900, Tm.tm_mon);
                                     else if (count < 0)
                                        t_out.S -= 86400 * GetDaysInMonth(Tm.tm_year + 1900 - (Tm.tm_mon == 0 ? 1 : 0), Tm.tm_mon == 0 ? 11 : Tm.tm_mon - 1); // NOT WORKING
                                 }
                                 break;
        case ImPlotTimeUnit_Yr:  for (int i = 0; i < abs(count); ++i) {
                                    if (count > 0)
                                        t_out.S += 86400 * (365 + (int)IsLeapYear(GetYear(t_out)));
                                    else if (count < 0)
                                        t_out.S -= 86400 * (365 + (int)IsLeapYear(GetYear(t_out) - 1));
                                    // this is incorrect if leap year and we are past Feb 28
                                 }
                                 break;
        default:                 break;
    }
    t_out.RollOver();
    return t_out;
}

ImPlotTime FloorTime(const ImPlotTime& t, ImPlotTimeUnit unit) {
    GetTime(t, &GImPlot->Tm);
    switch (unit) {
        case ImPlotTimeUnit_S:   return ImPlotTime(t.S, 0);
        case ImPlotTimeUnit_Ms:  return ImPlotTime(t.S, (t.Us / 1000) * 1000);
        case ImPlotTimeUnit_Us:  return t;
        case ImPlotTimeUnit_Yr:  GImPlot->Tm.tm_mon  = 0; // fall-through
        case ImPlotTimeUnit_Mo:  GImPlot->Tm.tm_mday = 1; // fall-through
        case ImPlotTimeUnit_Day: GImPlot->Tm.tm_hour = 0; // fall-through
        case ImPlotTimeUnit_Hr:  GImPlot->Tm.tm_min  = 0; // fall-through
        case ImPlotTimeUnit_Min: GImPlot->Tm.tm_sec  = 0; break;
        default:                 return t;
    }
    return MkTime(&GImPlot->Tm);
}

ImPlotTime CeilTime(const ImPlotTime& t, ImPlotTimeUnit unit) {
    return AddTime(FloorTime(t, unit), unit, 1);
}

ImPlotTime RoundTime(const ImPlotTime& t, ImPlotTimeUnit unit) {
    ImPlotTime t1 = FloorTime(t, unit);
    ImPlotTime t2 = AddTime(t1,unit,1);
    if (t1.S == t2.S)
        return t.Us - t1.Us < t2.Us - t.Us ? t1 : t2;
    return t.S - t1.S < t2.S - t.S ? t1 : t2;
}

ImPlotTime CombineDateTime(const ImPlotTime& date_part, const ImPlotTime& tod_part) {
    tm& Tm = GImPlot->Tm;
    GetTime(date_part, &GImPlot->Tm);
    int y = Tm.tm_year;
    int m = Tm.tm_mon;
    int d = Tm.tm_mday;
    GetTime(tod_part, &GImPlot->Tm);
    Tm.tm_year = y;
    Tm.tm_mon  = m;
    Tm.tm_mday = d;
    ImPlotTime t = MkTime(&Tm);
    t.Us = tod_part.Us;
    return t;
}

static const char* MONTH_NAMES[]  = {"January","February","March","April","May","June","July","August","September","October","November","December"};
static const char* WD_ABRVS[]     = {"Su","Mo","Tu","We","Th","Fr","Sa"};
static const char* MONTH_ABRVS[]  = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

int FormatTime(const ImPlotTime& t, char* buffer, int size, ImPlotTimeFmt fmt, bool use_24_hr_clk) {
    tm& Tm = GImPlot->Tm;
    GetTime(t, &Tm);
    const int us   = t.Us % 1000;
    const int ms   = t.Us / 1000;
    const int sec  = Tm.tm_sec;
    const int min  = Tm.tm_min;
    if (use_24_hr_clk) {
        const int hr   = Tm.tm_hour;
        switch(fmt) {
            case ImPlotTimeFmt_Us:        return snprintf(buffer, size, ".%03d %03d", ms, us);
            case ImPlotTimeFmt_SUs:       return snprintf(buffer, size, ":%02d.%03d %03d", sec, ms, us);
            case ImPlotTimeFmt_SMs:       return snprintf(buffer, size, ":%02d.%03d", sec, ms);
            case ImPlotTimeFmt_S:         return snprintf(buffer, size, ":%02d", sec);
            case ImPlotTimeFmt_HrMinSMs:  return snprintf(buffer, size, "%02d:%02d:%02d.%03d", hr, min, sec, ms);
            case ImPlotTimeFmt_HrMinS:    return snprintf(buffer, size, "%02d:%02d:%02d", hr, min, sec);
            case ImPlotTimeFmt_HrMin:     return snprintf(buffer, size, "%02d:%02d", hr, min);
            case ImPlotTimeFmt_Hr:        return snprintf(buffer, size, "%02d:00", hr);
            default:                      return 0;
        }
    }
    else {
        const char* ap = Tm.tm_hour < 12 ? "am" : "pm";
        const int hr   = (Tm.tm_hour == 0 || Tm.tm_hour == 12) ? 12 : Tm.tm_hour % 12;
        switch(fmt) {
            case ImPlotTimeFmt_Us:        return snprintf(buffer, size, ".%03d %03d", ms, us);
            case ImPlotTimeFmt_SUs:       return snprintf(buffer, size, ":%02d.%03d %03d", sec, ms, us);
            case ImPlotTimeFmt_SMs:       return snprintf(buffer, size, ":%02d.%03d", sec, ms);
            case ImPlotTimeFmt_S:         return snprintf(buffer, size, ":%02d", sec);
            case ImPlotTimeFmt_HrMinSMs:  return snprintf(buffer, size, "%d:%02d:%02d.%03d%s", hr, min, sec, ms, ap);
            case ImPlotTimeFmt_HrMinS:    return snprintf(buffer, size, "%d:%02d:%02d%s", hr, min, sec, ap);
            case ImPlotTimeFmt_HrMin:     return snprintf(buffer, size, "%d:%02d%s", hr, min, ap);
            case ImPlotTimeFmt_Hr:        return snprintf(buffer, size, "%d%s", hr, ap);
            default:                      return 0;
        }
    }
}

int FormatDate(const ImPlotTime& t, char* buffer, int size, ImPlotDateFmt fmt, bool use_iso_8601) {
    tm& Tm = GImPlot->Tm;
    GetTime(t, &Tm);
    const int day  = Tm.tm_mday;
    const int mon  = Tm.tm_mon + 1;
    const int year = Tm.tm_year + 1900;
    const int yr   = year % 100;
    if (use_iso_8601) {
        switch (fmt) {
            case ImPlotDateFmt_DayMo:   return snprintf(buffer, size, "--%02d-%02d", mon, day);
            case ImPlotDateFmt_DayMoYr: return snprintf(buffer, size, "%d-%02d-%02d", year, mon, day);
            case ImPlotDateFmt_MoYr:    return snprintf(buffer, size, "%d-%02d", year, mon);
            case ImPlotDateFmt_Mo:      return snprintf(buffer, size, "--%02d", mon);
            case ImPlotDateFmt_Yr:      return snprintf(buffer, size, "%d", year);
            default:                    return 0;
        }
    }
    else {
        switch (fmt) {
            case ImPlotDateFmt_DayMo:   return snprintf(buffer, size, "%d/%d", mon, day);
            case ImPlotDateFmt_DayMoYr: return snprintf(buffer, size, "%d/%d/%02d", mon, day, yr);
            case ImPlotDateFmt_MoYr:    return snprintf(buffer, size, "%s %d", MONTH_ABRVS[Tm.tm_mon], year);
            case ImPlotDateFmt_Mo:      return snprintf(buffer, size, "%s", MONTH_ABRVS[Tm.tm_mon]);
            case ImPlotDateFmt_Yr:      return snprintf(buffer, size, "%d", year);
            default:                    return 0;
        }
    }
 }

int FormatDateTime(const ImPlotTime& t, char* buffer, int size, ImPlotDateTimeFmt fmt) {
    int written = 0;
    if (fmt.Date != ImPlotDateFmt_None)
        written += FormatDate(t, buffer, size, fmt.Date, fmt.UseISO8601);
    if (fmt.Time != ImPlotTimeFmt_None) {
        if (fmt.Date != ImPlotDateFmt_None)
            buffer[written++] = ' ';
        written += FormatTime(t, &buffer[written], size - written, fmt.Time, fmt.Use24HourClock);
    }
    return written;
}

inline float GetDateTimeWidth(ImPlotDateTimeFmt fmt) {
    static const ImPlotTime t_max_width = MakeTime(2888, 12, 22, 12, 58, 58, 888888); // best guess at time that maximizes pixel width
    char buffer[32];
    FormatDateTime(t_max_width, buffer, 32, fmt);
    return ImGui::CalcTextSize(buffer).x;
}

void LabelTickTime(ImPlotTick& tick, ImGuiTextBuffer& buffer, const ImPlotTime& t, ImPlotDateTimeFmt fmt) {
    char temp[32];
    if (tick.ShowLabel) {
        tick.TextOffset = buffer.size();
        FormatDateTime(t, temp, 32, fmt);
        buffer.append(temp, temp + strlen(temp) + 1);
        tick.LabelSize = ImGui::CalcTextSize(buffer.Buf.Data + tick.TextOffset);
    }
}

inline bool TimeLabelSame(const char* l1, const char* l2) {
    size_t len1 = strlen(l1);
    size_t len2 = strlen(l2);
    size_t n  = len1 < len2 ? len1 : len2;
    return strcmp(l1 + len1 - n, l2 + len2 - n) == 0;
}

static const ImPlotDateTimeFmt TimeFormatLevel0[ImPlotTimeUnit_COUNT] = {
    ImPlotDateTimeFmt(ImPlotDateFmt_None,  ImPlotTimeFmt_Us),
    ImPlotDateTimeFmt(ImPlotDateFmt_None,  ImPlotTimeFmt_SMs),
    ImPlotDateTimeFmt(ImPlotDateFmt_None,  ImPlotTimeFmt_S),
    ImPlotDateTimeFmt(ImPlotDateFmt_None,  ImPlotTimeFmt_HrMin),
    ImPlotDateTimeFmt(ImPlotDateFmt_None,  ImPlotTimeFmt_Hr),
    ImPlotDateTimeFmt(ImPlotDateFmt_DayMo, ImPlotTimeFmt_None),
    ImPlotDateTimeFmt(ImPlotDateFmt_Mo,    ImPlotTimeFmt_None),
    ImPlotDateTimeFmt(ImPlotDateFmt_Yr,    ImPlotTimeFmt_None)
};

static const ImPlotDateTimeFmt TimeFormatLevel1[ImPlotTimeUnit_COUNT] = {
    ImPlotDateTimeFmt(ImPlotDateFmt_None,    ImPlotTimeFmt_HrMin),
    ImPlotDateTimeFmt(ImPlotDateFmt_None,    ImPlotTimeFmt_HrMinS),
    ImPlotDateTimeFmt(ImPlotDateFmt_None,    ImPlotTimeFmt_HrMin),
    ImPlotDateTimeFmt(ImPlotDateFmt_None,    ImPlotTimeFmt_HrMin),
    ImPlotDateTimeFmt(ImPlotDateFmt_DayMoYr, ImPlotTimeFmt_None),
    ImPlotDateTimeFmt(ImPlotDateFmt_DayMoYr, ImPlotTimeFmt_None),
    ImPlotDateTimeFmt(ImPlotDateFmt_Yr,      ImPlotTimeFmt_None),
    ImPlotDateTimeFmt(ImPlotDateFmt_Yr,      ImPlotTimeFmt_None)
};

static const ImPlotDateTimeFmt TimeFormatLevel1First[ImPlotTimeUnit_COUNT] = {
    ImPlotDateTimeFmt(ImPlotDateFmt_DayMoYr, ImPlotTimeFmt_HrMinS),
    ImPlotDateTimeFmt(ImPlotDateFmt_DayMoYr, ImPlotTimeFmt_HrMinS),
    ImPlotDateTimeFmt(ImPlotDateFmt_DayMoYr, ImPlotTimeFmt_HrMin),
    ImPlotDateTimeFmt(ImPlotDateFmt_DayMoYr, ImPlotTimeFmt_HrMin),
    ImPlotDateTimeFmt(ImPlotDateFmt_DayMoYr, ImPlotTimeFmt_None),
    ImPlotDateTimeFmt(ImPlotDateFmt_DayMoYr, ImPlotTimeFmt_None),
    ImPlotDateTimeFmt(ImPlotDateFmt_Yr,      ImPlotTimeFmt_None),
    ImPlotDateTimeFmt(ImPlotDateFmt_Yr,      ImPlotTimeFmt_None)
};

static const ImPlotDateTimeFmt TimeFormatMouseCursor[ImPlotTimeUnit_COUNT] = {
    ImPlotDateTimeFmt(ImPlotDateFmt_None,     ImPlotTimeFmt_Us),
    ImPlotDateTimeFmt(ImPlotDateFmt_None,     ImPlotTimeFmt_SUs),
    ImPlotDateTimeFmt(ImPlotDateFmt_None,     ImPlotTimeFmt_SMs),
    ImPlotDateTimeFmt(ImPlotDateFmt_None,     ImPlotTimeFmt_HrMinS),
    ImPlotDateTimeFmt(ImPlotDateFmt_None,     ImPlotTimeFmt_HrMin),
    ImPlotDateTimeFmt(ImPlotDateFmt_DayMo,    ImPlotTimeFmt_Hr),
    ImPlotDateTimeFmt(ImPlotDateFmt_DayMoYr,  ImPlotTimeFmt_None),
    ImPlotDateTimeFmt(ImPlotDateFmt_MoYr,     ImPlotTimeFmt_None)
};

inline ImPlotDateTimeFmt GetDateTimeFmt(const ImPlotDateTimeFmt* ctx, ImPlotTimeUnit idx) {
    ImPlotStyle& style     = GetStyle();
    ImPlotDateTimeFmt fmt  = ctx[idx];
    fmt.UseISO8601         = style.UseISO8601;
    fmt.Use24HourClock     = style.Use24HourClock;
    return fmt;
}

void AddTicksTime(const ImLimits& range, float plot_width, ImPlotTickCollection& ticks) {
    // get units for level 0 and level 1 labels
    const ImPlotTimeUnit unit0 = GetUnitForRange(range.Size() / (plot_width / 100)); // level = 0 (top)
    const ImPlotTimeUnit unit1 = unit0 + 1;                                          // level = 1 (bottom)
    // get time format specs
    const ImPlotDateTimeFmt fmt0 = GetDateTimeFmt(TimeFormatLevel0, unit0);
    const ImPlotDateTimeFmt fmt1 = GetDateTimeFmt(TimeFormatLevel1, unit1);
    const ImPlotDateTimeFmt fmtf = GetDateTimeFmt(TimeFormatLevel1First, unit1);
    // min max times
    const ImPlotTime t_min = ImPlotTime::FromDouble(range.Min);
    const ImPlotTime t_max = ImPlotTime::FromDouble(range.Max);
    // maximum allowable density of labels
    const float max_density = 0.5f;
    // book keeping
    const char* last_major  = NULL;
    if (unit0 != ImPlotTimeUnit_Yr) {
        // pixels per major (level 1) division
        const float pix_per_major_div = plot_width / (float)(range.Size() / TimeUnitSpans[unit1]);
        // nominal pixels taken up by labels
        const float fmt0_width = GetDateTimeWidth(fmt0);
        const float fmt1_width = GetDateTimeWidth(fmt1);
        const float fmtf_width = GetDateTimeWidth(fmtf);
        // the maximum number of minor (level 0) labels that can fit between major (level 1) divisions
        const int   minor_per_major   = (int)(max_density * pix_per_major_div / fmt0_width);
        // the minor step size (level 0)
        const int step = GetTimeStep(minor_per_major, unit0);
        // generate ticks
        ImPlotTime t1 = FloorTime(ImPlotTime::FromDouble(range.Min), unit1);
        while (t1 < t_max) {
            // get next major
            const ImPlotTime t2 = AddTime(t1, unit1, 1);
            // add major tick
            if (t1 >= t_min && t1 <= t_max) {
                // minor level 0 tick
                ImPlotTick tick_min(t1.ToDouble(),true,true);
                tick_min.Level = 0;
                LabelTickTime(tick_min,ticks.TextBuffer,t1,fmt0);
                ticks.Append(tick_min);
                // major level 1 tick
                ImPlotTick tick_maj(t1.ToDouble(),true,true);
                tick_maj.Level = 1;
                LabelTickTime(tick_maj,ticks.TextBuffer,t1, last_major == NULL ? fmtf : fmt1);
                const char* this_major = ticks.TextBuffer.Buf.Data + tick_maj.TextOffset;
                if (last_major && TimeLabelSame(last_major,this_major))
                    tick_maj.ShowLabel = false;
                last_major = this_major;
                ticks.Append(tick_maj);
            }
            // add minor ticks up until next major
            if (minor_per_major > 1 && (t_min <= t2 && t1 <= t_max)) {
                ImPlotTime t12 = AddTime(t1, unit0, step);
                while (t12 < t2) {
                    float px_to_t2 = (float)((t2 - t12).ToDouble()/range.Size()) * plot_width;
                    if (t12 >= t_min && t12 <= t_max) {
                        ImPlotTick tick(t12.ToDouble(),false,px_to_t2 >= fmt0_width);
                        tick.Level =  0;
                        LabelTickTime(tick,ticks.TextBuffer,t12,fmt0);
                        ticks.Append(tick);
                        if (last_major == NULL && px_to_t2 >= fmt0_width && px_to_t2 >= (fmt1_width + fmtf_width) / 2) {
                            ImPlotTick tick_maj(t12.ToDouble(),true,true);
                            tick_maj.Level = 1;
                            LabelTickTime(tick_maj,ticks.TextBuffer,t12,fmtf);
                            last_major = ticks.TextBuffer.Buf.Data + tick_maj.TextOffset;
                            ticks.Append(tick_maj);
                        }
                    }
                    t12 = AddTime(t12, unit0, step);
                }
            }
            t1 = t2;
        }
    }
    else {
        const ImPlotDateTimeFmt fmty = GetDateTimeFmt(TimeFormatLevel0, ImPlotTimeUnit_Yr);
        const float label_width = GetDateTimeWidth(fmty);
        const int   max_labels  = (int)(max_density * plot_width / label_width);
        const int year_min      = GetYear(t_min);
        const int year_max      = GetYear(CeilTime(t_max, ImPlotTimeUnit_Yr));
        const double nice_range = NiceNum((year_max - year_min)*0.99,false);
        const double interval   = NiceNum(nice_range / (max_labels - 1), true);
        const int graphmin      = (int)(floor(year_min / interval) * interval);
        const int graphmax      = (int)(ceil(year_max  / interval) * interval);
        const int step          = (int)interval <= 0 ? 1 : (int)interval;

        for (int y = graphmin; y < graphmax; y += step) {
            ImPlotTime t = MakeTime(y);
            if (t >= t_min && t <= t_max) {
                ImPlotTick tick(t.ToDouble(), true, true);
                tick.Level = 0;
                LabelTickTime(tick, ticks.TextBuffer, t, fmty);
                ticks.Append(tick);
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Context Menu
//-----------------------------------------------------------------------------

template <typename F>
bool DragFloat(const char*, F*, float, F, F) {
    return false;
}

template <>
bool DragFloat<double>(const char* label, double* v, float v_speed, double v_min, double v_max) {
    return ImGui::DragScalar(label, ImGuiDataType_Double, v, v_speed, &v_min, &v_max, "%.3f", 1);
}

template <>
bool DragFloat<float>(const char* label, float* v, float v_speed, float v_min, float v_max) {
    return ImGui::DragScalar(label, ImGuiDataType_Float, v, v_speed, &v_min, &v_max, "%.3f", 1);
}

inline void BeginDisabledControls(bool cond) {
    if (cond) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.25f);
    }
}

inline void EndDisabledControls(bool cond) {
    if (cond) {
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    }
}

void ShowAxisContextMenu(ImPlotAxis& axis, ImPlotAxis* equal_axis, bool time_allowed) {

    ImGui::PushItemWidth(75);
    bool always_locked   = axis.IsRangeLocked() || axis.IsAutoFitting();
    bool label           = axis.HasLabel();
    bool grid            = axis.HasGridLines();
    bool ticks           = axis.HasTickMarks();
    bool labels          = axis.HasTickLabels();
    double drag_speed    = (axis.Range.Size() <= DBL_EPSILON) ? DBL_EPSILON * 1.0e+13 : 0.01 * axis.Range.Size(); // recover from almost equal axis limits.

    if (axis.IsTime()) {
        ImPlotTime tmin = ImPlotTime::FromDouble(axis.Range.Min);
        ImPlotTime tmax = ImPlotTime::FromDouble(axis.Range.Max);

        BeginDisabledControls(always_locked);
        ImGui::CheckboxFlags("##LockMin", (unsigned int*)&axis.Flags, ImPlotAxisFlags_LockMin);
        EndDisabledControls(always_locked);
        ImGui::SameLine();
        BeginDisabledControls(axis.IsLockedMin() || always_locked);
        if (ImGui::BeginMenu("Min Time")) {
            if (ShowTimePicker("mintime", &tmin)) {
                if (tmin >= tmax)
                    tmax = AddTime(tmin, ImPlotTimeUnit_S, 1);
                axis.SetRange(tmin.ToDouble(),tmax.ToDouble());
            }
            ImGui::Separator();
            if (ShowDatePicker("mindate",&axis.PickerLevel,&axis.PickerTimeMin,&tmin,&tmax)) {
                tmin = CombineDateTime(axis.PickerTimeMin, tmin);
                if (tmin >= tmax)
                    tmax = AddTime(tmin, ImPlotTimeUnit_S, 1);
                axis.SetRange(tmin.ToDouble(), tmax.ToDouble());
            }
            ImGui::EndMenu();
        }
        EndDisabledControls(axis.IsLockedMin() || always_locked);

        BeginDisabledControls(always_locked);
        ImGui::CheckboxFlags("##LockMax", (unsigned int*)&axis.Flags, ImPlotAxisFlags_LockMax);
        EndDisabledControls(always_locked);
        ImGui::SameLine();
        BeginDisabledControls(axis.IsLockedMax() || always_locked);
        if (ImGui::BeginMenu("Max Time")) {
            if (ShowTimePicker("maxtime", &tmax)) {
                if (tmax <= tmin)
                    tmin = AddTime(tmax, ImPlotTimeUnit_S, -1);
                axis.SetRange(tmin.ToDouble(),tmax.ToDouble());
            }
            ImGui::Separator();
            if (ShowDatePicker("maxdate",&axis.PickerLevel,&axis.PickerTimeMax,&tmin,&tmax)) {
                tmax = CombineDateTime(axis.PickerTimeMax, tmax);
                if (tmax <= tmin)
                    tmin = AddTime(tmax, ImPlotTimeUnit_S, -1);
                axis.SetRange(tmin.ToDouble(), tmax.ToDouble());
            }
            ImGui::EndMenu();
        }
        EndDisabledControls(axis.IsLockedMax() || always_locked);
    }
    else {
        BeginDisabledControls(always_locked);
        ImGui::CheckboxFlags("##LockMin", (unsigned int*)&axis.Flags, ImPlotAxisFlags_LockMin);
        EndDisabledControls(always_locked);
        ImGui::SameLine();
        BeginDisabledControls(axis.IsLockedMin() || always_locked);
        double temp_min = axis.Range.Min;
        if (DragFloat("Min", &temp_min, (float)drag_speed, -HUGE_VAL, axis.Range.Max - DBL_EPSILON)) {
            axis.SetMin(temp_min,true);
            if (equal_axis != NULL)
                equal_axis->SetAspect(axis.GetAspect());
        }
        EndDisabledControls(axis.IsLockedMin() || always_locked);

        BeginDisabledControls(always_locked);
        ImGui::CheckboxFlags("##LockMax", (unsigned int*)&axis.Flags, ImPlotAxisFlags_LockMax);
        EndDisabledControls(always_locked);
        ImGui::SameLine();
        BeginDisabledControls(axis.IsLockedMax() || always_locked);
        double temp_max = axis.Range.Max;
        if (DragFloat("Max", &temp_max, (float)drag_speed, axis.Range.Min + DBL_EPSILON, HUGE_VAL)) {
            axis.SetMax(temp_max,true);
            if (equal_axis != NULL)
                equal_axis->SetAspect(axis.GetAspect());
        }
        EndDisabledControls(axis.IsLockedMax() || always_locked);
    }

    ImGui::Separator();

    ImGui::CheckboxFlags("Auto-Fit",(unsigned int*)&axis.Flags, ImPlotAxisFlags_AutoFit);
    BeginDisabledControls(axis.IsTime() && time_allowed);
    ImGui::CheckboxFlags("Log Scale",(unsigned int*)&axis.Flags, ImPlotAxisFlags_LogScale);
    EndDisabledControls(axis.IsTime() && time_allowed);
    if (time_allowed) {
        BeginDisabledControls(axis.IsLog());
        ImGui::CheckboxFlags("Time",(unsigned int*)&axis.Flags, ImPlotAxisFlags_Time);
        EndDisabledControls(axis.IsLog());
    }
    ImGui::Separator();
    ImGui::CheckboxFlags("Invert",(unsigned int*)&axis.Flags, ImPlotAxisFlags_Invert);
    ImGui::CheckboxFlags("Opposite",(unsigned int*)&axis.Flags, ImPlotAxisFlags_Opposite);
    ImGui::Separator();
    BeginDisabledControls(axis.LabelOffset == -1);
    if (ImGui::Checkbox("Label", &label))
        ImFlipFlag(axis.Flags, ImPlotAxisFlags_NoLabel);
    EndDisabledControls(axis.LabelOffset == -1);
    if (ImGui::Checkbox("Grid Lines", &grid))
        ImFlipFlag(axis.Flags, ImPlotAxisFlags_NoGridLines);
    if (ImGui::Checkbox("Tick Marks", &ticks))
        ImFlipFlag(axis.Flags, ImPlotAxisFlags_NoTickMarks);
    if (ImGui::Checkbox("Tick Labels", &labels))
        ImFlipFlag(axis.Flags, ImPlotAxisFlags_NoTickLabels);

}

bool ShowLegendContextMenu(ImPlotLegend& legend, bool visible) {
    const float s = ImGui::GetFrameHeight();
    bool ret = false;
    if (ImGui::Checkbox("Show",&visible))
        ret = true;
    if (legend.CanGoInside)
        ImGui::CheckboxFlags("Outside",(unsigned int*)&legend.Flags, ImPlotLegendFlags_Outside);
    if (ImGui::RadioButton("H", ImHasFlag(legend.Flags, ImPlotLegendFlags_Horizontal)))
        legend.Flags |= ImPlotLegendFlags_Horizontal;
    ImGui::SameLine();
    if (ImGui::RadioButton("V", !ImHasFlag(legend.Flags, ImPlotLegendFlags_Horizontal)))
        legend.Flags &= ~ImPlotLegendFlags_Horizontal;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2,2));
    if (ImGui::Button("NW",ImVec2(1.5f*s,s))) { legend.Location = ImPlotLocation_NorthWest; } ImGui::SameLine();
    if (ImGui::Button("N", ImVec2(1.5f*s,s))) { legend.Location = ImPlotLocation_North;     } ImGui::SameLine();
    if (ImGui::Button("NE",ImVec2(1.5f*s,s))) { legend.Location = ImPlotLocation_NorthEast; }
    if (ImGui::Button("W", ImVec2(1.5f*s,s))) { legend.Location = ImPlotLocation_West;      } ImGui::SameLine();
    if (ImGui::InvisibleButton("C", ImVec2(1.5f*s,s))) {     } ImGui::SameLine();
    if (ImGui::Button("E", ImVec2(1.5f*s,s))) { legend.Location = ImPlotLocation_East;      }
    if (ImGui::Button("SW",ImVec2(1.5f*s,s))) { legend.Location = ImPlotLocation_SouthWest; } ImGui::SameLine();
    if (ImGui::Button("S", ImVec2(1.5f*s,s))) { legend.Location = ImPlotLocation_South;     } ImGui::SameLine();
    if (ImGui::Button("SE",ImVec2(1.5f*s,s))) { legend.Location = ImPlotLocation_SouthEast; }
    ImGui::PopStyleVar();
    return ret;
}

void ShowSubplotsContextMenu(ImPlotSubplot& subplot) {
    if ((ImGui::BeginMenu("Linking"))) {
        if (ImGui::MenuItem("Link Rows",NULL,ImHasFlag(subplot.Flags, ImPlotSubplotFlags_LinkRows)))
            ImFlipFlag(subplot.Flags, ImPlotSubplotFlags_LinkRows);
        if (ImGui::MenuItem("Link Cols",NULL,ImHasFlag(subplot.Flags, ImPlotSubplotFlags_LinkCols)))
            ImFlipFlag(subplot.Flags, ImPlotSubplotFlags_LinkCols);
        if (ImGui::MenuItem("Link All X",NULL,ImHasFlag(subplot.Flags, ImPlotSubplotFlags_LinkAllX)))
            ImFlipFlag(subplot.Flags, ImPlotSubplotFlags_LinkAllX);
        if (ImGui::MenuItem("Link All Y",NULL,ImHasFlag(subplot.Flags, ImPlotSubplotFlags_LinkAllY)))
            ImFlipFlag(subplot.Flags, ImPlotSubplotFlags_LinkAllY);
        ImGui::EndMenu();
    }
    if ((ImGui::BeginMenu("Settings"))) {
        if (ImGui::MenuItem("Title",NULL,!ImHasFlag(subplot.Flags, ImPlotSubplotFlags_NoTitle)))
            ImFlipFlag(subplot.Flags, ImPlotSubplotFlags_NoTitle);
        if (ImGui::MenuItem("Resizable",NULL,!ImHasFlag(subplot.Flags, ImPlotSubplotFlags_NoResize)))
            ImFlipFlag(subplot.Flags, ImPlotSubplotFlags_NoResize);
        if (ImGui::MenuItem("Align",NULL,!ImHasFlag(subplot.Flags, ImPlotSubplotFlags_NoAlign)))
            ImFlipFlag(subplot.Flags, ImPlotSubplotFlags_NoAlign);
        if (ImGui::MenuItem("Share Items",NULL,ImHasFlag(subplot.Flags, ImPlotSubplotFlags_ShareItems)))
            ImFlipFlag(subplot.Flags, ImPlotSubplotFlags_ShareItems);
        ImGui::EndMenu();
    }
}

void ShowPlotContextMenu(ImPlotPlot& plot) {
    const bool owns_legend = GImPlot->CurrentItems == &plot.Items;
    const bool equal = ImHasFlag(plot.Flags, ImPlotFlags_Equal);

    char buf[10] = {};

    for (int i = 0; i < IMPLOT_MAX_AXES; i++) {
        if (!plot.XAxis[i].Enabled || !plot.XAxis[i].HasMenus())
            continue;
        snprintf(buf, sizeof(buf) - 1, i == 0 ? "X-Axis" : "X-Axis %d", i + 1);
        if (ImGui::BeginMenu(buf)) {
            ImGui::PushID(i);
            ShowAxisContextMenu(plot.XAxis[i], equal ? &plot.YAxis[i] : NULL, false);
            ImGui::PopID();
            ImGui::EndMenu();
        }
    }

    for (int i = 0; i < IMPLOT_MAX_AXES; i++) {
        if (!plot.YAxis[i].Enabled || !plot.YAxis[i].HasMenus())
            continue;
        snprintf(buf, sizeof(buf) - 1, i == 0 ? "Y-Axis" : "Y-Axis %d", i + 1);
        if (ImGui::BeginMenu(buf)) {
            ImGui::PushID(i);
            ShowAxisContextMenu(plot.YAxis[i], equal ? &plot.XAxis[i] : NULL, false);
            ImGui::PopID();
            ImGui::EndMenu();
        }
    }

    ImGui::Separator();
    if (!ImHasFlag(GImPlot->CurrentItems->Legend.Flags, ImPlotLegendFlags_NoMenus)) {
        if ((ImGui::BeginMenu("Legend"))) {
            if (owns_legend) {
                if (ShowLegendContextMenu(plot.Items.Legend, !ImHasFlag(plot.Flags, ImPlotFlags_NoLegend)))
                    ImFlipFlag(plot.Flags, ImPlotFlags_NoLegend);
            }
            else if (GImPlot->CurrentSubplot != NULL) {
                if (ShowLegendContextMenu(GImPlot->CurrentSubplot->Items.Legend, !ImHasFlag(GImPlot->CurrentSubplot->Flags, ImPlotSubplotFlags_NoLegend)))
                    ImFlipFlag(GImPlot->CurrentSubplot->Flags, ImPlotSubplotFlags_NoLegend);
            }
            ImGui::EndMenu();
        }
    }
    if ((ImGui::BeginMenu("Settings"))) {
        if (ImGui::MenuItem("Anti-Aliased Lines",NULL,ImHasFlag(plot.Flags, ImPlotFlags_AntiAliased)))
            ImFlipFlag(plot.Flags, ImPlotFlags_AntiAliased);
        if (ImGui::MenuItem("Equal", NULL, ImHasFlag(plot.Flags, ImPlotFlags_Equal)))
            ImFlipFlag(plot.Flags, ImPlotFlags_Equal);
        if (ImGui::MenuItem("Box Select",NULL,!ImHasFlag(plot.Flags, ImPlotFlags_NoBoxSelect)))
            ImFlipFlag(plot.Flags, ImPlotFlags_NoBoxSelect);
        if (ImGui::MenuItem("Title",NULL,!ImHasFlag(plot.Flags, ImPlotFlags_NoTitle)))
            ImFlipFlag(plot.Flags, ImPlotFlags_NoTitle);
        if (ImGui::MenuItem("Mouse Position",NULL,!ImHasFlag(plot.Flags, ImPlotFlags_NoMouseText)))
            ImFlipFlag(plot.Flags, ImPlotFlags_NoMouseText);
        if (ImGui::MenuItem("Crosshairs",NULL,ImHasFlag(plot.Flags, ImPlotFlags_Crosshairs)))
            ImFlipFlag(plot.Flags, ImPlotFlags_Crosshairs);
        ImGui::EndMenu();
    }
    if (GImPlot->CurrentSubplot != NULL && !ImHasFlag(GImPlot->CurrentPlot->Flags, ImPlotSubplotFlags_NoMenus)) {
        ImGui::Separator();
        if ((ImGui::BeginMenu("Subplots"))) {
            ShowSubplotsContextMenu(*GImPlot->CurrentSubplot);
            ImGui::EndMenu();
        }
    }
}

//-----------------------------------------------------------------------------
// Axis Utils
//-----------------------------------------------------------------------------

static inline int AxisPrecision(const ImPlotAxis& axis, const ImPlotTickCollection& ticks) {
    const double range = ticks.Size > 1 ? (ticks.Ticks[1].PlotPos - ticks.Ticks[0].PlotPos) : axis.Range.Size();
    return Precision(range);
}

static inline double RoundAxisValue(const ImPlotAxis& axis, const ImPlotTickCollection& ticks, double value) {
    return RoundTo(value, AxisPrecision(axis,ticks));
}

int LabelAxisValue(const ImPlotAxis& axis, const ImPlotTickCollection& ticks, double value, char* buff, int size) {
    ImPlotContext& gp = *GImPlot;
    if (axis.IsTime()) {
        ImPlotTimeUnit unit = axis.Vertical 
                            ? GetUnitForRange(axis.Range.Size() / (gp.CurrentPlot->PlotRect.GetHeight() / 100))
                            : GetUnitForRange(axis.Range.Size() / (gp.CurrentPlot->PlotRect.GetWidth() / 100));
        return FormatDateTime(ImPlotTime::FromDouble(value), buff, size, GetDateTimeFmt(TimeFormatMouseCursor, unit));
    }
    else {
        double range = ticks.Size > 1 ? (ticks.Ticks[1].PlotPos - ticks.Ticks[0].PlotPos) : axis.Range.Size();
        return snprintf(buff, size, "%.*f", Precision(range), value);
    }
}

void UpdateAxisColors(ImPlotAxis& axis) {
    const ImVec4 col_grid = GetStyleColorVec4(ImPlotCol_AxisGrid);
    axis.ColorMaj         = ImGui::GetColorU32(col_grid);
    axis.ColorMin         = ImGui::GetColorU32(col_grid*ImVec4(1,1,1,GImPlot->Style.MinorAlpha));
    axis.ColorTxt         = GetStyleColorU32(ImPlotCol_AxisText);
    axis.ColorHov         = GetStyleColorU32(ImPlotCol_AxisHovered);
    axis.ColorAct         = GetStyleColorU32(ImPlotCol_AxisActive);
    axis.ColorHiLi        = IM_COL32_BLACK_TRANS;
}

void PadAndDatumAxesX(ImPlotPlot& plot, float& pad_T, float& pad_B) {

    ImPlotContext& gp = *GImPlot;

    const float T = ImGui::GetTextLineHeight();
    const float P = gp.Style.LabelPadding.y;
    const float K = gp.Style.MinorTickLen.x;

    int   count_T = 0;
    int   count_B = 0;
    float last_T  = plot.AxesRect.Min.y;
    float last_B  = plot.AxesRect.Max.y;

    for (int i = IMPLOT_MAX_AXES; i-- > 0;) { // FYI: can iterate forward
        ImPlotAxis& axis = plot.XAxis[i];
        if (!axis.Enabled)
            continue;
        const bool label = axis.HasLabel();
        const bool ticks = axis.HasTickLabels();
        const bool opp   = axis.IsOpposite();
        const bool time  = axis.IsTime();
        if (opp) {
            if (count_T++ > 0)
                pad_T += K + P;
            if (label)
                pad_T += T + P;
            if (ticks)
                pad_T += ImMax(T, gp.XTicks[i].MaxHeight) + P + (time ? T + P : 0);
            axis.Datum1 = plot.CanvasRect.Min.y + pad_T;
            axis.Datum2 = last_T;
            last_T = axis.Datum1;
        }
        else {
            if (count_B++ > 0)
                pad_B += K + P;
            if (label)
                pad_B += T + P;
            if (ticks)
                pad_B += ImMax(T, gp.XTicks[i].MaxHeight) + P + (time ? T + P : 0);
            axis.Datum1 = plot.CanvasRect.Max.y - pad_B;
            axis.Datum2 = last_B;
            last_B = axis.Datum1;
        }
    }
}

void PadAndDatumAxesY(ImPlotPlot& plot, float& pad_L, float& pad_R) {

    //   [   pad_L   ]                 [   pad_R   ]
    //   .................CanvasRect................
    //   :TPWPK.PTPWP _____PlotRect____ PWPTP.KPWPT:
    //   :A # |- A # |-               -| # A -| # A:
    //   :X   |  X   |                 |   X  |   x:
    //   :I # |- I # |-               -| # I -| # I:
    //   :S   |  S   |                 |   S  |   S:
    //   :3 # |- 0 # |-_______________-| # 1 -| # 2:
    //   :.........................................:
    //
    //   T = text height
    //   P = label padding
    //   K = minor tick length
    //   W = label width

    ImPlotContext& gp = *GImPlot;

    const float T = ImGui::GetTextLineHeight();
    const float P = gp.Style.LabelPadding.x;
    const float K = gp.Style.MinorTickLen.y;

    int   count_L = 0;
    int   count_R = 0;
    float last_L  = plot.AxesRect.Min.x;
    float last_R  = plot.AxesRect.Max.x;

    for (int i = IMPLOT_MAX_AXES; i-- > 0;) { // FYI: can iterate forward
        ImPlotAxis& axis = plot.YAxis[i];
        if (!axis.Enabled)
            continue;
        const bool label = axis.HasLabel();
        const bool ticks = axis.HasTickLabels();
        const bool opp   = axis.IsOpposite();
        if (opp) {
            if (count_R++ > 0)
                pad_R += K + P;
            if (label)
                pad_R += T + P;
            if (ticks)
                pad_R += gp.YTicks[i].MaxWidth + P;
            axis.Datum1 = plot.CanvasRect.Max.x - pad_R;
            axis.Datum2 = last_R;
            last_R = axis.Datum1;
        }
        else {
            if (count_L++ > 0)
                pad_L += K + P;
            if (label)
                pad_L += T + P;
            if (ticks)
                pad_L += gp.YTicks[i].MaxWidth + P;
            axis.Datum1 = plot.CanvasRect.Min.x + pad_L;
            axis.Datum2 = last_L;
            last_L = axis.Datum1;
        }
    }

    plot.PlotRect.Min.x = plot.CanvasRect.Min.x + pad_L;
    plot.PlotRect.Max.x = plot.CanvasRect.Max.x - pad_R;
}

//-----------------------------------------------------------------------------
// RENDERING
//-----------------------------------------------------------------------------

static inline void RenderGridLinesX(ImDrawList& DrawList, const ImPlotTickCollection& ticks, const ImRect& rect, ImU32 col_maj, ImU32 col_min, float size_maj, float size_min) {
    const float density   = ticks.Size / rect.GetWidth();
    ImVec4 col_min4  = ImGui::ColorConvertU32ToFloat4(col_min);
    col_min4.w      *= ImClamp(ImRemap(density, 0.1f, 0.2f, 1.0f, 0.0f), 0.0f, 1.0f);
    col_min = ImGui::ColorConvertFloat4ToU32(col_min4);
    for (int t = 0; t < ticks.Size; t++) {
        const ImPlotTick& xt = ticks.Ticks[t];
        if (xt.PixelPos < rect.Min.x || xt.PixelPos > rect.Max.x)
            continue;
        if (xt.Level == 0) {
            if (xt.Major)
                DrawList.AddLine(ImVec2(xt.PixelPos, rect.Min.y), ImVec2(xt.PixelPos, rect.Max.y), col_maj, size_maj);
            else if (density < 0.2f)
                DrawList.AddLine(ImVec2(xt.PixelPos, rect.Min.y), ImVec2(xt.PixelPos, rect.Max.y), col_min, size_min);
        }
    }
}

static inline void RenderGridLinesY(ImDrawList& DrawList, const ImPlotTickCollection& ticks, const ImRect& rect, ImU32 col_maj, ImU32 col_min, float size_maj, float size_min) {
    const float density   = ticks.Size / rect.GetHeight();
    ImVec4 col_min4  = ImGui::ColorConvertU32ToFloat4(col_min);
    col_min4.w      *= ImClamp(ImRemap(density, 0.1f, 0.2f, 1.0f, 0.0f), 0.0f, 1.0f);
    col_min = ImGui::ColorConvertFloat4ToU32(col_min4);
    for (int t = 0; t < ticks.Size; t++) {
        const ImPlotTick& yt = ticks.Ticks[t];
        if (yt.PixelPos < rect.Min.y || yt.PixelPos > rect.Max.y)
            continue;
        if (yt.Major)
            DrawList.AddLine(ImVec2(rect.Min.x, yt.PixelPos), ImVec2(rect.Max.x, yt.PixelPos), col_maj, size_maj);
        else if (density < 0.2f)
            DrawList.AddLine(ImVec2(rect.Min.x, yt.PixelPos), ImVec2(rect.Max.x, yt.PixelPos), col_min, size_min);
    }
}

static inline void RenderSelectionRect(ImDrawList& DrawList, const ImVec2& p_min, const ImVec2& p_max, const ImVec4& col) {
    const ImU32 col_bg = ImGui::GetColorU32(col * ImVec4(1,1,1,0.25f));
    const ImU32 col_bd = ImGui::GetColorU32(col);
    DrawList.AddRectFilled(p_min, p_max, col_bg);
    DrawList.AddRect(p_min, p_max, col_bd);
}

//-----------------------------------------------------------------------------
// Input Handling
//-----------------------------------------------------------------------------

static const float MOUSE_CURSOR_DRAG_THRESHOLD = 5.0f;
static const float BOX_SELECT_DRAG_THRESHOLD   = 4.0f;

bool UpdateInput(ImPlotPlot& plot) {

    bool changed = false;

    ImPlotContext& gp = *GImPlot;
    ImGuiIO& IO = ImGui::GetIO();

    // BUTTON STATE -----------------------------------------------------------

    const ImGuiButtonFlags plot_button_flags = ImGuiButtonFlags_AllowItemOverlap
                                             | ImGuiButtonFlags_PressedOnClick
                                             | ImGuiButtonFlags_PressedOnDoubleClick
                                             | ImGuiButtonFlags_MouseButtonLeft 
                                             | ImGuiButtonFlags_MouseButtonRight 
                                             | ImGuiButtonFlags_MouseButtonMiddle;
    const ImGuiButtonFlags axis_button_flags = ImGuiButtonFlags_FlattenChildren
                                             | ImGuiButtonFlags_AllowItemOverlap
                                             | ImGuiButtonFlags_PressedOnClick
                                             | ImGuiButtonFlags_PressedOnDoubleClick;

    const bool plot_clicked = ImGui::ButtonBehavior(plot.PlotRect,plot.ID,&plot.Hovered,&plot.Held,axis_button_flags);   
    ImGui::SetItemAllowOverlap();

    if (plot_clicked) {
        if (!ImHasFlag(plot.Flags, ImPlotFlags_NoBoxSelect) && IO.MouseClicked[gp.InputMap.BoxSelectButton] && ImHasFlag(IO.KeyMods, gp.InputMap.BoxSelectMod)) {
            plot.Selecting   = true;
            plot.SelectStart = IO.MousePos;
            plot.SelectRect  = ImRect(0,0,0,0);
        }
        if (IO.MouseDoubleClicked[gp.InputMap.FitButton]) {
            gp.FitThisFrame = true;
            for (int i = 0; i < IMPLOT_MAX_AXES; ++i) 
                gp.FitX[i] = gp.FitY[i] = true;
        }
    }

    bool x_click[IMPLOT_MAX_AXES];
    bool y_click[IMPLOT_MAX_AXES];
    bool x_held[IMPLOT_MAX_AXES];
    bool y_held[IMPLOT_MAX_AXES];
    bool x_hov[IMPLOT_MAX_AXES];
    bool y_hov[IMPLOT_MAX_AXES];

    for (int i = 0; i < IMPLOT_MAX_AXES; ++i) {
        // x buttons
        ImPlotAxis& xax = plot.XAxis[i];
        ImGui::KeepAliveID(xax.ID);
        x_click[i]  = ImGui::ButtonBehavior(xax.HoverRect,xax.ID,&xax.Hovered,&xax.Held, axis_button_flags);
        if (x_click[i] && IO.MouseDoubleClicked[gp.InputMap.FitButton])
            gp.FitThisFrame = gp.FitX[i] = true;
        x_hov[i]  = xax.Hovered || plot.Hovered;
        x_held[i] = xax.Held    || plot.Held;
        // y buttons
        ImPlotAxis& yax = plot.YAxis[i];
        ImGui::KeepAliveID(yax.ID);
        y_click[i]  = ImGui::ButtonBehavior(yax.HoverRect,yax.ID,&yax.Hovered,&yax.Held, axis_button_flags);
        if (y_click[i] && IO.MouseDoubleClicked[gp.InputMap.FitButton])
            gp.FitThisFrame = gp.FitY[i] = true;
        y_hov[i]  = yax.Hovered || plot.Hovered;
        y_held[i] = yax.Held    || plot.Held;
    }

    const bool any_x_hov  = plot.Hovered || AnyAxesHovered(plot.XAxis, IMPLOT_MAX_AXES);
    const bool any_y_hov  = plot.Hovered || AnyAxesHovered(plot.YAxis, IMPLOT_MAX_AXES);
    const bool any_x_held = plot.Held    || AnyAxesHeld(plot.XAxis, IMPLOT_MAX_AXES);
    const bool any_y_held = plot.Held    || AnyAxesHeld(plot.YAxis, IMPLOT_MAX_AXES);
    const bool axis_equal = ImHasFlag(plot.Flags, ImPlotFlags_Equal);

    // DRAG INPUT -------------------------------------------------------------

    if ((any_x_held || any_y_held) && !plot.Selecting) {
        UpdateTransformCache();
        for (int i = 0; i < IMPLOT_MAX_AXES; i++) {
            // special case for axis equal and both x and y0 hovered
            if (axis_equal && !plot.XAxis[i].IsInputLocked() && x_held[i] && !plot.YAxis[i].IsInputLocked() && y_held[i]) {
                ImPoint plot_tl = PixelsToPlot(plot.PlotRect.Min - IO.MouseDelta, ImAxis_X1+i, ImAxis_Y1+i);
                ImPoint plot_br = PixelsToPlot(plot.PlotRect.Max - IO.MouseDelta, ImAxis_X1+i, ImAxis_Y1+i);
                plot.XAxis[i].SetMin(plot.XAxis[i].IsInverted() ? plot_br.x : plot_tl.x);
                plot.XAxis[i].SetMax(plot.XAxis[i].IsInverted() ? plot_tl.x : plot_br.x);
                plot.YAxis[i].SetMin(plot.YAxis[i].IsInverted() ? plot_tl.y : plot_br.y);
                plot.YAxis[i].SetMax(plot.YAxis[i].IsInverted() ? plot_br.y : plot_tl.y);
                double xar = plot.XAxis[i].GetAspect();
                double yar = plot.YAxis[i].GetAspect();
                if (!ImAlmostEqual(xar,yar) && !plot.YAxis[i].IsInputLocked())
                    plot.XAxis[i].SetAspect(yar);
                continue;
            }
            if (!plot.XAxis[i].IsInputLocked() && x_held[i]) {
                ImPoint plot_tl = PixelsToPlot(plot.PlotRect.Min - IO.MouseDelta, ImAxis_X1+i, ImAxis_Y1);
                ImPoint plot_br = PixelsToPlot(plot.PlotRect.Max - IO.MouseDelta, ImAxis_X1+i, ImAxis_Y1);
                plot.XAxis[i].SetMin(plot.XAxis[i].IsInverted() ? plot_br.x : plot_tl.x);
                plot.XAxis[i].SetMax(plot.XAxis[i].IsInverted() ? plot_tl.x : plot_br.x);
                if (axis_equal)
                    plot.YAxis[i].SetAspect(plot.XAxis[i].GetAspect());
            }
            if (!plot.YAxis[i].IsInputLocked() && y_held[i]) {
                ImPoint plot_tl = PixelsToPlot(plot.PlotRect.Min - IO.MouseDelta, ImAxis_X1, ImAxis_Y1+i);
                ImPoint plot_br = PixelsToPlot(plot.PlotRect.Max - IO.MouseDelta, ImAxis_X1, ImAxis_Y1+i);
                plot.YAxis[i].SetMin(plot.YAxis[i].IsInverted() ? plot_tl.y : plot_br.y);
                plot.YAxis[i].SetMax(plot.YAxis[i].IsInverted() ? plot_br.y : plot_tl.y);
                if (axis_equal)
                    plot.XAxis[i].SetAspect(plot.YAxis[i].GetAspect());
            }
        }
        // Set the mouse cursor based on which axes are moving.
        int direction = 0;
         for (int i = 0; i < IMPLOT_MAX_AXES; i++) {
            if (plot.XAxis[i].Enabled && x_held[i] && !plot.XAxis[i].IsInputLocked())
                direction |= (1 << 1);
            if (plot.YAxis[i].Enabled && y_held[i] && !plot.YAxis[i].IsInputLocked())
                direction |= (1 << 2);

        }
        if (IO.MouseDragMaxDistanceSqr[0] > MOUSE_CURSOR_DRAG_THRESHOLD) {
            if (direction == 0)
                ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
            else if (direction == (1 << 1))
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            else if (direction == (1 << 2))
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
            else
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
        }
    }

    // SCROLL INPUT -----------------------------------------------------------

    if ((any_x_hov || any_y_hov) && IO.MouseWheel != 0) {
        UpdateTransformCache();
        float zoom_rate = IMPLOT_ZOOM_RATE;
        if (IO.MouseWheel > 0)
            zoom_rate = (-zoom_rate) / (1.0f + (2.0f * zoom_rate));
        float tx = ImRemap(IO.MousePos.x, plot.PlotRect.Min.x, plot.PlotRect.Max.x, 0.0f, 1.0f);
        float ty = ImRemap(IO.MousePos.y, plot.PlotRect.Min.y, plot.PlotRect.Max.y, 0.0f, 1.0f);
        bool equal_zoomed = false;
        for (int i = 0; i < IMPLOT_MAX_AXES; i++) {
            // special case for axis equal and both x and y0 hovered
            if (axis_equal && x_hov[i] && !plot.XAxis[i].IsInputLocked() && y_hov[i] && !plot.YAxis[i].IsInputLocked()) {
                const ImPoint& plot_tl = PixelsToPlot(plot.PlotRect.Min - plot.PlotRect.GetSize() * ImVec2(tx * zoom_rate, ty * zoom_rate), ImAxis_X1+i, ImAxis_Y1+i);
                const ImPoint& plot_br = PixelsToPlot(plot.PlotRect.Max + plot.PlotRect.GetSize() * ImVec2((1 - tx) * zoom_rate, (1 - ty) * zoom_rate), ImAxis_X1+i, ImAxis_Y1+i);
                plot.XAxis[i].SetMin(plot.XAxis[i].IsInverted() ? plot_br.x : plot_tl.x);
                plot.XAxis[i].SetMax(plot.XAxis[i].IsInverted() ? plot_tl.x : plot_br.x);
                plot.YAxis[i].SetMin(plot.YAxis[i].IsInverted() ? plot_tl.y : plot_br.y);
                plot.YAxis[i].SetMax(plot.YAxis[i].IsInverted() ? plot_br.y : plot_tl.y);
                double xar = plot.XAxis[i].GetAspect();
                double yar = plot.YAxis[i].GetAspect();
                if (!ImAlmostEqual(xar,yar) && !plot.YAxis[i].IsInputLocked())
                    plot.XAxis[i].SetAspect(yar);
                equal_zoomed = true;
            }
            if (x_hov[i] && !plot.XAxis[i].IsInputLocked() && !equal_zoomed) {
                const ImPoint& plot_tl = PixelsToPlot(plot.PlotRect.Min - plot.PlotRect.GetSize() * ImVec2(tx * zoom_rate, ty * zoom_rate), ImAxis_X1+i, ImAxis_Y1);
                const ImPoint& plot_br = PixelsToPlot(plot.PlotRect.Max + plot.PlotRect.GetSize() * ImVec2((1 - tx) * zoom_rate, (1 - ty) * zoom_rate), ImAxis_X1+i, ImAxis_Y1);
                plot.XAxis[i].SetMin(plot.XAxis[i].IsInverted() ? plot_br.x : plot_tl.x);
                plot.XAxis[i].SetMax(plot.XAxis[i].IsInverted() ? plot_tl.x : plot_br.x);
                if (axis_equal)
                    plot.YAxis[i].SetAspect(plot.XAxis[i].GetAspect());
            }
            if (y_hov[i] && !plot.YAxis[i].IsInputLocked() && !equal_zoomed) {
                const ImPoint& plot_tl = PixelsToPlot(plot.PlotRect.Min - plot.PlotRect.GetSize() * ImVec2(tx * zoom_rate, ty * zoom_rate), ImAxis_X1, ImAxis_Y1+i);
                const ImPoint& plot_br = PixelsToPlot(plot.PlotRect.Max + plot.PlotRect.GetSize() * ImVec2((1 - tx) * zoom_rate, (1 - ty) * zoom_rate), ImAxis_X1, ImAxis_Y1+i);
                plot.YAxis[i].SetMin(plot.YAxis[i].IsInverted() ? plot_tl.y : plot_br.y);
                plot.YAxis[i].SetMax(plot.YAxis[i].IsInverted() ? plot_br.y : plot_tl.y);
                if (axis_equal)
                    plot.XAxis[i].SetAspect(plot.YAxis[i].GetAspect());
            }
        }
    }

    // BOX-SELECTION ----------------------------------------------------------

    if (plot.Selecting) {
        UpdateTransformCache();
        const ImVec2 d = plot.SelectStart - IO.MousePos;
        const bool x_can_change = !ImHasFlag(IO.KeyMods,gp.InputMap.HorizontalMod) && ImFabs(d.x) > 2;
        const bool y_can_change = !ImHasFlag(IO.KeyMods,gp.InputMap.VerticalMod)   && ImFabs(d.y) > 2;
        // confirm
        if (IO.MouseReleased[gp.InputMap.BoxSelectButton] || !IO.MouseDown[gp.InputMap.BoxSelectButton]) {

            for (int i = 0; i < IMPLOT_MAX_AXES; i++) {
                if (!plot.XAxis[i].IsInputLocked() && x_can_change) {
                    ImPoint p1 = PixelsToPlot(plot.SelectStart, ImAxis_X1+i, ImAxis_Y1);
                    ImPoint p2 = PixelsToPlot(IO.MousePos, ImAxis_X1+i, ImAxis_Y1);
                    plot.XAxis[i].SetMin(ImMin(p1.x, p2.x));
                    plot.XAxis[i].SetMax(ImMax(p1.x, p2.x));
                }
                if (!plot.YAxis[i].IsInputLocked() && y_can_change) {
                    ImPoint p1 = PixelsToPlot(plot.SelectStart, ImAxis_X1, ImAxis_Y1+i);
                    ImPoint p2 = PixelsToPlot(IO.MousePos, ImAxis_X1, ImAxis_Y1+i);
                    plot.YAxis[i].SetMin(ImMin(p1.y, p2.y));
                    plot.YAxis[i].SetMax(ImMax(p1.y, p2.y));
                }
            }
            if (x_can_change || y_can_change || (ImHasFlag(IO.KeyMods,gp.InputMap.HorizontalMod) && ImHasFlag(IO.KeyMods,gp.InputMap.VerticalMod)))
                plot.ContextLocked = gp.InputMap.BoxSelectButton == gp.InputMap.ContextMenuButton;
            plot.Selected = plot.Selecting = false;
        }
        // cancel
        else if (IO.MouseClicked[gp.InputMap.BoxSelectCancelButton] || IO.MouseDown[gp.InputMap.BoxSelectCancelButton]) {
            plot.Selected = plot.Selecting = false;
            plot.ContextLocked = gp.InputMap.BoxSelectButton == gp.InputMap.ContextMenuButton;
        }
        else if (ImLengthSqr(d) > BOX_SELECT_DRAG_THRESHOLD) {
            // bad selection
            if (plot.IsInputLocked()) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
                plot.ContextLocked = gp.InputMap.BoxSelectButton == gp.InputMap.ContextMenuButton;
                plot.Selected      = false;
            }
            else {
                // TODO: Handle only min or max locked cases
                plot.SelectRect.Min.x = ImHasFlag(IO.KeyMods, gp.InputMap.HorizontalMod) || AllAxesInputLocked(plot.XAxis, IMPLOT_MAX_AXES) ? plot.PlotRect.Min.x : ImMin(plot.SelectStart.x, IO.MousePos.x);
                plot.SelectRect.Max.x = ImHasFlag(IO.KeyMods, gp.InputMap.HorizontalMod) || AllAxesInputLocked(plot.XAxis, IMPLOT_MAX_AXES) ? plot.PlotRect.Max.x : ImMax(plot.SelectStart.x, IO.MousePos.x);
                plot.SelectRect.Min.y = ImHasFlag(IO.KeyMods, gp.InputMap.VerticalMod)   || AllAxesInputLocked(plot.YAxis, IMPLOT_MAX_AXES) ? plot.PlotRect.Min.y : ImMin(plot.SelectStart.y, IO.MousePos.y);
                plot.SelectRect.Max.y = ImHasFlag(IO.KeyMods, gp.InputMap.VerticalMod)   || AllAxesInputLocked(plot.YAxis, IMPLOT_MAX_AXES) ? plot.PlotRect.Max.y : ImMax(plot.SelectStart.y, IO.MousePos.y);
                plot.SelectRect.Min  -= plot.PlotRect.Min;
                plot.SelectRect.Max  -= plot.PlotRect.Min;
                plot.Selected = true;
            }
        }
        else {
            plot.Selected = false;
        }
    }


    return changed;
}

//-----------------------------------------------------------------------------
// Setup
//-----------------------------------------------------------------------------

void SetupPlotLimits(float x_min, float x_max, float y_min, float y_max, ImGuiCond cond) {
    SetupAxisLimits(ImAxis_X1, x_min, x_max, cond);
    SetupAxisLimits(ImAxis_Y1, y_min, y_max, cond);
}

void SetupAxis(ImAxis idx, const char* label, ImPlotAxisFlags flags) {
    IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot != NULL && !GImPlot->CurrentPlot->SetupLocked,
                         "Setup needs to be called after BeginPlot and before any setup locking functions (e.g. PlotX)!");
    IM_ASSERT_USER_ERROR(!(ImHasFlag(flags, ImPlotAxisFlags_Time) && ImHasFlag(flags, ImPlotAxisFlags_LogScale)),
                         "ImPlotAxisFlags_Time and ImPlotAxisFlags_LogScale cannot be enabled at the same time!");
    IM_ASSERT_USER_ERROR(!(ImHasFlag(flags, ImPlotAxisFlags_Time) && idx >= ImAxis_Y1),
                         "Y axes cannot display time formatted labels!");

    // get globals
    ImPlotContext& gp = *GImPlot;

    // get plot and axis
    ImPlotPlot& plot = *GImPlot->CurrentPlot;
    ImPlotAxis& axis = *plot.GetAxis(idx);

    // set ID
    axis.ID = plot.ID + idx + 1;

    // check and set flags
    if (plot.JustCreated || flags != axis.PreviousFlags)
        axis.Flags = flags;
    axis.PreviousFlags = flags;

    // enable axis
    axis.Enabled = true;

    // set label
    plot.SetAxisLabel(axis,label);

    // next plot data (legacy) TODO
    double*     npd_lmin = gp.NextPlotData.LinkedMin[idx];
    double*     npd_lmax = gp.NextPlotData.LinkedMax[idx];
    bool        npd_rngh = gp.NextPlotData.HasRange[idx];
    ImGuiCond   npd_rngc = gp.NextPlotData.RangeCond[idx];
    ImLimits npd_rngv = gp.NextPlotData.Range[idx];
    bool        npd_fmth = gp.NextPlotData.HasFmt[idx];
    char*       npd_fmtc = gp.NextPlotData.Fmt[idx];
    bool        npd_deft = gp.NextPlotData.ShowDefaultTicks[idx];

    axis.LinkedMin = npd_lmin;
    axis.LinkedMax = npd_lmax;
    PullLinkedAxis(axis);
    if (npd_rngh) {
        if (!plot.Initialized || npd_rngc == ImGuiCond_Always)
            axis.SetRange(npd_rngv);
    }
    axis.HasRange         = npd_rngh;
    axis.RangeCond        = npd_rngc;
    axis.ShowDefaultTicks = npd_deft;

    axis.HasFormatSpec = npd_fmth != NULL;
    if (npd_fmth)
        ImStrncpy(axis.FormatSpec,npd_fmtc,sizeof(axis.FormatSpec));

    // cache colors
    UpdateAxisColors(axis);
}

void SetupAxisLimits(ImAxis idx, double min_lim, double max_lim, ImGuiCond cond) {
    IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot != NULL && !GImPlot->CurrentPlot->SetupLocked,
                         "Setup needs to be called after BeginPlot and before any setup locking functions (e.g. PlotX)!");    // get plot and axis
    ImPlotPlot& plot = *GImPlot->CurrentPlot;
    ImPlotAxis& axis = *plot.GetAxis(idx);
    IM_ASSERT_USER_ERROR(axis.Enabled, "Axis is not enabled! Did you forget to call SetupAxis()?");
    if (!plot.Initialized || cond == ImGuiCond_Always)
        axis.SetRange(min_lim, max_lim);
    axis.HasRange  = true;
    axis.RangeCond = cond;
}

void SetupAxisFormat(ImAxis idx, const char* fmt) {
    IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot != NULL && !GImPlot->CurrentPlot->SetupLocked,
                         "Setup needs to be called after BeginPlot and before any setup locking functions (e.g. PlotX)!");
    ImPlotPlot& plot = *GImPlot->CurrentPlot;
    ImPlotAxis& axis = *plot.GetAxis(idx);
    IM_ASSERT_USER_ERROR(axis.Enabled, "Axis is not enabled! Did you forget to call SetupAxis()?");
    axis.HasFormatSpec = fmt != NULL;
    if (fmt != NULL)
        ImStrncpy(axis.FormatSpec,fmt,sizeof(axis.FormatSpec));
}

void SetupAxisLinks(ImAxis idx, double* min_lnk, double* max_lnk) {
    IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot != NULL && !GImPlot->CurrentPlot->SetupLocked,
                         "Setup needs to be called after BeginPlot and before any setup locking functions (e.g. PlotX)!");
    ImPlotPlot& plot = *GImPlot->CurrentPlot;
    ImPlotAxis& axis = *plot.GetAxis(idx);
    IM_ASSERT_USER_ERROR(axis.Enabled, "Axis is not enabled! Did you forget to call SetupAxis()?");
    axis.LinkedMin = min_lnk;
    axis.LinkedMax = max_lnk;
    PullLinkedAxis(axis);
}

void SetupAxisFormat(ImAxis idx, void (*formatter)(double value, char* buff, int size, void* data), void* data) {
    IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot != NULL && !GImPlot->CurrentPlot->SetupLocked,
                         "Setup needs to be called after BeginPlot and before any setup locking functions (e.g. PlotX)!");
    ImPlotPlot& plot = *GImPlot->CurrentPlot;
    ImPlotAxis& axis = *plot.GetAxis(idx);
    IM_ASSERT_USER_ERROR(axis.Enabled, "Axis is not enabled! Did you forget to call SetupAxis()?");
    axis.Formatter = formatter;
    axis.FormatterData = data;
}

void SetupAxisTicks(ImAxis idx, const double* values, int n_ticks, const char* const labels[], bool show_default) {
    IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot != NULL && !GImPlot->CurrentPlot->SetupLocked,
                        "Setup needs to be called after BeginPlot and before any setup locking functions (e.g. PlotX)!");
    ImPlotPlot& plot = *GImPlot->CurrentPlot;
    ImPlotAxis& axis = *plot.GetAxis(idx);
    IM_ASSERT_USER_ERROR(axis.Enabled, "Axis is not enabled! Did you forget to call SetupAxis()?");
    axis.ShowDefaultTicks = show_default;
    AddTicksCustom(values, 
                  labels, 
                  n_ticks, 
                  *(GImPlot->XTicks + idx), // FIXME
                  axis.Formatter ? axis.Formatter : DefaultFormatter,
                  axis.HasFormatSpec ? axis.FormatSpec : IMPLOT_LABEL_FORMAT);
}

void SetupAxisTicks(ImAxis idx, double x_min, double x_max, int n_ticks, const char* const labels[], bool show_default) {
    IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot != NULL && !GImPlot->CurrentPlot->SetupLocked,
                         "Setup needs to be called after BeginPlot and before any setup locking functions (e.g. PlotX)!");
    IM_ASSERT_USER_ERROR(n_ticks > 1, "The number of ticks must be greater than 1");
    FillRange(GImPlot->Temp1, n_ticks, x_min, x_max);
    SetupAxisTicks(idx, GImPlot->Temp1.Data, n_ticks, labels, show_default);
}

void SetupLegend(ImPlotLocation location, ImPlotLegendFlags flags) {
    IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot != NULL && !GImPlot->CurrentPlot->SetupLocked,
                         "Setup needs to be called after BeginPlot and before any setup locking functions (e.g. PlotX)!");
    IM_ASSERT_USER_ERROR(GImPlot->CurrentItems != NULL, 
                         "SetupLegend() needs to be called within an itemized context!");
    ImPlotLegend& legend = GImPlot->CurrentItems->Legend;
    // check and set location
    if (location != legend.PreviousLocation)
        legend.Location = location;
    legend.PreviousLocation = location;
    // check and set flags
    if (flags != legend.PreviousFlags)
        legend.Flags = flags;
    legend.PreviousFlags = flags;
}

void SetupMouseText(ImPlotLocation location) {
    IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot != NULL && !GImPlot->CurrentPlot->SetupLocked,
                         "Setup needs to be called after BeginPlot and before any setup locking functions (e.g. PlotX)!");
    GImPlot->CurrentPlot->MousePosLocation = location;
}

//-----------------------------------------------------------------------------
// BeginPlot
//-----------------------------------------------------------------------------

bool BeginPlot(const char* title_id, const ImVec2& size, ImPlotFlags flags) {
    IM_ASSERT_USER_ERROR(GImPlot != NULL, "No current context. Did you call ImPlot::CreateContext() or ImPlot::SetCurrentContext()?");
    IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot == NULL, "Mismatched BeginPlot()/EndPlot()!");

    // FRONT MATTER -----------------------------------------------------------

    if (GImPlot->CurrentSubplot != NULL)
        ImGui::PushID(GImPlot->CurrentSubplot->CurrentIdx);

    // get globals
    ImPlotContext& gp        = *GImPlot;
    ImGuiContext &G          = *GImGui;
    ImGuiWindow* Window      = G.CurrentWindow;

    // skip if needed
    if (Window->SkipItems && !gp.CurrentSubplot) {
        ResetCtxForNextPlot(GImPlot);
        return false;
    }

    // ID and age (TODO: keep track of plot age in frames)
    const ImGuiID ID         = Window->GetID(title_id);
    const bool just_created  = gp.Plots.GetByKey(ID) == NULL;
    gp.CurrentPlot           = gp.Plots.GetOrAddByKey(ID);

    ImPlotPlot &plot         = *gp.CurrentPlot;
    plot.ID                  = ID;
    plot.Items.ID            = ID;
    plot.JustCreated         = just_created;
    plot.SetupLocked         = false;

    // check flags
    if (plot.JustCreated)
        plot.Flags = flags;
    else if (flags != plot.PreviousFlags)
        plot.Flags = flags;
    plot.PreviousFlags = flags;

    // setup default axes
    if (plot.JustCreated) {
        SetupAxis(ImAxis_X1);
        SetupAxis(ImAxis_Y1);
    }

    // reset axes
    for (int i = 0; i < IMPLOT_MAX_AXES; ++i) {
        plot.XAxis[i].Reset();
        plot.YAxis[i].Reset();
        UpdateAxisColors(plot.XAxis[i]);
        UpdateAxisColors(plot.YAxis[i]);
    }    
    // ensure first axes enabled
    plot.XAxis[0].Enabled = plot.YAxis[0].Enabled = true;
    // set initial axes
    plot.CurrentX = plot.CurrentY = 0;

    // capture scroll with a child region
    if (!ImHasFlag(plot.Flags, ImPlotFlags_NoChild)) {
        ImVec2 child_size;
        if (gp.CurrentSubplot != NULL)
            child_size = gp.CurrentSubplot->CellSize;
        else
            child_size = ImVec2(size.x == 0 ? gp.Style.PlotDefaultSize.x : size.x, size.y == 0 ? gp.Style.PlotDefaultSize.y : size.y);
        ImGui::BeginChild(title_id, child_size, false, ImGuiWindowFlags_NoScrollbar);
        Window = ImGui::GetCurrentWindow();
        Window->ScrollMax.y = 1.0f;
        gp.ChildWindowMade = true;
    }
    else {
        gp.ChildWindowMade = false;
    }

    // clear text buffers
    plot.ClearTextBuffer();
    plot.SetTitle(title_id);

    // set frame size
    ImVec2 frame_size;
    if (gp.CurrentSubplot != NULL)
        frame_size = gp.CurrentSubplot->CellSize;
    else
        frame_size = ImGui::CalcItemSize(size, gp.Style.PlotDefaultSize.x, gp.Style.PlotDefaultSize.y);

    if (frame_size.x < gp.Style.PlotMinSize.x && (size.x < 0.0f || gp.CurrentSubplot != NULL))
        frame_size.x = gp.Style.PlotMinSize.x;
    if (frame_size.y < gp.Style.PlotMinSize.y && (size.y < 0.0f || gp.CurrentSubplot != NULL))
        frame_size.y = gp.Style.PlotMinSize.y;

    plot.FrameRect = ImRect(Window->DC.CursorPos, Window->DC.CursorPos + frame_size);
    ImGui::ItemSize(plot.FrameRect);
    if (!ImGui::ItemAdd(plot.FrameRect, plot.ID, &plot.FrameRect) && !gp.CurrentSubplot) {
        ResetCtxForNextPlot(GImPlot);
        return false;
    }

    // setup items (or dont)
    if (gp.CurrentItems == NULL)
        gp.CurrentItems = &plot.Items;

    return true;
}

void SetupFinish() {
    IM_ASSERT_USER_ERROR(GImPlot != NULL, "No current context. Did you call ImPlot::CreateContext() or ImPlot::SetCurrentContext()?");
    IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot != NULL, "SetupFinish needs to be called after BeginPlot!");

    ImPlotContext& gp       = *GImPlot;
    ImGuiContext& G         = *GImGui;
    ImDrawList& DrawList    = *G.CurrentWindow->DrawList;
    const ImGuiIO&    IO    = ImGui::GetIO();
    const ImGuiStyle& Style = G.Style;

    ImPlotPlot &plot  = *gp.CurrentPlot;

    // lock setup
    plot.SetupLocked = true;

    // finalize axes
    for (int i = 0; i < IMPLOT_MAX_AXES; ++i) {
        ImPlotAxis& xax = plot.XAxis[i];
        ImPlotAxis& yax = plot.YAxis[i];
        xax.Constrain();
        yax.Constrain();
        if (!plot.Initialized && xax.CanInitFit())
            gp.FitThisFrame = gp.FitX[i] = true;
        if (!plot.Initialized && yax.CanInitFit())
            gp.FitThisFrame = gp.FitY[i] = true;
    }

    // constrain axis (maybe move to finish setup?)

    // canvas/axes bb
    plot.CanvasRect      = ImRect(plot.FrameRect.Min + gp.Style.PlotPadding, plot.FrameRect.Max - gp.Style.PlotPadding);
    plot.AxesRect        = plot.FrameRect;

    // outside legend adjustments
    if (!ImHasFlag(plot.Flags, ImPlotFlags_NoLegend) && plot.Items.GetLegendCount() > 0 && ImHasFlag(plot.Items.Legend.Flags, ImPlotLegendFlags_Outside)) { 
        ImPlotLegend& legend = plot.Items.Legend;
        const bool horz = ImHasFlag(legend.Flags, ImPlotLegendFlags_Horizontal);
        const ImVec2 legend_size = CalcLegendSize(plot.Items, gp.Style.LegendInnerPadding, gp.Style.LegendSpacing, !horz);
        const bool west = ImHasFlag(legend.Location, ImPlotLocation_West) && !ImHasFlag(legend.Location, ImPlotLocation_East);
        const bool east = ImHasFlag(legend.Location, ImPlotLocation_East) && !ImHasFlag(legend.Location, ImPlotLocation_West);
        const bool north = ImHasFlag(legend.Location, ImPlotLocation_North) && !ImHasFlag(legend.Location, ImPlotLocation_South);
        const bool south = ImHasFlag(legend.Location, ImPlotLocation_South) && !ImHasFlag(legend.Location, ImPlotLocation_North);
        if ((west && !horz) || (west && horz && !north && !south)) {
            plot.CanvasRect.Min.x += (legend_size.x + gp.Style.LegendPadding.x);
            plot.AxesRect.Min.x   += (legend_size.x + gp.Style.PlotPadding.x);
        }
        if ((east && !horz) || (east && horz && !north && !south)) {
            plot.CanvasRect.Max.x -= (legend_size.x + gp.Style.LegendPadding.x);
            plot.AxesRect.Max.x   -= (legend_size.x + gp.Style.PlotPadding.x);
        }
        if ((north && horz) || (north && !horz && !west && !east)) {
            plot.CanvasRect.Min.y += (legend_size.y + gp.Style.LegendPadding.y);
            plot.AxesRect.Min.y   += (legend_size.y + gp.Style.PlotPadding.y);
        }
        if ((south && horz) || (south && !horz && !west && !east)) {
            plot.CanvasRect.Max.y -= (legend_size.y + gp.Style.LegendPadding.y);
            plot.AxesRect.Max.y   -= (legend_size.y + gp.Style.PlotPadding.y);
        }
    }

    // plot bb
    float pad_top = 0, pad_bot = 0, pad_left = 0, pad_right = 0;

    // (0) calc top padding form title
    ImVec2 title_size(0.0f, 0.0f);
    if (plot.HasTitle())
         title_size = ImGui::CalcTextSize(plot.GetTitle(), NULL, true);
    if (title_size.x > 0) {
        pad_top += title_size.y + gp.Style.LabelPadding.y;
        plot.AxesRect.Min.y += gp.Style.PlotPadding.y + pad_top;
    }

    // (1) calc addition top padding and bot padding
    PadAndDatumAxesX(plot,pad_top,pad_bot);

    // (1*) align plots group (TODO: account for outside legends!)
    if (gp.CurrentAlignmentH)
        gp.CurrentAlignmentH->Update(pad_top,pad_bot);

    const float plot_height = plot.CanvasRect.GetHeight() - pad_top - pad_bot;

    // (2) get y tick labels (needed for left/right pad)
    for (int i = 0; i < IMPLOT_MAX_AXES; i++) {
        ImPlotAxis& axis = plot.YAxis[i];
        if (axis.WillRender() && axis.ShowDefaultTicks) {
            if (axis.IsLog())
                AddTicksLogarithmic(axis.Range,
                                    plot_height,
                                    true,
                                    gp.YTicks[i],
                                    axis.Formatter     ? axis.Formatter  : DefaultFormatter,
                                    axis.HasFormatSpec ? axis.FormatSpec : IMPLOT_LABEL_FORMAT);
            else
                AddTicksDefault(axis.Range,
                                plot_height,
                                true,
                                gp.YTicks[i],
                                axis.Formatter     ? axis.Formatter  : DefaultFormatter,
                                axis.HasFormatSpec ? axis.FormatSpec : IMPLOT_LABEL_FORMAT);
        }
    }

    // (3) calc left/right pad
    PadAndDatumAxesY(plot,pad_left,pad_right);

    // (3*) align plots group (TODO: account for outside legends!)
    if (gp.CurrentAlignmentV)
        gp.CurrentAlignmentV->Update(pad_left,pad_right);

    const float plot_width = plot.CanvasRect.GetWidth() - pad_left - pad_right;

    // (4) get x ticks
    for (int i = 0; i < IMPLOT_MAX_AXES; i++) {
        ImPlotAxis& axis = plot.XAxis[i];
        if (axis.WillRender() && axis.ShowDefaultTicks) {
            if (axis.IsTime())
                AddTicksTime(axis.Range, plot_width, gp.XTicks[i]);
            else if (axis.IsLog())
                AddTicksLogarithmic(axis.Range,
                                    plot_width,
                                    false,
                                    gp.XTicks[i],
                                    axis.Formatter     ? axis.Formatter  : DefaultFormatter,
                                    axis.HasFormatSpec ? axis.FormatSpec : IMPLOT_LABEL_FORMAT);
            else
                AddTicksDefault(axis.Range,
                                plot_width,
                                false,
                                gp.XTicks[i],
                                axis.Formatter     ? axis.Formatter  : DefaultFormatter,
                                axis.HasFormatSpec ? axis.FormatSpec : IMPLOT_LABEL_FORMAT);
        }
    }

    // (5) calc plot bb
    plot.PlotRect  = ImRect(plot.CanvasRect.Min + ImVec2(pad_left, pad_top), plot.CanvasRect.Max - ImVec2(pad_right, pad_bot));

    // HOVER------------------------------------------------------------

    // axes hover rect, aspect ratio
    for (int i = 0; i < IMPLOT_MAX_AXES; ++i) {
        ImPlotAxis& xax = plot.XAxis[i];
        xax.HoverRect   = ImRect(ImVec2(plot.PlotRect.Min.x, ImMin(xax.Datum1,xax.Datum2)),
                                 ImVec2(plot.PlotRect.Max.x, ImMax(xax.Datum1,xax.Datum2)));
        xax.Pixels      = plot.PlotRect.GetWidth();
        ImPlotAxis& yax = plot.YAxis[i];
        yax.HoverRect   = ImRect(ImVec2(ImMin(yax.Datum1,yax.Datum2),plot.PlotRect.Min.y),
                                 ImVec2(ImMax(yax.Datum1,yax.Datum2),plot.PlotRect.Max.y));
        yax.Pixels      = plot.PlotRect.GetHeight();
    }

    // Equal axis constraint. Must happen after we set Pixels
    // constrain equal axes for primary x and y if not approximately equal
    // constrains x to y since x pixel size depends on y labels width, and causes feedback loops in opposite case
    if (ImHasFlag(plot.Flags, ImPlotFlags_Equal)) {
        for (int i = 0; i < IMPLOT_MAX_AXES; ++i) {
            double xar = plot.XAxis[i].GetAspect();
            double yar = plot.YAxis[i].GetAspect();
            // edge case: user has set x range this frame, so fit y to x so that we honor their request for x range
            // NB: because of feedback across several frames, the user's x request may not be perfectly honored
            if (plot.XAxis[i].HasRange) {
                plot.YAxis[i].SetAspect(xar);
            }
            else {
                if (!ImAlmostEqual(xar,yar) && !plot.YAxis[i].IsInputLocked())
                    plot.XAxis[i].SetAspect(yar);
            }
        }
    }

    // INPUT ------------------------------------------------------------------
    UpdateInput(plot);
    UpdateTransformCache();

    // fit from FitNextPlotAxes or auto fit
    for (int i = 0; i < IMPLOT_MAX_AXES; ++i) {
        if (gp.NextPlotData.Fit[ImAxis_X1+i] || plot.XAxis[i].IsAutoFitting()) {
            gp.FitThisFrame = true;
            gp.FitX[i]      = true;
        }
        if (gp.NextPlotData.Fit[ImAxis_Y1+i] || plot.YAxis[i].IsAutoFitting()) {
            gp.FitThisFrame = true;
            gp.FitY[i]      = true;
        }
    }

    // set mouse position
    for (int i = 0; i < IMPLOT_MAX_AXES; i++) {
        gp.MousePos[i] = PixelsToPlot(IO.MousePos, ImAxis_X1+i, ImAxis_Y1+i);
    }

    // RENDER -----------------------------------------------------------------

    // render frame
    ImGui::RenderFrame(plot.FrameRect.Min, plot.FrameRect.Max, GetStyleColorU32(ImPlotCol_FrameBg), true, Style.FrameRounding);

    // grid bg
    DrawList.AddRectFilled(plot.PlotRect.Min, plot.PlotRect.Max, GetStyleColorU32(ImPlotCol_PlotBg));

    // transform ticks (TODO: Move this into ImPlotTickCollection)

    for (int i = 0; i < IMPLOT_MAX_AXES; i++) {
        if (plot.XAxis[i].WillRender()) {
            for (int t = 0; t < gp.XTicks[i].Size; t++) {
                ImPlotTick *xt = &gp.XTicks[i].Ticks[t];
                xt->PixelPos = IM_ROUND(PlotToPixels(xt->PlotPos, 0, ImAxis_X1+i, ImAxis_Y1).x);
            }
        }
        if (plot.YAxis[i].WillRender()) {
            for (int t = 0; t < gp.YTicks[i].Size; t++) {
                ImPlotTick *yt = &gp.YTicks[i].Ticks[t];
                yt->PixelPos = IM_ROUND(PlotToPixels(0, yt->PlotPos, ImAxis_X1, ImAxis_Y1+i).y);
            }
        }
    }

    // render grid (background)
    for (int i = 0; i < IMPLOT_MAX_AXES; i++) {
        if (plot.XAxis[i].Enabled && plot.XAxis[i].HasGridLines() && !plot.XAxis[i].IsForeground())
            RenderGridLinesX(DrawList, gp.XTicks[i], plot.PlotRect, plot.XAxis[i].ColorMaj, plot.XAxis[i].ColorMin, gp.Style.MajorGridSize.x, gp.Style.MinorGridSize.x);
        if (plot.YAxis[i].Enabled && plot.YAxis[i].HasGridLines() && !plot.YAxis[i].IsForeground())
            RenderGridLinesY(DrawList, gp.YTicks[i], plot.PlotRect,  plot.YAxis[i].ColorMaj,  plot.YAxis[i].ColorMin, gp.Style.MajorGridSize.y, gp.Style.MinorGridSize.y);
    }

    // clear legend (TODO: put elsewhere)
    plot.Items.Legend.Reset();
    // push ID to set item hashes (TODO: SetupFinish?)
    ImGui::PushOverrideID(gp.CurrentItems->ID);
}

//-----------------------------------------------------------------------------
// EndPlot()
//-----------------------------------------------------------------------------

void EndPlot() {
    IM_ASSERT_USER_ERROR(GImPlot != NULL, "No current context. Did you call ImPlot::CreateContext() or ImPlot::SetCurrentContext()?");
    IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot != NULL, "Mismatched BeginPlot()/EndPlot()!");

    SetupLock();

    ImPlotContext& gp     = *GImPlot;
    ImGuiContext &G       = *GImGui;
    ImPlotPlot &plot      = *gp.CurrentPlot;
    ImGuiWindow * Window  = G.CurrentWindow;
    ImDrawList & DrawList = *Window->DrawList;
    const ImGuiIO &   IO  = ImGui::GetIO();

    // FINAL RENDER -----------------------------------------------------------

    const float txt_height     = ImGui::GetTextLineHeight();
    const bool  render_border  = gp.Style.PlotBorderSize > 0 && gp.Style.Colors[ImPlotCol_PlotBorder].z > 0;
    const bool  any_x_held     = plot.Held || AnyAxesHeld(plot.XAxis, IMPLOT_MAX_AXES);
    const bool  any_y_held     = plot.Held || AnyAxesHeld(plot.YAxis, IMPLOT_MAX_AXES);

    ImGui::PushClipRect(plot.FrameRect.Min, plot.FrameRect.Max, true);

    // render grid (foreground)
    for (int i = 0; i < IMPLOT_MAX_AXES; i++) {
        if (plot.XAxis[i].Enabled && plot.XAxis[i].HasGridLines() && plot.XAxis[i].IsForeground())
            RenderGridLinesX(DrawList, gp.XTicks[i], plot.PlotRect, plot.XAxis[i].ColorMaj, plot.XAxis[i].ColorMaj, gp.Style.MajorGridSize.x, gp.Style.MinorGridSize.x);
        if (plot.YAxis[i].Enabled && plot.YAxis[i].HasGridLines() && plot.YAxis[i].IsForeground())
            RenderGridLinesY(DrawList, gp.YTicks[i], plot.PlotRect,  plot.YAxis[i].ColorMaj,  plot.YAxis[i].ColorMin, gp.Style.MajorGridSize.y, gp.Style.MinorGridSize.y);
    }


    // render title
    if (plot.HasTitle()) {
        ImU32 col = GetStyleColorU32(ImPlotCol_TitleText);
        AddTextCentered(&DrawList,ImVec2(plot.PlotRect.GetCenter().x, plot.CanvasRect.Min.y),col,plot.GetTitle());
    }

    // render x hover, labels, ticks and tick labels
    int count_B = 0, count_T = 0;
    for (int i = 0; i < IMPLOT_MAX_AXES; i++) {
        const ImPlotAxis& ax = plot.XAxis[i];
        if (!ax.Enabled)
            continue;
        if ((ax.Hovered || ax.Held) && !plot.Held)
            DrawList.AddRectFilled(ax.HoverRect.Min, ax.HoverRect.Max, ax.Held ? ax.ColorAct : ax.ColorHov);
        else if (ax.ColorHiLi != IM_COL32_BLACK)
            DrawList.AddRectFilled(ax.HoverRect.Min, ax.HoverRect.Max, ax.ColorHiLi);
        const ImPlotTickCollection& tkc = gp.XTicks[i];
        const bool opp = ax.IsOpposite();
        const bool aux = ((opp && count_T > 0)||(!opp && count_B > 0));
        if (ax.HasLabel()) {
            const char* label        = plot.GetAxisLabel(ax);
            const ImVec2 label_size  = ImGui::CalcTextSize(label);
            const float label_offset = (ax.HasTickLabels() ? gp.XTicks[i].MaxHeight + gp.Style.LabelPadding.y : 0.0f)
                                     + (ax.IsTime() ? txt_height + gp.Style.LabelPadding.y : 0)
                                     + gp.Style.LabelPadding.y;
            const ImVec2 label_pos(plot.PlotRect.GetCenter().x - label_size.x * 0.5f,
                                   opp ? ax.Datum1 - label_offset - label_size.y : ax.Datum1 + label_offset);
            DrawList.AddText(label_pos, ax.ColorTxt, label);
        }
        if (ax.HasTickMarks()) {
            const float direction = opp ? 1.0f : -1.0f;
            for (int j = 0; j < tkc.Size; ++j) {
                const ImPlotTick& tk = tkc.Ticks[j];
                if (tk.Level != 0 || tk.PixelPos < plot.PlotRect.Min.x || tk.PixelPos > plot.PlotRect.Max.x)
                    continue;
                const ImVec2 start(tk.PixelPos, ax.Datum1);
                const float len = (!aux && tk.Major) ? gp.Style.MajorTickLen.x  : gp.Style.MinorTickLen.x;
                const float thk = (!aux && tk.Major) ? gp.Style.MajorTickSize.x : gp.Style.MinorTickSize.x;
                DrawList.AddLine(start, start + ImVec2(0,direction*len), ax.ColorMaj, thk);
            }
            if (aux || !render_border)
                DrawList.AddLine(ImVec2(plot.PlotRect.Min.x,ax.Datum1), ImVec2(plot.PlotRect.Max.x,ax.Datum1), ax.ColorMaj, gp.Style.MinorTickSize.x);
        }
        if (ax.HasTickLabels()) {
            for (int j = 0; j < tkc.Size; ++j) {
                const ImPlotTick& tk = tkc.Ticks[j];
                const float datum = ax.Datum1 + (opp ? (-gp.Style.LabelPadding.y -txt_height -tk.Level * (txt_height + gp.Style.LabelPadding.y))
                                                     : gp.Style.LabelPadding.y + tk.Level * (txt_height + gp.Style.LabelPadding.y));
                if (tk.ShowLabel && tk.PixelPos >= plot.PlotRect.Min.x - 1 && tk.PixelPos <= plot.PlotRect.Max.x + 1) {
                    ImVec2 start(tk.PixelPos - 0.5f * tk.LabelSize.x, datum);
                    DrawList.AddText(start, ax.ColorTxt, tkc.GetText(j));
                }
            }
        }
        count_B += !opp;
        count_T +=  opp;
    }

    // render y hover, labels, ticks and tick labels
    int count_L = 0, count_R = 0;
    for (int i = 0; i < IMPLOT_MAX_AXES; i++) {
        const ImPlotAxis& ax = plot.YAxis[i];
        if (!ax.Enabled)
            continue;
        if ((ax.Hovered || ax.Held) && !plot.Held)
            DrawList.AddRectFilled(ax.HoverRect.Min, ax.HoverRect.Max, ax.Held ? ax.ColorAct : ax.ColorHov);
        else if (ax.ColorHiLi != IM_COL32_BLACK)
            DrawList.AddRectFilled(ax.HoverRect.Min, ax.HoverRect.Max, ax.ColorHiLi);
        const ImPlotTickCollection& tkc = gp.YTicks[i];
        const bool opp = ax.IsOpposite();
        const bool aux = ((opp && count_R > 0)||(!opp && count_L > 0));
        if (ax.HasLabel()) {
            const char* label        = plot.GetAxisLabel(ax);
            const ImVec2 label_size  = CalcTextSizeVertical(label);
            const float label_offset = (ax.HasTickLabels() ? gp.YTicks[i].MaxWidth + gp.Style.LabelPadding.x : 0.0f)
                                     + gp.Style.LabelPadding.x;
            const ImVec2 label_pos(opp ? ax.Datum1 + label_offset : ax.Datum1 - label_offset - label_size.x,
                                   plot.PlotRect.GetCenter().y + label_size.y * 0.5f);
            AddTextVertical(&DrawList, label_pos, ax.ColorTxt, label);
        }
        if (ax.HasTickMarks()) {
            const float direction = opp ? -1.0f : 1.0f;
            for (int j = 0; j < tkc.Size; ++j) {
                const ImPlotTick& tk = tkc.Ticks[j];
                if (tk.Level != 0 || tk.PixelPos < plot.PlotRect.Min.y || tk.PixelPos > plot.PlotRect.Max.y)
                    continue;
                const ImVec2 start(ax.Datum1, tk.PixelPos);
                const float len = (!aux && tk.Major) ? gp.Style.MajorTickLen.y  : gp.Style.MinorTickLen.y;
                const float thk = (!aux && tk.Major) ? gp.Style.MajorTickSize.y : gp.Style.MinorTickSize.y;
                DrawList.AddLine(start, start + ImVec2(direction*len,0), ax.ColorMaj, thk);
            }
            if (aux || !render_border)
                DrawList.AddLine(ImVec2(ax.Datum1, plot.PlotRect.Min.y), ImVec2(ax.Datum1, plot.PlotRect.Max.y), ax.ColorMaj, gp.Style.MinorTickSize.y);
        }
        if (ax.HasTickLabels()) {
            for (int j = 0; j < tkc.Size; ++j) {
                const ImPlotTick& tk = tkc.Ticks[j];
                const float datum = ax.Datum1 + (opp ? gp.Style.LabelPadding.x : (-gp.Style.LabelPadding.x - tk.LabelSize.x));
                if (tk.ShowLabel && tk.PixelPos >= plot.PlotRect.Min.y - 1 && tk.PixelPos <= plot.PlotRect.Max.y + 1) {
                    ImVec2 start(datum, tk.PixelPos - 0.5f * tk.LabelSize.y);
                    DrawList.AddText(start, ax.ColorTxt, tkc.GetText(j));
                }
            }
        }
        count_L += !opp;
        count_R +=  opp;
    }
    ImGui::PopClipRect();

    // render annotations
    PushPlotClipRect();
    for (int i = 0; i < gp.Annotations.Size; ++i) {
        const char* txt       = gp.Annotations.GetText(i);
        ImPlotAnnotation& an  = gp.Annotations.Annotations[i];
        const ImVec2 txt_size = ImGui::CalcTextSize(txt);
        const ImVec2 size     = txt_size + gp.Style.AnnotationPadding * 2;
        ImVec2 pos            = an.Pos;
        if (an.Offset.x == 0)
            pos.x -= size.x / 2;
        else if (an.Offset.x > 0)
            pos.x += an.Offset.x;
        else
            pos.x -= size.x - an.Offset.x;
        if (an.Offset.y == 0)
            pos.y -= size.y / 2;
        else if (an.Offset.y > 0)
            pos.y += an.Offset.y;
        else
            pos.y -= size.y - an.Offset.y;
        if (an.Clamp)
            pos = ClampLabelPos(pos, size, plot.PlotRect.Min, plot.PlotRect.Max);
        ImRect rect(pos,pos+size);
        if (an.Offset.x != 0 || an.Offset.y != 0) {
            ImVec2 corners[4] = {rect.GetTL(), rect.GetTR(), rect.GetBR(), rect.GetBL()};
            int min_corner = 0;
            float min_len = FLT_MAX;
            for (int c = 0; c < 4; ++c) {
                float len = ImLengthSqr(an.Pos - corners[c]);
                if (len < min_len) {
                    min_corner = c;
                    min_len = len;
                }
            }
            DrawList.AddLine(an.Pos, corners[min_corner], an.ColorBg);
        }
        DrawList.AddRectFilled(rect.Min, rect.Max, an.ColorBg);
        DrawList.AddText(pos + gp.Style.AnnotationPadding, an.ColorFg, txt);
    }

    // render selection
    if (plot.Selected)
        RenderSelectionRect(DrawList, plot.SelectRect.Min + plot.PlotRect.Min, plot.SelectRect.Max + plot.PlotRect.Min, GetStyleColorVec4(ImPlotCol_Selection));

    // render crosshairs
    if (ImHasFlag(plot.Flags, ImPlotFlags_Crosshairs) && plot.Hovered && !(any_x_held || any_y_held) && !plot.Selecting && !plot.Items.Legend.Hovered) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_None);
        ImVec2 xy = IO.MousePos;
        ImVec2 h1(plot.PlotRect.Min.x, xy.y);
        ImVec2 h2(xy.x - 5, xy.y);
        ImVec2 h3(xy.x + 5, xy.y);
        ImVec2 h4(plot.PlotRect.Max.x, xy.y);
        ImVec2 v1(xy.x, plot.PlotRect.Min.y);
        ImVec2 v2(xy.x, xy.y - 5);
        ImVec2 v3(xy.x, xy.y + 5);
        ImVec2 v4(xy.x, plot.PlotRect.Max.y);
        ImU32 col = GetStyleColorU32(ImPlotCol_Crosshairs);
        DrawList.AddLine(h1, h2, col);
        DrawList.AddLine(h3, h4, col);
        DrawList.AddLine(v1, v2, col);
        DrawList.AddLine(v3, v4, col);
    }

    // render mouse pos
    if (!ImHasFlag(plot.Flags, ImPlotFlags_NoMouseText) && plot.Hovered) {
        // char buffer[128] = {};
        // ImBufferWriter writer(buffer, sizeof(buffer));
        // // x
        // if (plot.XAxis.IsTime()) {
        //     ImPlotTimeUnit unit = GetUnitForRange(plot.XAxis.Range.Size() / (plot.PlotRect.GetWidth() / 100));
        //     const int written = FormatDateTime(ImPlotTime::FromDouble(gp.MousePos[0].x), &writer.Buffer[writer.Pos], writer.Size - writer.Pos - 1, GetDateTimeFmt(TimeFormatMouseCursor, unit));
        //     if (written > 0)
        //         writer.Pos += ImMin(written, writer.Size - writer.Pos - 1);
        // }
        // else {
        //     writer.Write(plot.XAxis.Fmt, RoundAxisValue(plot.XAxis, gp.XTicks, gp.MousePos[0].x));
        // }
        // // y1
        // writer.Write(", ");
        // writer.Write(plot.YAxis[0].Fmt, RoundAxisValue(plot.YAxis[0], gp.YTicks[0], gp.MousePos[0].y));
        // // y2
        // if (plot.YAxis[1].Enabled) {
        //     writer.Write(", (");
        //     writer.Write(plot.YAxis[1].Fmt, RoundAxisValue(plot.YAxis[1], gp.YTicks[1], gp.MousePos[1].y));
        //     writer.Write(")");
        // }
        // // y3
        // if (plot.YAxis[2].Enabled) {
        //     writer.Write(", (");
        //     writer.Write(plot.YAxis[2].Fmt, RoundAxisValue(plot.YAxis[2], gp.YTicks[2], gp.MousePos[2].y));
        //     writer.Write(")");
        // }
        // const ImVec2 size = ImGui::CalcTextSize(buffer);
        // const ImVec2 pos = GetLocationPos(plot.PlotRect, size, plot.MousePosLocation, gp.Style.MousePosPadding);
        // DrawList.AddText(pos, GetStyleColorU32(ImPlotCol_InlayText), buffer);
    }
    PopPlotClipRect();

    // reset legend hovers
    plot.Items.Legend.Hovered = false;
    for (int i = 0; i < plot.Items.GetItemCount(); ++i)
        plot.Items.GetItemByIndex(i)->LegendHovered = false;
    // render legend
    if (!ImHasFlag(plot.Flags, ImPlotFlags_NoLegend) && plot.Items.GetLegendCount() > 0) {
        ImPlotLegend& legend = plot.Items.Legend;
        const bool   legend_out  = ImHasFlag(legend.Flags, ImPlotLegendFlags_Outside);
        const bool   legend_horz = ImHasFlag(legend.Flags, ImPlotLegendFlags_Horizontal);
        const ImVec2 legend_size = CalcLegendSize(plot.Items, gp.Style.LegendInnerPadding, gp.Style.LegendSpacing, !legend_horz);
        const ImVec2 legend_pos  = GetLocationPos(legend_out ? plot.FrameRect : plot.PlotRect,
                                                  legend_size,
                                                  legend.Location,
                                                  legend_out ? gp.Style.PlotPadding : gp.Style.LegendPadding);
        legend.Rect = ImRect(legend_pos, legend_pos + legend_size);
        // test hover
        legend.Hovered = ImGui::IsWindowHovered() && legend.Rect.Contains(IO.MousePos);

        if (legend_out)
            ImGui::PushClipRect(plot.FrameRect.Min, plot.FrameRect.Max, true);
        else
            PushPlotClipRect();
        ImU32  col_bg      = GetStyleColorU32(ImPlotCol_LegendBg);
        ImU32  col_bd      = GetStyleColorU32(ImPlotCol_LegendBorder);
        DrawList.AddRectFilled(legend.Rect.Min, legend.Rect.Max, col_bg);
        DrawList.AddRect(legend.Rect.Min, legend.Rect.Max, col_bd);
        bool legend_contextable = ShowLegendEntries(plot.Items, legend.Rect, legend.Hovered, gp.Style.LegendInnerPadding, gp.Style.LegendSpacing, !legend_horz, DrawList)
                                && !ImHasFlag(legend.Flags, ImPlotLegendFlags_NoMenus);
        // main ctx menu
        if (legend_contextable && !ImHasFlag(plot.Flags, ImPlotFlags_NoMenus) && IO.MouseReleased[gp.InputMap.ContextMenuButton] && !plot.ContextLocked)
            ImGui::OpenPopup("##LegendContext");
        ImGui::PopClipRect();
        if (ImGui::BeginPopup("##LegendContext")) {
            ImGui::Text("Legend"); ImGui::Separator();
            if (ShowLegendContextMenu(legend, !ImHasFlag(plot.Flags, ImPlotFlags_NoLegend)))
                ImFlipFlag(plot.Flags, ImPlotFlags_NoLegend);
            ImGui::EndPopup();
        }
    }
    else {
        plot.Items.Legend.Rect = ImRect();
    }

    // render border
    if (render_border)
        DrawList.AddRect(plot.PlotRect.Min, plot.PlotRect.Max, GetStyleColorU32(ImPlotCol_PlotBorder), 0, ImDrawFlags_RoundCornersAll, gp.Style.PlotBorderSize);

    // FIT DATA --------------------------------------------------------------
    const bool axis_equal = ImHasFlag(plot.Flags, ImPlotFlags_Equal);
    if (gp.FitThisFrame) {
        for (int i = 0; i < IMPLOT_MAX_AXES; i++) {
            ImPlotAxis& xax = plot.XAxis[i];
            ImPlotAxis& yax = plot.YAxis[i];
            if (gp.FitX[i]) {
                const double ext_size = gp.ExtentsX[i].Size() * 0.5;
                gp.ExtentsX[i].Min -= ext_size * gp.Style.FitPadding.x;
                gp.ExtentsX[i].Max += ext_size * gp.Style.FitPadding.x;
                if (!xax.IsLockedMin() && !ImNanOrInf(gp.ExtentsX[i].Min))
                    xax.Range.Min = (gp.ExtentsX[i].Min);
                if (!xax.IsLockedMax() && !ImNanOrInf(gp.ExtentsX[i].Max))
                    xax.Range.Max = (gp.ExtentsX[i].Max);
                if (ImAlmostEqual(xax.Range.Max, xax.Range.Min))  {
                    xax.Range.Max += 0.5;
                    xax.Range.Min -= 0.5;
                }
                xax.Constrain();
                if (axis_equal && !gp.FitY[0])
                    yax.SetAspect(xax.GetAspect());
            }

            if (gp.FitY[i]) {
                const double ext_size = gp.ExtentsY[i].Size() * 0.5;
                gp.ExtentsY[i].Min -= ext_size * gp.Style.FitPadding.y;
                gp.ExtentsY[i].Max += ext_size * gp.Style.FitPadding.y;
                if (!yax.IsLockedMin() && !ImNanOrInf(gp.ExtentsY[i].Min))
                    yax.Range.Min = (gp.ExtentsY[i].Min);
                if (!yax.IsLockedMax() && !ImNanOrInf(gp.ExtentsY[i].Max))
                    yax.Range.Max = (gp.ExtentsY[i].Max);
                if (ImAlmostEqual(yax.Range.Max, yax.Range.Min)) {
                    yax.Range.Max += 0.5;
                    yax.Range.Min -= 0.5;
                }
                yax.Constrain();
                if (axis_equal && !gp.FitX)
                    plot.XAxis[i].SetAspect(yax.GetAspect());
            }

            // was outside loop
            if (axis_equal && gp.FitX[i] && gp.FitY[i]) {
                double aspect = ImMax(xax.GetAspect(), yax.GetAspect());
                xax.SetAspect(aspect);
                yax.SetAspect(aspect);
            }
        }
    }

    // CONTEXT MENUS -----------------------------------------------------------

    ImGui::PushOverrideID(plot.ID);

    const bool can_ctx = !ImHasFlag(plot.Flags, ImPlotFlags_NoMenus) && 
                         !plot.Items.Legend.Hovered                  && 
                         !plot.ContextLocked                         &&
                         IO.MouseReleased[gp.InputMap.ContextMenuButton];


    // main ctx menu
    if (can_ctx && plot.Hovered)
        ImGui::OpenPopup("##PlotContext");
    if (ImGui::BeginPopup("##PlotContext")) {
        ShowPlotContextMenu(plot);
        ImGui::EndPopup();
    }

    // axes ctx menus
    for (int i = 0; i < IMPLOT_MAX_AXES; ++i) {
        ImGui::PushID(i);
        if (can_ctx && plot.XAxis[i].Hovered && plot.XAxis[i].HasMenus()) // TODO: clicked
            ImGui::OpenPopup("##XContext");
        if (ImGui::BeginPopup("##XContext")) {
            ImGui::Text(i == 0 ? "X-Axis" : "X-Axis %d", i + 1); ImGui::Separator();
            ShowAxisContextMenu(plot.XAxis[i], ImHasFlag(plot.Flags, ImPlotFlags_Equal) ? &plot.YAxis[i] : NULL, true);
            ImGui::EndPopup();
        }
        if (can_ctx && plot.YAxis[i].Hovered && plot.YAxis[i].HasMenus()) // TODO: clicked
            ImGui::OpenPopup("##YContext");
        if (ImGui::BeginPopup("##YContext")) {
            ImGui::Text(i == 0 ? "Y-Axis" : "Y-Axis %d", i + 1); ImGui::Separator();
            ShowAxisContextMenu(plot.YAxis[i], ImHasFlag(plot.Flags, ImPlotFlags_Equal) ? &plot.XAxis[i] : NULL, false);
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
    ImGui::PopID();

    // LINKED AXES ------------------------------------------------------------

    for (int i = 0; i < IMPLOT_MAX_AXES; ++i) {
        PushLinkedAxis(plot.XAxis[i]);
        PushLinkedAxis(plot.YAxis[i]);
    }

    // CLEANUP ----------------------------------------------------------------

    // resset context locked flag
    if (plot.ContextLocked && IO.MouseReleased[gp.InputMap.BoxSelectButton])
        plot.ContextLocked = false;

    // remove items
    if (gp.CurrentItems == &plot.Items)
        gp.CurrentItems = NULL;
    // reset the plot items for the next frame
    for (int i = 0; i < plot.Items.GetItemCount(); ++i) {
        plot.Items.GetItemByIndex(i)->SeenThisFrame = false;
    }

    // mark the plot as initialized, i.e. having made it through one frame completely
    plot.Initialized = true;
    // Pop ImGui::PushID at the end of BeginPlot
    ImGui::PopID();
    // Reset context for next plot
    ResetCtxForNextPlot(GImPlot);

    // setup next subplot
    if (gp.CurrentSubplot != NULL) {
        ImGui::PopID();
        SubplotNextCell();
    }
}

//-----------------------------------------------------------------------------
// BEGIN/END SUBPLOT
//-----------------------------------------------------------------------------

static const float SUBPLOT_BORDER_SIZE             = 1.0f;
static const float SUBPLOT_SPLITTER_HALF_THICKNESS = 4.0f;
static const float SUBPLOT_SPLITTER_FEEDBACK_TIMER = 0.06f;

void SubplotSetCell(int row, int col) {
    ImPlotContext& gp      = *GImPlot;
    ImPlotSubplot& subplot = *gp.CurrentSubplot;
    if (row >= subplot.Rows || col >= subplot.Cols)
        return;
    float xoff = 0;
    float yoff = 0;
    for (int c = 0; c < col; ++c)
        xoff += subplot.ColRatios[c];
    for (int r = 0; r < row; ++r)
        yoff += subplot.RowRatios[r];
    const ImVec2 grid_size = subplot.GridRect.GetSize();
    ImVec2 cpos            = subplot.GridRect.Min + ImVec2(xoff*grid_size.x,yoff*grid_size.y);
    cpos.x = IM_ROUND(cpos.x);
    cpos.y = IM_ROUND(cpos.y);
    ImGui::GetCurrentWindow()->DC.CursorPos =  cpos;
    // set cell size
    subplot.CellSize.x = IM_ROUND(subplot.GridRect.GetWidth()  * subplot.ColRatios[col]);
    subplot.CellSize.y = IM_ROUND(subplot.GridRect.GetHeight() * subplot.RowRatios[row]);
    // setup links
    const bool lx = ImHasFlag(subplot.Flags, ImPlotSubplotFlags_LinkAllX);
    const bool ly = ImHasFlag(subplot.Flags, ImPlotSubplotFlags_LinkAllY);
    const bool lr = ImHasFlag(subplot.Flags, ImPlotSubplotFlags_LinkRows);
    const bool lc = ImHasFlag(subplot.Flags, ImPlotSubplotFlags_LinkCols);
    LinkNextPlotLimits(lx ? &subplot.ColLinkData[0].Min : lc ? &subplot.ColLinkData[col].Min : NULL,
                       lx ? &subplot.ColLinkData[0].Max : lc ? &subplot.ColLinkData[col].Max : NULL,
                       ly ? &subplot.RowLinkData[0].Min : lr ? &subplot.RowLinkData[row].Min : NULL,
                       ly ? &subplot.RowLinkData[0].Max : lr ? &subplot.RowLinkData[row].Max : NULL);
    // setup alignment
    if (!ImHasFlag(subplot.Flags, ImPlotSubplotFlags_NoAlign)) {
        gp.CurrentAlignmentH = &subplot.RowAlignmentData[row];
        gp.CurrentAlignmentV = &subplot.ColAlignmentData[col];
    }
    // set idx
    if (ImHasFlag(subplot.Flags, ImPlotSubplotFlags_ColMajor))
        subplot.CurrentIdx = col * subplot.Rows + row;
    else
        subplot.CurrentIdx = row * subplot.Cols + col;
}

void SubplotSetCell(int idx) {
    ImPlotContext& gp      = *GImPlot;
    ImPlotSubplot& subplot = *gp.CurrentSubplot;
    if (idx >= subplot.Rows * subplot.Cols)
        return;
    int row = 0, col = 0;
    if (ImHasFlag(subplot.Flags, ImPlotSubplotFlags_ColMajor)) {
        row = idx % subplot.Rows;
        col = idx / subplot.Rows;
    }
    else {
        row = idx / subplot.Cols;
        col = idx % subplot.Cols;
    }
    return SubplotSetCell(row, col);
}

void SubplotNextCell() {
    ImPlotContext& gp      = *GImPlot;
    ImPlotSubplot& subplot = *gp.CurrentSubplot;
    SubplotSetCell(++subplot.CurrentIdx);
}

bool BeginSubplots(const char* title, int rows, int cols, const ImVec2& size, ImPlotSubplotFlags flags, float* row_sizes, float* col_sizes) {
    IM_ASSERT_USER_ERROR(rows > 0 && cols > 0, "Invalid sizing arguments!");
    IM_ASSERT_USER_ERROR(GImPlot != NULL, "No current context. Did you call ImPlot::CreateContext() or ImPlot::SetCurrentContext()?");
    IM_ASSERT_USER_ERROR(GImPlot->CurrentSubplot == NULL, "Mismatched BeginSubplots()/EndSubplots()!");
    ImPlotContext& gp = *GImPlot;
    ImGuiContext &G = *GImGui;
    ImGuiWindow * Window = G.CurrentWindow;
    if (Window->SkipItems)
        return false;
    const ImGuiID ID = Window->GetID(title);
    bool just_created = gp.Subplots.GetByKey(ID) == NULL;
    gp.CurrentSubplot = gp.Subplots.GetOrAddByKey(ID);
    ImPlotSubplot& subplot = *gp.CurrentSubplot;
    subplot.ID       = ID;
    subplot.Items.ID = ID;
    // push ID
    ImGui::PushID(ID);

    if (just_created)
        subplot.Flags = flags;
    else if (flags != subplot.PreviousFlags)
        subplot.Flags = flags;
    subplot.PreviousFlags = flags;

    // check for change in rows and cols
    if (subplot.Rows != rows || subplot.Cols != cols) {
        subplot.RowAlignmentData.resize(rows);
        subplot.RowLinkData.resize(rows);
        subplot.RowRatios.resize(rows);
        for (int r = 0; r < rows; ++r) {
            subplot.RowAlignmentData[r].Reset();
            subplot.RowLinkData[r] = ImLimits(0,1);
            subplot.RowRatios[r] = 1.0f / rows;
        }
        subplot.ColAlignmentData.resize(cols);
        subplot.ColLinkData.resize(cols);
        subplot.ColRatios.resize(cols);
        for (int c = 0; c < cols; ++c) {
            subplot.ColAlignmentData[c].Reset();
            subplot.ColLinkData[c] = ImLimits(0,1);
            subplot.ColRatios[c] = 1.0f / cols;
        }
    }
    // check incoming size requests
    float row_sum = 0, col_sum = 0;
    if (row_sizes != NULL) {
        row_sum = ImSum(row_sizes, rows);
        for (int r = 0; r < rows; ++r)
            subplot.RowRatios[r] = row_sizes[r] / row_sum;
    }
    if (col_sizes != NULL) {
        col_sum = ImSum(col_sizes, cols);
        for (int c = 0; c < cols; ++c)
            subplot.ColRatios[c] = col_sizes[c] / col_sum;
    }
    subplot.Rows = rows;
    subplot.Cols = cols;

    // calc plot frame sizes
    ImVec2 title_size(0.0f, 0.0f);
    if (!ImHasFlag(subplot.Flags, ImPlotSubplotFlags_NoTitle))
         title_size = ImGui::CalcTextSize(title, NULL, true);
    const float pad_top = title_size.x > 0.0f ? title_size.y + gp.Style.LabelPadding.y : 0;
    const ImVec2 half_pad = gp.Style.PlotPadding/2;
    const ImVec2 frame_size = ImGui::CalcItemSize(size, gp.Style.PlotDefaultSize.x, gp.Style.PlotDefaultSize.y);
    subplot.FrameRect = ImRect(Window->DC.CursorPos, Window->DC.CursorPos + frame_size);
    subplot.GridRect.Min = subplot.FrameRect.Min + half_pad + ImVec2(0,pad_top);
    subplot.GridRect.Max = subplot.FrameRect.Max - half_pad;
    subplot.FrameHovered = subplot.FrameRect.Contains(ImGui::GetMousePos()) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

    // outside legend adjustments (TODO: make function)
    const bool share_items = ImHasFlag(subplot.Flags, ImPlotSubplotFlags_ShareItems);
    if (share_items)
        gp.CurrentItems = &subplot.Items;
    if (share_items && !ImHasFlag(subplot.Flags, ImPlotSubplotFlags_NoLegend) && subplot.Items.GetLegendCount() > 0) {
        ImPlotLegend& legend = subplot.Items.Legend;
        const bool horz = ImHasFlag(legend.Flags, ImPlotLegendFlags_Horizontal);
        const ImVec2 legend_size = CalcLegendSize(subplot.Items, gp.Style.LegendInnerPadding, gp.Style.LegendSpacing, !horz);
        const bool west = ImHasFlag(legend.Location, ImPlotLocation_West) && !ImHasFlag(legend.Location, ImPlotLocation_East);
        const bool east = ImHasFlag(legend.Location, ImPlotLocation_East) && !ImHasFlag(legend.Location, ImPlotLocation_West);
        const bool north = ImHasFlag(legend.Location, ImPlotLocation_North) && !ImHasFlag(legend.Location, ImPlotLocation_South);
        const bool south = ImHasFlag(legend.Location, ImPlotLocation_South) && !ImHasFlag(legend.Location, ImPlotLocation_North);
        if ((west && !horz) || (west && horz && !north && !south))
            subplot.GridRect.Min.x += (legend_size.x + gp.Style.LegendPadding.x);
        if ((east && !horz) || (east && horz && !north && !south))
            subplot.GridRect.Max.x -= (legend_size.x + gp.Style.LegendPadding.x);
        if ((north && horz) || (north && !horz && !west && !east))
            subplot.GridRect.Min.y += (legend_size.y + gp.Style.LegendPadding.y);
        if ((south && horz) || (south && !horz && !west && !east))
            subplot.GridRect.Max.y -= (legend_size.y + gp.Style.LegendPadding.y);
    }

    // render single background frame
    ImGui::RenderFrame(subplot.FrameRect.Min, subplot.FrameRect.Max, GetStyleColorU32(ImPlotCol_FrameBg), true, ImGui::GetStyle().FrameRounding);
    // render title
    if (title_size.x > 0.0f && !ImHasFlag(subplot.Flags, ImPlotFlags_NoTitle)) {
        const ImU32 col = GetStyleColorU32(ImPlotCol_TitleText);
        AddTextCentered(ImGui::GetWindowDrawList(),ImVec2(subplot.GridRect.GetCenter().x, subplot.GridRect.Min.y - pad_top + half_pad.y),col,title);
    }

    // render splitters
    if (!ImHasFlag(subplot.Flags, ImPlotSubplotFlags_NoResize)) {
        ImDrawList& DrawList = *ImGui::GetWindowDrawList();
        const ImU32 hov_col = ImGui::ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_SeparatorHovered]);
        const ImU32 act_col = ImGui::ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_SeparatorActive]);
        float xpos = subplot.GridRect.Min.x;
        float ypos = subplot.GridRect.Min.y;
        int separator = 1;
        // bool pass = false;
        for (int r = 0; r < subplot.Rows-1; ++r) {
            ypos += subplot.RowRatios[r] * subplot.GridRect.GetHeight();
            const ImGuiID sep_id = subplot.ID + separator;
            ImGui::KeepAliveID(sep_id);
            const ImRect sep_bb = ImRect(subplot.GridRect.Min.x, ypos-SUBPLOT_SPLITTER_HALF_THICKNESS, subplot.GridRect.Max.x, ypos+SUBPLOT_SPLITTER_HALF_THICKNESS);
            bool sep_hov = false, sep_hld = false;
            const bool sep_clk = ImGui::ButtonBehavior(sep_bb, sep_id, &sep_hov, &sep_hld, ImGuiButtonFlags_FlattenChildren | ImGuiButtonFlags_AllowItemOverlap | ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_PressedOnDoubleClick);
            if ((sep_hov && G.HoveredIdTimer > SUBPLOT_SPLITTER_FEEDBACK_TIMER) || sep_hld) {
                if (sep_clk && ImGui::IsMouseDoubleClicked(0)) {
                    float p = (subplot.RowRatios[r] + subplot.RowRatios[r+1])/2;
                    subplot.RowRatios[r] = subplot.RowRatios[r+1] = p;
                }
                if (sep_clk) {
                    subplot.TempSizes[0] = subplot.RowRatios[r];
                    subplot.TempSizes[1] = subplot.RowRatios[r+1];
                }
                if (sep_hld) {
                    float dp = ImGui::GetMouseDragDelta(0).y  / subplot.GridRect.GetHeight();
                    if (subplot.TempSizes[0] + dp > 0.1f && subplot.TempSizes[1] - dp > 0.1f) {
                        subplot.RowRatios[r]   = subplot.TempSizes[0] + dp;
                        subplot.RowRatios[r+1] = subplot.TempSizes[1] - dp;
                    }
                }
                DrawList.AddLine(ImVec2(IM_ROUND(subplot.GridRect.Min.x),IM_ROUND(ypos)),
                                 ImVec2(IM_ROUND(subplot.GridRect.Max.x),IM_ROUND(ypos)),
                                 sep_hld ? act_col : hov_col, SUBPLOT_BORDER_SIZE);
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
            }
            separator++;
        }
        for (int c = 0; c < subplot.Cols-1; ++c) {
            xpos += subplot.ColRatios[c] * subplot.GridRect.GetWidth();
            const ImGuiID sep_id = subplot.ID + separator;
            ImGui::KeepAliveID(sep_id);
            const ImRect sep_bb = ImRect(xpos-SUBPLOT_SPLITTER_HALF_THICKNESS, subplot.GridRect.Min.y, xpos+SUBPLOT_SPLITTER_HALF_THICKNESS, subplot.GridRect.Max.y);
            bool sep_hov = false, sep_hld = false;
            const bool sep_clk = ImGui::ButtonBehavior(sep_bb, sep_id, &sep_hov, &sep_hld, ImGuiButtonFlags_FlattenChildren | ImGuiButtonFlags_AllowItemOverlap | ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_PressedOnDoubleClick);
            if ((sep_hov && G.HoveredIdTimer > SUBPLOT_SPLITTER_FEEDBACK_TIMER) || sep_hld) {
                if (sep_clk && ImGui::IsMouseDoubleClicked(0)) {
                    float p = (subplot.ColRatios[c] + subplot.ColRatios[c+1])/2;
                    subplot.ColRatios[c] = subplot.ColRatios[c+1] = p;
                }
                if (sep_clk) {
                    subplot.TempSizes[0] = subplot.ColRatios[c];
                    subplot.TempSizes[1] = subplot.ColRatios[c+1];
                }
                if (sep_hld) {
                    float dp = ImGui::GetMouseDragDelta(0).x / subplot.GridRect.GetWidth();
                    if (subplot.TempSizes[0] + dp > 0.1f && subplot.TempSizes[1] - dp > 0.1f) {
                        subplot.ColRatios[c]   = subplot.TempSizes[0] + dp;
                        subplot.ColRatios[c+1] = subplot.TempSizes[1] - dp;
                    }
                }
                DrawList.AddLine(ImVec2(IM_ROUND(xpos),IM_ROUND(subplot.GridRect.Min.y)),
                                 ImVec2(IM_ROUND(xpos),IM_ROUND(subplot.GridRect.Max.y)),
                                 sep_hld ? act_col : hov_col, SUBPLOT_BORDER_SIZE);
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            }
            separator++;
        }
    }

    // set outgoing sizes
    if (row_sizes != NULL) {
        for (int r = 0; r < rows; ++r)
            row_sizes[r] = subplot.RowRatios[r] * row_sum;
    }
    if (col_sizes != NULL) {
        for (int c = 0; c < cols; ++c)
            col_sizes[c] = subplot.ColRatios[c] * col_sum;
    }

    // push styling
    PushStyleColor(ImPlotCol_FrameBg, IM_COL32_BLACK_TRANS);
    PushStyleVar(ImPlotStyleVar_PlotPadding, half_pad);
    PushStyleVar(ImPlotStyleVar_PlotMinSize, ImVec2(0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize,0);

    // set initial cursor pos
    Window->DC.CursorPos = subplot.GridRect.Min;
    // begin alignrmnts
    for (int r = 0; r < subplot.Rows; ++r)
        subplot.RowAlignmentData[r].Begin();
    for (int c = 0; c < subplot.Cols; ++c)
        subplot.ColAlignmentData[c].Begin();
    // clear legend data
    subplot.Items.Legend.Reset();
    // Setup first subplot
    SubplotSetCell(0,0);
    return true;
}

void EndSubplots() {
    IM_ASSERT_USER_ERROR(GImPlot != NULL, "No current context. Did you call ImPlot::CreateContext() or ImPlot::SetCurrentContext()?");
    IM_ASSERT_USER_ERROR(GImPlot->CurrentSubplot != NULL, "Mismatched BeginSubplots()/EndSubplots()!");
    ImPlotContext& gp = *GImPlot;
    ImPlotSubplot& subplot = *GImPlot->CurrentSubplot;
    // set alignments
    for (int r = 0; r < subplot.Rows; ++r)
        subplot.RowAlignmentData[r].End();
    for (int c = 0; c < subplot.Cols; ++c)
        subplot.ColAlignmentData[c].End();
    // pop styling
    PopStyleColor();
    PopStyleVar();
    PopStyleVar();
    ImGui::PopStyleVar();
    // legend
    subplot.Items.Legend.Hovered = false;
    for (int i = 0; i < subplot.Items.GetItemCount(); ++i)
        subplot.Items.GetItemByIndex(i)->LegendHovered = false;
    // render legend
    const bool share_items = ImHasFlag(subplot.Flags, ImPlotSubplotFlags_ShareItems);
    ImDrawList& DrawList = *ImGui::GetWindowDrawList();
    if (share_items && !ImHasFlag(subplot.Flags, ImPlotSubplotFlags_NoLegend) && subplot.Items.GetLegendCount() > 0) {
        const bool   legend_horz = ImHasFlag(subplot.Items.Legend.Flags, ImPlotLegendFlags_Horizontal);
        const ImVec2 legend_size = CalcLegendSize(subplot.Items, gp.Style.LegendInnerPadding, gp.Style.LegendSpacing, !legend_horz);
        const ImVec2 legend_pos  = GetLocationPos(subplot.FrameRect, legend_size, subplot.Items.Legend.Location, gp.Style.PlotPadding);
        subplot.Items.Legend.Rect = ImRect(legend_pos, legend_pos + legend_size);
        subplot.Items.Legend.Hovered = subplot.FrameHovered && subplot.Items.Legend.Rect.Contains(ImGui::GetIO().MousePos);
        ImGui::PushClipRect(subplot.FrameRect.Min, subplot.FrameRect.Max, true);
        ImU32  col_bg      = GetStyleColorU32(ImPlotCol_LegendBg);
        ImU32  col_bd      = GetStyleColorU32(ImPlotCol_LegendBorder);
        DrawList.AddRectFilled(subplot.Items.Legend.Rect.Min, subplot.Items.Legend.Rect.Max, col_bg);
        DrawList.AddRect(subplot.Items.Legend.Rect.Min, subplot.Items.Legend.Rect.Max, col_bd);
        bool legend_contextable = ShowLegendEntries(subplot.Items, subplot.Items.Legend.Rect, subplot.Items.Legend.Hovered, gp.Style.LegendInnerPadding, gp.Style.LegendSpacing, !legend_horz, DrawList)
                                && !ImHasFlag(subplot.Items.Legend.Flags, ImPlotLegendFlags_NoMenus);
        if (legend_contextable && !ImHasFlag(subplot.Flags, ImPlotSubplotFlags_NoMenus) && ImGui::GetIO().MouseReleased[gp.InputMap.ContextMenuButton])
            ImGui::OpenPopup("##LegendContext");
        ImGui::PopClipRect();
        if (ImGui::BeginPopup("##LegendContext")) {
            ImGui::Text("Legend"); ImGui::Separator();
            if (ShowLegendContextMenu(subplot.Items.Legend, !ImHasFlag(subplot.Flags, ImPlotFlags_NoLegend)))
                ImFlipFlag(subplot.Flags, ImPlotFlags_NoLegend);
            ImGui::EndPopup();
        }
    }
    else {
        subplot.Items.Legend.Rect = ImRect();
    }
    // remove items
    if (gp.CurrentItems == &subplot.Items)
        gp.CurrentItems = NULL;
    // reset the plot items for the next frame (TODO: put this elswhere)
    for (int i = 0; i < subplot.Items.GetItemCount(); ++i) {
        subplot.Items.GetItemByIndex(i)->SeenThisFrame = false;
    }
    // pop id
    ImGui::PopID();
    // set DC back correctly
    GImGui->CurrentWindow->DC.CursorPos = subplot.FrameRect.Min;
    ImGui::Dummy(subplot.FrameRect.GetSize());
    ResetCtxForNextSubplot(GImPlot);

}

//-----------------------------------------------------------------------------
// MISC API
//-----------------------------------------------------------------------------


ImPlotInputMap& GetInputMap() {
    return GImPlot->InputMap;
}

//-----------------------------------------------------------------------------
// [SECTION] Plot Utils
//-----------------------------------------------------------------------------

void SetAxis(ImAxis axis) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "SetAxis() needs to be called between BeginPlot() and EndPlot()!");
    IM_ASSERT_USER_ERROR(axis >= ImAxis_X1 && axis < ImAxis_COUNT, "Axis indices out of bounds!");
    SetupLock();
    if (axis < ImAxis_Y1)
        gp.CurrentPlot->CurrentX = axis;
    else
        gp.CurrentPlot->CurrentY = axis % IMPLOT_MAX_AXES;
}

void SetAxes(ImAxis x_axis, ImAxis y_axis) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "SetAxes() needs to be called between BeginPlot() and EndPlot()!");
    IM_ASSERT_USER_ERROR(x_axis >= ImAxis_X1 && x_axis < ImAxis_Y1 && y_axis >= ImAxis_Y1 && y_axis < ImAxis_COUNT, "Axis indices out of bounds!");
    SetupLock();
    gp.CurrentPlot->CurrentX = x_axis;
    gp.CurrentPlot->CurrentY = y_axis % IMPLOT_MAX_AXES;
}

ImPoint PixelsToPlot(float x, float y, ImAxis x_axis, ImAxis y_axis) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PixelsToPlot() needs to be called between BeginPlot() and EndPlot()!");
    SetupLock();
    ImPlotPlot& plot = *gp.CurrentPlot;
    const int ix = plot.GetAxisIdxX(x_axis);
    const int iy = plot.GetAxisIdxY(y_axis);
    ImPoint plt;
    plt.x = (x - gp.PixelRange[iy].Min.x) / gp.Mx[ix] + plot.XAxis[ix].Range.Min;
    plt.y = (y - gp.PixelRange[iy].Min.y) / gp.My[iy] + plot.YAxis[iy].Range.Min;
    if (plot.XAxis[ix].IsLog()) {
        double t = (plt.x - plot.XAxis[ix].Range.Min) / plot.XAxis[ix].Range.Size();
        plt.x = ImPow(10, t * gp.LogDenX[ix]) * plot.XAxis[ix].Range.Min;
    }
    if (plot.YAxis[iy].IsLog()) {
        double t = (plt.y - plot.YAxis[iy].Range.Min) / plot.YAxis[iy].Range.Size();
        plt.y = ImPow(10, t * gp.LogDenY[iy]) * plot.YAxis[iy].Range.Min;
    }
    return plt;
}

ImPoint PixelsToPlot(const ImVec2& pix, ImAxis x_axis, ImAxis y_axis) {
    return PixelsToPlot(pix.x, pix.y, x_axis, y_axis);
}

ImVec2 PlotToPixels(double x, double y, ImAxis x_axis, ImAxis y_axis) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PlotToPixels() needs to be called between BeginPlot() and EndPlot()!");
    SetupLock();
    ImPlotPlot& plot = *gp.CurrentPlot;
    const int ix = plot.GetAxisIdxX(x_axis);
    const int iy = plot.GetAxisIdxY(y_axis);
    ImVec2 pix;
    if (plot.XAxis[ix].IsLog()) {
        x        = x <= 0.0 ? IMPLOT_LOG_ZERO : x;
        double t = ImLog10(x / gp.CurrentPlot->XAxis[ix].Range.Min) / gp.LogDenX[ix];
        x        = ImLerp(gp.CurrentPlot->XAxis[ix].Range.Min, gp.CurrentPlot->XAxis[ix].Range.Max, (float)t);
    }
    if (plot.YAxis[iy].IsLog()) {
        y        = y <= 0.0 ? IMPLOT_LOG_ZERO : y;
        double t = ImLog10(y / gp.CurrentPlot->YAxis[iy].Range.Min) / gp.LogDenY[iy];
        y        = ImLerp(gp.CurrentPlot->YAxis[iy].Range.Min, gp.CurrentPlot->YAxis[iy].Range.Max, (float)t);
    }
    pix.x = (float)(gp.PixelRange[iy].Min.x + gp.Mx[ix] * (x - gp.CurrentPlot->XAxis[ix].Range.Min));
    pix.y = (float)(gp.PixelRange[iy].Min.y + gp.My[iy] * (y - gp.CurrentPlot->YAxis[iy].Range.Min));
    return pix;
}

ImVec2 PlotToPixels(const ImPoint& plt, ImAxis x_axis, ImAxis y_axis) {
    return PlotToPixels(plt.x, plt.y, x_axis, y_axis);
}

ImVec2 GetPlotPos() {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "GetPlotPos() needs to be called between BeginPlot() and EndPlot()!");
    SetupLock();
    return gp.CurrentPlot->PlotRect.Min;
}

ImVec2 GetPlotSize() {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "GetPlotSize() needs to be called between BeginPlot() and EndPlot()!");
    SetupLock();
    return gp.CurrentPlot->PlotRect.GetSize();
}

ImPoint GetPlotMousePos(ImAxis x_axis, ImAxis y_axis) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "GetPlotMousePos() needs to be called between BeginPlot() and EndPlot()!");
    SetupLock();
    const int ix = gp.CurrentPlot->GetAxisIdxX(x_axis);
    const int iy = gp.CurrentPlot->GetAxisIdxY(y_axis);
    return ImPoint(gp.MousePos[ix].x, gp.MousePos[iy].y);
}

ImLimitsXY GetPlotLimits(ImAxis x_axis, ImAxis y_axis) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "GetPlotLimits() needs to be called between BeginPlot() and EndPlot()!");
    SetupLock();
    const int ix = gp.CurrentPlot->GetAxisIdxX(x_axis);
    const int iy = gp.CurrentPlot->GetAxisIdxY(y_axis);
    ImPlotPlot& plot = *gp.CurrentPlot;
    ImLimitsXY limits;
    limits.X = plot.XAxis[ix].Range;
    limits.Y = plot.YAxis[iy].Range;
    return limits;
}

bool IsPlotHovered() {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "IsPlotHovered() needs to be called between BeginPlot() and EndPlot()!");
    SetupLock();
    return gp.CurrentPlot->Hovered;
}

bool IsAxisHovered(ImAxis axis) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "IsPlotXAxisHovered() needs to be called between BeginPlot() and EndPlot()!");
    SetupLock();
    return gp.CurrentPlot->GetAxis(axis)->Hovered;
}

bool IsSubplotsHovered() {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentSubplot != NULL, "IsSubplotsHovered() needs to be called between BeginSubplots() and EndSubplots()!");
    return gp.CurrentSubplot->FrameHovered;
}

bool IsPlotSelected() {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "IsPlotSelected() needs to be called between BeginPlot() and EndPlot()!");
    SetupLock();
    return gp.CurrentPlot->Selected;
}

ImLimitsXY GetPlotSelection(ImAxis x_axis, ImAxis y_axis) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "GetPlotSelection() needs to be called between BeginPlot() and EndPlot()!");
    SetupLock();
    ImPlotPlot& plot = *gp.CurrentPlot;
    if (!plot.Selected)
        return ImLimitsXY(0,0,0,0);
    UpdateTransformCache();
    ImPoint p1 = PixelsToPlot(plot.SelectRect.Min + plot.PlotRect.Min, x_axis, y_axis);
    ImPoint p2 = PixelsToPlot(plot.SelectRect.Max + plot.PlotRect.Min, x_axis, y_axis);
    ImLimitsXY result;
    result.X.Min = ImMin(p1.x, p2.x);
    result.X.Max = ImMax(p1.x, p2.x);
    result.Y.Min = ImMin(p1.y, p2.y);
    result.Y.Max = ImMax(p1.y, p2.y);
    return result;
}

void HideNextItem(bool hidden, ImGuiCond cond) {
    ImPlotContext& gp = *GImPlot;
    gp.NextItemData.HasHidden  = true;
    gp.NextItemData.Hidden     = hidden;
    gp.NextItemData.HiddenCond = cond;
}

//-----------------------------------------------------------------------------
// [SECTION] Plot Tools
//-----------------------------------------------------------------------------

void AnnotateEx(double x, double y, bool clamp, const ImVec4& col, const ImVec2& off, const char* fmt, va_list args) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "Annotate() needs to be called between BeginPlot() and EndPlot()!");
    SetupLock();
    ImVec2 pos = PlotToPixels(x,y,IMPLOT_AUTO,IMPLOT_AUTO);
    ImU32  bg  = ImGui::GetColorU32(col);
    ImU32  fg  = col.w == 0 ? GetStyleColorU32(ImPlotCol_InlayText) : CalcTextColor(col);
    gp.Annotations.AppendV(pos, off, bg, fg, clamp, fmt, args);
}

void AnnotateV(double x, double y, const ImVec2& offset, const char* fmt, va_list args) {
    AnnotateEx(x,y,false,ImVec4(0,0,0,0),offset,fmt,args);
}

void Annotate(double x, double y, const ImVec2& offset, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    AnnotateV(x,y,offset,fmt,args);
    va_end(args);
}

void AnnotateV(double x, double y, const ImVec2& offset, const ImVec4& col, const char* fmt, va_list args) {
    AnnotateEx(x,y,false,col,offset,fmt,args);
}

void Annotate(double x, double y, const ImVec2& offset, const ImVec4& col, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    AnnotateV(x,y,offset,col,fmt,args);
    va_end(args);
}

void AnnotateClampedV(double x, double y, const ImVec2& offset, const char* fmt, va_list args) {
    AnnotateEx(x,y,true,ImVec4(0,0,0,0),offset,fmt,args);
}

void AnnotateClamped(double x, double y, const ImVec2& offset, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    AnnotateClampedV(x,y,offset,fmt,args);
    va_end(args);
}

void AnnotateClampedV(double x, double y, const ImVec2& offset, const ImVec4& col, const char* fmt, va_list args) {
    AnnotateEx(x,y,true,col,offset,fmt,args);
}

void AnnotateClamped(double x, double y, const ImVec2& offset, const ImVec4& col, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    AnnotateClampedV(x,y,offset,col,fmt,args);
    va_end(args);
}

bool DragLineX(const char* id, double* value, bool show_label, const ImVec4& col, float thickness) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "DragLineX() needs to be called between BeginPlot() and EndPlot()!");
    SetupLock();
    const float grab_size = ImMax(5.0f, thickness);
    float yt = gp.CurrentPlot->PlotRect.Min.y;
    float yb = gp.CurrentPlot->PlotRect.Max.y;
    float x  = IM_ROUND(PlotToPixels(*value,0,IMPLOT_AUTO,IMPLOT_AUTO).x);
    const bool outside = x < (gp.CurrentPlot->PlotRect.Min.x - grab_size / 2) || x > (gp.CurrentPlot->PlotRect.Max.x + grab_size / 2);
    if (outside)
        return false;
    float len = gp.Style.MajorTickLen.x;
    ImVec4 color = IsColorAuto(col) ? ImGui::GetStyleColorVec4(ImGuiCol_Text) : col;
    ImU32 col32 = ImGui::ColorConvertFloat4ToU32(color);
    ImDrawList& DrawList = *GetPlotDrawList();
    PushPlotClipRect();
    DrawList.AddLine(ImVec2(x,yt), ImVec2(x,yb),     col32, thickness);
    DrawList.AddLine(ImVec2(x,yt), ImVec2(x,yt+len), col32, 3*thickness);
    DrawList.AddLine(ImVec2(x,yb), ImVec2(x,yb-len), col32, 3*thickness);
    PopPlotClipRect();
    if (gp.CurrentPlot->Selecting)
        return false;
    ImVec2 old_cursor_pos = ImGui::GetCursorScreenPos();
    ImVec2 new_cursor_pos = ImVec2(x - grab_size / 2.0f, yt);
    ImGui::GetCurrentWindow()->DC.CursorPos = new_cursor_pos;
    ImGui::InvisibleButton(id, ImVec2(grab_size, yb-yt));
    ImGui::GetCurrentWindow()->DC.CursorPos = old_cursor_pos;
    const int xax = GetCurrentAxisX();
    if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
        gp.CurrentPlot->Hovered = false;
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        if (show_label) {
            char buff[32];
            LabelAxisValue(gp.CurrentPlot->XAxis[xax], gp.XTicks[xax], *value, buff, 32);
            gp.Annotations.Append(ImVec2(x,yb),ImVec2(0,0),col32,CalcTextColor(color),true,"%s = %s", id, buff);
        }
    }
    bool dragging = false;
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
        *value = ImPlot::GetPlotMousePos(IMPLOT_AUTO,IMPLOT_AUTO).x;
        *value = ImClamp(*value, gp.CurrentPlot->XAxis[xax].Range.Min, gp.CurrentPlot->XAxis[xax].Range.Max);
        dragging = true;
    }
    return dragging;
}

bool DragLineY(const char* id, double* value, bool show_label, const ImVec4& col, float thickness) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "DragLineY() needs to be called between BeginPlot() and EndPlot()!");
    SetupLock();
    const float grab_size = ImMax(5.0f, thickness);
    float xl = gp.CurrentPlot->PlotRect.Min.x;
    float xr = gp.CurrentPlot->PlotRect.Max.x;
    float y  = IM_ROUND(PlotToPixels(0, *value,IMPLOT_AUTO,IMPLOT_AUTO).y);
    const bool outside = y < (gp.CurrentPlot->PlotRect.Min.y - grab_size / 2) || y > (gp.CurrentPlot->PlotRect.Max.y + grab_size / 2);
    if (outside)
        return false;
    float len = gp.Style.MajorTickLen.y;
    ImVec4 color = IsColorAuto(col) ? ImGui::GetStyleColorVec4(ImGuiCol_Text) : col;
    ImU32 col32 = ImGui::ColorConvertFloat4ToU32(color);
    ImDrawList& DrawList = *GetPlotDrawList();
    PushPlotClipRect();
    DrawList.AddLine(ImVec2(xl,y), ImVec2(xr,y),     col32, thickness);
    DrawList.AddLine(ImVec2(xl,y), ImVec2(xl+len,y), col32, 3*thickness);
    DrawList.AddLine(ImVec2(xr,y), ImVec2(xr-len,y), col32, 3*thickness);
    PopPlotClipRect();
    if (gp.CurrentPlot->Selecting)
        return false;
    ImVec2 old_cursor_pos = ImGui::GetCursorScreenPos();
    ImVec2 new_cursor_pos = ImVec2(xl, y - grab_size / 2.0f);
    ImGui::SetItemAllowOverlap();
    ImGui::GetCurrentWindow()->DC.CursorPos = new_cursor_pos;
    ImGui::InvisibleButton(id, ImVec2(xr - xl, grab_size));
    ImGui::GetCurrentWindow()->DC.CursorPos = old_cursor_pos;
    const int yax = GetCurrentAxisY();
    if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
        gp.CurrentPlot->Hovered = false;
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        if (show_label) {
            char buff[32];
            LabelAxisValue(gp.CurrentPlot->YAxis[yax], gp.YTicks[yax], *value, buff, 32);
            gp.Annotations.Append(ImVec2(yax == 0 ? xl : xr,y), ImVec2(0,0), col32, CalcTextColor(color), true,  "%s = %s", id, buff);
        }
    }
    bool dragging = false;
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
        *value = ImPlot::GetPlotMousePos(IMPLOT_AUTO,IMPLOT_AUTO).y;
        *value = ImClamp(*value, gp.CurrentPlot->YAxis[yax].Range.Min, gp.CurrentPlot->YAxis[yax].Range.Max);
        dragging = true;
    }
    return dragging;
}

bool DragPoint(const char* id, double* x, double* y, bool show_label, const ImVec4& col, float radius) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "DragPoint() needs to be called between BeginPlot() and EndPlot()!");
    SetupLock();
    const float grab_size = ImMax(5.0f, 2*radius);
    const bool outside = !GetPlotLimits(IMPLOT_AUTO,IMPLOT_AUTO).Contains(*x,*y);
    if (outside)
        return false;
    const ImVec4 color = IsColorAuto(col) ? ImGui::GetStyleColorVec4(ImGuiCol_Text) : col;
    const ImU32 col32 = ImGui::ColorConvertFloat4ToU32(color);
    ImDrawList& DrawList = *GetPlotDrawList();
    const ImVec2 pos = PlotToPixels(*x,*y,IMPLOT_AUTO,IMPLOT_AUTO);
    const int xax = GetCurrentAxisX();
    const int yax = GetCurrentAxisY();
    ImVec2 old_cursor_pos = ImGui::GetCursorScreenPos();
    ImVec2 new_cursor_pos = ImVec2(pos - ImVec2(grab_size,grab_size)*0.5f);
    ImGui::GetCurrentWindow()->DC.CursorPos = new_cursor_pos;
    ImGui::InvisibleButton(id, ImVec2(grab_size, grab_size));
    ImGui::GetCurrentWindow()->DC.CursorPos = old_cursor_pos;
    PushPlotClipRect();
    if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
        DrawList.AddCircleFilled(pos, 1.5f*radius, (col32));
        // gp.CurrentPlot->Hovered = false;
        if (show_label) {
            ImVec2 label_pos = pos + ImVec2(16 * GImGui->Style.MouseCursorScale, 8 * GImGui->Style.MouseCursorScale);
            char buff1[32];
            char buff2[32];
            LabelAxisValue(gp.CurrentPlot->XAxis[xax], gp.XTicks[xax], *x, buff1, 32);
            LabelAxisValue(gp.CurrentPlot->YAxis[yax], gp.YTicks[yax], *y, buff2, 32);
            gp.Annotations.Append(label_pos, ImVec2(0.0001f,0.00001f), col32, CalcTextColor(color), true, "%s = %s,%s", id, buff1, buff2);
        }
    }
    else {
        DrawList.AddCircleFilled(pos, radius, col32);
    }
    PopPlotClipRect();


    bool dragging = false;
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
        *x = ImPlot::GetPlotMousePos(IMPLOT_AUTO,IMPLOT_AUTO).x;
        *y = ImPlot::GetPlotMousePos(IMPLOT_AUTO,IMPLOT_AUTO).y;
        *x = ImClamp(*x, gp.CurrentPlot->XAxis[xax].Range.Min, gp.CurrentPlot->XAxis[xax].Range.Max);
        *y = ImClamp(*y, gp.CurrentPlot->YAxis[yax].Range.Min, gp.CurrentPlot->YAxis[yax].Range.Max);
        dragging = true;
    }
    return dragging;
}

//-----------------------------------------------------------------------------
// [SECTION] Legend Utils and Tools
//-----------------------------------------------------------------------------

bool IsLegendEntryHovered(const char* label_id) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentItems != NULL, "IsPlotItemHighlight() needs to be called within an itemized context!");
    SetupLock();
    ImGuiID id = ImGui::GetIDWithSeed(label_id, NULL, gp.CurrentItems->ID);
    ImPlotItem* item = gp.CurrentItems->GetItem(id);
    return item && item->LegendHovered;
}

bool BeginLegendPopup(const char* label_id, ImGuiMouseButton mouse_button) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentItems != NULL, "BeginLegendPopup() needs to be called within an itemized context!");
    SetupLock();
    ImGuiWindow* window = GImGui->CurrentWindow;
    if (window->SkipItems)
        return false;
    ImGuiID id = ImGui::GetIDWithSeed(label_id, NULL, gp.CurrentItems->ID);
    if (ImGui::IsMouseReleased(mouse_button)) {
        ImPlotItem* item = gp.CurrentItems->GetItem(id);
        if (item && item->LegendHovered)
            ImGui::OpenPopupEx(id);
    }
    return ImGui::BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
}

void EndLegendPopup() {
    SetupLock();
    ImGui::EndPopup();
}

void ShowAltLegend(const char* title_id, bool vertical, const ImVec2 size, bool interactable) {
    ImPlotContext& gp    = *GImPlot;
    ImGuiContext &G      = *GImGui;
    ImGuiWindow * Window = G.CurrentWindow;
    if (Window->SkipItems)
        return;
    ImDrawList &DrawList = *Window->DrawList;
    ImPlotPlot* plot = GetPlot(title_id);
    ImVec2 legend_size;
    ImVec2 default_size = gp.Style.LegendPadding * 2;
    if (plot != NULL) {
        legend_size  = CalcLegendSize(plot->Items, gp.Style.LegendInnerPadding, gp.Style.LegendSpacing, vertical);
        default_size = legend_size + gp.Style.LegendPadding * 2;
    }
    ImVec2 frame_size = ImGui::CalcItemSize(size, default_size.x, default_size.y);
    ImRect bb_frame = ImRect(Window->DC.CursorPos, Window->DC.CursorPos + frame_size);
    ImGui::ItemSize(bb_frame);
    if (!ImGui::ItemAdd(bb_frame, 0, &bb_frame))
        return;
    ImGui::RenderFrame(bb_frame.Min, bb_frame.Max, GetStyleColorU32(ImPlotCol_FrameBg), true, G.Style.FrameRounding);
    DrawList.PushClipRect(bb_frame.Min, bb_frame.Max, true);
    if (plot != NULL) {
        const ImVec2 legend_pos  = GetLocationPos(bb_frame, legend_size, 0, gp.Style.LegendPadding);
        const ImRect legend_bb(legend_pos, legend_pos + legend_size);
        interactable = interactable && bb_frame.Contains(ImGui::GetIO().MousePos);
        // render legend box
        ImU32  col_bg      = GetStyleColorU32(ImPlotCol_LegendBg);
        ImU32  col_bd      = GetStyleColorU32(ImPlotCol_LegendBorder);
        DrawList.AddRectFilled(legend_bb.Min, legend_bb.Max, col_bg);
        DrawList.AddRect(legend_bb.Min, legend_bb.Max, col_bd);
        // render entries
        ShowLegendEntries(plot->Items, legend_bb, interactable, gp.Style.LegendInnerPadding, gp.Style.LegendSpacing, vertical, DrawList);
    }
    DrawList.PopClipRect();
}

//-----------------------------------------------------------------------------
// [SECTION] Drag and Drop Utils
//-----------------------------------------------------------------------------

bool BeginDragDropTargetEx(int id, const ImRect& rect) {
    ImGuiContext& G  = *GImGui;
    const ImGuiID ID = G.CurrentWindow->GetID(id);
    if (ImGui::ItemAdd(rect, ID, &rect) && ImGui::BeginDragDropTarget())
        return true;
    return false;
}

bool BeginDragDropTargetPlot() {
    SetupLock();
    return BeginDragDropTargetEx(IMPLOT_ID_PLT, GImPlot->CurrentPlot->PlotRect);
}

bool BeginDragDropTargetAxis(ImAxis axis) {
    SetupLock();
    ImPlotAxis* ax = GImPlot->CurrentPlot->GetAxis(axis);
    return BeginDragDropTargetEx(ax->ID, ax->HoverRect);
}

bool BeginDragDropTargetLegend() {
    SetupLock();
    return BeginDragDropTargetEx(IMPLOT_ID_LEG, GImPlot->CurrentItems->Legend.Rect);
}

void EndDragDropTarget() {
    SetupLock();
	ImGui::EndDragDropTarget();
}

bool BeginDragDropSourceEx(ImGuiID source_id, bool is_hovered, ImGuiDragDropFlags flags, ImGuiKeyModFlags key_mods) {
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiMouseButton mouse_button = ImGuiMouseButton_Left;

    if (g.IO.MouseDown[mouse_button] == false) {
        if (g.ActiveId == source_id)
            ImGui::ClearActiveID();
        return false;
    }

    if (is_hovered && g.IO.MouseClicked[mouse_button] && g.IO.KeyMods == key_mods) {
        ImGui::SetActiveID(source_id, window);
        ImGui::FocusWindow(window);
    }

    if (g.ActiveId != source_id) {
        return false;
    }

    g.ActiveIdAllowOverlap = is_hovered;
    g.ActiveIdUsingNavDirMask = ~(ImU32)0;
    g.ActiveIdUsingNavInputMask = ~(ImU32)0;
    g.ActiveIdUsingKeyInputMask = ~(ImU64)0;

    if (ImGui::IsMouseDragging(mouse_button)) {

        if (!g.DragDropActive) {
            ImGui::ClearDragDrop();
            ImGuiPayload& payload = g.DragDropPayload;
            payload.SourceId = source_id;
            payload.SourceParentId = 0;
            g.DragDropActive = true;
            g.DragDropSourceFlags = 0;
            g.DragDropMouseButton = mouse_button;
        }
        g.DragDropSourceFrameCount = g.FrameCount;
        g.DragDropWithinSource = true;

        if (!(flags & ImGuiDragDropFlags_SourceNoPreviewTooltip)) {
            ImGui::BeginTooltip();
            if (g.DragDropAcceptIdPrev && (g.DragDropAcceptFlags & ImGuiDragDropFlags_AcceptNoPreviewTooltip)) {
                ImGuiWindow* tooltip_window = g.CurrentWindow;
                tooltip_window->SkipItems = true;
                tooltip_window->HiddenFramesCanSkipItems = 1;
            }
        }

        return true;
    }

    return false;
}

bool BeginDragDropSourcePlot(ImGuiKeyModFlags key_mods, ImGuiDragDropFlags flags) {
    SetupLock();
    const ImGuiID ID = GImGui->CurrentWindow->GetID(IMPLOT_ID_PLT);
    ImRect rect = GImPlot->CurrentPlot->PlotRect;
    return  ImGui::ItemAdd(rect, ID, &rect) && BeginDragDropSourceEx(ID, GImPlot->CurrentPlot->Hovered, flags, key_mods);
}

bool BeginDragDropSourceAxis(ImAxis axis, ImGuiKeyModFlags key_mods, ImGuiDragDropFlags flags) {
    SetupLock();
    ImPlotAxis* ax = GImPlot->CurrentPlot->GetAxis(axis);
    const ImGuiID ID = GImGui->CurrentWindow->GetID(IMPLOT_ID_AAX+axis);
    ImRect rect = ax->HoverRect;
    return  ImGui::ItemAdd(rect, ID, &rect) && BeginDragDropSourceEx(ID, ax->Hovered, flags, key_mods);
}

bool BeginDragDropSourceItem(const char* label_id, ImGuiDragDropFlags flags) {
    SetupLock();
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentItems != NULL, "BeginDragDropSourceItem() needs to be called within an itemized context!");
    ImGuiID item_id = ImGui::GetIDWithSeed(label_id, NULL, gp.CurrentItems->ID);
    ImPlotItem* item = gp.CurrentItems->GetItem(item_id);
    bool is_hovered = item && item->LegendHovered;
    ImGuiID temp_id = ImGui::GetIDWithSeed("dnd",NULL,item->ID); // total hack
    return BeginDragDropSourceEx(temp_id, is_hovered, flags, ImGuiKeyModFlags_None);
}

void EndDragDropSource() {
    SetupLock();
    ImGui::EndDragDropSource();
}

//-----------------------------------------------------------------------------
// [SECTION] Aligned Plots
//-----------------------------------------------------------------------------

bool BeginAlignedPlots(const char* group_id, bool vertical) {
    IM_ASSERT_USER_ERROR(GImPlot != NULL, "No current context. Did you call ImPlot::CreateContext() or ImPlot::SetCurrentContext()?");
    IM_ASSERT_USER_ERROR(GImPlot->CurrentAlignmentH == NULL && GImPlot->CurrentAlignmentV == NULL, "Mismatched BeginAlignedPlots()/EndAlignedPlots()!");
    ImPlotContext& gp = *GImPlot;
    ImGuiContext &G = *GImGui;
    ImGuiWindow * Window = G.CurrentWindow;
    if (Window->SkipItems)
        return false;
    const ImGuiID ID = Window->GetID(group_id);
    ImPlotAlignmentData* alignment = gp.AlignmentData.GetOrAddByKey(ID);
    if (vertical)
        gp.CurrentAlignmentV = alignment;
    else
        gp.CurrentAlignmentH = alignment;
    if (alignment->Vertical != vertical)
        alignment->Reset();
    alignment->Vertical = vertical;
    alignment->Begin();
    return true;
}

void EndAlignedPlots() {
    IM_ASSERT_USER_ERROR(GImPlot != NULL, "No current context. Did you call ImPlot::CreateContext() or ImPlot::SetCurrentContext()?");
    IM_ASSERT_USER_ERROR(GImPlot->CurrentAlignmentH != NULL || GImPlot->CurrentAlignmentV != NULL, "Mismatched BeginAlignedPlots()/EndAlignedPlots()!");
    ImPlotContext& gp = *GImPlot;
    ImPlotAlignmentData* alignment = gp.CurrentAlignmentH != NULL ? gp.CurrentAlignmentH : (gp.CurrentAlignmentV != NULL ? gp.CurrentAlignmentV : NULL);
    if (alignment)
        alignment->End();
    ResetCtxForNextAlignedPlots(GImPlot);
}

//-----------------------------------------------------------------------------
// [SECTION] Plot and Item Styling
//-----------------------------------------------------------------------------

ImPlotStyle& GetStyle() {
    ImPlotContext& gp = *GImPlot;
    return gp.Style;
}

void PushStyleColor(ImPlotCol idx, ImU32 col) {
    ImPlotContext& gp = *GImPlot;
    ImGuiColorMod backup;
    backup.Col = idx;
    backup.BackupValue = gp.Style.Colors[idx];
    gp.ColorModifiers.push_back(backup);
    gp.Style.Colors[idx] = ImGui::ColorConvertU32ToFloat4(col);
}

void PushStyleColor(ImPlotCol idx, const ImVec4& col) {
    ImPlotContext& gp = *GImPlot;
    ImGuiColorMod backup;
    backup.Col = idx;
    backup.BackupValue = gp.Style.Colors[idx];
    gp.ColorModifiers.push_back(backup);
    gp.Style.Colors[idx] = col;
}

void PopStyleColor(int count) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(count <= gp.ColorModifiers.Size, "You can't pop more modifiers than have been pushed!");
    while (count > 0)
    {
        ImGuiColorMod& backup = gp.ColorModifiers.back();
        gp.Style.Colors[backup.Col] = backup.BackupValue;
        gp.ColorModifiers.pop_back();
        count--;
    }
}

void PushStyleVar(ImPlotStyleVar idx, float val) {
    ImPlotContext& gp = *GImPlot;
    const ImPlotStyleVarInfo* var_info = GetPlotStyleVarInfo(idx);
    if (var_info->Type == ImGuiDataType_Float && var_info->Count == 1) {
        float* pvar = (float*)var_info->GetVarPtr(&gp.Style);
        gp.StyleModifiers.push_back(ImGuiStyleMod(idx, *pvar));
        *pvar = val;
        return;
    }
    IM_ASSERT(0 && "Called PushStyleVar() float variant but variable is not a float!");
}

void PushStyleVar(ImPlotStyleVar idx, int val) {
    ImPlotContext& gp = *GImPlot;
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
    IM_ASSERT(0 && "Called PushStyleVar() int variant but variable is not a int!");
}

void PushStyleVar(ImGuiStyleVar idx, const ImVec2& val)
{
    ImPlotContext& gp = *GImPlot;
    const ImPlotStyleVarInfo* var_info = GetPlotStyleVarInfo(idx);
    if (var_info->Type == ImGuiDataType_Float && var_info->Count == 2)
    {
        ImVec2* pvar = (ImVec2*)var_info->GetVarPtr(&gp.Style);
        gp.StyleModifiers.push_back(ImGuiStyleMod(idx, *pvar));
        *pvar = val;
        return;
    }
    IM_ASSERT(0 && "Called PushStyleVar() ImVec2 variant but variable is not a ImVec2!");
}

void PopStyleVar(int count) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(count <= gp.StyleModifiers.Size, "You can't pop more modifiers than have been pushed!");
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

//------------------------------------------------------------------------------
// [Section] Colormaps
//------------------------------------------------------------------------------

ImPlotColormap AddColormap(const char* name, const ImVec4* colormap, int size, bool qual) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(size > 1, "The colormap size must be greater than 1!");
    IM_ASSERT_USER_ERROR(gp.ColormapData.GetIndex(name) == -1, "The colormap name has already been used!");
    ImVector<ImU32> buffer;
    buffer.resize(size);
    for (int i = 0; i < size; ++i)
        buffer[i] = ImGui::ColorConvertFloat4ToU32(colormap[i]);
    return gp.ColormapData.Append(name, buffer.Data, size, qual);
}

ImPlotColormap AddColormap(const char* name, const ImU32*  colormap, int size, bool qual) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(size > 1, "The colormap size must be greater than 1!");
    IM_ASSERT_USER_ERROR(gp.ColormapData.GetIndex(name) == -1, "The colormap name has already be used!");
    return gp.ColormapData.Append(name, colormap, size, qual);
}

int GetColormapCount() {
    ImPlotContext& gp = *GImPlot;
    return gp.ColormapData.Count;
}

const char* GetColormapName(ImPlotColormap colormap) {
    ImPlotContext& gp = *GImPlot;
    return gp.ColormapData.GetName(colormap);
}

ImPlotColormap GetColormapIndex(const char* name) {
    ImPlotContext& gp = *GImPlot;
    return gp.ColormapData.GetIndex(name);
}

void PushColormap(ImPlotColormap colormap) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(colormap >= 0 && colormap < gp.ColormapData.Count, "The colormap index is invalid!");
    gp.ColormapModifiers.push_back(gp.Style.Colormap);
    gp.Style.Colormap = colormap;
}

void PushColormap(const char* name) {
    ImPlotContext& gp = *GImPlot;
    ImPlotColormap idx = gp.ColormapData.GetIndex(name);
    IM_ASSERT_USER_ERROR(idx != -1, "The colormap name is invalid!");
    PushColormap(idx);
}

void PopColormap(int count) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(count <= gp.ColormapModifiers.Size, "You can't pop more modifiers than have been pushed!");
    while (count > 0) {
        const ImPlotColormap& backup = gp.ColormapModifiers.back();
        gp.Style.Colormap     = backup;
        gp.ColormapModifiers.pop_back();
        count--;
    }
}

ImU32 NextColormapColorU32() {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentItems != NULL, "NextColormapColor() needs to be called between BeginPlot() and EndPlot()!");
    int idx = gp.CurrentItems->ColormapIdx % gp.ColormapData.GetKeyCount(gp.Style.Colormap);
    ImU32 col  = gp.ColormapData.GetKeyColor(gp.Style.Colormap, idx);
    gp.CurrentItems->ColormapIdx++;
    return col;
}

ImVec4 NextColormapColor() {
    return ImGui::ColorConvertU32ToFloat4(NextColormapColorU32());
}

int GetColormapSize(ImPlotColormap cmap) {
    ImPlotContext& gp = *GImPlot;
    cmap = cmap == IMPLOT_AUTO ? gp.Style.Colormap : cmap;
    IM_ASSERT_USER_ERROR(cmap >= 0 && cmap < gp.ColormapData.Count, "Invalid colormap index!");
    return gp.ColormapData.GetKeyCount(cmap);
}

ImU32 GetColormapColorU32(int idx, ImPlotColormap cmap) {
    ImPlotContext& gp = *GImPlot;
    cmap = cmap == IMPLOT_AUTO ? gp.Style.Colormap : cmap;
    IM_ASSERT_USER_ERROR(cmap >= 0 && cmap < gp.ColormapData.Count, "Invalid colormap index!");
    idx = idx % gp.ColormapData.GetKeyCount(cmap);
    return gp.ColormapData.GetKeyColor(cmap, idx);
}

ImVec4 GetColormapColor(int idx, ImPlotColormap cmap) {
    return ImGui::ColorConvertU32ToFloat4(GetColormapColorU32(idx,cmap));
}

ImU32  SampleColormapU32(float t, ImPlotColormap cmap) {
    ImPlotContext& gp = *GImPlot;
    cmap = cmap == IMPLOT_AUTO ? gp.Style.Colormap : cmap;
    IM_ASSERT_USER_ERROR(cmap >= 0 && cmap < gp.ColormapData.Count, "Invalid colormap index!");
    return gp.ColormapData.LerpTable(cmap, t);
}

ImVec4 SampleColormap(float t, ImPlotColormap cmap) {
    return ImGui::ColorConvertU32ToFloat4(SampleColormapU32(t,cmap));
}

void RenderColorBar(const ImU32* colors, int size, ImDrawList& DrawList, const ImRect& bounds, bool vert, bool reversed, bool continuous) {
    const int n = continuous ? size - 1 : size;
    ImU32 col1, col2;
    if (vert) {
        const float step = bounds.GetHeight() / n;
        ImRect rect(bounds.Min.x, bounds.Min.y, bounds.Max.x, bounds.Min.y + step);
        for (int i = 0; i < n; ++i) {
            if (reversed) {
                col1 = colors[size-i-1];
                col2 = continuous ? colors[size-i-2] : col1;
            }
            else {
                col1 = colors[i];
                col2 = continuous ? colors[i+1] : col1;
            }
            DrawList.AddRectFilledMultiColor(rect.Min, rect.Max, col1, col1, col2, col2);
            rect.TranslateY(step);
        }
    }
    else {
        const float step = bounds.GetWidth() / n;
        ImRect rect(bounds.Min.x, bounds.Min.y, bounds.Min.x + step, bounds.Max.y);
        for (int i = 0; i < n; ++i) {
            if (reversed) {
                col1 = colors[size-i-1];
                col2 = continuous ? colors[size-i-2] : col1;
            }
            else {
                col1 = colors[i];
                col2 = continuous ? colors[i+1] : col1;
            }
            DrawList.AddRectFilledMultiColor(rect.Min, rect.Max, col1, col2, col2, col1);
            rect.TranslateX(step);
        }
    }
}

void ColormapScale(const char* label, double scale_min, double scale_max, const ImVec2& size, ImPlotColormap cmap, char* fmt) {
    ImGuiContext &G      = *GImGui;
    ImGuiWindow * Window = G.CurrentWindow;
    if (Window->SkipItems)
        return;

    const ImGuiID ID = Window->GetID(label);
    ImVec2 label_size(0,0);
    label_size = ImGui::CalcTextSize(label,NULL,true);

    ImPlotContext& gp = *GImPlot;
    cmap = cmap == IMPLOT_AUTO ? gp.Style.Colormap : cmap;
    IM_ASSERT_USER_ERROR(cmap >= 0 && cmap < gp.ColormapData.Count, "Invalid colormap index!");

    ImVec2 frame_size  = ImGui::CalcItemSize(size, 0, gp.Style.PlotDefaultSize.y);
    if (frame_size.y < gp.Style.PlotMinSize.y && size.y < 0.0f)
        frame_size.y = gp.Style.PlotMinSize.y;

    ImLimits range(scale_min,scale_max);
    gp.CTicks.Reset();
    AddTicksDefault(range, frame_size.y, true, gp.CTicks, DefaultFormatter, fmt);

    const float txt_off   = gp.Style.LabelPadding.x;
    const float pad_right = txt_off + gp.CTicks.MaxWidth + (label_size.x > 0 ? txt_off + label_size.y : 0);
    float bar_w           = 20;

    if (frame_size.x == 0)
        frame_size.x = bar_w + pad_right + 2 * gp.Style.PlotPadding.x;
    else {
        bar_w = frame_size.x - (pad_right + 2 * gp.Style.PlotPadding.x);
        if (bar_w < gp.Style.MajorTickLen.y)
            bar_w = gp.Style.MajorTickLen.y;
    }

    ImDrawList &DrawList = *Window->DrawList;
    ImRect bb_frame = ImRect(Window->DC.CursorPos, Window->DC.CursorPos + frame_size);
    ImGui::ItemSize(bb_frame);
    if (!ImGui::ItemAdd(bb_frame, ID, &bb_frame))
        return;

    ImGui::RenderFrame(bb_frame.Min, bb_frame.Max, GetStyleColorU32(ImPlotCol_FrameBg), true, G.Style.FrameRounding);
    ImRect bb_grad(bb_frame.Min + gp.Style.PlotPadding, bb_frame.Min + ImVec2(bar_w + gp.Style.PlotPadding.x, frame_size.y - gp.Style.PlotPadding.y));

    ImGui::PushClipRect(bb_frame.Min, bb_frame.Max, true);
    RenderColorBar(gp.ColormapData.GetKeys(cmap), gp.ColormapData.GetKeyCount(cmap), DrawList, bb_grad, true, true, !gp.ColormapData.IsQual(cmap));
    const ImU32 col_tick = GetStyleColorU32(ImPlotCol_AxisText);
    const ImU32 col_text = ImGui::GetColorU32(ImGuiCol_Text);
    for (int i = 0; i < gp.CTicks.Size; ++i) {
        const float ypos = ImRemap((float)gp.CTicks.Ticks[i].PlotPos, (float)range.Max, (float)range.Min, bb_grad.Min.y, bb_grad.Max.y);
        const float tick_width = gp.CTicks.Ticks[i].Major ? gp.Style.MajorTickLen.y : gp.Style.MinorTickLen.y;
        const float tick_thick = gp.CTicks.Ticks[i].Major ? gp.Style.MajorTickSize.y : gp.Style.MinorTickSize.y;
        if (ypos < bb_grad.Max.y - 2 && ypos > bb_grad.Min.y + 2)
            DrawList.AddLine(ImVec2(bb_grad.Max.x-1, ypos), ImVec2(bb_grad.Max.x - tick_width, ypos), col_tick, tick_thick);
        DrawList.AddText(ImVec2(bb_grad.Max.x-1, ypos) + ImVec2(txt_off, -gp.CTicks.Ticks[i].LabelSize.y * 0.5f), col_text, gp.CTicks.GetText(i));
    }
    if (label_size.x > 0) {
        ImVec2 label_pos(bb_grad.Max.x - 1 + 2*txt_off + gp.CTicks.MaxWidth, bb_grad.GetCenter().y + label_size.x*0.5f );
        const char* label_end = ImGui::FindRenderedTextEnd(label);
        AddTextVertical(&DrawList,label_pos,col_text,label,label_end);
    }
    DrawList.AddRect(bb_grad.Min, bb_grad.Max, GetStyleColorU32(ImPlotCol_PlotBorder));
    ImGui::PopClipRect();
}

bool ColormapSlider(const char* label, float* t, ImVec4* out, const char* format, ImPlotColormap cmap) {
    *t = ImClamp(*t,0.0f,1.0f);
    ImGuiContext &G      = *GImGui;
    ImGuiWindow * Window = G.CurrentWindow;
    if (Window->SkipItems)
        return false;
    ImPlotContext& gp = *GImPlot;
    cmap = cmap == IMPLOT_AUTO ? gp.Style.Colormap : cmap;
    IM_ASSERT_USER_ERROR(cmap >= 0 && cmap < gp.ColormapData.Count, "Invalid colormap index!");
    const ImU32* keys  = GImPlot->ColormapData.GetKeys(cmap);
    const int    count = GImPlot->ColormapData.GetKeyCount(cmap);
    const bool   qual  = GImPlot->ColormapData.IsQual(cmap);
    const ImVec2 pos  = ImGui::GetCurrentWindow()->DC.CursorPos;
    const float w     = ImGui::CalcItemWidth();
    const float h     = ImGui::GetFrameHeight();
    const ImRect rect = ImRect(pos.x,pos.y,pos.x+w,pos.y+h);
    RenderColorBar(keys,count,*ImGui::GetWindowDrawList(),rect,false,false,!qual);
    const ImU32 grab = CalcTextColor(GImPlot->ColormapData.LerpTable(cmap,*t));
    // const ImU32 text = CalcTextColor(GImPlot->ColormapData.LerpTable(cmap,0.5f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,IM_COL32_BLACK_TRANS);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,IM_COL32_BLACK_TRANS);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,ImVec4(1,1,1,0.1f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab,grab);
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, grab);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize,2);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,0);
    const bool changed = ImGui::SliderFloat(label,t,0,1,format);
    ImGui::PopStyleColor(5);
    ImGui::PopStyleVar(2);
    if (out != NULL)
        *out = ImGui::ColorConvertU32ToFloat4(GImPlot->ColormapData.LerpTable(cmap,*t));
    return changed;
}

bool ColormapButton(const char* label, const ImVec2& size_arg, ImPlotColormap cmap) {
    ImGuiContext &G      = *GImGui;
    const ImGuiStyle& style = G.Style;
    ImGuiWindow * Window = G.CurrentWindow;
    if (Window->SkipItems)
        return false;
    ImPlotContext& gp = *GImPlot;
    cmap = cmap == IMPLOT_AUTO ? gp.Style.Colormap : cmap;
    IM_ASSERT_USER_ERROR(cmap >= 0 && cmap < gp.ColormapData.Count, "Invalid colormap index!");
    const ImU32* keys  = GImPlot->ColormapData.GetKeys(cmap);
    const int    count = GImPlot->ColormapData.GetKeyCount(cmap);
    const bool   qual  = GImPlot->ColormapData.IsQual(cmap);
    const ImVec2 pos  = ImGui::GetCurrentWindow()->DC.CursorPos;
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
    const ImRect rect = ImRect(pos.x,pos.y,pos.x+size.x,pos.y+size.y);
    RenderColorBar(keys,count,*ImGui::GetWindowDrawList(),rect,false,false,!qual);
    const ImU32 text = CalcTextColor(GImPlot->ColormapData.LerpTable(cmap,G.Style.ButtonTextAlign.x));
    ImGui::PushStyleColor(ImGuiCol_Button,IM_COL32_BLACK_TRANS);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,ImVec4(1,1,1,0.1f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,ImVec4(1,1,1,0.2f));
    ImGui::PushStyleColor(ImGuiCol_Text,text);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,0);
    const bool pressed = ImGui::Button(label,size);
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(1);
    return pressed;
}

//-----------------------------------------------------------------------------
// [Section] Miscellaneous
//-----------------------------------------------------------------------------

void ItemIcon(const ImVec4& col) {
    ItemIcon(ImGui::ColorConvertFloat4ToU32(col));
}

void ItemIcon(ImU32 col) {
    const float txt_size = ImGui::GetTextLineHeight();
    ImVec2 size(txt_size-4,txt_size);
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImVec2 pos = window->DC.CursorPos;
    ImGui::GetWindowDrawList()->AddRectFilled(pos + ImVec2(0,2), pos + size - ImVec2(0,2), col);
    ImGui::Dummy(size);
}

void ColormapIcon(ImPlotColormap cmap) {
    ImPlotContext& gp = *GImPlot;
    const float txt_size = ImGui::GetTextLineHeight();
    ImVec2 size(txt_size-4,txt_size);
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImVec2 pos = window->DC.CursorPos;
    ImRect rect(pos+ImVec2(0,2),pos+size-ImVec2(0,2));
    ImDrawList& DrawList = *ImGui::GetWindowDrawList();
    RenderColorBar(gp.ColormapData.GetKeys(cmap),gp.ColormapData.GetKeyCount(cmap),DrawList,rect,false,false,!gp.ColormapData.IsQual(cmap));
    ImGui::Dummy(size);
}

ImDrawList* GetPlotDrawList() {
    return ImGui::GetWindowDrawList();
}

void PushPlotClipRect(float expand) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != NULL, "PushPlotClipRect() needs to be called between BeginPlot() and EndPlot()!");
    SetupLock();
    ImRect rect = gp.CurrentPlot->PlotRect;
    rect.Expand(expand);
    ImGui::PushClipRect(rect.Min, rect.Max, true);
}

void PopPlotClipRect() {
    SetupLock();
    ImGui::PopClipRect();
}

static void HelpMarker(const char* desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool ShowStyleSelector(const char* label)
{
    static int style_idx = -1;
    if (ImGui::Combo(label, &style_idx, "Auto\0Classic\0Dark\0Light\0"))
    {
        switch (style_idx)
        {
        case 0: StyleColorsAuto(); break;
        case 1: StyleColorsClassic(); break;
        case 2: StyleColorsDark(); break;
        case 3: StyleColorsLight(); break;
        }
        return true;
    }
    return false;
}

bool ShowColormapSelector(const char* label) {
    ImPlotContext& gp = *GImPlot;
    bool set = false;
    if (ImGui::BeginCombo(label, gp.ColormapData.GetName(gp.Style.Colormap))) {
        for (int i = 0; i < gp.ColormapData.Count; ++i) {
            const char* name = gp.ColormapData.GetName(i);
            if (ImGui::Selectable(name, gp.Style.Colormap == i)) {
                gp.Style.Colormap = i;
                ImPlot::BustItemCache();
                set = true;
            }
        }
        ImGui::EndCombo();
    }
    return set;
}

void ShowStyleEditor(ImPlotStyle* ref) {
    ImPlotContext& gp = *GImPlot;
    ImPlotStyle& style = GetStyle();
    static ImPlotStyle ref_saved_style;
    // Default to using internal storage as reference
    static bool init = true;
    if (init && ref == NULL)
        ref_saved_style = style;
    init = false;
    if (ref == NULL)
        ref = &ref_saved_style;

    if (ImPlot::ShowStyleSelector("Colors##Selector"))
        ref_saved_style = style;

    // Save/Revert button
    if (ImGui::Button("Save Ref"))
        *ref = ref_saved_style = style;
    ImGui::SameLine();
    if (ImGui::Button("Revert Ref"))
        style = *ref;
    ImGui::SameLine();
    HelpMarker("Save/Revert in local non-persistent storage. Default Colors definition are not affected. "
               "Use \"Export\" below to save them somewhere.");
    if (ImGui::BeginTabBar("##StyleEditor")) {
        if (ImGui::BeginTabItem("Variables")) {
            ImGui::Text("Item Styling");
            ImGui::SliderFloat("LineWeight", &style.LineWeight, 0.0f, 5.0f, "%.1f");
            ImGui::SliderFloat("MarkerSize", &style.MarkerSize, 2.0f, 10.0f, "%.1f");
            ImGui::SliderFloat("MarkerWeight", &style.MarkerWeight, 0.0f, 5.0f, "%.1f");
            ImGui::SliderFloat("FillAlpha", &style.FillAlpha, 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat("ErrorBarSize", &style.ErrorBarSize, 0.0f, 10.0f, "%.1f");
            ImGui::SliderFloat("ErrorBarWeight", &style.ErrorBarWeight, 0.0f, 5.0f, "%.1f");
            ImGui::SliderFloat("DigitalBitHeight", &style.DigitalBitHeight, 0.0f, 20.0f, "%.1f");
            ImGui::SliderFloat("DigitalBitGap", &style.DigitalBitGap, 0.0f, 20.0f, "%.1f");
            float indent = ImGui::CalcItemWidth() - ImGui::GetFrameHeight();
            ImGui::Indent(ImGui::CalcItemWidth() - ImGui::GetFrameHeight());
            ImGui::Checkbox("AntiAliasedLines", &style.AntiAliasedLines);
            ImGui::Unindent(indent);
            ImGui::Text("Plot Styling");
            ImGui::SliderFloat("PlotBorderSize", &style.PlotBorderSize, 0.0f, 2.0f, "%.0f");
            ImGui::SliderFloat("MinorAlpha", &style.MinorAlpha, 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat2("MajorTickLen", (float*)&style.MajorTickLen, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("MinorTickLen", (float*)&style.MinorTickLen, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("MajorTickSize",  (float*)&style.MajorTickSize, 0.0f, 2.0f, "%.1f");
            ImGui::SliderFloat2("MinorTickSize", (float*)&style.MinorTickSize, 0.0f, 2.0f, "%.1f");
            ImGui::SliderFloat2("MajorGridSize", (float*)&style.MajorGridSize, 0.0f, 2.0f, "%.1f");
            ImGui::SliderFloat2("MinorGridSize", (float*)&style.MinorGridSize, 0.0f, 2.0f, "%.1f");
            ImGui::SliderFloat2("PlotDefaultSize", (float*)&style.PlotDefaultSize, 0.0f, 1000, "%.0f");
            ImGui::SliderFloat2("PlotMinSize", (float*)&style.PlotMinSize, 0.0f, 300, "%.0f");
            ImGui::Text("Plot Padding");
            ImGui::SliderFloat2("PlotPadding", (float*)&style.PlotPadding, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("LabelPadding", (float*)&style.LabelPadding, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("LegendPadding", (float*)&style.LegendPadding, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("LegendInnerPadding", (float*)&style.LegendInnerPadding, 0.0f, 10.0f, "%.0f");
            ImGui::SliderFloat2("LegendSpacing", (float*)&style.LegendSpacing, 0.0f, 5.0f, "%.0f");
            ImGui::SliderFloat2("MousePosPadding", (float*)&style.MousePosPadding, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("AnnotationPadding", (float*)&style.AnnotationPadding, 0.0f, 5.0f, "%.0f");
            ImGui::SliderFloat2("FitPadding", (float*)&style.FitPadding, 0, 0.2f, "%.2f");

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Colors")) {
            static int output_dest = 0;
            static bool output_only_modified = false;

            if (ImGui::Button("Export", ImVec2(75,0))) {
                if (output_dest == 0)
                    ImGui::LogToClipboard();
                else
                    ImGui::LogToTTY();
                ImGui::LogText("ImVec4* colors = ImPlot::GetStyle().Colors;\n");
                for (int i = 0; i < ImPlotCol_COUNT; i++) {
                    const ImVec4& col = style.Colors[i];
                    const char* name = ImPlot::GetStyleColorName(i);
                    if (!output_only_modified || memcmp(&col, &ref->Colors[i], sizeof(ImVec4)) != 0) {
                        if (IsColorAuto(i))
                            ImGui::LogText("colors[ImPlotCol_%s]%*s= IMPLOT_AUTO_COL;\n",name,14 - (int)strlen(name), "");
                        else
                            ImGui::LogText("colors[ImPlotCol_%s]%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);\n",
                                        name, 14 - (int)strlen(name), "", col.x, col.y, col.z, col.w);
                    }
                }
                ImGui::LogFinish();
            }
            ImGui::SameLine(); ImGui::SetNextItemWidth(120); ImGui::Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
            ImGui::SameLine(); ImGui::Checkbox("Only Modified Colors", &output_only_modified);

            static ImGuiTextFilter filter;
            filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

            static ImGuiColorEditFlags alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf;
            if (ImGui::RadioButton("Opaque", alpha_flags == ImGuiColorEditFlags_None))             { alpha_flags = ImGuiColorEditFlags_None; } ImGui::SameLine();
            if (ImGui::RadioButton("Alpha",  alpha_flags == ImGuiColorEditFlags_AlphaPreview))     { alpha_flags = ImGuiColorEditFlags_AlphaPreview; } ImGui::SameLine();
            if (ImGui::RadioButton("Both",   alpha_flags == ImGuiColorEditFlags_AlphaPreviewHalf)) { alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf; } ImGui::SameLine();
            HelpMarker(
                "In the color list:\n"
                "Left-click on colored square to open color picker,\n"
                "Right-click to open edit options menu.");
            ImGui::Separator();
            ImGui::PushItemWidth(-160);
            for (int i = 0; i < ImPlotCol_COUNT; i++) {
                const char* name = ImPlot::GetStyleColorName(i);
                if (!filter.PassFilter(name))
                    continue;
                ImGui::PushID(i);
                ImVec4 temp = GetStyleColorVec4(i);
                const bool is_auto = IsColorAuto(i);
                if (!is_auto)
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.25f);
                if (ImGui::Button("Auto")) {
                    if (is_auto)
                        style.Colors[i] = temp;
                    else
                        style.Colors[i] = IMPLOT_AUTO_COL;
                    BustItemCache();
                }
                if (!is_auto)
                    ImGui::PopStyleVar();
                ImGui::SameLine();
                if (ImGui::ColorEdit4(name, &temp.x, ImGuiColorEditFlags_NoInputs | alpha_flags)) {
                    style.Colors[i] = temp;
                    BustItemCache();
                }
                if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(ImVec4)) != 0) {
                    ImGui::SameLine(175); if (ImGui::Button("Save")) { ref->Colors[i] = style.Colors[i]; }
                    ImGui::SameLine(); if (ImGui::Button("Revert")) {
                        style.Colors[i] = ref->Colors[i];
                        BustItemCache();
                    }
                }
                ImGui::PopID();
            }
            ImGui::PopItemWidth();
            ImGui::Separator();
            ImGui::Text("Colors that are set to Auto (i.e. IMPLOT_AUTO_COL) will\n"
                        "be automatically deduced from your ImGui style or the\n"
                        "current ImPlot Colormap. If you want to style individual\n"
                        "plot items, use Push/PopStyleColor around its function.");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Colormaps")) {
            static int output_dest = 0;
            if (ImGui::Button("Export", ImVec2(75,0))) {
                if (output_dest == 0)
                    ImGui::LogToClipboard();
                else
                    ImGui::LogToTTY();
                int size = GetColormapSize();
                const char* name = GetColormapName(gp.Style.Colormap);
                ImGui::LogText("static const ImU32 %s_Data[%d] = {\n", name, size);
                for (int i = 0; i < size; ++i) {
                    ImU32 col = GetColormapColorU32(i,gp.Style.Colormap);
                    ImGui::LogText("    %u%s\n", col, i == size - 1 ? "" : ",");
                }
                ImGui::LogText("};\nImPlotColormap %s = ImPlot::AddColormap(\"%s\", %s_Data, %d);", name, name, name, size);
                ImGui::LogFinish();
            }
            ImGui::SameLine(); ImGui::SetNextItemWidth(120); ImGui::Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
            ImGui::SameLine();
            static bool edit = false;
            ImGui::Checkbox("Edit Mode",&edit);

            // built-in/added
            ImGui::Separator();
            for (int i = 0; i < gp.ColormapData.Count; ++i) {
                ImGui::PushID(i);
                int size = gp.ColormapData.GetKeyCount(i);
                bool selected = i == gp.Style.Colormap;

                const char* name = GetColormapName(i);
                if (!selected)
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.25f);
                if (ImGui::Button(name, ImVec2(100,0))) {
                    gp.Style.Colormap = i;
                    BustItemCache();
                }
                if (!selected)
                    ImGui::PopStyleVar();
                ImGui::SameLine();
                ImGui::BeginGroup();
                if (edit) {
                    for (int c = 0; c < size; ++c) {
                        ImGui::PushID(c);
                        ImVec4 col4 = ImGui::ColorConvertU32ToFloat4(gp.ColormapData.GetKeyColor(i,c));
                        if (ImGui::ColorEdit4("",&col4.x,ImGuiColorEditFlags_NoInputs)) {
                            ImU32 col32 = ImGui::ColorConvertFloat4ToU32(col4);
                            gp.ColormapData.SetKeyColor(i,c,col32);
                            BustItemCache();
                        }
                        if ((c + 1) % 12 != 0 && c != size -1)
                            ImGui::SameLine();
                        ImGui::PopID();
                    }
                }
                else {
                    if (ImPlot::ColormapButton("##",ImVec2(-1,0),i))
                        edit = true;
                }
                ImGui::EndGroup();
                ImGui::PopID();
            }


            static ImVector<ImVec4> custom;
            if (custom.Size == 0) {
                custom.push_back(ImVec4(1,0,0,1));
                custom.push_back(ImVec4(0,1,0,1));
                custom.push_back(ImVec4(0,0,1,1));
            }
            ImGui::Separator();
            ImGui::BeginGroup();
            static char name[16] = "MyColormap";


            if (ImGui::Button("+", ImVec2((100 - ImGui::GetStyle().ItemSpacing.x)/2,0)))
                custom.push_back(ImVec4(0,0,0,1));
            ImGui::SameLine();
            if (ImGui::Button("-", ImVec2((100 - ImGui::GetStyle().ItemSpacing.x)/2,0)) && custom.Size > 2)
                custom.pop_back();
            ImGui::SetNextItemWidth(100);
            ImGui::InputText("##Name",name,16,ImGuiInputTextFlags_CharsNoBlank);
            static bool qual = true;
            ImGui::Checkbox("Qualitative",&qual);
            if (ImGui::Button("Add", ImVec2(100, 0)) && gp.ColormapData.GetIndex(name)==-1)
                AddColormap(name,custom.Data,custom.Size,qual);

            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();
            for (int c = 0; c < custom.Size; ++c) {
                ImGui::PushID(c);
                if (ImGui::ColorEdit4("##Col1", &custom[c].x, ImGuiColorEditFlags_NoInputs)) {

                }
                if ((c + 1) % 12 != 0)
                    ImGui::SameLine();
                ImGui::PopID();
            }
            ImGui::EndGroup();


            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void ShowUserGuide() {
        ImGui::BulletText("Left-click drag within the plot area to pan X and Y axes.");
    ImGui::Indent();
        ImGui::BulletText("Left-click drag on axis labels to pan an individual axis.");
    ImGui::Unindent();
    ImGui::BulletText("Scroll in the plot area to zoom both X any Y axes.");
    ImGui::Indent();
        ImGui::BulletText("Scroll on axis labels to zoom an individual axis.");
    ImGui::Unindent();
    ImGui::BulletText("Right-click drag to box select data.");
    ImGui::Indent();
        ImGui::BulletText("Hold Alt to expand box selection horizontally.");
        ImGui::BulletText("Hold Shift to expand box selection vertically.");
        ImGui::BulletText("Left-click while box selecting to cancel the selection.");
    ImGui::Unindent();
    ImGui::BulletText("Double left-click to fit all visible data.");
    ImGui::Indent();
        ImGui::BulletText("Double left-click axis labels to fit the individual axis.");
    ImGui::Unindent();
    ImGui::BulletText("Right-click open the full plot context menu.");
    ImGui::Indent();
        ImGui::BulletText("Right-click axis labels to open an individual axis context menu.");
    ImGui::Unindent();
    ImGui::BulletText("Click legend label icons to show/hide plot items.");
}

void ShowAxisMetrics(const ImPlotPlot& plot, const ImPlotAxis& axis) {
    ImGui::Bullet(); ImGui::Text("Label: %s", axis.LabelOffset == -1 ? "none" : plot.GetAxisLabel(axis));
    ImGui::Bullet(); ImGui::Text("Flags: %d", axis.Flags);
    ImGui::Bullet(); ImGui::Text("Range: [%f,%f]",axis.Range.Min, axis.Range.Max);
    ImGui::Bullet(); ImGui::Text("Pixels: %f", axis.Pixels);
    ImGui::Bullet(); ImGui::Text("Aspect: %f", axis.GetAspect());
    ImGui::Bullet(); ImGui::Text("Hovered: %s", axis.Hovered ? "true" : "false");
    ImGui::Bullet(); ImGui::Text("Held: %s", axis.Held ? "true" : "false");
    ImGui::Bullet(); ImGui::Text("Present: %s", axis.Enabled ? "true" : "false");
    ImGui::Bullet(); ImGui::Text("HasRange: %s", axis.HasRange ? "true" : "false");
    ImGui::Bullet(); ImGui::Text("LinkedMin: %p", (void*)axis.LinkedMin);
    ImGui::Bullet(); ImGui::Text("LinkedMax: %p", (void*)axis.LinkedMax);
}

void ShowMetricsWindow(bool* p_popen) {

    static bool show_plot_rects = false;
    static bool show_axes_rects = false;
    static bool show_axis_rects = false;
    static bool show_canvas_rects = false;
    static bool show_frame_rects = false;
    static bool show_subplot_frame_rects = false;
    static bool show_subplot_grid_rects = false;

    ImDrawList& fg = *ImGui::GetForegroundDrawList();

    ImPlotContext& gp = *GImPlot;
    // ImGuiContext& g = *GImGui;
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Begin("ImPlot Metrics", p_popen);
    ImGui::Text("ImPlot " IMPLOT_VERSION);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::Separator();
    if (ImGui::TreeNode("Tools")) {
        if (ImGui::Button("Bust Plot Cache"))
            BustPlotCache();
        ImGui::SameLine();
        if (ImGui::Button("Bust Item Cache"))
            BustItemCache();
        ImGui::Checkbox("Show Frame Rects", &show_frame_rects);
        ImGui::Checkbox("Show Canvas Rects",&show_canvas_rects);
        ImGui::Checkbox("Show Plot Rects",  &show_plot_rects);
        ImGui::Checkbox("Show Axes Rects",  &show_axes_rects);
        ImGui::Checkbox("Show Axis Rects",  &show_axis_rects);
        ImGui::Checkbox("Show Subplot Frame Rects",  &show_subplot_frame_rects);
        ImGui::Checkbox("Show Subplot Grid Rects",  &show_subplot_grid_rects);

        ImGui::TreePop();
    }
    const int n_plots = gp.Plots.GetBufSize();
    const int n_subplots = gp.Subplots.GetBufSize();
    // render rects
    for (int p = 0; p < n_plots; ++p) {
        ImPlotPlot* plot = gp.Plots.GetByIndex(p);
        if (show_frame_rects)
            fg.AddRect(plot->FrameRect.Min, plot->FrameRect.Max, IM_COL32(255,0,255,255));
        if (show_canvas_rects)
            fg.AddRect(plot->CanvasRect.Min, plot->CanvasRect.Max, IM_COL32(0,255,255,255));
        if (show_plot_rects)
            fg.AddRect(plot->PlotRect.Min, plot->PlotRect.Max, IM_COL32(255,255,0,255));
        if (show_axes_rects)
            fg.AddRect(plot->AxesRect.Min, plot->AxesRect.Max, IM_COL32(0,255,128,255));
        if (show_axis_rects) {
            for (int i = 0; i < IMPLOT_MAX_AXES; ++i) {
                if (plot->XAxis[i].Enabled)
                    fg.AddRect(plot->XAxis[i].HoverRect.Min, plot->XAxis[i].HoverRect.Max, IM_COL32(0,255,0,255));
                if (plot->YAxis[i].Enabled)
                    fg.AddRect(plot->YAxis[i].HoverRect.Min, plot->YAxis[i].HoverRect.Max, IM_COL32(0,255,0,255));
            }
        }
    }
    for (int p = 0; p < n_subplots; ++p) {
        ImPlotSubplot* subplot = gp.Subplots.GetByIndex(p);
        if (show_subplot_frame_rects)
            fg.AddRect(subplot->FrameRect.Min, subplot->FrameRect.Max, IM_COL32(255,0,0,255));
        if (show_subplot_grid_rects)
            fg.AddRect(subplot->GridRect.Min, subplot->GridRect.Max, IM_COL32(0,0,255,255));
    }
    if (ImGui::TreeNode("Plots","Plots (%d)", n_plots)) {
        for (int p = 0; p < n_plots; ++p) {
            // plot
            ImPlotPlot& plot = *gp.Plots.GetByIndex(p);
            ImGui::PushID(p);
            if (ImGui::TreeNode("Plot", "Plot [ID=0x%08X]", plot.ID)) {
                int n_items = plot.Items.GetItemCount();
                if (ImGui::TreeNode("Items", "Items (%d)", n_items)) {
                    for (int i = 0; i < n_items; ++i) {
                        ImPlotItem* item = plot.Items.GetItemByIndex(i);
                        ImGui::PushID(i);
                        if (ImGui::TreeNode("Item", "Item [ID=0x%08X]", item->ID)) {
                            ImGui::Bullet(); ImGui::Checkbox("Show", &item->Show);
                            ImGui::Bullet();
                            ImVec4 temp = ImGui::ColorConvertU32ToFloat4(item->Color);
                            if (ImGui::ColorEdit4("Color",&temp.x, ImGuiColorEditFlags_NoInputs))
                                item->Color = ImGui::ColorConvertFloat4ToU32(temp);

                            ImGui::Bullet(); ImGui::Text("NameOffset: %d",item->NameOffset);
                            ImGui::Bullet(); ImGui::Text("Name: %s", item->NameOffset != -1 ? plot.Items.Legend.Labels.Buf.Data + item->NameOffset : "N/A");
                            ImGui::Bullet(); ImGui::Text("Hovered: %s",item->LegendHovered ? "true" : "false");
                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                    }
                    ImGui::TreePop();
                }
                char buff[16];
                for (int i = 0; i < IMPLOT_MAX_AXES; ++i) {
                    snprintf(buff,16,"X-Axis %d", i+1);
                    if (plot.XAxis[i].Enabled && ImGui::TreeNode(buff, "X-Axis %d [ID=0x%08X]", i+1, plot.XAxis[i].ID)) {
                        ShowAxisMetrics(plot, plot.XAxis[i]);
                        ImGui::TreePop();
                    }
                    snprintf(buff,16,"Y-Axis %d", i+1);
                    if (plot.YAxis[i].Enabled && ImGui::TreeNode(buff, "Y-Axis %d [ID=0x%08X]", i+1, plot.YAxis[i].ID)) {
                        ShowAxisMetrics(plot, plot.YAxis[i]);
                        ImGui::TreePop();
                    }
                }
                ImGui::Bullet(); ImGui::Text("Flags: 0x%08X", plot.Flags);
                ImGui::Bullet(); ImGui::Text("Initialized: %s", plot.Initialized ? "true" : "false");
                ImGui::Bullet(); ImGui::Text("Selecting: %s", plot.Selecting ? "true" : "false");
                ImGui::Bullet(); ImGui::Text("Selected: %s", plot.Selected ? "true" : "false");
                ImGui::Bullet(); ImGui::Text("Hovered: %s", plot.Hovered ? "true" : "false");
                ImGui::Bullet(); ImGui::Text("Held: %s", plot.Held ? "true" : "false");
                ImGui::Bullet(); ImGui::Text("LegendHovered: %s", plot.Items.Legend.Hovered ? "true" : "false");
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Subplots","Subplots (%d)", n_subplots)) {
        for (int p = 0; p < n_subplots; ++p) {
            // plot
            ImPlotSubplot& plot = *gp.Subplots.GetByIndex(p);
            ImGui::PushID(p);
            if (ImGui::TreeNode("Subplot", "Subplot [ID=0x%08X]", plot.ID)) {
                int n_items = plot.Items.GetItemCount();
                if (ImGui::TreeNode("Items", "Items (%d)", n_items)) {
                    for (int i = 0; i < n_items; ++i) {
                        ImPlotItem* item = plot.Items.GetItemByIndex(i);
                        ImGui::PushID(i);
                        if (ImGui::TreeNode("Item", "Item [ID=0x%08X]", item->ID)) {
                            ImGui::Bullet(); ImGui::Checkbox("Show", &item->Show);
                            ImGui::Bullet();
                            ImVec4 temp = ImGui::ColorConvertU32ToFloat4(item->Color);
                            if (ImGui::ColorEdit4("Color",&temp.x, ImGuiColorEditFlags_NoInputs))
                                item->Color = ImGui::ColorConvertFloat4ToU32(temp);

                            ImGui::Bullet(); ImGui::Text("NameOffset: %d",item->NameOffset);
                            ImGui::Bullet(); ImGui::Text("Name: %s", item->NameOffset != -1 ? plot.Items.Legend.Labels.Buf.Data + item->NameOffset : "N/A");
                            ImGui::Bullet(); ImGui::Text("Hovered: %s",item->LegendHovered ? "true" : "false");
                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                    }
                    ImGui::TreePop();
                }
                ImGui::Bullet(); ImGui::Text("Flags: 0x%08X", plot.Flags);
                ImGui::Bullet(); ImGui::Text("FrameHovered: %s", plot.FrameHovered ? "true" : "false");
                ImGui::Bullet(); ImGui::Text("LegendHovered: %s", plot.Items.Legend.Hovered ? "true" : "false");
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Colormaps")) {
        ImGui::BulletText("Colormaps:  %d", gp.ColormapData.Count);
        ImGui::BulletText("Memory: %d bytes", gp.ColormapData.Tables.Size * 4);
        if (ImGui::TreeNode("Data")) {
            for (int m = 0; m < gp.ColormapData.Count; ++m) {
                if (ImGui::TreeNode(gp.ColormapData.GetName(m))) {
                    int count = gp.ColormapData.GetKeyCount(m);
                    int size = gp.ColormapData.GetTableSize(m);
                    bool qual = gp.ColormapData.IsQual(m);
                    ImGui::BulletText("Qualitative: %s", qual ? "true" : "false");
                    ImGui::BulletText("Key Count: %d", count);
                    ImGui::BulletText("Table Size: %d", size);
                    ImGui::Indent();

                    static float t = 0.5;
                    ImVec4 samp;
                    float wid = 32 * 10 - ImGui::GetFrameHeight() - ImGui::GetStyle().ItemSpacing.x;
                    ImGui::SetNextItemWidth(wid);
                    ImPlot::ColormapSlider("##Sample",&t,&samp,"%.3f",m);
                    ImGui::SameLine();
                    ImGui::ColorButton("Sampler",samp);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0,0,0,0));
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
                    for (int c = 0; c < size; ++c) {
                        ImVec4 col = ImGui::ColorConvertU32ToFloat4(gp.ColormapData.GetTableColor(m,c));
                        ImGui::PushID(m*1000+c);
                        ImGui::ColorButton("",col,0,ImVec2(10,10));
                        ImGui::PopID();
                        if ((c + 1) % 32 != 0 && c != size - 1)
                            ImGui::SameLine();
                    }
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor();
                    ImGui::Unindent();
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
    ImGui::End();
}

bool ShowDatePicker(const char* id, int* level, ImPlotTime* t, const ImPlotTime* t1, const ImPlotTime* t2) {

    ImGui::PushID(id);
    ImGui::BeginGroup();

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4 col_txt    = style.Colors[ImGuiCol_Text];
    ImVec4 col_dis    = style.Colors[ImGuiCol_TextDisabled];
    ImVec4 col_btn    = style.Colors[ImGuiCol_Button];
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));

    const float ht    = ImGui::GetFrameHeight();
    ImVec2 cell_size(ht*1.25f,ht);
    char buff[32];
    bool clk = false;
    tm& Tm = GImPlot->Tm;

    const int min_yr = 1970;
    const int max_yr = 2999;

    // t1 parts
    int t1_mo = 0; int t1_md = 0; int t1_yr = 0;
    if (t1 != NULL) {
        GetTime(*t1,&Tm);
        t1_mo = Tm.tm_mon;
        t1_md = Tm.tm_mday;
        t1_yr = Tm.tm_year + 1900;
    }

     // t2 parts
    int t2_mo = 0; int t2_md = 0; int t2_yr = 0;
    if (t2 != NULL) {
        GetTime(*t2,&Tm);
        t2_mo = Tm.tm_mon;
        t2_md = Tm.tm_mday;
        t2_yr = Tm.tm_year + 1900;
    }

    // day widget
    if (*level == 0) {
        *t = FloorTime(*t, ImPlotTimeUnit_Day);
        GetTime(*t, &Tm);
        const int this_year = Tm.tm_year + 1900;
        const int last_year = this_year - 1;
        const int next_year = this_year + 1;
        const int this_mon  = Tm.tm_mon;
        const int last_mon  = this_mon == 0 ? 11 : this_mon - 1;
        const int next_mon  = this_mon == 11 ? 0 : this_mon + 1;
        const int days_this_mo = GetDaysInMonth(this_year, this_mon);
        const int days_last_mo = GetDaysInMonth(this_mon == 0 ? last_year : this_year, last_mon);
        ImPlotTime t_first_mo = FloorTime(*t,ImPlotTimeUnit_Mo);
        GetTime(t_first_mo,&Tm);
        const int first_wd = Tm.tm_wday;
        // month year
        snprintf(buff, 32, "%s %d", MONTH_NAMES[this_mon], this_year);
        if (ImGui::Button(buff))
            *level = 1;
        ImGui::SameLine(5*cell_size.x);
        BeginDisabledControls(this_year <= min_yr && this_mon == 0);
        if (ImGui::ArrowButtonEx("##Up",ImGuiDir_Up,cell_size))
            *t = AddTime(*t, ImPlotTimeUnit_Mo, -1);
        EndDisabledControls(this_year <= min_yr && this_mon == 0);
        ImGui::SameLine();
        BeginDisabledControls(this_year >= max_yr && this_mon == 11);
        if (ImGui::ArrowButtonEx("##Down",ImGuiDir_Down,cell_size))
            *t = AddTime(*t, ImPlotTimeUnit_Mo, 1);
        EndDisabledControls(this_year >= max_yr && this_mon == 11);
        // render weekday abbreviations
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        for (int i = 0; i < 7; ++i) {
            ImGui::Button(WD_ABRVS[i],cell_size);
            if (i != 6) { ImGui::SameLine(); }
        }
        ImGui::PopItemFlag();
        // 0 = last mo, 1 = this mo, 2 = next mo
        int mo = first_wd > 0 ? 0 : 1;
        int day = mo == 1 ? 1 : days_last_mo - first_wd + 1;
        for (int i = 0; i < 6; ++i) {
            for (int j = 0; j < 7; ++j) {
                if (mo == 0 && day > days_last_mo) {
                    mo = 1; 
                    day = 1;
                }
                else if (mo == 1 && day > days_this_mo) {
                    mo = 2; 
                    day = 1;
                }
                const int now_yr = (mo == 0 && this_mon == 0) ? last_year : ((mo == 2 && this_mon == 11) ? next_year : this_year);
                const int now_mo = mo == 0 ? last_mon : (mo == 1 ? this_mon : next_mon);
                const int now_md = day;

                const bool off_mo   = mo == 0 || mo == 2;
                const bool t1_or_t2 = (t1 != NULL && t1_mo == now_mo && t1_yr == now_yr && t1_md == now_md) ||
                                      (t2 != NULL && t2_mo == now_mo && t2_yr == now_yr && t2_md == now_md);

                if (off_mo)
                    ImGui::PushStyleColor(ImGuiCol_Text, col_dis);
                if (t1_or_t2) {
                    ImGui::PushStyleColor(ImGuiCol_Button, col_btn);
                    ImGui::PushStyleColor(ImGuiCol_Text, col_txt);
                }
                ImGui::PushID(i*7+j);
                snprintf(buff,32,"%d",day);
                if (now_yr == min_yr-1 || now_yr == max_yr+1) {
                    ImGui::Dummy(cell_size);
                }
                else if (ImGui::Button(buff,cell_size) && !clk) {
                    *t = MakeTime(now_yr, now_mo, now_md);
                    clk = true;
                }
                ImGui::PopID();
                if (t1_or_t2)
                    ImGui::PopStyleColor(2);
                if (off_mo)
                    ImGui::PopStyleColor();
                if (j != 6)
                    ImGui::SameLine();
                day++;
            }
        }
    }
    // month widget
    else if (*level == 1) {
        *t = FloorTime(*t, ImPlotTimeUnit_Mo);
        GetTime(*t, &Tm);
        int this_yr  = Tm.tm_year + 1900;
        snprintf(buff, 32, "%d", this_yr);
        if (ImGui::Button(buff))
            *level = 2;
        BeginDisabledControls(this_yr <= min_yr);
        ImGui::SameLine(5*cell_size.x);
        if (ImGui::ArrowButtonEx("##Up",ImGuiDir_Up,cell_size))
            *t = AddTime(*t, ImPlotTimeUnit_Yr, -1);
        EndDisabledControls(this_yr <= min_yr);
        ImGui::SameLine();
        BeginDisabledControls(this_yr >= max_yr);
        if (ImGui::ArrowButtonEx("##Down",ImGuiDir_Down,cell_size))
            *t = AddTime(*t, ImPlotTimeUnit_Yr, 1);
        EndDisabledControls(this_yr >= max_yr);
        // ImGui::Dummy(cell_size);
        cell_size.x *= 7.0f/4.0f;
        cell_size.y *= 7.0f/3.0f;
        int mo = 0;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 4; ++j) {
                const bool t1_or_t2 = (t1 != NULL && t1_yr == this_yr && t1_mo == mo) ||
                                      (t2 != NULL && t2_yr == this_yr && t2_mo == mo);
                if (t1_or_t2)
                    ImGui::PushStyleColor(ImGuiCol_Button, col_btn);
                if (ImGui::Button(MONTH_ABRVS[mo],cell_size) && !clk) {
                    *t = MakeTime(this_yr, mo);
                    *level = 0;
                }
                if (t1_or_t2)
                    ImGui::PopStyleColor();
                if (j != 3)
                    ImGui::SameLine();
                mo++;
            }
        }
    }
    else if (*level == 2) {
        *t = FloorTime(*t, ImPlotTimeUnit_Yr);
        int this_yr = GetYear(*t);
        int yr = this_yr  - this_yr % 20;
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        snprintf(buff,32,"%d-%d",yr,yr+19);
        ImGui::Button(buff);
        ImGui::PopItemFlag();
        ImGui::SameLine(5*cell_size.x);
        BeginDisabledControls(yr <= min_yr);
        if (ImGui::ArrowButtonEx("##Up",ImGuiDir_Up,cell_size))
            *t = MakeTime(yr-20);
        EndDisabledControls(yr <= min_yr);
        ImGui::SameLine();
        BeginDisabledControls(yr + 20 >= max_yr);
        if (ImGui::ArrowButtonEx("##Down",ImGuiDir_Down,cell_size))
            *t = MakeTime(yr+20);
        EndDisabledControls(yr+ 20 >= max_yr);
        // ImGui::Dummy(cell_size);
        cell_size.x *= 7.0f/4.0f;
        cell_size.y *= 7.0f/5.0f;
        for (int i = 0; i < 5; ++i) {
            for (int j = 0; j < 4; ++j) {
                const bool t1_or_t2 = (t1 != NULL && t1_yr == yr) || (t2 != NULL && t2_yr == yr);
                if (t1_or_t2)
                    ImGui::PushStyleColor(ImGuiCol_Button, col_btn);
                snprintf(buff,32,"%d",yr);
                if (yr<1970||yr>3000) {
                    ImGui::Dummy(cell_size);
                }
                else if (ImGui::Button(buff,cell_size)) {
                    *t = MakeTime(yr);
                    *level = 1;
                }
                if (t1_or_t2)
                    ImGui::PopStyleColor();
                if (j != 3)
                    ImGui::SameLine();
                yr++;
            }
        }
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::EndGroup();
    ImGui::PopID();
    return clk;
}

bool ShowTimePicker(const char* id, ImPlotTime* t) {
    ImGui::PushID(id);
    tm& Tm = GImPlot->Tm;
    GetTime(*t,&Tm);

    static const char* nums[] = { "00","01","02","03","04","05","06","07","08","09",
                                  "10","11","12","13","14","15","16","17","18","19",
                                  "20","21","22","23","24","25","26","27","28","29",
                                  "30","31","32","33","34","35","36","37","38","39",
                                  "40","41","42","43","44","45","46","47","48","49",
                                  "50","51","52","53","54","55","56","57","58","59"};

    static const char* am_pm[] = {"am","pm"};

    bool hour24 = GImPlot->Style.Use24HourClock;

    int hr  = hour24 ? Tm.tm_hour : ((Tm.tm_hour == 0 || Tm.tm_hour == 12) ? 12 : Tm.tm_hour % 12);
    int min = Tm.tm_min;
    int sec = Tm.tm_sec;
    int ap  = Tm.tm_hour < 12 ? 0 : 1;

    bool changed = false;

    ImVec2 spacing = ImGui::GetStyle().ItemSpacing;
    spacing.x = 0;
    float width    = ImGui::CalcTextSize("888").x;
    float height   = ImGui::GetFrameHeight();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, spacing);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize,2.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));

    ImGui::SetNextItemWidth(width);
    if (ImGui::BeginCombo("##hr",nums[hr],ImGuiComboFlags_NoArrowButton)) {
        const int ia = hour24 ? 0 : 1;
        const int ib = hour24 ? 24 : 13;
        for (int i = ia; i < ib; ++i) {
            if (ImGui::Selectable(nums[i],i==hr)) {
                hr = i;
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    ImGui::Text(":");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(width);
    if (ImGui::BeginCombo("##min",nums[min],ImGuiComboFlags_NoArrowButton)) {
        for (int i = 0; i < 60; ++i) {
            if (ImGui::Selectable(nums[i],i==min)) {
                min = i;
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    ImGui::Text(":");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(width);
    if (ImGui::BeginCombo("##sec",nums[sec],ImGuiComboFlags_NoArrowButton)) {
        for (int i = 0; i < 60; ++i) {
            if (ImGui::Selectable(nums[i],i==sec)) {
                sec = i;
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    if (!hour24) {
        ImGui::SameLine();
        if (ImGui::Button(am_pm[ap],ImVec2(0,height))) {
            ap = 1 - ap;
            changed = true;
        }
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
    ImGui::PopID();

    if (changed) {
        if (!hour24)
            hr = hr % 12 + ap * 12;
        Tm.tm_hour = hr;
        Tm.tm_min  = min;
        Tm.tm_sec  = sec;
        *t = MkTime(&Tm);
    }

    return changed;
}

void StyleColorsAuto(ImPlotStyle* dst) {
    ImPlotStyle* style              = dst ? dst : &ImPlot::GetStyle();
    ImVec4* colors                  = style->Colors;

    style->MinorAlpha               = 0.25f;

    colors[ImPlotCol_Line]          = IMPLOT_AUTO_COL;
    colors[ImPlotCol_Fill]          = IMPLOT_AUTO_COL;
    colors[ImPlotCol_MarkerOutline] = IMPLOT_AUTO_COL;
    colors[ImPlotCol_MarkerFill]    = IMPLOT_AUTO_COL;
    colors[ImPlotCol_ErrorBar]      = IMPLOT_AUTO_COL;
    colors[ImPlotCol_FrameBg]       = IMPLOT_AUTO_COL;
    colors[ImPlotCol_PlotBg]        = IMPLOT_AUTO_COL;
    colors[ImPlotCol_PlotBorder]    = IMPLOT_AUTO_COL;
    colors[ImPlotCol_LegendBg]      = IMPLOT_AUTO_COL;
    colors[ImPlotCol_LegendBorder]  = IMPLOT_AUTO_COL;
    colors[ImPlotCol_LegendText]    = IMPLOT_AUTO_COL;
    colors[ImPlotCol_TitleText]     = IMPLOT_AUTO_COL;
    colors[ImPlotCol_InlayText]     = IMPLOT_AUTO_COL;
    colors[ImPlotCol_PlotBorder]    = IMPLOT_AUTO_COL;
    colors[ImPlotCol_AxisText]      = IMPLOT_AUTO_COL;
    colors[ImPlotCol_AxisGrid]      = IMPLOT_AUTO_COL;
    colors[ImPlotCol_AxisHovered]   = IMPLOT_AUTO_COL;
    colors[ImPlotCol_AxisActive]    = IMPLOT_AUTO_COL;
    colors[ImPlotCol_Selection]     = IMPLOT_AUTO_COL;
    colors[ImPlotCol_Crosshairs]    = IMPLOT_AUTO_COL;
}

void StyleColorsClassic(ImPlotStyle* dst) {
    ImPlotStyle* style              = dst ? dst : &ImPlot::GetStyle();
    ImVec4* colors                  = style->Colors;

    style->MinorAlpha               = 0.5f;

    colors[ImPlotCol_Line]          = IMPLOT_AUTO_COL;
    colors[ImPlotCol_Fill]          = IMPLOT_AUTO_COL;
    colors[ImPlotCol_MarkerOutline] = IMPLOT_AUTO_COL;
    colors[ImPlotCol_MarkerFill]    = IMPLOT_AUTO_COL;
    colors[ImPlotCol_ErrorBar]      = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImPlotCol_FrameBg]       = ImVec4(0.43f, 0.43f, 0.43f, 0.39f);
    colors[ImPlotCol_PlotBg]        = ImVec4(0.00f, 0.00f, 0.00f, 0.35f);
    colors[ImPlotCol_PlotBorder]    = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    colors[ImPlotCol_LegendBg]      = ImVec4(0.11f, 0.11f, 0.14f, 0.92f);
    colors[ImPlotCol_LegendBorder]  = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    colors[ImPlotCol_LegendText]    = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImPlotCol_TitleText]     = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImPlotCol_InlayText]     = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImPlotCol_AxisText]      = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImPlotCol_AxisGrid]      = ImVec4(0.90f, 0.90f, 0.90f, 0.25f);
    colors[ImPlotCol_AxisHovered]   = IMPLOT_AUTO_COL; // TODO
    colors[ImPlotCol_AxisActive]    = IMPLOT_AUTO_COL; // TODO
    colors[ImPlotCol_Selection]     = ImVec4(0.97f, 0.97f, 0.39f, 1.00f);
    colors[ImPlotCol_Crosshairs]    = ImVec4(0.50f, 0.50f, 0.50f, 0.75f);
}

void StyleColorsDark(ImPlotStyle* dst) {
    ImPlotStyle* style              = dst ? dst : &ImPlot::GetStyle();
    ImVec4* colors                  = style->Colors;

    style->MinorAlpha               = 0.25f;

    colors[ImPlotCol_Line]          = IMPLOT_AUTO_COL;
    colors[ImPlotCol_Fill]          = IMPLOT_AUTO_COL;
    colors[ImPlotCol_MarkerOutline] = IMPLOT_AUTO_COL;
    colors[ImPlotCol_MarkerFill]    = IMPLOT_AUTO_COL;
    colors[ImPlotCol_ErrorBar]      = IMPLOT_AUTO_COL;
    colors[ImPlotCol_FrameBg]       = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
    colors[ImPlotCol_PlotBg]        = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
    colors[ImPlotCol_PlotBorder]    = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImPlotCol_LegendBg]      = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImPlotCol_LegendBorder]  = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImPlotCol_LegendText]    = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImPlotCol_TitleText]     = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImPlotCol_InlayText]     = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImPlotCol_AxisText]      = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImPlotCol_AxisGrid]      = ImVec4(1.00f, 1.00f, 1.00f, 0.25f);
    colors[ImPlotCol_AxisHovered]   = IMPLOT_AUTO_COL; // TODO
    colors[ImPlotCol_AxisActive]    = IMPLOT_AUTO_COL; // TODO
    colors[ImPlotCol_Selection]     = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImPlotCol_Crosshairs]    = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
}

void StyleColorsLight(ImPlotStyle* dst) {
    ImPlotStyle* style              = dst ? dst : &ImPlot::GetStyle();
    ImVec4* colors                  = style->Colors;

    style->MinorAlpha               = 1.0f;

    colors[ImPlotCol_Line]          = IMPLOT_AUTO_COL;
    colors[ImPlotCol_Fill]          = IMPLOT_AUTO_COL;
    colors[ImPlotCol_MarkerOutline] = IMPLOT_AUTO_COL;
    colors[ImPlotCol_MarkerFill]    = IMPLOT_AUTO_COL;
    colors[ImPlotCol_ErrorBar]      = IMPLOT_AUTO_COL;
    colors[ImPlotCol_FrameBg]       = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImPlotCol_PlotBg]        = ImVec4(0.42f, 0.57f, 1.00f, 0.13f);
    colors[ImPlotCol_PlotBorder]    = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImPlotCol_LegendBg]      = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
    colors[ImPlotCol_LegendBorder]  = ImVec4(0.82f, 0.82f, 0.82f, 0.80f);
    colors[ImPlotCol_LegendText]    = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImPlotCol_TitleText]     = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImPlotCol_InlayText]     = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImPlotCol_AxisText]      = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImPlotCol_AxisGrid]      = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImPlotCol_AxisHovered]   = IMPLOT_AUTO_COL; // TODO
    colors[ImPlotCol_AxisActive]    = IMPLOT_AUTO_COL; // TODO
    colors[ImPlotCol_Selection]     = ImVec4(0.82f, 0.64f, 0.03f, 1.00f);
    colors[ImPlotCol_Crosshairs]    = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
}

//-----------------------------------------------------------------------------
// [SECTION] Obsolete Functions/Types
//-----------------------------------------------------------------------------

#ifndef IMPLOT_DISABLE_OBSOLETE_FUNCTIONS

bool BeginPlot(const char* title, const char* x_label, const char* y1_label, const ImVec2& size,
               ImPlotFlags flags, ImPlotAxisFlags x_flags, ImPlotAxisFlags y1_flags, ImPlotAxisFlags y2_flags, ImPlotAxisFlags y3_flags,
               const char* y2_label, const char* y3_label)
{
    IM_ASSERT_USER_ERROR(GImPlot != NULL, "No current context. Did you call ImPlot::CreateContext() or ImPlot::SetCurrentContext()?");
    IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot == NULL, "Mismatched BeginPlot()/EndPlot()!");

    if (!BeginPlot(title, size, flags))
        return false;

    SetupAxis(ImAxis_X1, x_label, x_flags);
    SetupAxis(ImAxis_Y1, y1_label, y1_flags);
    if (ImHasFlag(flags, ImPlotFlags_YAxis2))
        SetupAxis(ImAxis_Y2, y2_label, y2_flags);
    if (ImHasFlag(flags, ImPlotFlags_YAxis3))
        SetupAxis(ImAxis_Y3, y3_label, y3_flags);

    // SetupFinish();

    return true;
}

void SetNextPlotLimits(double x_min, double x_max, double y_min, double y_max, ImGuiCond cond) {
    IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot == NULL, "SetNextPlotLimits() needs to be called before BeginPlot()!");
    SetNextPlotLimitsX(x_min, x_max, cond);
    SetNextPlotLimitsY(y_min, y_max, cond);
}

void SetNextPlotLimitsX(double x_min, double x_max, ImGuiCond cond) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot == NULL, "SetNextPlotLSetNextPlotLimitsXimitsY() needs to be called before BeginPlot()!");
    IM_ASSERT(cond == 0 || ImIsPowerOfTwo(cond)); // Make sure the user doesn't attempt to combine multiple condition flags.
    gp.NextPlotData.HasRange[ImAxis_X1]  = true;
    gp.NextPlotData.RangeCond[ImAxis_X1] = cond;
    gp.NextPlotData.Range[ImAxis_X1].Min = x_min;
    gp.NextPlotData.Range[ImAxis_X1].Max = x_max;
}

void SetNextPlotLimitsY(double y_min, double y_max, ImGuiCond cond, ImAxis y_axis) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot == NULL, "SetNextPlotLimitsY() needs to be called before BeginPlot()!");
    IM_ASSERT_USER_ERROR(y_axis >= ImAxis_Y1 && y_axis < ImAxis_COUNT, "y_axis out of range!");
    IM_ASSERT(cond == 0 || ImIsPowerOfTwo(cond)); // Make sure the user doesn't attempt to combine multiple condition flags.
    gp.NextPlotData.HasRange[y_axis] = true;
    gp.NextPlotData.RangeCond[y_axis] = cond;
    gp.NextPlotData.Range[y_axis].Min = y_min;
    gp.NextPlotData.Range[y_axis].Max = y_max;
}

void LinkNextPlotLimits(double* xmin, double* xmax, double* ymin, double* ymax, double* ymin2, double* ymax2, double* ymin3, double* ymax3) {
    ImPlotContext& gp = *GImPlot;
    gp.NextPlotData.LinkedMin[ImAxis_X1] = xmin;
    gp.NextPlotData.LinkedMax[ImAxis_X1] = xmax;
    gp.NextPlotData.LinkedMin[ImAxis_Y1] = ymin;
    gp.NextPlotData.LinkedMax[ImAxis_Y1] = ymax;
    gp.NextPlotData.LinkedMin[ImAxis_Y2] = ymin2;
    gp.NextPlotData.LinkedMax[ImAxis_Y2] = ymax2;
    gp.NextPlotData.LinkedMin[ImAxis_Y3] = ymin3;
    gp.NextPlotData.LinkedMax[ImAxis_Y3] = ymax3;
}

void FitNextPlotAxes(bool x, bool y, bool y2, bool y3) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot == NULL, "FitNextPlotAxes() needs to be called before BeginPlot()!");
    gp.NextPlotData.Fit[ImAxis_X1] = x;
    gp.NextPlotData.Fit[ImAxis_Y1] = y;
    gp.NextPlotData.Fit[ImAxis_Y2] = y2;
    gp.NextPlotData.Fit[ImAxis_Y3] = y3;
}

void SetNextPlotTicksX(const double* values, int n_ticks, const char* const labels[], bool show_default) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot == NULL, "SetNextPlotTicksX() needs to be called before BeginPlot()!");
    gp.NextPlotData.ShowDefaultTicks[ImAxis_X1] = show_default;
    AddTicksCustom(values, labels, n_ticks, gp.XTicks[0], DefaultFormatter, gp.NextPlotData.HasFmt[ImAxis_X1] ? gp.NextPlotData.Fmt[ImAxis_X1] : IMPLOT_LABEL_FORMAT);
}

void SetNextPlotTicksX(double x_min, double x_max, int n_ticks, const char* const labels[], bool show_default) {
    IM_ASSERT_USER_ERROR(n_ticks > 1, "The number of ticks must be greater than 1");
    static ImVector<double> buffer;
    FillRange(buffer, n_ticks, x_min, x_max);
    SetNextPlotTicksX(&buffer[0], n_ticks, labels, show_default);
}

void SetNextPlotTicksY(const double* values, int n_ticks, const char* const labels[], bool show_default, ImAxis y_axis) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot == NULL, "SetNextPlotTicksY() needs to be called before BeginPlot()!");
    IM_ASSERT_USER_ERROR(y_axis >= ImAxis_Y1 && y_axis < ImAxis_COUNT, "y_axis out of range!");
    gp.NextPlotData.ShowDefaultTicks[y_axis] = show_default;
    AddTicksCustom(values, labels, n_ticks, gp.YTicks[y_axis%IMPLOT_MAX_AXES], DefaultFormatter, gp.NextPlotData.HasFmt[y_axis] ? gp.NextPlotData.Fmt[y_axis] : IMPLOT_LABEL_FORMAT);
}

void SetNextPlotTicksY(double y_min, double y_max, int n_ticks, const char* const labels[], bool show_default, ImAxis y_axis) {
    IM_ASSERT_USER_ERROR(n_ticks > 1, "The number of ticks must be greater than 1");
    static ImVector<double> buffer;
    FillRange(buffer, n_ticks, y_min, y_max);
    SetNextPlotTicksY(&buffer[0], n_ticks, labels, show_default,y_axis);
}

void SetNextPlotFormatX(const char* fmt){
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot == NULL, "SetNextPlotFormatX() needs to be called before BeginPlot()!");
    gp.NextPlotData.HasFmt[ImAxis_X1] = true;
    ImStrncpy(gp.NextPlotData.Fmt[ImAxis_X1], fmt, 16);
}

void SetNextPlotFormatY(const char* fmt, ImAxis y_axis) {
    ImPlotContext& gp = *GImPlot;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot == NULL, "SetNextPlotFormatY() needs to be called before BeginPlot()!");
    IM_ASSERT_USER_ERROR(y_axis >= ImAxis_Y1 && y_axis < ImAxis_COUNT, "y_axis out of range!");
    gp.NextPlotData.HasFmt[y_axis] = true;
    ImStrncpy(gp.NextPlotData.Fmt[y_axis], fmt, 16);
}

#endif

}  // namespace ImPlot
