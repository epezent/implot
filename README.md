# ImPlot
ImPlot is an advanced 2D plotting widget for Dear ImGui.  

## Features

- mutliple plot types: line, scatter, virtical/horizontal bars, stem, error bars
- mix/match multiple plot items on a single plot
- several plot styling options: 10 marker types, adjustable marker sizes, line weights, outline colors, fill colors, etc.
- optional plot titles, axis labels, and grid labels
- legend with toggle buttons to show/hide items
- reversible and lockable axes
- logarithmic axis scaling
- size-aware, auto subdividing grid
- auto styling based on current theme, but most elements can be overridden
- nice grid labels that are always power-of-ten multiples of 1, 2, and 5
- mouse cursor location display and optional crosshairs mode
- relatively good performance for high density plots

## Controls
- scroll wheel zoom (both axes if plot area hovered, individual axes if axis labels hovered)
- panning/dragging (both axes if plot area dragged, individual axes if axis labels dragged)
- auto fit data (double-left-click plot area)
- selection box (right-drag in plot area)
- context menu (double-right-click plot area)

## Usage

```cpp
if (ImGui::BeginPlot("My Plot") {
    ImGui::Plot("Line Plot", xs ys, 1000);
    ImGui::EndPlot();
}
```

## Special Notes
- By default, no anti-aliasing is done on line plots for performance reasons. My apps use 4X MSAA, so I didn't see any reason to waste cycles on software AA. However, you can enable AA with the `ImPlotFlags_AntiAliased` flag.
- If you plan to render several thousands lines or points, then you should consider enabling 32-bit indices by uncommenting `#define ImDrawIdx unsigned int` in your `imconfig.h` file, OR handling the `ImGuiBackendFlags_RendererHasVtxOffset` flag in your renderer (the official OpenGL3 renderer supports this). If you fail to do this, then you may at some point hit the maximum number of indices that can be rendered.

## Known Issues (Fix Me!)

- Mouse scroll zooming on a plot that is in scrollable ImGui region will both zoom and scroll the window since there is no built in scroll capture for ImGui. The current workaround is to CTRL+Scroll the plot (this disables window scrolling). 
- Zooming to a range beyond the limits of `FLT_MAX` and `FLT_MIN` causes axes labels to disappear.
