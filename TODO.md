The list below represents a combination of high-priority work, nice-to-have features, and random ideas. We make no guarantees that all of this work will be completed or even started. If you see something that you need or would like to have, let us know, or better yet consider submitting a PR for the feature.

## Plots

- remove axis-related args from signature of `BeginPlot` and add `SetupNextAxis` API
    - add a few overloads of `BeginPlot` that bypass `SetupNextAxis` for common scenarios
    - make current `BeginPlot` a wrapper to this API

## Axes

- add support for multiple x-axes and don't limit count to 3
    - will require `SetupNextAxis` API
- make axis side configurable (top/left, right/bottom) via new flag `ImPlotAxisFlags_Opposite`
- add support for setting tick label strings via callback
- add flag to remove weekends on Time axis

## Plot Items

- add `ImPlotLineFlags`, `ImPlotBarsFlags`, etc. for each plot type
- add `PlotBarGroups` wrapper that makes rendering groups of bars easier
- add non-zero references for `PlotBars` etc.


## Styling

- support gradient and/or colormap sampled fills (e.g. ImPlotFillStyle_)
- add hover/active color for plot

## Legend

- change `SetLegendLocation` API to be more consistent, i.e. `SetNextLegendLocation`
- add legend scroll
- improve legend icons (e.g. adopt markers, gradients, etc)
- `ImPlotLegendFlags`

## Tools / Misc.

- add `IsPlotChanging` to detect change in limits
- add ability to extend plot/axis context menus
- add LTTB downsampling for lines

## Optimizations

- find faster way to buffer data into ImDrawList (very slow)
- reduce number of calls to `PushClipRect`
- explore SIMD operations for high density plot items
