# ImPlot
Advanced 2D Plotting for Dear ImGui

# Features


# Known Issues (Fix Me!)

- Mouse scroll zooming on a plot that is in scrollable ImGui region will both zoom and scroll the window since there is no built in scroll capture for ImGui. The current workaround is to CTRL+Scroll the plot (this disables window scrolling). 
- Zooming to a range beyond the limits of `FLT_MAX` and `FLT_MIN` causes axes labels to disappear.
