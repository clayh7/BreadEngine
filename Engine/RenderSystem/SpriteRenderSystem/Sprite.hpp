#pragma once
#include <string>
#include "Engine/Math/Vector2f.hpp"
#include "Engine/RenderSystem/Color.hpp"


//-------------------------------------------------------------------------------------------------
class Material;
class Mesh;
class MeshBuilder;
class BSpriteGameRenderer;
class SpriteResource;
class Texture;


//-------------------------------------------------------------------------------------------------
class Sprite
{
	//-------------------------------------------------------------------------------------------------
	// Members
	//-------------------------------------------------------------------------------------------------
private:
	bool m_enabled;
	int m_layerID;
	SpriteResource const * m_spriteResource;
	BSpriteGameRenderer const * m_renderer; //I wanted this to be const, but I can't because of the deconstructor
	std::string m_spriteID;

public:
	Vector2f m_position;
	Vector2f m_pivot;
	Vector2f m_scale;
	float m_rotationDegrees;
	Color m_color;
	bool m_ignoreView;

	Sprite * m_nextSprite;
	Sprite * m_prevSprite;

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	// Every time a sprite is created, it is added to the SpriteGameRenderer.
	// We don't want to accidentally make a new duplicate and add it to the SpriteGameRenderer.
	// Therefore, not copy constructors.
	Sprite(const Sprite&) = delete;
	Sprite& operator= (const Sprite&) = delete;
	Sprite(std::string const & spriteID, int layer = 0, bool ignoreView = false);
	~Sprite();
	void Render() const;
	void ConstructMesh(Mesh * out_updateMesh, MeshBuilder & meshBuilder) const;

	Material * GetMaterial() const;
	Texture const * GetTexture() const;
	float GetWidth() const;
	float GetHeight() const;
	Vector2f GetPivot() const;
	Vector2f GetScale() const;
	float GetRadiusScaled() const;
	std::string GetID() const;
	bool IsEnabled() const;

	void SetEnabled(bool isEnabled);
	void SetScale(Vector2f const & scale);
	void SetColor(Color const & color);
	void SetPivot(Vector2f const & pivot);
	void SetPosition(Vector2f const & position);
	void SetRotation(float degrees);
	void SetRenderer(BSpriteGameRenderer const * renderer);
	void SetID(std::string const & spriteID);
	void SetLayer(int layerID);
};