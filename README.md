# ImPlot
ImPlot is an immediate mode plotting widget for [Dear ImGui](https://github.com/ocornut/imgui). It aims to provide a first-class API that will make ImGui users feel right at home. ImPlot is well suited for visualizing program data in real-time and requires minimal code to integrate. Like ImGui, it does not use any C++11 features, headers, or STL containers, and has no external dependencies except for ImGui itself. 

## Features

- multiple plot types: line, scatter, vertical/horizontal bars, error bars, with more likely to come
- mix/match multiple plot items on a single plot
- configurable axes ranges and scaling (linear/log)
- reversible and lockable axes
- controls for zooming, panning, box selection, and auto-fitting data
- several plot styling options: 10 marker types, adjustable marker sizes, line weights, outline colors, fill colors, etc.
- optional plot titles, axis labels, and grid labels
- optional legend with toggle buttons to quickly show/hide items
- size-aware grid with smart labels that are always power-of-ten multiples of 1, 2, and 5
- default styling based on current ImGui theme, but most elements can be customized independently 
- mouse cursor location display and optional crosshairs cursor
- customizable data getters and data striding (just like ImGui:PlotLines)
- relatively good performance for high density plots

## Usage

The API is used just like any other ImGui `Begin`/`End` function. First, start a plotting context with `BeginPlot()`. Next, plot as many items as you want with the provided API functions (e.g. `Plot()`, `PlotBar()`, `PlotErrorBars()`, etc). Finally, wrap things up with a call to `EndPlot()`. That's it! 

```cpp
if (ImGui::BeginPlot("My Plot") {
    ImGui::Plot("My Line Plot", xs ys, 1000);
    ImGui::PlotBar("My Bar Plot", values, 20);
    ...
    ImGui::EndPlot();
}
```

Consult `implot_demo.cpp` for a comprehensive example of ImPlot's features. 

## Integration

Just add `implot.h`, `implot.cpp`, and optionally `implot_demo.cpp` to your sources. This assumes you already have an ImGui-ready environment. If not, consider trying [mahi-gui], which bundles ImGui, ImPlot, and several other packages for you.

## Special Notes
- By default, no anti-aliasing is done on line plots for performance reasons. My apps use 4X MSAA, so I didn't see any reason to waste cycles on software AA. However, you can enable AA with the `ImPlotFlags_AntiAliased` flag.
- If you plan to render several thousands lines or points, then you should consider enabling 32-bit indices by uncommenting `#define ImDrawIdx unsigned int` in your `imconfig.h` file, OR handling the `ImGuiBackendFlags_RendererHasVtxOffset` flag in your renderer (the official OpenGL3 renderer supports this). If you fail to do this, then you may at some point hit the maximum number of indices that can be rendered.

## Known Issues (Fix Me!)

- Zooming to a range beyond the limits of `FLT_MAX` and `FLT_MIN` causes axes labels to disappear.

## Gallery

<img src="https://raw.githubusercontent.com/wiki/epezent/implot/screenshots/zoom_pan.gif" width="285"> <img src="https://raw.githubusercontent.com/wiki/epezent/implot/screenshots/controls.gif" width="285"> <img src="https://raw.githubusercontent.com/wiki/epezent/implot/screenshots/dnd.gif" width="285">

<img src="https://raw.githubusercontent.com/wiki/epezent/implot/screenshots/line_plot.png" width="285"> <img src="https://raw.githubusercontent.com/wiki/epezent/implot/screenshots/scatter_plot.png" width="285"> <img src="https://raw.githubusercontent.com/wiki/epezent/implot/screenshots/bar_plot.png" width="285"> 

<img src="https://raw.githubusercontent.com/wiki/epezent/implot/screenshots/log_plot.png" width="285"> <img src="https://raw.githubusercontent.com/wiki/epezent/implot/screenshots/error_bars.png" width="285"> <img src="https://raw.githubusercontent.com/wiki/epezent/implot/screenshots/markers.png" width="285"> 

