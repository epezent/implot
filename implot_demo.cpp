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

// ImPlot v0.1 WIP

#include <implot.h>
#include <math.h>

namespace {

float RandomRange( float min, float max ) {
    float scale = rand() / (float) RAND_MAX; 
    return min + scale * ( max - min );
}

struct ScrollingData {
    int MaxSize = 1000;
    int Offset  = 0;
    ImVector<ImVec2> Data;
    ScrollingData() { Data.reserve(MaxSize); }
    void AddPoint(float x, float y) {
        if (Data.size() < MaxSize)
            Data.push_back(ImVec2(x,y));
        else {
            Data[Offset] = ImVec2(x,y);
            Offset =  (Offset + 1) % MaxSize;
        }
    }
};

struct RollingData {
    float Span = 10.0f;
    ImVector<ImVec2> Data;
    RollingData() { Data.reserve(1000); }
    void AddPoint(float x, float y) {
        float xmod = fmodf(x, Span);
        if (!Data.empty() && xmod < Data.back().x)
            Data.shrink(0);
        Data.push_back(ImVec2(xmod, y));
    }
};

struct BenchmarkItem {
    BenchmarkItem() {
        float y = RandomRange(0,1);
        Xs = new float[1000];
        Ys = new float[1000];
        for (int i = 0; i < 1000; ++i) {
            Xs[i] = i*0.001f;
            Ys[i] = y + RandomRange(-0.01f,0.01f);
        }
        Col = ImVec4(RandomRange(0,1),RandomRange(0,1),RandomRange(0,1),1);
    }
    ~BenchmarkItem() { delete Xs; delete Ys; }
    float* Xs;
    float* Ys;
    ImVec4 Col;
};

}

namespace ImGui {   
    
void ShowImPlotDemoWindow(bool* p_open) {

    ImVec2 main_viewport_pos = ImGui::GetMainViewport()->Pos;
    ImGui::SetNextWindowPos(ImVec2(main_viewport_pos.x + 650, main_viewport_pos.y + 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
    ImGui::Begin("ImPlot Demo", p_open);
    ImGui::Text("ImPlot says hello. (0.1 WIP)");
    if (ImGui::CollapsingHeader("Help")) {
        ImGui::Text("USER GUIDE:");
        ImGui::BulletText("Left click and drag within the plot area to pan X and Y axes.");
        ImGui::Indent();
            ImGui::BulletText("Left click and drag on an axis to pan an individual axis.");
        ImGui::Unindent();
        ImGui::BulletText("Scroll in the plot area to zoom both X any Y axes.");
        ImGui::Indent();
            ImGui::BulletText("Scroll on an axis to zoom an individual axis.");
        ImGui::Unindent();
        ImGui::BulletText("Right click and drag to box select data.");
        ImGui::Indent();
            ImGui::BulletText("Hold Alt to expand box selection horizontally.");
            ImGui::BulletText("Hold Shift to expand box selection vertically.");
            ImGui::BulletText("Left click while box selecting to cancel the selection.");
        ImGui::Unindent();
        ImGui::BulletText("Middle click (or Ctrl + right click) and drag to create a query range.");
        ImGui::Indent();
            ImGui::BulletText("Hold Alt to expand query horizontally.");
            ImGui::BulletText("Hold Shift to expand query vertically.");
        ImGui::Unindent();
        ImGui::BulletText("Double left click to fit all visible data.");
        ImGui::Indent();
            ImGui::BulletText("Double left click on an axis to fit the individual axis.");
        ImGui::Unindent();
        ImGui::BulletText("Double right click to open the plot context menu.");
        ImGui::BulletText("Click legend label icons to show/hide plot items.");
    }
    //-------------------------------------------------------------------------
    if (ImGui::CollapsingHeader("Line Plots")) {
        static float xs1[1001], ys1[1001];
        for (int i = 0; i < 1001; ++i) {
            xs1[i] = i * 0.001f;
            ys1[i] = 0.5f + 0.5f * sin(50 * xs1[i]);
        }
        static float xs2[11], ys2[11];
        for (int i = 0; i < 11; ++i) {
            xs2[i] = i * 0.1f;
            ys2[i] = xs2[i] * xs2[i];
        }
        if (ImGui::BeginPlot("Line Plot", "x", "f(x)", {-1,300})) {
            ImGui::Plot("sin(50*x)", xs1, ys1, 1001);
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Circle);
            ImGui::Plot("x^2", xs2, ys2, 11);
            ImGui::PopPlotStyleVar();
            ImGui::EndPlot();
        }
    }
    //-------------------------------------------------------------------------
    if (ImGui::CollapsingHeader("Scatter Plots")) {
        srand(0);
        static float xs1[100], ys1[100];
        for (int i = 0; i < 100; ++i) {
            xs1[i] = i * 0.01f;
            ys1[i] = xs1[i] + 0.1f * ((float)rand() / (float)RAND_MAX);
        }
        static float xs2[50], ys2[50];
        for (int i = 0; i < 50; i++) {
            xs2[i] = 0.25f + 0.2f * ((float)rand() / (float)RAND_MAX);
            ys2[i] = 0.75f + 0.2f * ((float)rand() / (float)RAND_MAX);
        }
        if (ImGui::BeginPlot("Scatter Plot", NULL, NULL, {-1,300})) {
            ImGui::PushPlotStyleVar(ImPlotStyleVar_LineWeight, 0);
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Cross);            
            ImGui::PushPlotStyleVar(ImPlotStyleVar_MarkerSize, 3);
            ImGui::Plot("Data 1", xs1, ys1, 100);
            ImGui::PopPlotStyleVar(2);
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Circle);
            ImGui::PushPlotColor(ImPlotCol_MarkerFill, ImVec4{1,0,0,0.25f});
            ImGui::Plot("Data 2", xs2, ys2, 50);
            ImGui::PopPlotColor();
            ImGui::PopPlotStyleVar(2);
            ImGui::EndPlot();
        }
    }
    //-------------------------------------------------------------------------
    if (ImGui::CollapsingHeader("Bar Plots")) {
        static bool horz = false;
        ImGui::Checkbox("Horizontal",&horz);
        if (horz)
            ImGui::SetNextPlotRange(0, 110, -0.5f, 9.5f, ImGuiCond_Always);
        else
            ImGui::SetNextPlotRange(-0.5f, 9.5f, 0, 110, ImGuiCond_Always);
        if (ImGui::BeginPlot("Bar Plot", horz ? "Score":  "Student", horz ? "Student" : "Score", {-1, 300})) {
            static float midtm[10] = {83, 67, 23, 89, 83, 78, 91, 82, 85, 90};
            static float final[10] = {80, 62, 56, 99, 55, 78, 88, 78, 90, 100};
            static float grade[10] = {80, 69, 52, 92, 72, 78, 75, 76, 89, 95};
            if (horz) {
                ImGui::PlotBarH("Midterm Exam", midtm, 10, 0.2f, -0.2f);
                ImGui::PlotBarH("Final Exam", final, 10, 0.2f,  0);
                ImGui::PlotBarH("Course Grade", grade, 10, 0.2f, 0.2f);
            }
            else {
                ImGui::PlotBar("Midterm Exam", midtm, 10, 0.2f, -0.2f);
                ImGui::PlotBar("Final Exam", final, 10, 0.2f,  0);
                ImGui::PlotBar("Course Grade", grade, 10, 0.2f, 0.2f);
            }
            ImGui::EndPlot();
        }
    }
    //-------------------------------------------------------------------------
    if (ImGui::CollapsingHeader("Error Bars")) {
        float xs[5]  = {1,2,3,4,5};
        float lin[5] = {8,8,9,7,8};
        float bar[5] = {1,2,5,3,4};
        float err1[5] = {0.2, 0.4, 0.2, 0.6, 0.4};
        float err2[5] = {0.4, 0.2, 0.4, 0.8, 0.6};
        ImGui::SetNextPlotRange(0, 6, 0, 10);
        if (ImGui::BeginPlot("##ErrorBars",NULL,NULL,ImVec2(-1,300))) {

            ImGui::PlotBar("Bar", xs, bar, 5, 0.5f);
            ImGui::PlotErrorBars("Bar", xs, bar, err1, 5);

            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Circle);
            ImGui::PushPlotStyleVar(ImPlotStyleVar_MarkerSize, 3);
            ImGui::PushPlotColor(ImPlotCol_ErrorBar, ImVec4(1,0,0,1));
            ImGui::PlotErrorBars("Line", xs, lin, err1, err2, 5);
            ImGui::Plot("Line", xs, lin, 5);
            ImGui::PopPlotStyleVar(2);
            ImGui::PopPlotColor();

            ImGui::EndPlot();
        }
    }
    //-------------------------------------------------------------------------
    if (ImGui::CollapsingHeader("Realtime Plots")) {
        ImGui::BulletText("Move your mouse to change the data!");
        ImGui::BulletText("This example assumes 60 FPS. Higher FPS requires larger buffer size.");
        static bool paused = false;
        static ScrollingData sdata1, sdata2;
        static RollingData   rdata1, rdata2;
        ImVec2 mouse = ImGui::GetMousePos();
        static float t = 0;
        if (!paused) {
            t += ImGui::GetIO().DeltaTime;
            sdata1.AddPoint(t, mouse.x * 0.0005f);
            rdata1.AddPoint(t, mouse.x * 0.0005f);
            sdata2.AddPoint(t, mouse.y * 0.0005f);
            rdata2.AddPoint(t, mouse.y * 0.0005f);
        }
        ImGui::SetNextPlotRangeX(t - 10, t, paused ? ImGuiCond_Once : ImGuiCond_Always);
        static int rt_axis = ImAxisFlags_Default & ~ImAxisFlags_TickLabels;
        if (ImGui::BeginPlot("##Scrolling", NULL, NULL, {-1,150}, ImPlotFlags_Default, rt_axis, rt_axis)) {
            ImGui::Plot("Data 1", &sdata1.Data[0].x, &sdata1.Data[0].y, sdata1.Data.size(), sdata1.Offset, 2 * sizeof(float));
            ImGui::Plot("Data 2", &sdata2.Data[0].x, &sdata2.Data[0].y, sdata2.Data.size(), sdata2.Offset, 2 * sizeof(float));
            ImGui::EndPlot();
        }
        ImGui::SetNextPlotRangeX(0, 10, ImGuiCond_Always);
        if (ImGui::BeginPlot("##Rolling", NULL, NULL, {-1,150}, ImPlotFlags_Default, rt_axis, rt_axis)) {
            ImGui::Plot("Data 1", &rdata1.Data[0].x, &rdata1.Data[0].y, rdata1.Data.size(), 0, 2 * sizeof(float));
            ImGui::Plot("Data 2", &rdata2.Data[0].x, &rdata2.Data[0].y, rdata2.Data.size(), 0, 2 * sizeof(float));
            ImGui::EndPlot();
        }
    }

    //-------------------------------------------------------------------------
    if (ImGui::CollapsingHeader("Markers and Labels")) {
        ImGui::SetNextPlotRange(0, 10, 0, 12);
        if (ImGui::BeginPlot("##MarkerStyles", NULL, NULL, ImVec2(-1,300), 0, 0, 0)) {
            float xs[2] = {1,4};
            float ys[2] = {10,11};
            // filled
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Circle);
            ImGui::Plot("Circle##Fill", xs, ys, 2);   
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Square);   ys[0]--; ys[1]--;
            ImGui::Plot("Square##Fill", xs, ys, 2);   
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Diamond);  ys[0]--; ys[1]--;
            ImGui::Plot("Diamond##Fill", xs, ys, 2);   
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Up);       ys[0]--; ys[1]--;
            ImGui::Plot("Up##Fill", xs, ys, 2);   
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Down);     ys[0]--; ys[1]--;
            ImGui::Plot("Down##Fill", xs, ys, 2);   
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Left);     ys[0]--; ys[1]--;
            ImGui::Plot("Left##Fill", xs, ys, 2);   
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Right);    ys[0]--; ys[1]--;
            ImGui::Plot("Right##Fill", xs, ys, 2);   
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Cross);    ys[0]--; ys[1]--;
            ImGui::Plot("Cross##Fill", xs, ys, 2);   
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Plus);     ys[0]--; ys[1]--;
            ImGui::Plot("Plus##Fill", xs, ys, 2);   
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Asterisk); ys[0]--; ys[1]--;
            ImGui::Plot("Asterisk##Fill", xs, ys, 2);   
            ImGui::PopPlotStyleVar(10);

            xs[0] = 6; xs[1] = 9;
            ys[0] = 10; ys[1] = 11;
            ImGui::PushPlotColor(ImPlotCol_MarkerFill, ImVec4(0,0,0,0));
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Circle);
            ImGui::Plot("Circle", xs, ys, 2);   
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Square);   ys[0]--; ys[1]--;
            ImGui::Plot("Square", xs, ys, 2);   
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Diamond);  ys[0]--; ys[1]--;
            ImGui::Plot("Diamond", xs, ys, 2);   
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Up);       ys[0]--; ys[1]--;
            ImGui::Plot("Up", xs, ys, 2);   
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Down);     ys[0]--; ys[1]--;
            ImGui::Plot("Down", xs, ys, 2);   
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Left);     ys[0]--; ys[1]--;
            ImGui::Plot("Left", xs, ys, 2);   
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Right);    ys[0]--; ys[1]--;
            ImGui::Plot("Right", xs, ys, 2);   
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Cross);    ys[0]--; ys[1]--;
            ImGui::Plot("Cross", xs, ys, 2);   
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Plus);     ys[0]--; ys[1]--;
            ImGui::Plot("Plus", xs, ys, 2);   
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Asterisk); ys[0]--; ys[1]--;
            ImGui::Plot("Asterisk", xs, ys, 2);   
            ImGui::PopPlotColor();
            ImGui::PopPlotStyleVar(10);

            xs[0] = 5; xs[1] = 5;
            ys[0] = 1; ys[1] = 11;

            ImGui::PushPlotStyleVar(ImPlotStyleVar_LineWeight, 2);
            ImGui::PushPlotStyleVar(ImPlotStyleVar_MarkerSize, 8);
            ImGui::PushPlotStyleVar(ImPlotStyleVar_MarkerWeight, 2);
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Circle | ImMarker_Cross); 
            ImGui::PushPlotColor(ImPlotCol_MarkerOutline, ImVec4(0,0,0,1));
            ImGui::PushPlotColor(ImPlotCol_MarkerFill, ImVec4(1,1,1,1));
            ImGui::PushPlotColor(ImPlotCol_Line, ImVec4(0,0,0,1));
            ImGui::Plot("Circle|Cross", xs, ys, 2);  
            ImGui::PopPlotStyleVar(4);  
            ImGui::PopPlotColor(3);

            ImGui::PlotLabel("Filled Markers", 1.5, 11.75);
            ImGui::PlotLabel("Open Markers", 6.75, 11.75);
            ImGui::PlotLabel("Fancy Markers", 4.5, 4.25, true);

            ImGui::EndPlot();
        }
    }
    //-------------------------------------------------------------------------
    if (ImGui::CollapsingHeader("Log Scale")) {
        ImGui::BulletText("Open the plot context menu (double right click) to change scales.");
        static float xs[1001], ys1[1001], ys2[1001], ys3[1001];
        for (int i = 0; i < 1001; ++i) {
            xs[i] = (float)(i*0.1f);
            ys1[i] = sin(xs[i]) + 1;
            ys2[i] = log(xs[i]);
            ys3[i] = pow(10.0f, xs[i]);
        }
        ImGui::SetNextPlotRange(0.1f, 100, 0, 10);
        if (ImGui::BeginPlot("Log Plot", NULL, NULL, ImVec2(-1,300), ImPlotFlags_Default, ImAxisFlags_Default | ImAxisFlags_LogScale )) {
            ImGui::Plot("f(x) = x",      xs, xs,  1001);
            ImGui::Plot("f(x) = sin(x)+1", xs, ys1, 1001);
            ImGui::Plot("f(x) = log(x)", xs, ys2, 1001);
            ImGui::Plot("f(x) = 10^x",   xs, ys3, 21);
            ImGui::EndPlot();
        }
    }
    //-------------------------------------------------------------------------
    if (ImGui::CollapsingHeader("Querying")) {
        ImGui::BulletText("Ctrl + click in the plot area to draw points.");
        ImGui::BulletText("Middle click (or Ctrl + right click) and drag to query points.");
        ImGui::BulletText("Hold the Alt and/or Shift keys to expand the query range.");
        static ImVector<ImVec2> data;
        ImPlotRange range, query;
        if (ImGui::BeginPlot("##Drawing", NULL, NULL, ImVec2(-1,300), ImPlotFlags_Default, ImAxisFlags_GridLines, ImAxisFlags_GridLines)) {
            if (ImGui::IsPlotHovered() && ImGui::IsMouseClicked(0) && ImGui::GetIO().KeyCtrl) 
                data.push_back(ImGui::GetPlotMousePos());
            ImGui::PushPlotStyleVar(ImPlotStyleVar_LineWeight, 0);
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Diamond);
            if (data.size() > 0)
                ImGui::Plot("Points", &data[0].x, &data[0].y, data.size(), 0, 2 * sizeof(float));
            if (ImGui::IsPlotQueried() && data.size() > 0) {
                ImPlotRange range = ImGui::GetPlotQuery();
                int cnt = 0;
                ImVec2 avg;
                for (int i = 0; i < data.size(); ++i) {
                    if (range.Contains(data[i])) {
                        avg.x += data[i].x;
                        avg.y += data[i].y;
                        cnt++;
                    } 
                }
                if (cnt > 0) {
                    avg.x = avg.x / cnt;
                    avg.y = avg.y / cnt;
                    ImGui::Plot("Average", &avg.x, &avg.y, 1);
                }
            }
            ImGui::PopPlotStyleVar(2);
            range = ImGui::GetPlotRange();
            query = ImGui::GetPlotQuery();
            ImGui::EndPlot();
        }
        ImGui::Text("The current plot range is:  [%g,%g,%g,%g]", range.XMin, range.XMax, range.YMin, range.YMax);
        ImGui::Text("The current query range is: [%g,%g,%g,%g]", query.XMin, query.XMax, query.YMin, query.YMax);
    }
    //-------------------------------------------------------------------------
    if (ImGui::CollapsingHeader("Views")) {
        // mimic's soulthread's imgui_plot demo
        static float x_data[512];
        static float y_data1[512];
        static float y_data2[512];
        static float y_data3[512];
        static float sampling_freq = 44100;
        static float freq = 500;
        for (size_t i = 0; i < 512; ++i) {
            const float t = i / sampling_freq;
            x_data[i] = t;
            const float arg = 2 * 3.14 * freq * t;
            y_data1[i] = sin(arg);
            y_data2[i] = y_data1[i] * -0.6 + sin(2 * arg) * 0.4;
            y_data3[i] = y_data2[i] * -0.6 + sin(3 * arg) * 0.4;
        }
        ImGui::BulletText("Query the first plot to render a subview in the second plot.");
        ImGui::BulletText("Toggle \"Pixel Query\" in the context menu and then pan the plot.");
        ImGui::SetNextPlotRange(0,0.01f,-1,1);
        ImAxisFlags flgs = ImAxisFlags_Default & ~ImAxisFlags_TickLabels;
        ImPlotRange query;
        if (ImGui::BeginPlot("##View1",NULL,NULL,ImVec2(-1,150), ImPlotFlags_Default, flgs, flgs)) {
            ImGui::Plot("Signal 1", x_data, y_data1, 512);
            ImGui::Plot("Signal 2", x_data, y_data2, 512);
            ImGui::Plot("Signal 3", x_data, y_data3, 512);
            query = ImGui::GetPlotQuery();
            ImGui::EndPlot();
        }
        ImGui::SetNextPlotRange(query.XMin, query.XMax, query.YMin, query.YMax, ImGuiCond_Always);
        if (ImGui::BeginPlot("##View2",NULL,NULL,ImVec2(-1,150), 0, 0, 0)) {
            ImGui::Plot("Signal 1", x_data, y_data1, 512);
            ImGui::Plot("Signal 2", x_data, y_data2, 512);
            ImGui::Plot("Signal 3", x_data, y_data3, 512);
            ImGui::EndPlot();
        }
    }



    //-------------------------------------------------------------------------
    if (ImGui::CollapsingHeader("Drag and Drop")) {
        srand(10000000 * ImGui::GetTime());
        static bool paused = false;
        static bool init = true;
        static ScrollingData data[10];
        static bool show[10];
        if (init) {
            for (int i = 0; i < 10; ++i) {
                show[i] = false;
            }
            init = false;
        }
        ImGui::BulletText("Drag data items from the left column onto the plot.");
        ImGui::BeginGroup();
        if (ImGui::Button("Clear", {100, 0})) {
            for (int i = 0; i < 10; ++i) {
                show[i] = false;
                data[i].Data.shrink(0);
                data[i].Offset = 0;
            }
        }
        if (ImGui::Button(paused ? "Resume" : "Pause", {100,0}))
            paused = !paused;
        ImGui::Separator();
        for (int i = 0; i < 10; ++i) {
            char label[8];
            sprintf(label, "data_%d", i);
            ImGui::Selectable(label, false, 0, {100, 0});
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                ImGui::SetDragDropPayload("DND_PLOT", &i, sizeof(int));
                ImGui::TextUnformatted(label);
                ImGui::EndDragDropSource();
            }
        }
        ImGui::EndGroup();
        ImGui::SameLine();
        static float t = 0;
        if (!paused) {
            t += ImGui::GetIO().DeltaTime;
            for (int i = 0; i < 10; ++i) {
                if (show[i])
                    data[i].AddPoint(t, data[i].Data.empty() ?  
                                        0.25f + 0.5f * (float)rand() / float(RAND_MAX) :                    
                                        data[i].Data.back().y + (0.005f + 0.0002f * (float)rand() / float(RAND_MAX)) * (-1 + 2 * (float)rand() / float(RAND_MAX)));
            }
        }
        ImGui::SetNextPlotRangeX(t - 10, t, paused ? ImGuiCond_Once : ImGuiCond_Always);
        if (ImGui::BeginPlot("##DND", NULL, NULL, ImVec2(-1,300), ImPlotFlags_Default)) {
            for (int i = 0; i < 10; ++i) {
                if (show[i]) {
                    char label[8];
                    sprintf(label, "data_%d", i);
                    ImGui::Plot(label, &data[i].Data[0].x, &data[i].Data[0].y, data[i].Data.size(), data[i].Offset, 2 * sizeof(float));
                }
            }
            ImGui::EndPlot();
        }
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_PLOT")) {
                int i = *(int*)payload->Data;
                show[i] = true;
            }
            ImGui::EndDragDropTarget();
        }
    }
    //-------------------------------------------------------------------------
    if (ImGui::CollapsingHeader("Custom Styles")) {
        static ImVec4 my_palette[3] = {
            {0.000f, 0.980f, 0.604f, 1.0f},
            {0.996f, 0.278f, 0.380f, 1.0f},
            {(0.1176470593F), (0.5647059083F), (1.0F), (1.0F)},
        };
        ImGui::SetPlotPalette(my_palette, 3);
        ImGui::PushPlotColor(ImPlotCol_FrameBg, IM_COL32(32,51,77,255));
        ImGui::PushPlotColor(ImPlotCol_PlotBg, {0,0,0,0});
        ImGui::PushPlotColor(ImPlotCol_PlotBorder, {0,0,0,0});
        ImGui::PushPlotColor(ImPlotCol_XAxis, IM_COL32(192, 192, 192, 192));
        ImGui::PushPlotColor(ImPlotCol_YAxis, IM_COL32(192, 192, 192, 192));
        ImGui::PushPlotStyleVar(ImPlotStyleVar_LineWeight, 2);
        ImGui::SetNextPlotRange(-0.5f, 9.5f, -0.5f, 9.5f);
        if (ImGui::BeginPlot("##Custom", NULL, NULL, {-1,300}, ImPlotFlags_Default & ~ImPlotFlags_Legend, 0)) {
            float lin[10] = {8,8,9,7,8,8,8,9,7,8};
            float bar[10] = {1,2,5,3,4,1,2,5,3,4};       
            float dot[10] = {7,6,6,7,8,5,6,5,8,7}; 
            ImGui::PlotBar("Bar", bar, 10, 0.5f);
            ImGui::Plot("Line", lin, 10);
            ImGui::PushPlotStyleVar(ImPlotStyleVar_LineWeight, 0);
            ImGui::PushPlotStyleVar(ImPlotStyleVar_Marker, ImMarker_Square);
            ImGui::Plot("Dot", dot, 10);
            ImGui::PopPlotStyleVar(2);
            ImGui::EndPlot();
        }
        ImGui::PopPlotColor(5);
        ImGui::PopPlotStyleVar();
        ImGui::RestorePlotPalette();
    }
    if (ImGui::CollapsingHeader("Custom Rendering")) {
        if (ImGui::BeginPlot("##CustomRend",NULL,NULL,{-1,300})) {
            ImVec2 cntr = ImGui::PlotToPixels({0.5f,  0.5f});
            ImVec2 rmin = ImGui::PlotToPixels({0.25f, 0.75f});
            ImVec2 rmax = ImGui::PlotToPixels({0.75f, 0.25f});
            ImGui::PushPlotClipRect();
            ImGui::GetWindowDrawList()->AddCircleFilled(cntr,20,IM_COL32(255,255,0,255),20);
            ImGui::GetWindowDrawList()->AddRect(rmin, rmax, IM_COL32(128,0,255,255));
            ImGui::PopPlotClipRect();
            ImGui::EndPlot();
        }
    }
    //-------------------------------------------------------------------------
    if (ImGui::CollapsingHeader("Benchmark")) {
        static const int n_items = 100;
        static BenchmarkItem items[n_items];
        ImGui::BulletText("Make sure VSync is disabled.");
        ImGui::BulletText("%d lines with %d points each @ %.3f FPS.",n_items,1000,ImGui::GetIO().Framerate);
        if (ImGui::BeginPlot("##Bench",NULL,NULL,{-1,300})) {
            char buff[16];
            for (int i = 0; i < 100; ++i) {
                sprintf(buff, "item_%d",i);
                ImGui::PushPlotColor(ImPlotCol_Line, items[i].Col);
                ImGui::Plot(buff, items[i].Xs, items[i].Ys, 1000);
                ImGui::PopPlotColor();
            }   
            ImGui::EndPlot();
        }
    }
    //-------------------------------------------------------------------------
    ImGui::End();
    
}

} // namespace ImGui