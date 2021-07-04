#include "../implot.h"
#include "../implot_internal.h"
#include "implot_backend.h"
#include "implot_impl_opengl3.h"

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

struct HeatmapShader
{
	GLuint ID = 0;                       ///< Shader ID for the heatmap shader
	GLuint AttribLocationProjection = 0; ///< Attribute location for the projection matrix uniform
	GLuint AttribLocationMinValue = 0;   ///< Attribute location for the minimum value uniform
	GLuint AttribLocationMaxValue = 0;   ///< Attribute location for the maximum value uniform
	GLuint AttribLocationAxisLog = 0;    ///< Attribute location for the logarithmic axes uniform
	GLuint AttribLocationMinBounds = 0;  ///< Attribute location for the minimum bounds uniform
	GLuint AttribLocationMaxBounds = 0;  ///< Attribute location for the maximum bounds uniform
};

struct HeatmapData
{
	ImGuiID ID;
	HeatmapShader* ShaderProgram; ///< Shader to be used by this heatmap (either ShaderInt or ShaderFloat)
	GLuint HeatmapTexID;          ///< Texture ID of the heatmap 2D texture
	GLuint ColormapTexID;         ///< Texture ID of the colormap 1D texture
	ImPlotPoint MinBounds;        ///< Minimum bounds of the heatmap
	ImPlotPoint MaxBounds;        ///< Maximum bounds of the heatmap
	float MinValue;               ///< Minimum value of the colormap
	float MaxValue;               ///< Maximum value of the colormap
	bool AxisLogX;                ///< Whether the X axis is logarithmic or not
	bool AxisLogY;                ///< Whether the Y axis is logarithmic or not

	HeatmapData() {
		glGenTextures(1, &HeatmapTexID);
		glBindTexture(GL_TEXTURE_2D, HeatmapTexID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
};

struct ContextData
{
	HeatmapShader ShaderInt;                  ///< Shader for integer heatmaps
	HeatmapShader ShaderFloat;                ///< Shader for floating-point heatmaps

	GLuint AttribLocationImGuiProjection = 0; ///< Attribute location for the projection matrix uniform (ImGui default shader)
	GLuint ImGuiShader = 0;                   ///< Shader ID of ImGui's default shader

	ImPool<HeatmapData> Heatmaps;             ///< Array of heatmap data
	ImVector<GLuint> ColormapIDs;             ///< Texture IDs of the colormap textures

	ImVector<float> TempFloat;                ///< Temporary data
	ImVector<ImS32> TempS32;                  ///< Temporary data
	ImVector<ImU32> TempU32;                  ///< Temporary data
};

void* CreateContext() {
	return IM_NEW(ContextData)();
}

void DestroyContext() {
	ContextData* Context = ((ContextData*)GImPlot->backendCtx);
	// free OpenGL resources
	for(int i = 0; i < Context->Heatmaps.GetBufSize(); ++i)
		glDeleteTextures(1, &Context->Heatmaps.GetByIndex(i)->HeatmapTexID);
	for(GLuint texID : Context->ColormapIDs)
		glDeleteTextures(1, &texID);
	glDeleteProgram(Context->ShaderInt.ID);
	glDeleteProgram(Context->ShaderFloat.ID);
	Context->Heatmaps.Clear();
	IM_DELETE(Context);
}

#define HEATMAP_VERTEX_SHADER_CODE                                   \
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
	"    Frag_UV = UV;\n"                                            \
	"    gl_Position = ProjMtx * vec4(Position.xy, 0.0f, 1.0f);\n"   \
	"}\n"

#define HEATMAP_FRAGMENT_SHADER_CODE                                  \
	"#version 330 core\n"                                             \
	"precision mediump float;\n"                                      \
	"\n"                                                              \
	"in vec2 Frag_UV;\n"                                              \
	"out vec4 Out_Color;\n"                                           \
	"\n"                                                              \
	"uniform sampler1D colormap;\n"                                   \
	"uniform %csampler2D heatmap;\n"                                  \
	"uniform float min_val;\n"                                        \
	"uniform float max_val;\n"                                        \
	"\n"                                                              \
	"uniform vec2 bounds_min;\n"                                      \
	"uniform vec2 bounds_max;\n"                                      \
	"uniform bvec2 ax_log;\n"                                         \
	"\n"                                                              \
	"float log_den(float x, float min_rng, float max_rng)\n"          \
	"{\n"                                                             \
	"    float minrl = log(min_rng);\n"                               \
	"    float maxrl = log(max_rng);\n"                               \
	"\n"                                                              \
	"    return (exp((maxrl - minrl) * x + minrl) - min_rng) / (max_rng - min_rng);" \
	"}\n"                                                             \
	"\n"                                                              \
	"void main()\n"                                                   \
	"{\n"                                                             \
	"    float min_tex_offs = 0.5 / float(textureSize(colormap, 0));\n" \
	"    float uv_x = ax_log.x ? log_den(Frag_UV.x, bounds_min.x, bounds_max.x) : Frag_UV.x;\n" \
	"    float uv_y = ax_log.y ? log_den(Frag_UV.y, bounds_min.y, bounds_max.y) : Frag_UV.y;\n" \
	"\n"                                                               \
	"    float value = float(texture(heatmap, vec2(uv_x, uv_y)).r);\n" \
	"    float offset = (value - min_val) / (max_val - min_val);\n"    \
	"          offset = mix(min_tex_offs, 1.0 - min_tex_offs, clamp(offset, 0.0f, 1.0f));\n" \
	"    Out_Color = texture(colormap, offset);\n"                     \
	"}\n"

static void CompileShader(HeatmapShader& ShaderProgram, GLchar* VertexShaderCode, GLchar* FragmentShaderCode) {
	GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VertexShader, 1, &VertexShaderCode, nullptr);
	glCompileShader(VertexShader);

	GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShader, 1, &FragmentShaderCode, nullptr);
	glCompileShader(FragmentShader);

	ShaderProgram.ID = glCreateProgram();
	glAttachShader(ShaderProgram.ID, VertexShader);
	glAttachShader(ShaderProgram.ID, FragmentShader);
	glLinkProgram(ShaderProgram.ID);

	glDetachShader(ShaderProgram.ID, VertexShader);
	glDetachShader(ShaderProgram.ID, FragmentShader);
	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);

	ShaderProgram.AttribLocationProjection = glGetUniformLocation(ShaderProgram.ID, "ProjMtx");
	ShaderProgram.AttribLocationMinValue   = glGetUniformLocation(ShaderProgram.ID, "min_val");
	ShaderProgram.AttribLocationMaxValue   = glGetUniformLocation(ShaderProgram.ID, "max_val");
	ShaderProgram.AttribLocationMinBounds  = glGetUniformLocation(ShaderProgram.ID, "bounds_min");
	ShaderProgram.AttribLocationMaxBounds  = glGetUniformLocation(ShaderProgram.ID, "bounds_max");
	ShaderProgram.AttribLocationAxisLog    = glGetUniformLocation(ShaderProgram.ID, "ax_log");

	glUseProgram(ShaderProgram.ID);
	glUniform1i(glGetUniformLocation(ShaderProgram.ID, "heatmap"), 0); // Set texture slot of heatmap texture
	glUniform1i(glGetUniformLocation(ShaderProgram.ID, "colormap"), 1); // Set texture slot of colormap texture
}

static void CreateHeatmapShader(const ImDrawList*, const ImDrawCmd*) {
	ContextData& Context = *((ContextData*)GImPlot->backendCtx);

	glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&Context.ImGuiShader);

	Context.AttribLocationImGuiProjection = glGetUniformLocation(Context.ImGuiShader, "ProjMtx");
	GLuint AttribLocationVtxPos = (GLuint)glGetAttribLocation(Context.ImGuiShader, "Position");
	GLuint AttribLocationVtxUV  = (GLuint)glGetAttribLocation(Context.ImGuiShader, "UV");

	GLchar* VertexShaderCode = new GLchar[1000];
	GLchar* FragmentShaderCode = new GLchar[1000];

	snprintf(VertexShaderCode, 1000, HEATMAP_VERTEX_SHADER_CODE, AttribLocationVtxPos, AttribLocationVtxUV);
	snprintf(FragmentShaderCode, 1000, HEATMAP_FRAGMENT_SHADER_CODE, ' ');

	CompileShader(Context.ShaderFloat, VertexShaderCode, FragmentShaderCode);

	snprintf(VertexShaderCode, 1000, HEATMAP_VERTEX_SHADER_CODE, AttribLocationVtxPos, AttribLocationVtxUV);
	snprintf(FragmentShaderCode, 1000, HEATMAP_FRAGMENT_SHADER_CODE, 'i');

	CompileShader(Context.ShaderInt, VertexShaderCode, FragmentShaderCode);
	glUseProgram(0);

	delete[] VertexShaderCode;
	delete[] FragmentShaderCode;
}

static void RenderCallback(const ImDrawList*, const ImDrawCmd* cmd) {
	ContextData& Context = *((ContextData*)GImPlot->backendCtx);

	int itemID = (int)(intptr_t)cmd->UserCallbackData;
	HeatmapData& data = *Context.Heatmaps.GetByKey(itemID);

	// Get projection matrix of current shader
	float OrthoProjection[4][4];
	glGetUniformfv(Context.ImGuiShader, Context.AttribLocationImGuiProjection, &OrthoProjection[0][0]);

	// Enable our shader
	glUseProgram(data.ShaderProgram->ID);

	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, data.HeatmapTexID); // Set texture ID of data
	glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_1D, data.ColormapTexID); // Set texture ID of colormap

	glUniformMatrix4fv(data.ShaderProgram->AttribLocationProjection, 1, GL_FALSE, &OrthoProjection[0][0]);
	glUniform1f(data.ShaderProgram->AttribLocationMinValue, data.MinValue); // Set minimum range
	glUniform1f(data.ShaderProgram->AttribLocationMaxValue, data.MaxValue); // Set maximum range
	glUniform2i(data.ShaderProgram->AttribLocationAxisLog, data.AxisLogX, data.AxisLogY); // Logarithmic axis
	glUniform2f(data.ShaderProgram->AttribLocationMinBounds, (float)data.MinBounds.x, (float)data.MinBounds.y); // Set minimum bounds
	glUniform2f(data.ShaderProgram->AttribLocationMaxBounds, (float)data.MaxBounds.x, (float)data.MaxBounds.y); // Set maximum bounds
}

static void ResetState(const ImDrawList*, const ImDrawCmd*) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	ContextData& Context = *((ContextData*)GImPlot->backendCtx);
	glUseProgram(Context.ImGuiShader);
}

static void SetTextureData(GLuint textureID, const void* data, GLsizei rows, GLsizei cols, GLint internalFormat, GLenum format, GLenum type) {
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, cols, rows, 0, format, type, data);
}

void AddColormap(const ImU32* keys, int count, bool qual) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_1D, textureID);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, count, 0, GL_RGBA, GL_UNSIGNED_BYTE, keys);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, qual ? GL_NEAREST : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, qual ? GL_NEAREST : GL_LINEAR);
	glBindTexture(GL_TEXTURE_1D, 0);

	ContextData& Context = *((ContextData*)GImPlot->backendCtx);
	Context.ColormapIDs.push_back(textureID);
}

void SetHeatmapData(GLuint textureID, const double* values, int rows, int cols) {
	ContextData& Context = *((ContextData*)GImPlot->backendCtx);
	Context.TempFloat.resize(rows * cols);
	for(int i = 0; i < rows*cols; i++)
		Context.TempFloat[i] = (float)values[i];
	SetTextureData(textureID, Context.TempFloat.Data, rows, cols, GL_R32F, GL_RED, GL_FLOAT);
}

void SetHeatmapData(GLuint textureID, const ImS64* values, int rows, int cols) {
	ContextData& Context = *((ContextData*)GImPlot->backendCtx);
	Context.TempS32.resize(rows * cols);
	for(int i = 0; i < rows*cols; i++)
		Context.TempS32[i] = (ImS32)values[i];
	SetTextureData(textureID, Context.TempS32.Data, rows, cols, GL_R32I, GL_RED_INTEGER, GL_INT);
}

void SetHeatmapData(GLuint textureID, const ImU64* values, int rows, int cols) {
	ContextData& Context = *((ContextData*)GImPlot->backendCtx);
	Context.TempU32.resize(rows * cols);
	for(int i = 0; i < rows*cols; i++)
		Context.TempU32[i] = (ImU32)values[i];
	SetTextureData(textureID, Context.TempU32.Data, rows, cols, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT);
}

void RenderHeatmap(int itemID,
                   const void* values,
				   ImGuiDataType data_type,
				   int rows,
				   int cols,
				   float scale_min,
				   float scale_max,
	               const ImVec2& coords_min,
				   const ImVec2& coords_max,
				   const ImPlotPoint& bounds_min,
				   const ImPlotPoint& bounds_max,
	               ImPlotScale scale,
				   bool reverse_y,
				   ImPlotColormap cmap,
				   ImDrawList& DrawList)
{
	ContextData& Context = *((ContextData*)GImPlot->backendCtx);
	HeatmapData& data = *Context.Heatmaps.GetOrAddByKey(itemID);
	data.ID = itemID;
	data.ColormapTexID = Context.ColormapIDs[cmap];
	data.MinValue = scale_min;
	data.MaxValue = scale_max;
	data.AxisLogX = scale == ImPlotScale_LogLin || scale == ImPlotScale_LogLog;
	data.AxisLogY = scale == ImPlotScale_LinLog || scale == ImPlotScale_LogLog;
	data.MinBounds = bounds_min;
	data.MaxBounds = bounds_max;
	data.ShaderProgram = (data_type == ImGuiDataType_Float || data_type == ImGuiDataType_Double ? &Context.ShaderFloat : &Context.ShaderInt);

	switch(data_type) {
		case ImGuiDataType_S8:     SetTextureData(data.HeatmapTexID, (const ImS8*) values, rows, cols, GL_R8I,   GL_RED_INTEGER, GL_BYTE          ); break;
		case ImGuiDataType_U8:     SetTextureData(data.HeatmapTexID, (const ImU8*) values, rows, cols, GL_R8UI,  GL_RED_INTEGER, GL_UNSIGNED_BYTE ); break;
		case ImGuiDataType_S16:    SetTextureData(data.HeatmapTexID, (const ImS16*)values, rows, cols, GL_R16I,  GL_RED_INTEGER, GL_SHORT         ); break;
		case ImGuiDataType_U16:    SetTextureData(data.HeatmapTexID, (const ImU16*)values, rows, cols, GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT); break;
		case ImGuiDataType_S32:    SetTextureData(data.HeatmapTexID, (const ImS32*)values, rows, cols, GL_R32I,  GL_RED_INTEGER, GL_INT           ); break;
		case ImGuiDataType_U32:    SetTextureData(data.HeatmapTexID, (const ImU32*)values, rows, cols, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT  ); break;
		case ImGuiDataType_S64:    SetHeatmapData(data.HeatmapTexID, (const ImS64*)values, rows, cols); break;
		case ImGuiDataType_U64:    SetHeatmapData(data.HeatmapTexID, (const ImU64*)values, rows, cols); break;
		case ImGuiDataType_Float:  SetTextureData(data.HeatmapTexID, (const float*)values, rows, cols, GL_R32F, GL_RED, GL_FLOAT); break;
		case ImGuiDataType_Double: SetHeatmapData(data.HeatmapTexID, (double*)values, rows, cols); break;
	};

	if(Context.ShaderInt.ID == 0 || Context.ShaderFloat.ID == 0)
		DrawList.AddCallback(CreateHeatmapShader, nullptr);

	DrawList.AddCallback(RenderCallback, (void*)(intptr_t)itemID);
	DrawList.PrimReserve(6, 4);
	DrawList.PrimRectUV(coords_min, coords_max, ImVec2(0.0f, reverse_y ? 1.0f : 0.0f), ImVec2(1.0f, reverse_y ? 0.0f : 1.0f), 0);
	DrawList.AddCallback(ResetState, nullptr);
}

void BustPlotCache() {
	BustItemCache();
}

void BustItemCache() {
	ContextData* Context = ((ContextData*)GImPlot->backendCtx);
	for(int i = 0; i < Context->Heatmaps.GetBufSize(); ++i)
		glDeleteTextures(1, &Context->Heatmaps.GetByIndex(i)->HeatmapTexID);
	Context->Heatmaps.Clear();
}

void ShowBackendMetrics() {
	ContextData* Context = ((ContextData*)GImPlot->backendCtx);
	// heatmaps
	int n_heatmaps = Context->Heatmaps.GetBufSize();
	if (ImGui::TreeNode("Heatmaps","Heatmaps (%d)",n_heatmaps)) {
		for (int i = 0; i < n_heatmaps; ++i) {
			HeatmapData& data = *Context->Heatmaps.GetByIndex(i);
			ImGui::PushID(i);
			if (ImGui::TreeNode("Item","Item [0x%08X]", data.ID)) {
				ImGui::BulletText("HeatmapTexID: %u", data.HeatmapTexID);
				ImGui::BulletText("ColormapTexID: %u", data.ColormapTexID);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}
}

} // namespace Backend
} // namespace ImPlot
