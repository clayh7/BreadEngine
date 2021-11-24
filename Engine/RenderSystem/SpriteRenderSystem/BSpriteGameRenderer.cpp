#include "Engine/RenderSystem/SpriteRenderSystem/BSpriteGameRenderer.hpp"

#include "Engine/DebugSystem/BProfiler.hpp"
#include "Engine/DebugSystem/BConsoleSystem.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/RenderSystem/Framebuffer.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteResource.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteSheetResource.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteLayer.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/Sprite.hpp"
#include "Engine/RenderSystem/Material.hpp"
#include "Engine/RenderSystem/MeshRenderer.hpp"
#include "Engine/RenderSystem/BRenderSystem.hpp"
#include "Engine/Utils/FileUtils.hpp"
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/Utils/XMLUtils.hpp"


//-------------------------------------------------------------------------------------------------
void EnableLayerCommand(Command const & command)
{
	int layer = command.GetArg(0, 0);
	BSpriteGameRenderer * SGRSystem = BSpriteGameRenderer::s_System;
	if(SGRSystem && SGRSystem->SetLayerEnabled(layer, true))
	{
		BConsoleSystem::AddLog(Stringf("Layer %d enabled", layer), BConsoleSystem::GOOD);
	}
	else
	{
		BConsoleSystem::AddLog(Stringf("Layer %d does not exist", layer), BConsoleSystem::BAD);
	}
}


//-------------------------------------------------------------------------------------------------
void DisableLayerCommand(Command const & command)
{
	int layer = command.GetArg(0, 0);
	BSpriteGameRenderer * SGRSystem = BSpriteGameRenderer::s_System;
	if(SGRSystem && SGRSystem->SetLayerEnabled(layer, false))
	{
		BConsoleSystem::AddLog(Stringf("Layer %d disabled", layer), BConsoleSystem::GOOD);
	}
	else
	{
		BConsoleSystem::AddLog(Stringf("Layer %d does not exist", layer), BConsoleSystem::BAD);
	}
}


//-------------------------------------------------------------------------------------------------
void ExportSpritesCommand(Command const & command)
{
	if(BSpriteGameRenderer::s_System)
	{
		std::string filename = command.GetArg(0, "SpriteDatabase");
		BSpriteGameRenderer::s_System->ExportSpriteDatabase(filename);
		BConsoleSystem::AddLog(Stringf("Exported sprite resource database to %s.Sprite.xml", filename.c_str()), BConsoleSystem::GOOD);
	}
}


//-------------------------------------------------------------------------------------------------
STATIC char const * BSpriteGameRenderer::SPRITE_RESOURCE_NAME = "SpriteResource";
STATIC char const * BSpriteGameRenderer::SPRITE_SHEET_RESOURCE_NAME = "SpriteSheetResource";
STATIC BSpriteGameRenderer * BSpriteGameRenderer::s_System = nullptr;


//-------------------------------------------------------------------------------------------------
STATIC void BSpriteGameRenderer::Startup()
{
	if(!s_System)
	{
		s_System = new BSpriteGameRenderer();
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void BSpriteGameRenderer::Shutdown()
{
	if(s_System)
	{
		delete s_System;
		s_System = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
STATIC BSpriteGameRenderer * BSpriteGameRenderer::CreateOrGetSystem()
{
	if(!s_System)
	{
		Startup();
	}

	return s_System;
}


//-------------------------------------------------------------------------------------------------
BSpriteGameRenderer::BSpriteGameRenderer()
	: m_fboCurrent(nullptr)
	, m_fboEffect(nullptr)
	, m_screenMesh(nullptr)
	, m_screenEffect(nullptr)
	, m_virtualScreen(1.f, 1.f)
	, m_virtualSize(1.f)
	, m_importSize(1.f)
	, m_aspectRatio(1.f)
	, m_clearColor(Color::BLACK)
	, m_viewMatrix(Matrix4f::IDENTITY)
	, m_projectionMatrix(Matrix4f::IDENTITY)
	, m_spriteMesh(nullptr)
	, m_spriteMeshRenderer(nullptr)
	, m_tempBuilder()
{
	LoadAllSpriteResources();
	SetupMaterialEffects();
	m_spriteMesh = new Mesh(eVertexType_SPRITE);
	m_spriteMeshRenderer = new MeshRenderer(Transform(), RenderState::BASIC_2D);
	//SetClearColor(Color::WHITE);

	//Create Nothing Shader PostProcessFX
	m_fboCurrent = new Framebuffer();
	m_fboEffect = new Framebuffer();
	m_screenMesh = Mesh::GetMeshShape(eMeshShape_QUAD);
	m_screenEffect = new MeshRenderer(m_screenMesh, m_screenMaterials[eMaterialEffect_NOTHING], Transform(Vector3f::ZERO, Matrix4f::IDENTITY, 2.f), RenderState::BASIC_2D);


	BConsoleSystem::Register("sprite_layer_enable", EnableLayerCommand, " [num] : Enables sprite layer, allowing it to render. Default = 0");
	BConsoleSystem::Register("sprite_layer_disable", DisableLayerCommand, " [num] : Disable sprite layer, stopping it from rendering. Default = 0");
	BConsoleSystem::Register("sprite_export", ExportSpritesCommand, " [filename] : Save sprite resource database to an xml file. Delfault = SpriteDatabase");
	BEventSystem::RegisterEvent(EVENT_ENGINE_UPDATE, this, &BSpriteGameRenderer::OnUpdate);
	BEventSystem::RegisterEvent(EVENT_ENGINE_RENDER, this, &BSpriteGameRenderer::OnRender);
}


//-------------------------------------------------------------------------------------------------
BSpriteGameRenderer::~BSpriteGameRenderer()
{
	delete m_spriteMesh;
	m_spriteMesh = nullptr;

	delete m_spriteMeshRenderer;
	m_spriteMeshRenderer = nullptr;

	delete m_screenEffect;
	m_screenEffect = nullptr;

	for(Material * deleteMaterial : m_screenMaterials)
	{
		delete deleteMaterial;
		deleteMaterial = nullptr;
	}
	m_screenMaterials.clear();

	delete m_fboCurrent;
	m_fboCurrent = nullptr;

	delete m_fboEffect;
	m_fboEffect = nullptr;

	for(auto deleteResource : m_spriteResourceDatabase)
	{
		delete deleteResource.second;
		deleteResource.second = nullptr;
	}
	m_spriteResourceDatabase.clear();

	for(auto deleteResource : m_spriteSheetResourceDatabase)
	{
		delete deleteResource.second;
		deleteResource.second = nullptr;
	}
	m_spriteSheetResourceDatabase.clear();

	for(auto deleteLayer : m_spriteLayers)
	{
		delete deleteLayer.second;
		deleteLayer.second = nullptr;
	}
	m_spriteLayers.clear();
}


//-------------------------------------------------------------------------------------------------
void BSpriteGameRenderer::OnUpdate(NamedProperties&)
{
	Vector2i windowDimensions = GetWindowDimensions();
	SetAspectRatio((float)windowDimensions.x, (float)windowDimensions.y);
	SetVirtualSize((float)windowDimensions.y);
	SetImportSize((float)windowDimensions.x);

	if (s_System)
	{
		s_System->UpdateMaterialEffects();
	}
}


//-------------------------------------------------------------------------------------------------
void BSpriteGameRenderer::OnRender(NamedProperties&) const
{
	if (s_System)
	{
		s_System->SystemRender();
	}
}


//-------------------------------------------------------------------------------------------------
void BSpriteGameRenderer::UpdateMaterialEffects()
{
	m_screenMaterials[eMaterialEffect_WAVES]->SetUniform("uTime", Time::TOTAL_SECONDS);
}


//-------------------------------------------------------------------------------------------------
void BSpriteGameRenderer::SystemRender()
{
	BRenderSystem * RSystem = BRenderSystem::s_System;
	if(!RSystem)
	{
		ERROR_AND_DIE("No Render System.");
	}

	//Save active FBO
	Framebuffer * startingFBO = RSystem->GetActiveFBO();

	//Setup current FBO
	RSystem->BindFramebuffer(m_fboCurrent);
	BRenderSystem::ClearScreen(m_clearColor);

	//Render Sprite layers
	for(auto spriteLayerIter = m_spriteLayers.begin(); spriteLayerIter != m_spriteLayers.end(); ++spriteLayerIter)
	{
		//Render layer of sprites
		SpriteLayer * currentLayer = spriteLayerIter->second;
		RenderLayer(currentLayer);

		//Apply FBO effects
		for(eMaterialEffect const & effectID : currentLayer->m_effects)
		{
			RSystem->BindFramebuffer(m_fboEffect);
			m_screenEffect->SetMaterial(m_screenMaterials[effectID]);
			m_screenEffect->SetUniform("uDiffuseTex", m_fboCurrent->GetColorTexture(0));
			m_screenEffect->Render();
			SwapFBO(&m_fboCurrent, &m_fboEffect);
		}
	}

	//Redraw game onto starting FBO
	RSystem->BindFramebuffer(startingFBO);
	m_screenEffect->SetMaterial(m_screenMaterials[eMaterialEffect_NOTHING]);
	m_screenEffect->SetUniform("uDiffuseTex", m_fboCurrent->GetColorTexture(0));
	m_screenEffect->Render();
}


//-------------------------------------------------------------------------------------------------
void BSpriteGameRenderer::RenderLayer(SpriteLayer const * layer) const
{
	//Only draw enabled layers
	if(!layer->IsEnabled())
	{
		return;
	}

	//Traverse sprite list and draw each one to screen
	Sprite * currentSprite = layer->m_listStart;
	while(currentSprite)
	{
		RenderSprite(currentSprite);
		currentSprite = currentSprite->m_nextSprite;
	}
}


//-------------------------------------------------------------------------------------------------
void BSpriteGameRenderer::RenderSprite(Sprite const * sprite) const
{
	if(!sprite->IsEnabled())
	{
		return;
	}

	if(!IsSpriteOnScreen(sprite))
	{
		return;
	}

	//Setup Mesh
	sprite->ConstructMesh(m_spriteMesh, m_tempBuilder);

	//Setup Material
	Material * spriteMaterial = sprite->GetMaterial();

	if(sprite->m_ignoreView)
	{
		spriteMaterial->SetUniform("uView", Matrix4f::IDENTITY);
	}
	else
	{
		spriteMaterial->SetUniform("uView", GetView());
	}
	spriteMaterial->SetUniform("uProj", m_projectionMatrix);
	spriteMaterial->SetUniform("uTexDiffuse", sprite->GetTexture());

	//Setup MeshRenderer
	m_spriteMeshRenderer->SetMeshAndMaterial(m_spriteMesh, spriteMaterial);

	//Draw
	m_spriteMeshRenderer->Render();
}


//-------------------------------------------------------------------------------------------------
void BSpriteGameRenderer::SwapFBO(Framebuffer ** out_first, Framebuffer ** out_second)
{
	Framebuffer * temp = *out_first;
	*out_first = *out_second;
	*out_second = temp;
}


//-------------------------------------------------------------------------------------------------
void BSpriteGameRenderer::SetupMaterialEffects()
{
	m_screenMaterials.resize(eMaterialEffect_COUNT);
	m_screenMaterials[eMaterialEffect_NOTHING] = new Material("Data/Shaders/post.vert", "Data/Shaders/postNothing.frag");
	m_screenMaterials[eMaterialEffect_WAVES] = new Material("Data/Shaders/post.vert", "Data/Shaders/postWaves.frag");
	m_screenMaterials[eMaterialEffect_DARK] = new Material("Data/Shaders/post.vert", "Data/Shaders/postDark.frag");
	ASSERT_RECOVERABLE(m_screenMaterials.size() == eMaterialEffect_COUNT, "Material Effect not added");
}


//-------------------------------------------------------------------------------------------------
void BSpriteGameRenderer::LoadAllSpriteResources()
{
	std::vector<std::string> spriteFiles = EnumerateFilesInFolder("Data/Sprites/", "*.Sprite.xml");
	for(std::string const & spriteFile : spriteFiles)
	{
		XMLNode spriteFileNode = XMLNode::openFileHelper(spriteFile.c_str()).getChildNode(0);

		//Load all the sprite sheets
		for(int spriteIndex = 0; spriteIndex < spriteFileNode.nChildNode(); ++spriteIndex)
		{
			XMLNode spriteData = spriteFileNode.getChildNode(spriteIndex);
			std::string name = spriteData.getName();
			if(name == SPRITE_SHEET_RESOURCE_NAME)
			{
				std::string spriteSheetID = ReadXMLAttribute(spriteData, "id", "");
				std::string spriteSheetFilename = ReadXMLAttribute(spriteData, "filename", "error");
				Vector2i spriteSheetSize = ReadXMLAttribute(spriteData, "size", Vector2i(1, 1));
				AddSpriteSheetResource(spriteSheetID, spriteSheetFilename, spriteSheetSize);
			}
		}

		//Load all the sprite resources
		for(int spriteIndex = 0; spriteIndex < spriteFileNode.nChildNode(); ++spriteIndex)
		{
			XMLNode spriteData = spriteFileNode.getChildNode(spriteIndex);
			std::string name = spriteData.getName();
			if(name == SPRITE_RESOURCE_NAME)
			{
				std::string spriteID = ReadXMLAttribute(spriteData, "id", "");
				std::string spriteFilename = ReadXMLAttribute(spriteData, "filename", "error");
				//Must be a sprite from a sprite sheet
				if(spriteFilename == "error")
				{
					int spriteSheetIndex = ReadXMLAttribute(spriteData, "index", 0);
					std::string spriteSheetID = ReadXMLAttribute(spriteData, "spriteSheet", "error");
					AddSpriteResource(spriteID, spriteSheetID, spriteSheetIndex);
				}
				else
				{
					AddSpriteResource(spriteID, spriteFilename);
				}
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
void BSpriteGameRenderer::AddSpriteResource(std::string const & id, std::string const & filename)
{
	size_t idHash = std::hash<std::string>{}(id);
	auto foundResource = m_spriteResourceDatabase.find(idHash);

	//Does not exist yet
	if(foundResource == m_spriteResourceDatabase.end())
	{
		SpriteResource * spriteResource = new SpriteResource(id, filename);
		m_spriteResourceDatabase.insert(std::pair<size_t, SpriteResource*>(idHash, spriteResource));
	}
}


//-------------------------------------------------------------------------------------------------
void BSpriteGameRenderer::AddSpriteResource(std::string const & id, std::string const & spriteSheetID, int spriteSheetIndex)
{
	size_t idHash = std::hash<std::string>{}(id);
	auto foundResource = m_spriteResourceDatabase.find(idHash);

	//Does not exist yet
	if(foundResource == m_spriteResourceDatabase.end())
	{
		SpriteSheetResource const * spriteSheetResource = GetSpriteSheetResource(spriteSheetID);
		SpriteResource * spriteResource = new SpriteResource(id, spriteSheetResource, spriteSheetIndex);
		m_spriteResourceDatabase.insert(std::pair<size_t, SpriteResource*>(idHash, spriteResource));
	}
}


//-------------------------------------------------------------------------------------------------
void BSpriteGameRenderer::AddSpriteSheetResource(std::string const & spriteSheetID, std::string const & spriteSheetFilename, Vector2i const & spriteSheetSize)
{
	size_t idHash = std::hash<std::string>{}(spriteSheetID);
	auto foundResource = m_spriteSheetResourceDatabase.find(idHash);

	//Does not exist yet
	if(foundResource == m_spriteSheetResourceDatabase.end())
	{
		SpriteSheetResource * spriteSheetResource = new SpriteSheetResource(spriteSheetID, spriteSheetFilename, spriteSheetSize);
		m_spriteSheetResourceDatabase.insert(std::pair<size_t, SpriteSheetResource*>(idHash, spriteSheetResource));
	}
}


//-------------------------------------------------------------------------------------------------
bool BSpriteGameRenderer::AddLayerEffect(int layer, eMaterialEffect const & effect)
{
	//Return true if setting the layer effect was successful
	auto foundLayer = m_spriteLayers.find(layer);
	if(foundLayer != m_spriteLayers.end())
	{
		foundLayer->second->AddEffect(effect);
		return true;
	}
	else
	{
		return false;
	}
}


//-------------------------------------------------------------------------------------------------
bool BSpriteGameRenderer::ClearLayerEffects(int layer)
{
	//Return true if clearing the layer effects was successful
	auto foundLayer = m_spriteLayers.find(layer);
	if(foundLayer != m_spriteLayers.end())
	{
		foundLayer->second->RemoveAllEffects();
		return true;
	}
	else
	{
		return false;
	}
}


//-------------------------------------------------------------------------------------------------
void BSpriteGameRenderer::ExportSpriteDatabase(std::string const & filename)
{
	XMLNode databaseRoot = XMLNode::createXMLTopNode("SpriteResources");
	std::string outFile = Stringf("Data/Sprites/%s.Sprite.xml", filename.c_str());
	for(auto spriteResourceIter : m_spriteResourceDatabase)
	{
		SpriteResource * resource = spriteResourceIter.second;
		XMLNode resourceNode = databaseRoot.addChild("SpriteResource");
		resourceNode.addAttribute("id", resource->m_id);
		resourceNode.addAttribute("filename", resource->m_filename);
	}
	databaseRoot.writeToFile(outFile.c_str());
}


//-------------------------------------------------------------------------------------------------
SpriteLayer * BSpriteGameRenderer::CreateOrGetLayer(int layer) const
{
	auto layerFound = m_spriteLayers.find(layer);
	if(layerFound != m_spriteLayers.end())
	{
		return layerFound->second;
	}
	else
	{
		SpriteLayer * spriteLayer = new SpriteLayer(layer);
		m_spriteLayers.insert(std::pair<int, SpriteLayer*>(layer, spriteLayer));
		return spriteLayer;
	}
}


//-------------------------------------------------------------------------------------------------
SpriteResource const * BSpriteGameRenderer::GetSpriteResource(std::string const & spriteID) const
{
	size_t spriteIDHash = std::hash<std::string>{}(spriteID);
	auto spriteResourceFound = m_spriteResourceDatabase.find(spriteIDHash);
	if(spriteResourceFound == m_spriteResourceDatabase.end())
	{
		ERROR_AND_DIE("Sprite resource not found.");
	}
	return spriteResourceFound->second;
}


//-------------------------------------------------------------------------------------------------
SpriteSheetResource const * BSpriteGameRenderer::GetSpriteSheetResource(std::string const & spriteSheetID) const
{
	size_t spriteSheetIDHash = std::hash<std::string>{}(spriteSheetID);
	auto spriteSheetResourceFound = m_spriteSheetResourceDatabase.find(spriteSheetIDHash);
	if(spriteSheetResourceFound == m_spriteSheetResourceDatabase.end())
	{
		ERROR_AND_DIE("Sprite Sheet resource not found.");
	}
	return spriteSheetResourceFound->second;
}


//-------------------------------------------------------------------------------------------------
float BSpriteGameRenderer::GetVirtualSize() const
{
	return m_virtualSize;
}


//-------------------------------------------------------------------------------------------------
float BSpriteGameRenderer::GetImportSize() const
{
	return m_importSize;
}


//-------------------------------------------------------------------------------------------------
float BSpriteGameRenderer::GetAspectRatio() const
{
	return m_aspectRatio;
}


//-------------------------------------------------------------------------------------------------
Vector2f BSpriteGameRenderer::GetVirtualScreen() const
{
	return m_virtualScreen;
}


//-------------------------------------------------------------------------------------------------
Matrix4f BSpriteGameRenderer::GetView() const
{
	return m_viewMatrix;
}


//-------------------------------------------------------------------------------------------------
bool BSpriteGameRenderer::IsSpriteOnScreen(Sprite const * sprite) const
{
	//If you ignore the view matrix, never cull!
	if(sprite->m_ignoreView)
	{
		return true;
	}

	Vector2f camPosition = m_viewMatrix.GetWorldPositionFromViewNoScale().XY();
	float spriteDistance = DistanceBetweenPoints(sprite->m_position, camPosition);
	float maxDistance = m_virtualSize + sprite->GetRadiusScaled();

	return spriteDistance < maxDistance;
}


//-------------------------------------------------------------------------------------------------
void BSpriteGameRenderer::ResetVirtualScreen()
{
	if(m_aspectRatio > 1.f)
	{
		m_virtualScreen.x = m_virtualSize * m_aspectRatio;
		m_virtualScreen.y = m_virtualSize;
	}
	else
	{
		m_virtualScreen.x = m_virtualSize;
		m_virtualScreen.y = m_virtualSize / m_aspectRatio;
	}
	m_projectionMatrix.MakeOrthonormal(m_virtualScreen.x, m_virtualScreen.y, -1, 1);
}


//-------------------------------------------------------------------------------------------------
void BSpriteGameRenderer::SetVirtualSize(float size)
{
	m_virtualSize = size;
	ResetVirtualScreen();
}


//-------------------------------------------------------------------------------------------------
void BSpriteGameRenderer::SetImportSize(float size)
{
	m_importSize = size;
}


//-------------------------------------------------------------------------------------------------
void BSpriteGameRenderer::SetAspectRatio(float width, float height)
{
	m_aspectRatio = width / height;
	ResetVirtualScreen();
}


//-------------------------------------------------------------------------------------------------
void BSpriteGameRenderer::SetClearColor(Color const & color)
{
	m_clearColor = color;
}


//-------------------------------------------------------------------------------------------------
bool BSpriteGameRenderer::SetLayerEnabled(int layer, bool isEnabled)
{
	//Return true if setting layer was successful
	auto foundLayer = m_spriteLayers.find(layer);
	if(foundLayer != m_spriteLayers.end())
	{
		foundLayer->second->SetEnabled(isEnabled);
		return true;
	}
	else
	{
		return false;
	}
}


//-------------------------------------------------------------------------------------------------
void BSpriteGameRenderer::SetView(Matrix4f const & view)
{
	m_viewMatrix = view;
}