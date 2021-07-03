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

// ImPlot v0.10 WIP

#pragma once

#if defined(IMPLOT_BACKEND_ENABLE_OPENGL3)
	#include "implot_impl_opengl3.h"
#elif defined(IMPLOT_BACKEND_ENABLE_METAL)
	#include "implot_impl_metal.h"
#endif

namespace ImPlot {
namespace Backend {

#ifndef IMPLOT_BACKEND_ENABLED
	inline void* CreateContext() { return nullptr; }
	inline void DestroyContext() {}

	inline void BustPlotCache() {}
	inline void BustItemCache() {}
#endif

#ifndef IMPLOT_BACKEND_HAS_COLORMAP
	inline void AddColormap(const ImU32*, int, bool) {}
#endif

}
}
