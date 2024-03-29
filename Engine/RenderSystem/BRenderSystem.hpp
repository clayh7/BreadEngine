#pragma once

#include <string>
#include <vector>
#include <map>
#include "Engine/RenderSystem/OpenGLExtensions.hpp"
#include "Engine/RenderSystem/Color.hpp"
#include "Engine/RenderSystem/RenderState.hpp"
#include "Engine/RenderSystem/VertexDefinition.hpp"
#include "Engine/Math/AABB2f.hpp"
#include "Engine/Math/Vector2f.hpp"
#include "Engine/Math/Vector3f.hpp"
#include "Engine/Math/Vector4f.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/RenderSystem/Uniform.hpp"


//-------------------------------------------------------------------------------------------------
class AABB3;
class BitmapFont;
class Camera3D;
class Framebuffer;
class Texture;
class Material;
class Matrix4f;
class Mesh;
class MeshRenderer;
class NamedProperties;
class Vertex_PCU;
class Vertex_PC;
class VertexDefinition;
typedef unsigned int GLenum;


//-------------------------------------------------------------------------------------------------
class BRenderSystem
{
	//-------------------------------------------------------------------------------------------------
	// Static Members
	//-------------------------------------------------------------------------------------------------
public:
	static char const * DEFAULT_FONT;
	static BRenderSystem * s_System;

	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
public:
	int m_currentDrawCalls;

private:
	Framebuffer * m_activeFBO;
	Camera3D * m_activeCamera;
	RenderState m_currentRenderState;
	Color m_currentClearColor;
	unsigned int m_currentTextureID;
	float m_currentLineWidth;
	float m_currentPointSize;

	//-------------------------------------------------------------------------------------------------
	// Static Functions
	//-------------------------------------------------------------------------------------------------
public:
	static void Startup();
	static void Shutdown();
	static void OnUpdate(NamedProperties & params);
	static BRenderSystem * CreateOrGetSystem();
	static void ClearScreen(const Color& clearColor);

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	BRenderSystem();
	~BRenderSystem();

	void InitializeOpenGL();

	//-------------------------------------------------------------------------------------------------
	// SHADER DRAWING
	//-------------------------------------------------------------------------------------------------
	//#TODO: Should I move the Create methods into their respective classes?
	//#TODO: Add the ability to draw lines
	GLuint CreateSampler(GLenum const &min_filter, GLenum const &mag_filter, GLenum const &u_wrap, GLenum const &v_wrap);
	GLuint CreateRenderBuffer(void const *data, size_t count, size_t elem_size, GLenum const &usage = GL_STATIC_DRAW);
	void UpdateRenderBuffer(GLuint bufferID, void const *data, size_t count, size_t elem_size, GLenum const &usage = GL_STATIC_DRAW);
	void CreateOrUpdateRenderBuffer(GLuint * bufferID, void const *data, size_t count, size_t elem_size, GLenum const &usage = GL_STATIC_DRAW);
	GLuint CreateShader(std::string const &filename, GLenum shader_type);
	GLuint CreateAndLinkProgram(GLuint vs, GLuint fs, std::string const &debugFilepath);
	void BindMeshToVAO(GLuint vaoID, Mesh const *mesh, Material const *material);
	void BindShaderProgramProperty(int bindPoint, GLint count, GLenum type, GLboolean normalize, GLsizei stride, GLsizei offset);
	void BindTextureSampler(GLuint samplerID, GLuint textureID, unsigned int textureIndexPort, int bindLocation);

	Camera3D const * GetActiveCamera() const { return m_activeCamera; }
	GLenum GetVertexDataType(VertexDataType const & type) const;
	void SetShaderProgramUniforms(GLuint samplerID, const UniformMap& meshRenderer);
	void SetMatrixUniforms(MeshRenderer* setMeshRenderer);
	void MeshRender(MeshRenderer const *meshRenderer);

	//-------------------------------------------------------------------------------------------------
	// FRAME BUFFERS
	//-------------------------------------------------------------------------------------------------
	void DeleteFramebuffer(Framebuffer *fbo);
	void BindFramebuffer(Framebuffer *fbo);
	void FramebufferCopyToBack(Framebuffer *fbo);

	//-------------------------------------------------------------------------------------------------
	// RENDER STATE FUNCTIONS
	//-------------------------------------------------------------------------------------------------
	void ApplyRenderState(RenderState const &renderState);
	GLenum GetOpenGLDrawMode(const ePrimitiveType& drawMode) const;
	float GetLineWidth() const { return m_currentLineWidth; }
	float GetPointSize() const { return m_currentPointSize; }
	void SetCullFace(bool enable);
	void SetDepthWrite(bool enable);
	void SetDepthTest(bool enable);
	void SetBlending(const eBlending& setBlendingMode);
	void SetDrawMode(eDrawMode const &drawMode);
	void SetWindingOrder(bool clockwise);
	void SetLineWidth(float size);
	void SetPointSize(float size);
	void EnableAlphaTesting(float threshold = 0.5f);
	void DisableAlphaTesting();

	//-------------------------------------------------------------------------------------------------
	// INFORMATION ABOUT GRAPHICS
	//-------------------------------------------------------------------------------------------------
	std::string GetOpenGLVersion() const;
	std::string GetGLSLVersion() const;
	BitmapFont const * GetDefaultFont() const;
	Framebuffer * GetActiveFBO() const;
};