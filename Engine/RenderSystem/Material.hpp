#pragma once

#include <string>
#include <vector>
#include <map>
#include "Engine/Math/Vector3f.hpp"
#include "Engine/Math/Vector4f.hpp"
#include "Engine/Math/Vector4i.hpp"
#include "Engine/RenderSystem/Color.hpp"
#include "Engine/RenderSystem/Light.hpp"
#include "Engine/RenderSystem/Uniform.hpp"


//-------------------------------------------------------------------------------------------------
class Texture;
class ShaderProgram;
class Color;
class Attribute;
class Matrix4f;


//-------------------------------------------------------------------------------------------------
class Material
{
	//-------------------------------------------------------------------------------------------------
	// Static Members
	//-------------------------------------------------------------------------------------------------
public:
	static char const * DEFAULT_VERT;
	static char const * DEFAULT_FRAG;
	static Material const * DEFAULT_MATERIAL;
	static Material * FONT_MATERIAL; //Treat it as a const variable (not const for some setup)

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static void InitializeDefaultMaterials();
	static void DestroyDefaultMaterials();

	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
private:
	unsigned int m_samplerID;
	ShaderProgram const * m_program;

private:
	// Uniforms
	std::map<size_t, Attribute*> m_attributes;
	std::vector<char*> m_uniformNames;
	UniformMap m_uniforms;

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	Material();
	Material(ShaderProgram const * program);
	Material(std::string const & vsFilePath, std::string const & fsFilePath);
	~Material();

	//Get Uniforms
	std::map<size_t, Attribute*> const & GetAttributeList() const;
	const UniformMap& GetUniformList() const;
	unsigned int GetGPUProgramID() const;
	unsigned int GetSamplerID() const;

	//#TODO: Refactor to use templates
	//Set Uniforms
	void SetUniform(const char* uniformName, int uniformValue);
	void SetUniform(const char* uniformName, float uniformValue);
	void SetUniform(const char* uniformName, Vector3f const & uniformValue);
	void SetUniform(const char* uniformName, Vector4f const & uniformValue);
	void SetUniform(const char* uniformName, Vector4i const & uniformValue);
	void SetUniform(const char* uniformName, Matrix4f const & uniformValue);
	void SetUniform(const char* uniformName, Color const & uniformValue);
	void SetUniform(const char* uniformName, std::string const & uniformValue);
	void SetUniform(const char* uniformName, unsigned int uniformValue);
	void SetUniform(const char* uniformName, Texture const * uniformValue);

	void SetUniform(const char* uniformName, int * uniformValue);
	void SetUniform(const char* uniformName, float * uniformValue);
	void SetUniform(const char* uniformName, Vector2f * uniformValue);
	void SetUniform(const char* uniformName, Vector3f * uniformValue);
	void SetUniform(const char* uniformName, Vector4f * uniformValue);
	void SetUniform(const char* uniformName, Matrix4f * uniformValue);

	//Custom Uniform Arrays
	void SetUniform(std::vector<Light> const &uniformLights, int lightCount);
	void SetUniform(std::vector<Matrix4f> const &uniformMatricies, int matrixCount);

	void UpdateBindpoints();
};