#ifdef IMPLOT_ENABLE_OPENGL3_ACCELERATION

#include "../implot.h"
#include "../implot_internal.h"
#include "implot_gpu.h"

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h>            // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

namespace ImPlot {
namespace Backend {

struct Shader
{
	GLuint ID = 0;                              ///< Shader ID for the heatmap shader

	GLuint g_AttribLocationHeatmapSampler = 0;  ///< Attribute location for the heatmap texture
	GLuint g_AttribLocationColormapSampler = 0; ///< Attribute location for the colormap texture
	GLuint g_AttribLocationProjection = 0;      ///< Attribute location for the projection matrix uniform
	GLuint g_AttribLocationMinValue = 0;        ///< Attribute location for the minimum value uniform
	GLuint g_AttribLocationMaxValue = 0;        ///< Attribute location for the maximum value uniform
};

struct HeatmapData
{
	Shader* ShaderProgram;
	GLuint HeatmapTexID;
	GLuint ColormapTexID;
	float MinValue;
	float MaxValue;
};

struct ContextData
{
	Shader ShaderInt;                           ///< Shader for integer heatmaps
	Shader ShaderFloat;                         ///< Shader for floating-point heatmaps

	GLuint g_AttribLocationImGuiProjection = 0; ///< Attribute location for the projection matrix uniform (ImGui default shader)

	ImVector<HeatmapData> HeatmapDataList;      ///< Array of heatmap data
	ImVector<GLuint> ColormapIDs;               ///< Texture IDs of the colormap textures
	ImGuiStorage PlotIDs;                       ///< PlotID <-> Heatmap array index table

	ImVector<float> temp1;
	ImVector<ImS32> temp2;
	ImVector<ImU32> temp3;
};

void* CreateContext()
{
	return new ContextData;
}

void DestroyContext()
{
    ContextData& Context = *((ContextData*)GImPlot->backendCtx);

	for(const HeatmapData& data : Context.HeatmapDataList)
		glDeleteTextures(1, &data.HeatmapTexID);

	for(GLuint texID : Context.ColormapIDs)
		glDeleteTextures(1, &texID);

	glDeleteProgram(Context.ShaderInt.ID);
	glDeleteProgram(Context.ShaderFloat.ID);

	Context.HeatmapDataList.clear();
	Context.PlotIDs.Clear();
}

#define VERTEX_SHADER_CODE                                           \
	"#version 330 core\n"                                            \
	"precision mediump float;\n"                                     \
	"layout (location = %d) in vec2 Position;\n"                     \
	"layout (location = %d) in vec2 UV;\n"                           \
	"\n"                                                             \
	"uniform mat4 ProjMtx;\n"                                        \
	"out vec2 Frag_UV;\n"                                            \
	"\n"                                                             \
	"void main()\n"                                                  \
	"{\n"                                                            \
	"	Frag_UV = UV;\n"                                             \
	"	gl_Position = ProjMtx * vec4(Position.xy, 0.0f, 1.0f);\n"    \
	"}\n"

#define FRAGMENT_SHADER_CODE                                         \
	"#version 330 core\n"                                            \
	"precision mediump float;\n"                                     \
	"\n"                                                             \
	"in vec2 Frag_UV;\n"                                             \
	"out vec4 Out_Color;\n"                                          \
	"\n"                                                             \
	"uniform sampler1D colormap;\n"                                  \
	"uniform %csampler2D heatmap;\n"                                 \
	"uniform float min_val;\n"                                       \
	"uniform float max_val;\n"                                       \
	"\n"                                                             \
	"void main()\n"                                                  \
	"{\n"                                                            \
	"   float value = float(texture(heatmap, Frag_UV).r);\n"         \
	"	float offset = (value - min_val) / (max_val - min_val);\n"   \
	"	Out_Color = texture(colormap, clamp(offset, 0.0f, 1.0f));\n" \
	"}\n"

static void CompileShader(Shader& ShaderProgram, GLchar* VertexShaderCode, GLchar* FragmentShaderCode)
{
	GLuint g_VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(g_VertexShader, 1, &VertexShaderCode, nullptr);
	glCompileShader(g_VertexShader);

	GLuint g_FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(g_FragmentShader, 1, &FragmentShaderCode, nullptr);
	glCompileShader(g_FragmentShader);

	ShaderProgram.ID = glCreateProgram();
	glAttachShader(ShaderProgram.ID, g_VertexShader);
	glAttachShader(ShaderProgram.ID, g_FragmentShader);
	glLinkProgram(ShaderProgram.ID);

	glDetachShader(ShaderProgram.ID, g_VertexShader);
	glDetachShader(ShaderProgram.ID, g_FragmentShader);
	glDeleteShader(g_VertexShader);
	glDeleteShader(g_FragmentShader);

	ShaderProgram.g_AttribLocationHeatmapSampler  = glGetUniformLocation(ShaderProgram.ID, "heatmap");
	ShaderProgram.g_AttribLocationColormapSampler = glGetUniformLocation(ShaderProgram.ID, "colormap");
	ShaderProgram.g_AttribLocationProjection      = glGetUniformLocation(ShaderProgram.ID, "ProjMtx");
	ShaderProgram.g_AttribLocationMinValue        = glGetUniformLocation(ShaderProgram.ID, "min_val");
	ShaderProgram.g_AttribLocationMaxValue        = glGetUniformLocation(ShaderProgram.ID, "max_val");

	glUseProgram(ShaderProgram.ID);
	glUniform1i(ShaderProgram.g_AttribLocationHeatmapSampler, 0); // Set texture slot of heatmap texture
	glUniform1i(ShaderProgram.g_AttribLocationColormapSampler, 1); // Set texture slot of colormap texture
}

static void CreateShader(const ImDrawList*, const ImDrawCmd*)
{
    ContextData& Context = *((ContextData*)GImPlot->backendCtx);

	GLuint CurrentShader;
	glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&CurrentShader);

	Context.g_AttribLocationImGuiProjection = glGetUniformLocation(CurrentShader, "ProjMtx");

	GLuint g_AttribLocationVtxPos = (GLuint)glGetAttribLocation(CurrentShader, "Position");
	GLuint g_AttribLocationVtxUV  = (GLuint)glGetAttribLocation(CurrentShader, "UV");

	GLchar* VertexShaderCode = new GLchar[512];
	GLchar* FragmentShaderCode = new GLchar[512];

	snprintf(VertexShaderCode, 512, VERTEX_SHADER_CODE, g_AttribLocationVtxPos, g_AttribLocationVtxUV);
	snprintf(FragmentShaderCode, 512, FRAGMENT_SHADER_CODE, ' ');

	CompileShader(Context.ShaderFloat, VertexShaderCode, FragmentShaderCode);

	snprintf(VertexShaderCode, 512, VERTEX_SHADER_CODE, g_AttribLocationVtxPos, g_AttribLocationVtxUV);
	snprintf(FragmentShaderCode, 512, FRAGMENT_SHADER_CODE, 'i');

	CompileShader(Context.ShaderInt, VertexShaderCode, FragmentShaderCode);
	glUseProgram(0);

	delete[] VertexShaderCode;
	delete[] FragmentShaderCode;
}

static void RenderCallback(const ImDrawList*, const ImDrawCmd* cmd)
{
    ContextData& Context = *((ContextData*)GImPlot->backendCtx);

	int plotID = (int)(intptr_t)cmd->UserCallbackData;
	int plotIdx = Context.PlotIDs.GetInt(plotID, -1);
	HeatmapData& data = Context.HeatmapDataList[plotIdx];

	GLuint CurrentShader;
	glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&CurrentShader);

	// Get projection matrix of current shader
	float OrthoProjection[4][4];
	glGetUniformfv(CurrentShader, Context.g_AttribLocationImGuiProjection, &OrthoProjection[0][0]);

	// Enable our shader
	glUseProgram(data.ShaderProgram->ID);

	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, data.HeatmapTexID); // Set texture ID of data
	glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_1D, data.ColormapTexID); // Set texture ID of colormap

	glUniformMatrix4fv(data.ShaderProgram->g_AttribLocationProjection, 1, GL_FALSE, &OrthoProjection[0][0]);
	glUniform1f(data.ShaderProgram->g_AttribLocationMinValue, data.MinValue); // Set minimum range
	glUniform1f(data.ShaderProgram->g_AttribLocationMaxValue, data.MaxValue); // Set maximum range
}

static void UnbindTexture(const ImDrawList*, const ImDrawCmd*)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

static void SetTextureData(int plotID, const void* data, GLsizei rows, GLsizei cols, GLint internalFormat, GLenum format, GLenum type)
{
    ContextData& Context = *((ContextData*)GImPlot->backendCtx);

	int idx = Context.PlotIDs.GetInt(plotID, -1);
	GLuint texID = Context.HeatmapDataList[idx].HeatmapTexID;
	Context.HeatmapDataList[idx].ShaderProgram = (type == GL_FLOAT ? &Context.ShaderFloat : &Context.ShaderInt);

	// Set heatmap data
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, cols, rows, 0, format, type, data);
}

void AddColormap(const ImU32* keys, int count, bool qual)
{
	GLuint texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_1D, texID);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, count, 0, GL_RGBA, GL_UNSIGNED_BYTE, keys);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, qual ? GL_NEAREST : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, qual ? GL_NEAREST : GL_LINEAR);
	glBindTexture(GL_TEXTURE_1D, 0);

    ContextData& Context = *((ContextData*)GImPlot->backendCtx); // PETA AQUI (GImPlot es NULL)
	Context.ColormapIDs.push_back(texID);
}

static GLuint CreateHeatmapTexture()
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}

void SetHeatmapData(int plotID, const ImS8* values, int rows, int cols) { SetTextureData(plotID, values, rows, cols, GL_R8I, GL_RED_INTEGER, GL_BYTE); }
void SetHeatmapData(int plotID, const ImU8* values, int rows, int cols) { SetTextureData(plotID, values, rows, cols, GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE); }
void SetHeatmapData(int plotID, const ImS16* values, int rows, int cols) { SetTextureData(plotID, values, rows, cols, GL_R16I, GL_RED_INTEGER, GL_SHORT); }
void SetHeatmapData(int plotID, const ImU16* values, int rows, int cols) { SetTextureData(plotID, values, rows, cols, GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT); }
void SetHeatmapData(int plotID, const ImS32* values, int rows, int cols) { SetTextureData(plotID, values, rows, cols, GL_R32I, GL_RED_INTEGER, GL_INT); }
void SetHeatmapData(int plotID, const ImU32* values, int rows, int cols) { SetTextureData(plotID, values, rows, cols, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT); }
void SetHeatmapData(int plotID, const float* values, int rows, int cols) { SetTextureData(plotID, values, rows, cols, GL_R32F, GL_RED, GL_FLOAT); }

void SetHeatmapData(int plotID, const double* values, int rows, int cols)
{
    ContextData& Context = *((ContextData*)GImPlot->backendCtx);

	if(Context.temp1.Size < rows * cols)
		Context.temp1.resize(rows * cols);

	for(int i = 0; i < rows*cols; i++)
		Context.temp1[i] = (float)values[i];

	SetTextureData(plotID, Context.temp1.Data, rows, cols, GL_R32F, GL_RED, GL_FLOAT);
}

void SetHeatmapData(int plotID, const ImS64* values, int rows, int cols)
{
    ContextData& Context = *((ContextData*)GImPlot->backendCtx);

	if(Context.temp2.Size < rows * cols)
		Context.temp2.resize(rows * cols);

	for(int i = 0; i < rows*cols; i++)
		Context.temp2[i] = (ImS32)values[i];

	SetTextureData(plotID, Context.temp2.Data, rows, cols, GL_R32I, GL_RED_INTEGER, GL_INT);
}

void SetHeatmapData(int plotID, const ImU64* values, int rows, int cols)
{
    ContextData& Context = *((ContextData*)GImPlot->backendCtx);

	if(Context.temp3.Size < rows * cols)
		Context.temp3.resize(rows * cols);

	for(int i = 0; i < rows*cols; i++)
		Context.temp3[i] = (ImU32)values[i];

	SetTextureData(plotID, Context.temp3.Data, rows, cols, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT);
}

void RenderHeatmap(int plotID, ImDrawList& DrawList, const ImVec2& bounds_min, const ImVec2& bounds_max, float scale_min, float scale_max, ImPlotColormap colormap)
{
    ContextData& Context = *((ContextData*)GImPlot->backendCtx);
	int idx = Context.PlotIDs.GetInt(plotID, -1);

	if(idx < 0)
	{
		// New entry
		HeatmapData data;
		data.HeatmapTexID = CreateHeatmapTexture();
		data.ColormapTexID = Context.ColormapIDs[colormap];
		data.MinValue = scale_min;
		data.MaxValue = scale_max;

		Context.PlotIDs.SetInt(plotID, Context.HeatmapDataList.Size);
		Context.HeatmapDataList.push_back(data);
	}
	else
	{
		HeatmapData& data = Context.HeatmapDataList[idx];
		data.ColormapTexID = Context.ColormapIDs[colormap];
		data.MinValue = scale_min;
		data.MaxValue = scale_max;
	}

	if(Context.ShaderInt.ID == 0)
		DrawList.AddCallback(CreateShader, nullptr);

	DrawList.AddCallback(RenderCallback, (void*)(intptr_t)plotID);
	DrawList.PrimReserve(6, 4);
	DrawList.PrimRectUV(bounds_min, bounds_max, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 0);
	DrawList.AddCallback(UnbindTexture, nullptr);
	DrawList.AddCallback(ImDrawCallback_ResetRenderState, nullptr);
}

void BustPlotCache()
{
    ContextData& Context = *((ContextData*)GImPlot->backendCtx);

	for(const HeatmapData& data : Context.HeatmapDataList)
		glDeleteTextures(1, &data.HeatmapTexID);

	Context.HeatmapDataList.clear();
	Context.PlotIDs.Clear();
}

void BustItemCache() {}

}
}

#endif
