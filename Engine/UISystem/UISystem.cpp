#include "Engine/UISystem/UISystem.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/DebugSystem/BProfiler.hpp"
#include "Engine/EventSystem/BEventSystem.hpp"
#include "Engine/InputSystem/BMouseKeyboard.hpp"
#include "Engine/Math/Matrix4f.hpp"
#include "Engine/Math/Vector2f.hpp"
#include "Engine/Math/Vector3f.hpp"
#include "Engine/RenderSystem/MeshBuilder.hpp"
#include "Engine/UISystem/UIWidget.hpp"
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/Utils/XMLUtils.hpp"
#include "Engine/Utils/FileUtils.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteGameRenderer.hpp"
#include "Engine/RenderSystem/BRenderSystem.hpp"
#include "Engine/RenderSystem/Camera3D.hpp"

#include "Engine/UISystem/UIButton.hpp"
#include "Engine/UISystem/UIBox.hpp"
#include "Engine/UISystem/UIContainer.hpp"
#include "Engine/UISystem/UIItem.hpp"
#include "Engine/UISystem/UILabel.hpp"
#include "Engine/UISystem/UIProgressBar.hpp"
#include "Engine/UISystem/UISprite.hpp"
#include "Engine/UISystem/UITextField.hpp"


//-------------------------------------------------------------------------------------------------
STATIC char const * UISystem::DEFAULT_NAME = "";
STATIC char const * UISystem::UI_SKIN = "UISkin";
STATIC UISystem * UISystem::s_UISystem = nullptr;


//-------------------------------------------------------------------------------------------------
UIWidgetRegistration::UIWidgetRegistration(std::string const & widgetName, WidgetCreationFunc * creationFunc)
{
	UISystem::RegisterWidget(widgetName, creationFunc);
}


//-------------------------------------------------------------------------------------------------
//My compiler does not compile these classes, and therefore they are not being added
//So I will explicitly use them here, so that they will all forcibly be compiled
//void WakeUpUISystem()
//{
//	static bool doOnce = true;
//	if(doOnce)
//	{
//		UIWidget();
//		UIButton();
//		UIBox();
//		UIContainer();
//		UIItem();
//		UILabel();
//		UIProgressBar();
//		UISprite();
//		UITextField();
//		doOnce = false;
//	}
//}


//-------------------------------------------------------------------------------------------------
STATIC void UISystem::Startup()
{
	if(!s_UISystem)
	{
		s_UISystem = new UISystem();
		//s_UISystem->UpdateUISpriteRenderer();
		s_UISystem->LoadUIFromXML();
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void UISystem::Shutdown()
{
	delete s_UISystem;
	s_UISystem = nullptr;
}


//-------------------------------------------------------------------------------------------------
void UISystem::Update()
{
	if(s_UISystem)
	{
		s_UISystem->UpdateSystem();
	}
}


//-------------------------------------------------------------------------------------------------
void UISystem::Render()
{
	if(s_UISystem)
	{
		s_UISystem->RenderSystem();
	}
}


//-------------------------------------------------------------------------------------------------
STATIC UISystem * UISystem::GetSystem()
{
	return s_UISystem;
}


//-------------------------------------------------------------------------------------------------
STATIC Vector2f UISystem::ClipToUISystemPosition(Vector3f const & clipVector)
{
	float x = RangeMap(-1.f, 1.f, clipVector.x, 0.f, (float)VIRTUAL_WIDTH);
	float y = RangeMap(-1.f, 1.f, clipVector.y, 0.f, (float)VIRTUAL_HEIGHT);
	return Vector2f(x, y);
}


//-------------------------------------------------------------------------------------------------
STATIC void UISystem::RegisterWidget(std::string const & widgetName, WidgetCreationFunc * creationFunc)
{
	//Create it if it does not exist
	if(!s_UISystem)
	{
		Startup();
	}

	if(s_UISystem && !s_UISystem->m_registeredWidgets)
	{
		s_UISystem->m_registeredWidgets = new std::map<size_t, WidgetCreationFunc *>();
	}

	if(s_UISystem && s_UISystem->m_registeredWidgets)
	{
		size_t widgetNameHash = std::hash<std::string>{}(widgetName);
		s_UISystem->m_registeredWidgets->insert(std::pair<size_t, WidgetCreationFunc*>(widgetNameHash, creationFunc));
	}
}


//-------------------------------------------------------------------------------------------------
STATIC UIWidget * UISystem::CreateWidgetFromName(std::string const & name, XMLNode const & data)
{
	if(s_UISystem && s_UISystem->m_registeredWidgets)
	{
		size_t widgetNameHash = std::hash<std::string>{}(name);
		auto foundRegisteredWidget = s_UISystem->m_registeredWidgets->find(widgetNameHash);
		if(foundRegisteredWidget != s_UISystem->m_registeredWidgets->end())
		{
			return (foundRegisteredWidget->second)(data);
		}
	}
	return nullptr;
}


//-------------------------------------------------------------------------------------------------
STATIC Vector2f UISystem::GetCursorUIPosition()
{
	Vector2f cursorPosition = BMouseKeyboard::GetMousePosition(true);
	Vector3f cursorClipPosition;// = ScreenToClipPosition( cursorPosition );
	Vector2i windowDimensions = GetWindowDimensions();
	float x = RangeMap(0.f, (float)windowDimensions.x, cursorPosition.x, -1.f, 1.f);
	float y = RangeMap(0.f, (float)windowDimensions.y, cursorPosition.y, -1.f, 1.f);
	cursorClipPosition = Vector3f(x, y, 0.f);
	//Matrix4f projectionMatrix = g_RenderSystem->GetActiveCamera( )->GetProjectionMatrix( );
	//invert projMat
	//multiply
	return ClipToUISystemPosition(cursorClipPosition);
}


//-------------------------------------------------------------------------------------------------
UISystem::UISystem()
	: m_registeredWidgets(nullptr)
	, m_root(new UIWidget())
	, m_highlightedWidget(nullptr)
	, m_selectedWidget(nullptr)
	, m_heldItem(nullptr)
{
	//#TODO: Set root to have size of the virtual dimensions
	//#TODO: Maybe also set the aspect ratio some how?
	Vector2i windowDimensions = GetWindowDimensions();
	m_root->SetProperty(UIWidget::PROPERTY_WIDTH, (float)VIRTUAL_WIDTH);
	m_root->SetProperty(UIWidget::PROPERTY_HEIGHT, (float)VIRTUAL_HEIGHT);
	BEventSystem::RegisterEvent(BMouseKeyboard::EVENT_MOUSE_DOWN, this, &UISystem::OnMouseDown);
	BEventSystem::RegisterEvent(BMouseKeyboard::EVENT_MOUSE_UP, this, &UISystem::OnMouseUp);
}


//-------------------------------------------------------------------------------------------------
UISystem::~UISystem()
{
	delete m_root;
	m_root = nullptr;

	delete m_registeredWidgets;
	m_registeredWidgets = nullptr;
}


//-------------------------------------------------------------------------------------------------
//void UISystem::UpdateUISpriteRenderer()
//{
//	Vector2i windowDimensions = GetWindowDimensions();
//	m_uiSpriteRenderer->SetAspectRatio((float)windowDimensions.x, (float)windowDimensions.y);
//	m_uiSpriteRenderer->SetImportSize(900.f);
//	m_uiSpriteRenderer->SetVirtualSize((float)windowDimensions.y);
//	m_uiSpriteRenderer->SetClearColor(Color::WHITE);
//	Matrix4f view;
//	view.MakeView(-Vector3f((float)windowDimensions.x / 2.f, (float)windowDimensions.y / 2.f, 0.f));
//	m_uiSpriteRenderer->SetView(view);
//}


//-------------------------------------------------------------------------------------------------
void UISystem::UpdateSystem()
{
	//Update root dimensions to match screen
	//Only update if they've changed so we don't dirty everything every frame
	Vector2f windowDimensions = (Vector2f)GetWindowDimensions();
	Vector2f rootSize = m_root->GetSize();
	if(rootSize != windowDimensions)
	{
		//UpdateUISpriteRenderer( );
		//m_root->SetProperty( UIWidget::PROPERTY_WIDTH, windowDimensions.x );
		//m_root->SetProperty( UIWidget::PROPERTY_HEIGHT, windowDimensions.y );
	}

	//Update entire UI tree
	m_root->Update();

	//No highlighting widgets if you're holding an item
	if(!IsHoldingAnItem())
	{
		//Highlight widget
		UIWidget * currentHighlightedWidget = GetWidgetUnderCursor();
		if(m_highlightedWidget != currentHighlightedWidget)
		{
			if(m_highlightedWidget)
			{
				m_highlightedWidget->Unhighlight();
			}
			m_highlightedWidget = currentHighlightedWidget;
			if(m_highlightedWidget)
			{
				m_highlightedWidget->Highlight();
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------
void UISystem::RenderSystem() const
{
	m_root->Render();
	if(m_heldItem)
	{
		m_heldItem->Render();
	}
}


//-------------------------------------------------------------------------------------------------
UIWidget * UISystem::CreateWidget()
{
	UIWidget * widget = new UIWidget();
	m_root->AddChild(widget);
	return widget;
}


//-------------------------------------------------------------------------------------------------
void UISystem::RemoveWidget(UIWidget * widget)
{
	//Sorry, you can't remove the root
	if(widget == m_root)
	{
		return;
	}

	UIWidget * parent = widget->GetParent();
	parent->RemoveChild(widget);
	if(widget == m_highlightedWidget)
	{
		m_highlightedWidget = nullptr;
	}
	if(widget == m_selectedWidget)
	{
		m_selectedWidget = nullptr;
	}
	if(widget == m_heldItem)
	{
		m_heldItem = nullptr;
	}
	delete widget;
}


//-------------------------------------------------------------------------------------------------
UIWidget * UISystem::GetHighlightedWidget() const
{
	return m_highlightedWidget;
}


//-------------------------------------------------------------------------------------------------
UIWidget * UISystem::GetSelectedWidget() const
{
	return m_selectedWidget;
}


//-------------------------------------------------------------------------------------------------
UIItem * UISystem::GetHeldItem() const
{
	return m_heldItem;
}


//-------------------------------------------------------------------------------------------------
bool UISystem::IsHoldingAnItem() const
{
	return GetHeldItem() != nullptr;
}


//-------------------------------------------------------------------------------------------------
//SpriteGameRenderer const * UISystem::GetRenderer() const
//{
//	return m_uiSpriteRenderer;
//}


//-------------------------------------------------------------------------------------------------
UIWidget * UISystem::GetWidgetUnderCursor() const
{
	Vector2f cursorUIPosition = GetCursorUIPosition();
	UIWidget * widget = m_root->FindWidgetUnderPosition(cursorUIPosition);
	if(widget == nullptr)
	{
		return m_root;
	}
	else
	{
		return widget;
	}
}


//-------------------------------------------------------------------------------------------------
void UISystem::SetHeldItem(UIItem * item)
{
	m_heldItem = item;
}


//-------------------------------------------------------------------------------------------------
void UISystem::Select(UIWidget * selectedWidget)
{
	//UIWidget * lastSelected = m_selectedWidget;
	//m_selectedWidget = selectedWidget;
	if(m_selectedWidget)
	{
		m_selectedWidget->Dirty();
	}
	m_selectedWidget = selectedWidget;
}


//-------------------------------------------------------------------------------------------------
void UISystem::LoadUIFromXML()
{
	std::vector<std::string> uiFiles = EnumerateFilesInFolder("Data/UI/", "*.UI.xml");

	//First: Load skins
	for(std::string const & uiFile : uiFiles)
	{
		//Have to get the first child to get into the XML structure
		XMLNode uiLayoutXML = XMLNode::openFileHelper(uiFile.c_str()).getChildNode(0);
		int skinNodeCount = uiLayoutXML.nChildNode(UI_SKIN);
		for(int skinIndex = 0; skinIndex < skinNodeCount; ++skinIndex)
		{
			XMLNode skinNode = uiLayoutXML.getChildNode(UI_SKIN, skinIndex);
			AddSkinFromXML(skinNode);
		}
	}

	//Second: Load widgets
	for(std::string const & uiFile : uiFiles)
	{
		//Have to get the first child to get into the XML structure
		XMLNode uiLayoutXML = XMLNode::openFileHelper(uiFile.c_str()).getChildNode(0);
		int childNodeCount = uiLayoutXML.nChildNode();
		for(int childIndex = 0; childIndex < childNodeCount; ++childIndex)
		{
			XMLNode widgetNode = uiLayoutXML.getChildNode(childIndex);
			AddWidgetFromXML(widgetNode);
		}
	}
}


//-------------------------------------------------------------------------------------------------
void UISystem::AddSkinFromXML(XMLNode const & node)
{
	//Hash the name and store it in our skin map
	std::string skinName = ReadXMLAttribute(node, "name", "");
	size_t skinHash = std::hash<std::string>{}(skinName);

	//Add to skin dictionary
	m_skins.insert(std::pair<size_t, XMLNode>(skinHash, node));
}


//-------------------------------------------------------------------------------------------------
void UISystem::AddWidgetFromXML(XMLNode const & node)
{
	std::string childName = node.getName();
	UIWidget * childWidget = UISystem::CreateWidgetFromName(childName, node);
	if(childWidget)
	{
		m_root->AddChild(childWidget);
	}
}


//-------------------------------------------------------------------------------------------------
void UISystem::OnMouseDown(NamedProperties & params)
{
	eMouseButton button;
	params.Get(BMouseKeyboard::PARAM_MOUSE_BUTTON, button);

	//Pressed widget
	if(button == eMouseButton_LEFT)
	{
		if(m_highlightedWidget)
		{
			m_highlightedWidget->Press();
			Select(m_highlightedWidget);
		}
	}
}


//-------------------------------------------------------------------------------------------------
void UISystem::OnMouseUp(NamedProperties & params)
{
	eMouseButton button;
	params.Get(BMouseKeyboard::PARAM_MOUSE_BUTTON, button);

	//Released widget
	if(button == eMouseButton_LEFT)
	{
		if(m_highlightedWidget)
		{
			m_highlightedWidget->Release();
		}
	}
}
