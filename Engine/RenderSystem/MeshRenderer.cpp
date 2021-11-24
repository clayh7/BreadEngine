#include "Engine/RenderSystem/MeshRenderer.hpp"

#include "Engine/DebugSystem/BProfiler.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Math/Matrix4f.hpp"
#include "Engine/RenderSystem/Material.hpp"
#include "Engine/RenderSystem/Mesh.hpp"
#include "Engine/RenderSystem/BRenderSystem.hpp"
#include "Engine/RenderSystem/Uniform.hpp"
#include "Engine/RenderSystem/Attribute.hpp"
#include "Engine/RenderSystem/Texture.hpp"
#include "Engine/RenderSystem/Light.hpp"


//-------------------------------------------------------------------------------------------------
MeshRenderer::MeshRenderer(Transform const & transform /*= Transform( )*/, RenderState const & renderState /*= RenderState::BASIC_3D*/)
	: m_transform(transform)
	, m_renderState(renderState)
	, m_vaoID(NULL)
	, m_mesh(nullptr)
	, m_material(nullptr)
{
	CreateVAO();
}


//-------------------------------------------------------------------------------------------------
MeshRenderer::MeshRenderer(eMeshShape const & meshShape, Transform const &transform /*= Transform( )*/, RenderState const & renderState /*= RenderState::BASIC_3D*/)
	: m_transform(transform)
	, m_renderState(renderState)
	, m_vaoID(NULL)
	, m_mesh(Mesh::GetMeshShape(meshShape))
	, m_material(Material::DEFAULT_MATERIAL)
{
	CreateVAO();

	BRenderSystem * RSystem = BRenderSystem::s_System;
	if(!RSystem)
	{
		ERROR_AND_DIE("No Render System.");
	}
	RSystem->BindMeshToVAO(m_vaoID, m_mesh, m_material);

	//Clear Matricies
	SetUniform("uModel", GetModelMatrix());
	SetUniform("uView", Matrix4f::IDENTITY);
	SetUniform("uProj", Matrix4f::IDENTITY);
}


//-------------------------------------------------------------------------------------------------
MeshRenderer::MeshRenderer(Mesh const * mesh, Transform const & transform /*= Transform( )*/, RenderState const & renderState /*= RenderState::BASIC_3D */)
	: m_transform(transform)
	, m_renderState(renderState)
	, m_vaoID(NULL)
	, m_mesh(mesh)
	, m_material(Material::DEFAULT_MATERIAL)
{
	CreateVAO();

	BRenderSystem * RSystem = BRenderSystem::CreateOrGetSystem();
	if(!RSystem)
	{
		ERROR_AND_DIE("No Render System.");
	}
	RSystem->BindMeshToVAO(m_vaoID, m_mesh, m_material);

	//Clear Matricies
	SetUniform("uModel", GetModelMatrix());
	SetUniform("uView", Matrix4f::IDENTITY);
	SetUniform("uProj", Matrix4f::IDENTITY);
}


//-------------------------------------------------------------------------------------------------
MeshRenderer::MeshRenderer(Mesh const * mesh, Material const * material, Transform const & transform /*= Transform() */, RenderState const & renderState /*= RenderState::BASIC_3D*/)
	: m_transform(transform)
	, m_renderState(renderState)
	, m_vaoID(NULL)
	, m_mesh(mesh)
	, m_material(material)
{
	CreateVAO();

	BRenderSystem * RSystem = BRenderSystem::CreateOrGetSystem();
	if(!RSystem)
	{
		ERROR_AND_DIE("No Render System.");
	}
	RSystem->BindMeshToVAO(m_vaoID, m_mesh, m_material);

	//Clear Matricies
	SetUniform("uModel", GetModelMatrix());
	SetUniform("uView", Matrix4f::IDENTITY);
	SetUniform("uProj", Matrix4f::IDENTITY);
}


//-------------------------------------------------------------------------------------------------
MeshRenderer::~MeshRenderer()
{
	glDeleteVertexArrays(1, &m_vaoID);

	for(auto deleteMe : m_uniforms)
	{
		delete deleteMe.second;
		deleteMe.second = nullptr;
	}
	m_uniforms.clear();
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::Update(bool onScreen /*= false*/)
{
	SetUniform("uModel", GetModelMatrix());
	if(!onScreen)
	{
		BRenderSystem * RSystem = BRenderSystem::CreateOrGetSystem();
		if(RSystem)
		{
			RSystem->SetMatrixUniforms(this);
		}
	}
	else
	{
		SetUniform("uView", Matrix4f::IDENTITY);
		SetUniform("uProj", Matrix4f::IDENTITY);
	}
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::Render() const
{
	BRenderSystem * RSystem = BRenderSystem::CreateOrGetSystem();
	if(RSystem)
	{
		RSystem->MeshRender(this);
	}
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::CreateVAO()
{
	//Create VAO
	glGenVertexArrays(1, &m_vaoID);
	ASSERT_RECOVERABLE(m_vaoID != NULL, "VAO didn't generate.");;
}


//-------------------------------------------------------------------------------------------------
RenderState MeshRenderer::GetRenderState() const
{
	return m_renderState;
}


//-------------------------------------------------------------------------------------------------
//#TODO: Could pre-calculate for each time the model is transformed
Matrix4f MeshRenderer::GetModelMatrix() const
{
	return m_transform.GetModelMatrix();
}


//-------------------------------------------------------------------------------------------------
Matrix4f MeshRenderer::GetViewMatrix() const
{
	auto found = m_uniforms.find("uView");
	return *((Matrix4f *)((*found->second).m_data));
}


//-------------------------------------------------------------------------------------------------
Matrix4f MeshRenderer::GetProjectionMatrix() const
{
	auto found = m_uniforms.find("uProj");
	return *((Matrix4f *)((*found->second).m_data));
}


//-------------------------------------------------------------------------------------------------
unsigned int MeshRenderer::GetGPUProgramID() const
{
	return m_material->GetGPUProgramID();
}


//-------------------------------------------------------------------------------------------------
unsigned int MeshRenderer::GetSamplerID() const
{
	return m_material->GetSamplerID();
}


//-------------------------------------------------------------------------------------------------
unsigned int MeshRenderer::GetVAOID() const
{
	return m_vaoID;
}


//-------------------------------------------------------------------------------------------------
unsigned int MeshRenderer::GetIBOID() const
{
	return m_mesh->GetIBOID();
}


//-------------------------------------------------------------------------------------------------
std::vector<DrawInstruction> const & MeshRenderer::GetDrawInstructions() const
{
	return m_mesh->GetDrawInstructions();
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetLineWidth(float lineWidth)
{
	m_renderState.m_lineWidth = lineWidth;
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetPosition(Vector3f const &pos)
{
	m_transform.m_position = pos;
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetPosition(float xPos, float yPos, float zPos)
{
	m_transform.m_position = Vector3f(xPos, yPos, zPos);
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetTransform(Transform const & transform)
{
	m_transform = transform;
	SetUniform("uModel", GetModelMatrix());
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetMesh(Mesh const * mesh)
{
	m_mesh = mesh;
	RebindMeshAndMaterialToVAO();
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetMaterial(Material const * material)
{
	m_material = material;
	RebindMeshAndMaterialToVAO();
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetMeshAndMaterial(Mesh const * mesh, Material const * material)
{
	m_mesh = mesh;
	m_material = material;
	RebindMeshAndMaterialToVAO();
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::RebindMeshAndMaterialToVAO()
{
	if(m_mesh && m_material)
	{
		BRenderSystem * RSystem = BRenderSystem::CreateOrGetSystem();
		if(RSystem)
		{
			RSystem->BindMeshToVAO(m_vaoID, m_mesh, m_material);
			UpdateUniformBindpoints();
		}
	}
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::UpdateUniformBindpoints()
{
	const UniformMap& uniformList = m_material->GetUniformList();

	// Reset all uniform bindpoints
	for(auto uniform : m_uniforms)
	{
		auto foundUniform = uniformList.find(uniform.second->m_name.c_str());
		if(foundUniform != uniformList.end())
		{
			uniform.second->m_bindPoint = foundUniform->second->m_bindPoint;
		}
		else
		{
			uniform.second->m_bindPoint = -1;
		}
	}
}


//-------------------------------------------------------------------------------------------------
const UniformMap& MeshRenderer::GetUniformList() const
{
	return m_uniforms;
}


//-------------------------------------------------------------------------------------------------
const UniformMap& MeshRenderer::GetMaterialUniformList() const
{
	return m_material->GetUniformList();
}


//-------------------------------------------------------------------------------------------------
std::map<size_t, Attribute*> const & MeshRenderer::GetMaterialAttributeList() const
{
	return m_material->GetAttributeList();
}


//-------------------------------------------------------------------------------------------------
Vector3f MeshRenderer::GetPosition()
{
	return m_transform.m_position;
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetUniform(const char* uniformName, const Color& uniformValue)
{
	SetUniform(uniformName, uniformValue.GetVector4f());
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetUniform(const char* uniformName, const Texture* uniformValue)
{
	SetUniform(uniformName, uniformValue->m_openglTextureID);
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetUniform(std::vector<Light> const &uniformLights, int lightCount)
{
	SetUniform("uLightCount", lightCount);
	Vector4f lightColor[16];
	Vector3f lightPosition[16];
	Vector3f lightDirection[16];
	float lightIsDirectional[16];
	float lightDistanceMin[16];
	float lightDistanceMax[16];
	float lightStrengthAtMin[16];
	float lightStrengthAtMax[16];
	float lightInnerAngle[16];
	float lightOuterAngle[16];
	float lightStrengthInside[16];
	float lightStrengthOutside[16];
	for(unsigned int lightIndex = 0; lightIndex < uniformLights.size(); ++lightIndex)
	{
		lightColor[lightIndex] = uniformLights[lightIndex].m_lightColor.GetVector4f();
		lightPosition[lightIndex] = uniformLights[lightIndex].m_position;
		lightDirection[lightIndex] = uniformLights[lightIndex].m_lightDirection;
		lightIsDirectional[lightIndex] = uniformLights[lightIndex].m_isLightDirectional;
		lightDistanceMin[lightIndex] = uniformLights[lightIndex].m_minLightDistance;
		lightDistanceMax[lightIndex] = uniformLights[lightIndex].m_maxLightDistance;
		lightStrengthAtMin[lightIndex] = uniformLights[lightIndex].m_strengthAtMin;
		lightStrengthAtMax[lightIndex] = uniformLights[lightIndex].m_strengthAtMax;
		lightInnerAngle[lightIndex] = uniformLights[lightIndex].m_innerLightCosAngle;
		lightOuterAngle[lightIndex] = uniformLights[lightIndex].m_outerLightCosAngle;
		lightStrengthInside[lightIndex] = uniformLights[lightIndex].m_strengthInside;
		lightStrengthOutside[lightIndex] = uniformLights[lightIndex].m_strengthOutside;
	}

	SetUniform("uLightColor", lightColor);
	SetUniform("uLightPosition", lightPosition);
	SetUniform("uLightDirection", lightDirection);
	SetUniform("uLightIsDirectional", lightIsDirectional);
	SetUniform("uLightDistanceMin", lightDistanceMin);
	SetUniform("uLightDistanceMax", lightDistanceMax);
	SetUniform("uLightStrengthAtMin", lightStrengthAtMin);
	SetUniform("uLightStrengthAtMax", lightStrengthAtMax);
	SetUniform("uLightInnerAngle", lightInnerAngle);
	SetUniform("uLightOuterAngle", lightOuterAngle);
	SetUniform("uLightStrengthInside", lightStrengthInside);
	SetUniform("uLightStrengthOutside", lightStrengthOutside);
}