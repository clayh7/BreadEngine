#pragma once

#include <map>
#include "Engine/Math/Transform.hpp"
#include "Engine/RenderSystem/Mesh.hpp" //Need MeshShape enum
#include "Engine/RenderSystem/RenderState.hpp"
#include "Engine/RenderSystem/Uniform.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"


//-------------------------------------------------------------------------------------------------
class Color;
class Material;
class Texture;
class Matrix4f;
class Attribute;
class Light;


//-------------------------------------------------------------------------------------------------
class MeshRenderer
{
	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
protected:
	Transform m_transform;
	RenderState m_renderState;
	unsigned int m_vaoID;
	Mesh const * m_mesh;
	Material const * m_material;
	//#TODO: Replace all uniforms with NamedProperties (here and in Materials)

protected:
	UniformMap m_uniforms; //Uniforms individual to this MeshRenderer. Overwrites Material

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	//#TODO: Be careful with this implementation of MeshRenderer, catch the cases where VAO = NULL
	MeshRenderer(Transform const &transform = Transform(), RenderState const &renderState = RenderState::BASIC_3D);

	MeshRenderer(eMeshShape const &meshShape, Transform const &transform = Transform(), RenderState const &renderState = RenderState::BASIC_3D);
	MeshRenderer(Mesh const *mesh, Transform const &transform = Transform(), RenderState const &renderState = RenderState::BASIC_3D);
	MeshRenderer(Mesh const *mesh, Material const *material, Transform const &transform = Transform(), RenderState const &renderState = RenderState::BASIC_3D);
	~MeshRenderer();

	void Update(bool onScreen = false);
	void Render() const;

	void CreateVAO();

	RenderState GetRenderState() const;
	Matrix4f GetModelMatrix() const;
	Matrix4f GetViewMatrix() const;
	Matrix4f GetProjectionMatrix() const;
	unsigned int GetGPUProgramID() const;
	unsigned int GetSamplerID() const;
	unsigned int GetVAOID() const;
	unsigned int GetIBOID() const;
	std::vector<DrawInstruction> const & GetDrawInstructions() const;
	const UniformMap& GetUniformList() const;
	const UniformMap& GetMaterialUniformList() const;
	std::map<size_t, Attribute*> const & GetMaterialAttributeList() const;
	Vector3f GetPosition();

	void SetLineWidth(float lineWidth);
	void SetPosition(Vector3f const & pos);
	void SetPosition(float xPos, float yPos, float zPos);
	void SetTransform(Transform const &transform);
	void SetMesh(Mesh const * mesh);
	void SetMaterial(Material const * setMaterial);
	void SetMeshAndMaterial(Mesh const * mesh, Material const * material);
	void RebindMeshAndMaterialToVAO();//#TODO: Maybe make a force rebind
	void UpdateUniformBindpoints();

	void SetUniform(const char* uniformName, const Color& uniformValue);
	void SetUniform(const char* uniformName, const Texture* uniformValue);
	void SetUniform(std::vector<Light> const & uniformLights, int lightCount);

	//-------------------------------------------------------------------------------------------------
	template<typename Type>
	void SetUniform(const char* uniformName, const Type& uniformValue)
	{
		//Get Uniform Map
		const UniformMap& matUniformList = m_material->GetUniformList();

		//Find uniform on material
		auto foundMatUniform = matUniformList.find(uniformName);
		if (foundMatUniform == matUniformList.end())
		{
			ERROR_AND_DIE("Uniform doesn't exist on Material");
		}

		//Find uniform on meshrenderer
		auto foundUniform = m_uniforms.find(uniformName);
		if (foundUniform == m_uniforms.end())
		{
			Type* newData = new Type[foundMatUniform->second->m_size];
			newData[0] = (uniformValue);
			Uniform* addUniform = new Uniform(*foundMatUniform->second, newData);
			m_uniforms.insert(std::pair<const char*, Uniform*>(uniformName, addUniform));
		}
		else
		{
			//Update data
			*((Type*)m_uniforms[uniformName]->m_data) = uniformValue;
		}
	}


	//-------------------------------------------------------------------------------------------------
	template<typename Type>
	void SetUniform(const char* uniformName, Type* uniformValue)
	{
		const UniformMap& uniformList = m_material->GetUniformList();
		auto foundUniformIter = uniformList.find(uniformName);

		//Doesn't exist yet, lets make space for it and assign it to the data
		if (foundUniformIter != uniformList.end() && m_uniforms[uniformName] == nullptr)
		{
			const Uniform* foundUniform = foundUniformIter->second;
			int uniformSize = foundUniform->m_size;
			size_t dataSize = sizeof(Type) * uniformSize;
			Type* data = (Type*)malloc(dataSize);
			memcpy(data, uniformValue, dataSize);

			m_uniforms[uniformName] = new Uniform
			(
				foundUniform->m_bindPoint,
				foundUniform->m_length,
				uniformSize,
				foundUniform->m_type,
				foundUniform->m_name,
				data //Setting new value
			);
		}

		//update the data
		else if (foundUniformIter != uniformList.end() && m_uniforms[uniformName])
		{
			const Uniform* foundUniform = foundUniformIter->second;
			int uniformSize = foundUniform->m_size;
			size_t dataSize = sizeof(Type) * uniformSize;
			memcpy(m_uniforms[uniformName]->m_data, uniformValue, dataSize);
		}

		//That's not real
		else
		{
			ERROR_AND_DIE("Uniform doesn't exist on Material");
		}
	}
};