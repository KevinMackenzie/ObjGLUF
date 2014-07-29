#pragma once

#include "ObjGLUF.h"

//--------------------------------------------------------------------------------------
// Macro definitions
//--------------------------------------------------------------------------------------
#define GT_CENTER 0x00000001
#define GT_VCENTER 0X0000004

#define GR_SUCCESS 1
#define GR_FAILURE 0



enum GLUF_INPUT_TYPE
{
	GLUF_INPUT_TYPE_MB = 0,
	GLUF_INPUT_TYPE_CURSOR_POS,
	GLUF_INPUT_TYPE_SCROLL,
	GLUF_INPUT_TYPE_KEY//dont' support joysticks yet
};

typedef void(*PCALLBACKGLUFGUIEVENT)(GLUF_INPUT_TYPE msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4);


//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
class GLUFDialogResourceManager;
class GLUFControl;
class GLUFButton;
class GLUFStatic;
class GLUFCheckBox;
class GLUFRadioButton;
class GLUFComboBox;
class GLUFSlider;
class GLUFEditBox;
class GLUFListBox;
class GLUFScrollBar;
class GLUFElement;
class GLUFFont;
struct GLUFElementHolder;
struct GLUFTextureNode;
struct GLUFFontNode;



void OBJGLUF_API GLUFInitGui();

typedef std::shared_ptr<GLUFFont> GLUFFontPtr;

GLUFFontPtr GLUFLoadFont(char* rawData, uint64_t rawSize);


struct GLUFRect
{
	long top, bottom, left, right;
};

struct GLUFPoint
{
	long x, y;
};

//--------------------------------------------------------------------------------------
// Enums for pre-defined control types
//--------------------------------------------------------------------------------------
enum GLUF_CONTROL_TYPE
{
	GLUF_CONTROL_BUTTON,
	GLUF_CONTROL_STATIC,
	GLUF_CONTROL_CHECKBOX,
	GLUF_CONTROL_RADIOBUTTON,
	GLUF_CONTROL_COMBOBOX,
	GLUF_CONTROL_SLIDER,
	GLUF_CONTROL_EDITBOX,
	GLUF_CONTROL_IMEEDITBOX,
	GLUF_CONTROL_LISTBOX,
	GLUF_CONTROL_SCROLLBAR,
};

enum GLUF_CONTROL_STATE
{
	GLUF_STATE_NORMAL = 0,
	GLUF_STATE_DISABLED,
	GLUF_STATE_HIDDEN,
	GLUF_STATE_FOCUS,
	GLUF_STATE_MOUSEOVER,
	GLUF_STATE_PRESSED,
};


#define MAX_CONTROL_STATES 6

struct GLUFBlendColor
{
	void        Init(Color defaultColor, Color disabledColor = Color(200, 128, 128, 128), Color hiddenColor = Color());
	void        Blend(GLUF_CONTROL_STATE iState, float fElapsedTime, float fRate = 0.7f);

	Color		States[MAX_CONTROL_STATES]; // Modulate colors for all possible control states
	Color		Current;
};

typedef unsigned long GLUFResult;

//-----------------------------------------------------------------------------
// Contains all the display tweakables for a sub-control
//-----------------------------------------------------------------------------
class GLUFElement
{
public:
	void    SetTexture(GLUFTexturePtr iTexture, GLUFRect* prcTexture, Color defaultTextureColor = Color(255, 255, 255, 255));
	void    SetFont(GLUFFontPtr iFont, Color defaultFontColor = Color(255, 255, 255, 255), unsigned int dwTextFormat = GT_CENTER | GT_VCENTER);

	void    Refresh();

	GLUFTexturePtr iTexture;			// Index of the texture for this Element 
	GLUFFontPtr iFont;					// Index of the font for this Element
	unsigned int dwTextFormat;			// The format of the text

	GLUFRect rcTexture;					// Bounding Rect of this element on the composite texture

	GLUFBlendColor TextureColor;
	GLUFBlendColor FontColor;
};


//-----------------------------------------------------------------------------
// All controls must be assigned to a dialog, which handles
// input and rendering for the controls.
//-----------------------------------------------------------------------------
class GLUFDialog
{
	friend class GLUFDialogResourceManager;

public:
	GLUFDialog();
	~GLUFDialog();
	
	// Need to call this now
	void                Init(GLUFDialogResourceManager* pManager, bool bRegisterDialog = true);
	void                Init(GLUFDialogResourceManager* pManager, bool bRegisterDialog, GLUFTexturePtr texture);

	// message handler (this can handle any message type, where GLUF_INPUT_TYPE is what command it is actually responding from
	bool                MsgProc(GLUF_INPUT_TYPE msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4);

	// Control creation
	GLUFResult             AddStatic(int ID, const char* strText, int x, int y, int width, int height, bool bIsDefault = false,
								GLUFStatic** ppCreated = NULL);
	GLUFResult             AddButton(int ID, const char* strText, int x, int y, int width, int height, unsigned int nHotkey = 0,
								bool bIsDefault = false, GLUFButton** ppCreated = NULL);
	GLUFResult             AddCheckBox(int ID, const char* strText, int x, int y, int width, int height, bool bChecked = false,
								unsigned int nHotkey = 0, bool bIsDefault = false, GLUFCheckBox** ppCreated = NULL);
	GLUFResult             AddRadioButton(int ID, unsigned int nButtonGroup, const char* strText, int x, int y, int width,
								int height, bool bChecked = false, unsigned int nHotkey = 0, bool bIsDefault = false,
								GLUFRadioButton** ppCreated = NULL);
	GLUFResult             AddComboBox(int ID, int x, int y, int width, int height, unsigned int nHotKey = 0, bool bIsDefault =
								false, GLUFComboBox** ppCreated = NULL);
	GLUFResult             AddSlider(int ID, int x, int y, int width, int height, int min = 0, int max = 100, int value = 50,
								bool bIsDefault = false, GLUFSlider** ppCreated = NULL);
	GLUFResult             AddEditBox(int ID, const char* strText, int x, int y, int width, int height, bool bIsDefault =
								false, GLUFEditBox** ppCreated = NULL);
	GLUFResult             AddListBox(int ID, int x, int y, int width, int height, unsigned long dwStyle = 0,
								GLUFListBox** ppCreated = NULL);
	GLUFResult             AddControl(GLUFControl* pControl);
	GLUFResult             InitControl(GLUFControl* pControl);

	// Control retrieval
	GLUFStatic* GetStatic(int ID)
	{
		return (GLUFStatic*)GetControl(ID, GLUF_CONTROL_STATIC);
	}
	GLUFButton* GetButton(int ID)
	{
		return (GLUFButton*)GetControl(ID, GLUF_CONTROL_BUTTON);
	}
	GLUFCheckBox* GetCheckBox(int ID)
	{
		return (GLUFCheckBox*)GetControl(ID, GLUF_CONTROL_CHECKBOX);
	}
	GLUFRadioButton* GetRadioButton(int ID)
	{
		return (GLUFRadioButton*)GetControl(ID, GLUF_CONTROL_RADIOBUTTON);
	}
	GLUFComboBox* GetComboBox(int ID)
	{
		return (GLUFComboBox*)GetControl(ID, GLUF_CONTROL_COMBOBOX);
	}
	GLUFSlider* GetSlider(int ID)
	{
		return (GLUFSlider*)GetControl(ID, GLUF_CONTROL_SLIDER);
	}
	GLUFEditBox* GetEditBox(int ID)
	{
		return (GLUFEditBox*)GetControl(ID, GLUF_CONTROL_EDITBOX);
	}
	GLUFListBox* GetListBox(int ID)
	{
		return (GLUFListBox*)GetControl(ID, GLUF_CONTROL_LISTBOX);
	}

	GLUFControl* GetControl(int ID);
	GLUFControl* GetControl(int ID, unsigned int nControlType);
	GLUFControl* GetControlAtPoint(GLUFPoint pt);

	bool                GetControlEnabled(int ID);
	void                SetControlEnabled(int ID, bool bEnabled);

	void                ClearRadioButtonGroup(unsigned int nGroup);
	void                ClearComboBox(int ID);

	// Access the default display Elements used when adding new controls
	GLUFResult          SetDefaultElement(unsigned int nControlType, unsigned int iElement, GLUFElement* pElement);
	GLUFElement*		GetDefaultElement(unsigned int nControlType, unsigned int iElement);

	// Methods called by controls
	void                SendEvent(unsigned int nEvent, bool bTriggeredByUser, GLUFControl* pControl);
	void                RequestFocus(GLUFControl* pControl);

	// Render helpers
	GLUFResult          DrawGLUFRect(GLUFRect* pGLUFRect, Color color);
	GLUFResult          DrawPolyLine(GLUFPoint* apPoints, unsigned int nNumPoints, Color color);
	GLUFResult          DrawSprite(GLUFElement* pElement, GLUFRect* prcDest, float fDepth);
	GLUFResult          CalcTextGLUFRect(const char* strText, GLUFElement* pElement, GLUFRect* prcDest, int nCount = -1);
	GLUFResult          GLDrawText(const char* strText, GLUFElement* pElement, GLUFRect* prcDest, bool bShadow = false,
		int nCount = -1, bool bCenter = false);

	// Attributes
	bool                GetVisible()
	{
		return m_bVisible;
	}
	void                SetVisible(bool bVisible)
	{
		m_bVisible = bVisible;
	}
	bool                GetMinimized()
	{
		return m_bMinimized;
	}
	void                SetMinimized(bool bMinimized)
	{
		m_bMinimized = bMinimized;
	}
	void                SetBackgroundColors(Color colorAllCorners)
	{
		SetBackgroundColors(colorAllCorners, colorAllCorners, colorAllCorners, colorAllCorners);
	}
	void                SetBackgroundColors(Color colorTopLeft, Color colorTopRight, Color colorBottomLeft,
		Color colorBottomRight);
	void                EnableCaption(bool bEnable)
	{
		m_bCaption = bEnable;
	}
	int                 GetCaptionHeight() const
	{
		return m_nCaptionHeight;
	}
	void                SetCaptionHeight(int nHeight)
	{
		m_nCaptionHeight = nHeight;
	}
	void                SetCaptionText(const char* pwszText)
	{
		m_wszCaption = pwszText;
	}
	void                GetLocation(GLUFPoint& Pt) const
	{
		Pt.x = m_x; Pt.y = m_y;
	}
	void                SetLocation(int x, int y)
	{
		m_x = x; m_y = y;
	}
	void                SetSize(int width, int height)
	{
		m_width = width; m_height = height;
	}
	int                 GetWidth()
	{
		return m_width;
	}
	int                 GetHeight()
	{
		return m_height;
	}

	static void			SetRefreshTime(float fTime)
	{
		s_fTimeRefresh = fTime;
	}

	static GLUFControl* GetNextControl(GLUFControl* pControl);
	static GLUFControl* GetPrevControl(GLUFControl* pControl);

	void                RemoveControl(int ID);
	void                RemoveAllControls();

	// Sets the callback used to notify the app of control events
	void                SetCallback(PCALLBACKGLUFGUIEVENT pCallback, void* pUserContext = NULL);
	void                EnableNonUserEvents(bool bEnable)
	{
		m_bNonUserEvents = bEnable;
	}
	void                EnableKeyboardInput(bool bEnable)
	{
		m_bKeyboardInput = bEnable;
	}
	void                EnableMouseInput(bool bEnable)
	{
		m_bMouseInput = bEnable;
	}
	bool                IsKeyboardInputEnabled() const
	{
		return m_bKeyboardInput;
	}

	// Device state notification
	void                Refresh();
	GLUFResult          OnRender(float fElapsedTime);

	// Shared resource access. Indexed fonts and textures are shared among
	// all the controls.
	GLUFResult          SetFont(GLUFFontPtr font, long height, long weight);
	GLUFFontNode*		GetFont(unsigned int index);

	GLUFResult          SetTexture(GLUFTexturePtr texture);
	GLUFTextureNode* GetTexture(unsigned int index);

	GLUFDialogResourceManager* GetManager()
	{
		return m_pManager;
	}

	static void WINAPI  ClearFocus();
	void                FocusDefaultControl();

	bool m_bNonUserEvents;
	bool m_bKeyboardInput;
	bool m_bMouseInput;

private:
	int m_nDefaultControlID;

	HRESULT             OnRender9(float fElapsedTime);
	HRESULT             OnRender10(float fElapsedTime);
	HRESULT             OnRender11(float fElapsedTime);

	static double s_fTimeRefresh;
	double m_fTimeLastRefresh;

	// Initialize default Elements
	void                InitDefaultElements();

	// Windows message handlers
	void                OnMouseMove(GLUFPoint pt);
	void                OnMouseUp(GLUFPoint pt);

	void                SetNextDialog(GLUFDialog* pNextDialog);

	// Control events
	bool                OnCycleFocus(bool bForward);

	static GLUFControl* s_pControlFocus;        // The control which has focus
	static GLUFControl* s_pControlPressed;      // The control currently pressed

	GLUFControl* m_pControlMouseOver;           // The control which is hovered over

	bool m_bVisible;
	bool m_bCaption;
	bool m_bMinimized;
	bool m_bDrag;
	WCHAR               m_wszCaption[256];

	int m_x;
	int m_y;
	int m_width;
	int m_height;
	int m_nCaptionHeight;

	Color m_colorTopLeft;
	Color m_colorTopRight;
	Color m_colorBottomLeft;
	Color m_colorBottomRight;

	GLUFDialogResourceManager* m_pManager;
	PCALLBACKGLUFGUIEVENT m_pCallbackEvent;
	void* m_pCallbackEventUserContext;

	std::vector <int> m_Textures;   // Index into m_TextureCache;
	std::vector <int> m_Fonts;      // Index into m_FontCache;

	std::vector <GLUFControl*> m_Controls;
	std::vector <GLUFElementHolder*> m_DefaultElements;

	GLUFElement m_CapElement;  // Element for the caption

	GLUFDialog* m_pNextDialog;
	GLUFDialog* m_pPrevDialog;
};


//--------------------------------------------------------------------------------------
// Structs for shared resources
//--------------------------------------------------------------------------------------
struct GLUFTextureNode
{
	shared_ptr<ResHandle> m_pTextureResourceElement;
};

struct GLUFFontNode
{
	WCHAR strFace[MAX_PATH];
	LONG nHeight;
	LONG nWeight;
	shared_ptr<ResHandle> m_pFontType;
};

struct GLUFSpriteVertex
{
	glm::vec3 vPos;
	Color vColor;
	glm::vec2 vTex;
};

//-----------------------------------------------------------------------------
// Manages shared resources of dialogs
//-----------------------------------------------------------------------------
class GLUFDialogResourceManager
{
public:
	GLUFDialogResourceManager();
	~GLUFDialogResourceManager();

	bool    MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// D3D9 specific
	/*HRESULT OnD3D9CreateDevice( LPDIGLUFRect3DDEVICE9 pd3dDevice );
	HRESULT OnD3D9ResetDevice();
	void    OnD3D9LostDevice();
	void    OnD3D9DestroyDevice();
	IDiGLUFRect3DDevice9* GetD3D9Device()
	{
	return m_pd3d9Device;
	}*/

	// D3D11 specific
	//HRESULT OnD3D11CreateDevice( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3d11DeviceContext );
	//HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc );
	//void    OnD3D11ReleasingSwapChain();
	//void    OnD3D11DestroyDevice();
	//void    StoreD3D11State( ID3D11DeviceContext* pd3dImmediateContext );
	//void    RestoreD3D11State( ID3D11DeviceContext* pd3dImmediateContext );
	void    ApplyRenderUI();
	void	ApplyRenderUIUntex();
	void	BeginSprites();
	void	EndSprites();
	/*ID3D11Device* GetD3D11Device()
	{
	return m_pd3d11Device;
	}
	ID3D11DeviceContext* GetD3D11DeviceContext()
	{
	return m_pd3d11DeviceContext;
	}*/

	GLUFFontNode* GetFontNode(int iIndex)
	{
		return m_FontCache[iIndex];
	};
	GLUFTextureNode* GetTextureNode(int iIndex)
	{
		return m_TextureCache[iIndex];
	};

	int     AddFont(LPCWSTR strFaceName, LONG height, LONG weight);
	int     AddTexture(shared_ptr<ResHandle> pFont);

	bool    RegisterDialog(GLUFDialog* pDialog);
	void    UnregisterDialog(GLUFDialog* pDialog);
	void    EnableKeyboardInputForAllDialogs();

	// Shared between all dialogs

	// D3D9
	//IDiGLUFRect3DStateBlock9* m_pStateBlock;
	//ID3DXSprite* m_pSprite;          // Sprite used for drawing

	//TODO: do i need sprites

	// D3D11
	// Shaders
	//ID3D11VertexShader* m_pVSRenderUI11;
	//ID3D11PixelShader* m_pPSRenderUI11;
	//ID3D11PixelShader* m_pPSRenderUIUntex11;
	GLProgramPtr m_pShader;

	// States
	GLbitfield* m_pDepthStencilStateUI11;
	//ID3D11RasterizerState* m_pRasterizerStateUI11;
	GLbitfield* m_pBlendStateUI11;
	GLbitfield* m_pSamplerStateUI11;

	// Stored states
	GLbitfield* m_pDepthStencilStateStored;
	//UINT m_StencilRefStored11;
	//ID3D11RasterizerState* m_pRasterizerStateStored;
	GLbitfield* m_pBlendStateStored;
	//float m_BlendFactorStored[4];
	//UINT m_SampleMaskStored11;
	GLbitfield* m_pSamplerStateStored;

	//TODO:
	//ID3D11InputLayout* m_pInputLayout;
	GLVertexArrayPtr* m_pVBScreenQuad;

	// Sprite workaround
	/*ID3D11Buffer* m_pSpriteBuffer11;
	UINT m_SpriteBufferBytes11;
	CGrowableArray<GLUFSpriteVertex> m_SpriteVertices;*/

	UINT m_nBackBufferWidth;
	UINT m_nBackBufferHeight;

	std::vector <GLUFDialog*> m_Dialogs;            // Dialogs registered

protected:
	// D3D9 specific
	/*IDiGLUFRect3DDevice9* m_pd3d9Device;
	HRESULT CreateFont9( UINT index );
	HRESULT CreateTexture9( UINT index );*/

	// D3D11 specific
	//ID3D11Device* m_pd3d11Device;
	//ID3D11DeviceContext* m_pd3d11DeviceContext;
	HRESULT CreateFont_(UINT index);
	HRESULT CreateTexture(UINT index);

	std::vector <GLUFTextureNode*> m_TextureCache;   // Shared textures
	std::vector <GLUFFontNode*> m_FontCache;         // Shared fonts
};

void BeginText();
void DrawTextGLUF(LPCWSTR strText, GLUFRect rcScreen, Color vFontColor,
	float fBBWidth, float fBBHeight, bool bCenter);
void EndText();

//-----------------------------------------------------------------------------
// Base class for controls
//-----------------------------------------------------------------------------
class GLUFControl
{
public:
	GLUFControl(GLUFDialog* pDialog = NULL);
	virtual         ~GLUFControl();

	virtual HRESULT OnInit()
	{
		return S_OK;
	}
	virtual void    Refresh();
	virtual void    Render(float fElapsedTime)
	{
	};

	// Windows message handler
	virtual bool    MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return false;
	}

	virtual bool    HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return false;
	}
	virtual bool    HandleMouse(UINT uMsg, GLUFPoint pt, WPARAM wParam, LPARAM lParam)
	{
		return false;
	}

	virtual bool    CanHaveFocus()
	{
		return false;
	}
	virtual void    OnFocusIn()
	{
		m_bHasFocus = true;
	}
	virtual void    OnFocusOut()
	{
		m_bHasFocus = false;
	}
	virtual void    OnMouseEnter()
	{
		m_bMouseOver = true;
	}
	virtual void    OnMouseLeave()
	{
		m_bMouseOver = false;
	}
	virtual void    OnHotkey()
	{
	}

	virtual BOOL    ContainsPoint(GLUFPoint pt)
	{
		return PtInGLUFRect(&m_rcBoundingBox, pt);
	}

	virtual void    SetEnabled(bool bEnabled)
	{
		m_bEnabled = bEnabled;
	}
	virtual bool    GetEnabled()
	{
		return m_bEnabled;
	}
	virtual void    SetVisible(bool bVisible)
	{
		m_bVisible = bVisible;
	}
	virtual bool    GetVisible()
	{
		return m_bVisible;
	}

	UINT            GetType() const
	{
		return m_Type;
	}

	int             GetID() const
	{
		return m_ID;
	}
	void            SetID(int ID)
	{
		m_ID = ID;
	}

	void            SetLocation(int x, int y)
	{
		m_x = x; m_y = y; UpdateGLUFRects();
	}
	void            SetSize(int width, int height)
	{
		m_width = width; m_height = height; UpdateGLUFRects();
	}

	void            SetHotkey(UINT nHotkey)
	{
		m_nHotkey = nHotkey;
	}
	UINT            GetHotkey()
	{
		return m_nHotkey;
	}

	void            SetUserData(void* pUserData)
	{
		m_pUserData = pUserData;
	}
	void* GetUserData() const
	{
		return m_pUserData;
	}

	virtual void    SetTextColor(Color Color);
	GLUFElement* GetElement(UINT iElement)
	{
		return m_Elements[iElement];
	}
	HRESULT         SetElement(UINT iElement, GLUFElement* pElement);

	bool m_bVisible;                // Shown/hidden flag
	bool m_bMouseOver;              // Mouse pointer is above control
	bool m_bHasFocus;               // Control has input focus
	bool m_bIsDefault;              // Is the default control

	// Size, scale, and positioning members
	int m_x, m_y;
	int m_width, m_height;

	// These members are set by the container
	GLUFDialog* m_pDialog;    // Parent container
	UINT m_Index;              // Index within the control list

	std::vector <GLUFElement*> m_Elements;  // All display elements

protected:
	virtual void    UpdateGLUFRects();

	int m_ID;                 // ID number
	GLUF_CONTROL_TYPE m_Type;  // Control type, set once in constructor  
	UINT m_nHotkey;            // Virtual key code for this control's hotkey
	void* m_pUserData;         // Data associated with this control that is set by user.

	bool m_bEnabled;           // Enabled/disabled flag

	GLUFRect m_rcBoundingBox;      // GLUFRectangle defining the active region of the control
};


//-----------------------------------------------------------------------------
// Contains all the display information for a given control type
//-----------------------------------------------------------------------------
struct GLUFElementHolder
{
	UINT nControlType;
	UINT iElement;

	GLUFElement Element;
};


//-----------------------------------------------------------------------------
// Static control
//-----------------------------------------------------------------------------
class GLUFStatic : public GLUFControl
{
public:
	GLUFStatic(GLUFDialog* pDialog = NULL);

	virtual void    Render(float fElapsedTime);
	virtual BOOL    ContainsPoint(GLUFPoint pt)
	{
		return false;
	}

	HRESULT         GetTextCopy(__out_ecount(bufferCount) LPWSTR strDest,
		UINT bufferCount);
	LPCWSTR         GetText()
	{
		return m_strText;
	}
	HRESULT         SetText(LPCWSTR strText);


protected:
	WCHAR           m_strText[MAX_PATH];      // Window text  
};


//-----------------------------------------------------------------------------
// Button control
//-----------------------------------------------------------------------------
class GLUFButton : public GLUFStatic
{
public:
	GLUFButton(GLUFDialog* pDialog = NULL);

	virtual bool    HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual bool    HandleMouse(UINT uMsg, GLUFPoint pt, WPARAM wParam, LPARAM lParam);
	virtual void    OnHotkey()
	{
		if (m_pDialog->IsKeyboardInputEnabled()) m_pDialog->RequestFocus(this);
		m_pDialog->SendEvent(EVENT_BUTTON_CLICKED, true, this);
	}

	virtual BOOL    ContainsPoint(GLUFPoint pt)
	{
		return PtInGLUFRect(&m_rcBoundingBox, pt);
	}
	virtual bool    CanHaveFocus()
	{
		return (m_bVisible && m_bEnabled);
	}

	virtual void    Render(float fElapsedTime);

protected:
	bool m_bPressed;
};


//-----------------------------------------------------------------------------
// CheckBox control
//-----------------------------------------------------------------------------
class GLUFCheckBox : public GLUFButton
{
public:
	GLUFCheckBox(GLUFDialog* pDialog = NULL);

	virtual bool    HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual bool    HandleMouse(UINT uMsg, GLUFPoint pt, WPARAM wParam, LPARAM lParam);
	virtual void    OnHotkey()
	{
		if (m_pDialog->IsKeyboardInputEnabled()) m_pDialog->RequestFocus(this);
		SetCheckedInternal(!m_bChecked, true);
	}

	virtual BOOL    ContainsPoint(GLUFPoint pt);
	virtual void    UpdateGLUFRects();

	virtual void    Render(float fElapsedTime);

	bool            GetChecked()
	{
		return m_bChecked;
	}
	void            SetChecked(bool bChecked)
	{
		SetCheckedInternal(bChecked, false);
	}

protected:
	virtual void    SetCheckedInternal(bool bChecked, bool bFromInput);

	bool m_bChecked;
	GLUFRect m_rcButton;
	GLUFRect m_rcText;
};


//-----------------------------------------------------------------------------
// RadioButton control
//-----------------------------------------------------------------------------
class GLUFRadioButton : public GLUFCheckBox
{
public:
	GLUFRadioButton(GLUFDialog* pDialog = NULL);

	virtual bool    HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual bool    HandleMouse(UINT uMsg, GLUFPoint pt, WPARAM wParam, LPARAM lParam);
	virtual void    OnHotkey()
	{
		if (m_pDialog->IsKeyboardInputEnabled()) m_pDialog->RequestFocus(this);
		SetCheckedInternal(true, true, true);
	}

	void            SetChecked(bool bChecked, bool bClearGroup = true)
	{
		SetCheckedInternal(bChecked, bClearGroup, false);
	}
	void            SetButtonGroup(UINT nButtonGroup)
	{
		m_nButtonGroup = nButtonGroup;
	}
	UINT            GetButtonGroup()
	{
		return m_nButtonGroup;
	}

protected:
	virtual void    SetCheckedInternal(bool bChecked, bool bClearGroup, bool bFromInput);
	UINT m_nButtonGroup;
};


//-----------------------------------------------------------------------------
// Scrollbar control
//-----------------------------------------------------------------------------
class GLUFScrollBar : public GLUFControl
{
public:
	GLUFScrollBar(GLUFDialog* pDialog = NULL);
	virtual         ~GLUFScrollBar();

	virtual bool    HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual bool    HandleMouse(UINT uMsg, GLUFPoint pt, WPARAM wParam, LPARAM lParam);
	virtual bool    MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual void    Render(float fElapsedTime);
	virtual void    UpdateGLUFRects();

	void            SetTrackRange(int nStart, int nEnd);
	int             GetTrackPos()
	{
		return m_nPosition;
	}
	void            SetTrackPos(int nPosition)
	{
		m_nPosition = nPosition; Cap(); UpdateThumbGLUFRect();
	}
	int             GetPageSize()
	{
		return m_nPageSize;
	}
	void            SetPageSize(int nPageSize)
	{
		m_nPageSize = nPageSize; Cap(); UpdateThumbGLUFRect();
	}

	void            Scroll(int nDelta);    // Scroll by nDelta items (plus or minus)
	void            ShowItem(int nIndex);  // Ensure that item nIndex is displayed, scroll if necessary

protected:
	// ARROWSTATE indicates the state of the arrow buttons.
	// CLEAR            No arrow is down.
	// CLICKED_UP       Up arrow is clicked.
	// CLICKED_DOWN     Down arrow is clicked.
	// HELD_UP          Up arrow is held down for sustained period.
	// HELD_DOWN        Down arrow is held down for sustained period.
	enum ARROWSTATE
	{
		CLEAR,
		CLICKED_UP,
		CLICKED_DOWN,
		HELD_UP,
		HELD_DOWN
	};

	void            UpdateThumbGLUFRect();
	void            Cap();  // Clips position at boundaries. Ensures it stays within legal range.

	bool m_bShowThumb;
	bool m_bDrag;
	GLUFRect m_rcUpButton;
	GLUFRect m_rcDownButton;
	GLUFRect m_rcTrack;
	GLUFRect m_rcThumb;
	int m_nPosition;  // Position of the first displayed item
	int m_nPageSize;  // How many items are displayable in one page
	int m_nStart;     // First item
	int m_nEnd;       // The index after the last item
	GLUFPoint m_LastMouse;// Last mouse position
	ARROWSTATE m_Arrow; // State of the arrows
	double m_dArrowTS;  // Timestamp of last arrow event.
};


//-----------------------------------------------------------------------------
// ListBox control
//-----------------------------------------------------------------------------
struct GLUFListBoxItem
{
	WCHAR strText[256];
	void* pData;

	GLUFRect rcActive;
	bool bSelected;
};

class GLUFListBox : public GLUFControl
{
public:
	GLUFListBox(GLUFDialog* pDialog = NULL);
	virtual         ~GLUFListBox();

	virtual HRESULT OnInit()
	{
		return m_pDialog->InitControl(&m_ScrollBar);
	}
	virtual bool    CanHaveFocus()
	{
		return (m_bVisible && m_bEnabled);
	}
	virtual bool    HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual bool    HandleMouse(UINT uMsg, GLUFPoint pt, WPARAM wParam, LPARAM lParam);
	virtual bool    MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual void    Render(float fElapsedTime);
	virtual void    UpdateGLUFRects();

	DWORD           GetStyle() const
	{
		return m_dwStyle;
	}
	int             GetSize() const
	{
		return m_Items.size();
	}
	void            SetStyle(DWORD dwStyle)
	{
		m_dwStyle = dwStyle;
	}
	int             GetScrollBarWidth() const
	{
		return m_nSBWidth;
	}
	void            SetScrollBarWidth(int nWidth)
	{
		m_nSBWidth = nWidth; UpdateGLUFRects();
	}
	void            SetBorder(int nBorder, int nMargin)
	{
		m_nBorder = nBorder; m_nMargin = nMargin;
	}
	HRESULT         AddItem(const WCHAR* wszText, void* pData);
	HRESULT         InsertItem(int nIndex, const WCHAR* wszText, void* pData);
	void            RemoveItem(int nIndex);
	void            RemoveAllItems();

	GLUFListBoxItem* GetItem(int nIndex);
	int             GetSelectedIndex(int nPreviousSelected = -1);
	GLUFListBoxItem* GetSelectedItem(int nPreviousSelected = -1)
	{
		return GetItem(GetSelectedIndex(nPreviousSelected));
	}
	void            SelectItem(int nNewIndex);

	enum STYLE
	{
		MULTISELECTION = 1
	};

protected:
	GLUFRect m_rcText;      // Text rendering bound
	GLUFRect m_rcSelection; // Selection box bound
	GLUFScrollBar m_ScrollBar;
	int m_nSBWidth;
	int m_nBorder;
	int m_nMargin;
	int m_nTextHeight;  // Height of a single line of text
	DWORD m_dwStyle;    // List box style
	int m_nSelected;    // Index of the selected item for single selection list box
	int m_nSelStart;    // Index of the item where selection starts (for handling multi-selection)
	bool m_bDrag;       // Whether the user is dragging the mouse to select

	std::vector <GLUFListBoxItem*> m_Items;
};


//-----------------------------------------------------------------------------
// ComboBox control
//-----------------------------------------------------------------------------
struct GLUFComboBoxItem
{
	WCHAR strText[256];
	void* pData;

	GLUFRect rcActive;
	bool bVisible;
};


class GLUFComboBox : public GLUFButton
{
public:
	GLUFComboBox(GLUFDialog* pDialog = NULL);
	virtual         ~GLUFComboBox();

	virtual void    SetTextColor(Color Color);
	virtual HRESULT OnInit()
	{
		return m_pDialog->InitControl(&m_ScrollBar);
	}

	virtual bool    HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual bool    HandleMouse(UINT uMsg, GLUFPoint pt, WPARAM wParam, LPARAM lParam);
	virtual void    OnHotkey();

	virtual bool    CanHaveFocus()
	{
		return (m_bVisible && m_bEnabled);
	}
	virtual void    OnFocusOut();
	virtual void    Render(float fElapsedTime);

	virtual void    UpdateGLUFRects();

	HRESULT         AddItem(const WCHAR* strText, void* pData);
	void            RemoveAllItems();
	void            RemoveItem(UINT index);
	bool            ContainsItem(const WCHAR* strText, UINT iStart = 0);
	int             FindItem(const WCHAR* strText, UINT iStart = 0);
	void* GetItemData(const WCHAR* strText);
	void* GetItemData(int nIndex);
	void            SetDropHeight(UINT nHeight)
	{
		m_nDropHeight = nHeight; UpdateGLUFRects();
	}
	int             GetScrollBarWidth() const
	{
		return m_nSBWidth;
	}
	void            SetScrollBarWidth(int nWidth)
	{
		m_nSBWidth = nWidth; UpdateGLUFRects();
	}

	int             GetSelectedIndex() const
	{
		return m_iSelected;
	}
	void* GetSelectedData();
	GLUFComboBoxItem* GetSelectedItem();

	UINT            GetNumItems()
	{
		return m_Items.size();
	}
	GLUFComboBoxItem* GetItem(UINT index)
	{
		return m_Items[index];
	}

	HRESULT         SetSelectedByIndex(UINT index);
	HRESULT         SetSelectedByText(const WCHAR* strText);
	HRESULT         SetSelectedByData(void* pData);

protected:
	int m_iSelected;
	int m_iFocused;
	int m_nDropHeight;
	GLUFScrollBar m_ScrollBar;
	int m_nSBWidth;

	bool m_bOpened;

	GLUFRect m_rcText;
	GLUFRect m_rcButton;
	GLUFRect m_rcDropdown;
	GLUFRect m_rcDropdownText;


	std::vector <GLUFComboBoxItem*> m_Items;
};


//-----------------------------------------------------------------------------
// Slider control
//-----------------------------------------------------------------------------
class GLUFSlider : public GLUFControl
{
public:
	GLUFSlider(GLUFDialog* pDialog = NULL);

	virtual BOOL    ContainsPoint(GLUFPoint pt);
	virtual bool    CanHaveFocus()
	{
		return (m_bVisible && m_bEnabled);
	}
	virtual bool    HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual bool    HandleMouse(UINT uMsg, GLUFPoint pt, WPARAM wParam, LPARAM lParam);

	virtual void    UpdateGLUFRects();

	virtual void    Render(float fElapsedTime);

	void            SetValue(int nValue)
	{
		SetValueInternal(nValue, false);
	}
	int             GetValue() const
	{
		return m_nValue;
	};

	void            GetRange(int& nMin, int& nMax) const
	{
		nMin = m_nMin; nMax = m_nMax;
	}
	void            SetRange(int nMin, int nMax);

protected:
	void            SetValueInternal(int nValue, bool bFromInput);
	int             ValueFromPos(int x);

	int m_nValue;

	int m_nMin;
	int m_nMax;

	int m_nDragX;      // Mouse position at start of drag
	int m_nDragOffset; // Drag offset from the center of the button
	int m_nButtonX;

	bool m_bPressed;
	GLUFRect m_rcButton;
};


//-----------------------------------------------------------------------------
// CUniBuffer class for the edit control
//-----------------------------------------------------------------------------
class CUniBuffer
{
public:
	CUniBuffer(int nInitialSize = 1);
	~CUniBuffer();

	static void WINAPI      Initialize();
	static void WINAPI      Uninitialize();

	int                     GetBufferSize()
	{
		return m_nBufferSize;
	}
	bool                    SetBufferSize(int nSize);
	int                     GetTextSize()
	{
		return lstrlenW(m_pwszBuffer);
	}
	const WCHAR* GetBuffer()
	{
		return m_pwszBuffer;
	}
	const WCHAR& operator[](int n) const
	{
		return m_pwszBuffer[n];
	}
	WCHAR& operator[](int n);
	GLUFFontNode* GetFontNode()
	{
		return m_pFontNode;
	}
	void                    SetFontNode(GLUFFontNode* pFontNode)
	{
		m_pFontNode = pFontNode;
	}
	void                    Clear();

	bool                    InsertChar(int nIndex, WCHAR wChar); // Inserts the char at specified index. If nIndex == -1, insert to the end.
	bool                    RemoveChar(int nIndex);  // Removes the char at specified index. If nIndex == -1, remove the last char.
	bool                    InsertString(int nIndex, const WCHAR* pStr, int nCount = -1);  // Inserts the first nCount characters of the string pStr at specified index.  If nCount == -1, the entire string is inserted. If nIndex == -1, insert to the end.
	bool                    SetText(LPCWSTR wszText);

	// Uniscribe
	HRESULT                 CPtoX(int nCP, BOOL bTrail, int* pX);
	HRESULT                 XtoCP(int nX, int* pCP, int* pnTrail);
	void                    GetPriorItemPos(int nCP, int* pPrior);
	void                    GetNextItemPos(int nCP, int* pPrior);

private:
	HRESULT                 Analyse();      // Uniscribe -- Analyse() analyses the string in the buffer

	WCHAR* m_pwszBuffer;    // Buffer to hold text
	int m_nBufferSize;   // Size of the buffer allocated, in characters

	// Uniscribe-specific
	GLUFFontNode* m_pFontNode;          // Font node for the font that this buffer uses
	bool m_bAnalyseRequired;            // True if the string has changed since last analysis.
	SCRIPT_STRING_ANALYSIS m_Analysis;  // Analysis for the current string

private:
	// Empty implementation of the Uniscribe API
	static HRESULT WINAPI   Dummy_ScriptApplyDigitSubstitution(const SCRIPT_DIGITSUBSTITUTE*, SCRIPT_CONTROL*,
		SCRIPT_STATE*)
	{
		return E_NOTIMPL;
	}
	static HRESULT WINAPI   Dummy_ScriptStringAnalyse(HDC, const void*, int, int, int, DWORD, int, SCRIPT_CONTROL*,
		SCRIPT_STATE*, const int*, SCRIPT_TABDEF*, const BYTE*,
		SCRIPT_STRING_ANALYSIS*)
	{
		return E_NOTIMPL;
	}
	static HRESULT WINAPI   Dummy_ScriptStringCPtoX(SCRIPT_STRING_ANALYSIS, int, BOOL, int*)
	{
		return E_NOTIMPL;
	}
	static HRESULT WINAPI   Dummy_ScriptStringXtoCP(SCRIPT_STRING_ANALYSIS, int, int*, int*)
	{
		return E_NOTIMPL;
	}
	static HRESULT WINAPI   Dummy_ScriptStringFree(SCRIPT_STRING_ANALYSIS*)
	{
		return E_NOTIMPL;
	}
	static const SCRIPT_LOGATTR* WINAPI Dummy_ScriptString_pLogAttr(SCRIPT_STRING_ANALYSIS)
	{
		return NULL;
	}
	static const int* WINAPI Dummy_ScriptString_pcOutChars(SCRIPT_STRING_ANALYSIS)
	{
		return NULL;
	}

	// Function pointers
	static                  HRESULT(WINAPI* _ScriptApplyDigitSubstitution)(const SCRIPT_DIGITSUBSTITUTE*,
		SCRIPT_CONTROL*, SCRIPT_STATE*);
	static                  HRESULT(WINAPI* _ScriptStringAnalyse)(HDC, const void*, int, int, int, DWORD, int,
		SCRIPT_CONTROL*, SCRIPT_STATE*, const int*,
		SCRIPT_TABDEF*, const BYTE*,
		SCRIPT_STRING_ANALYSIS*);
	static                  HRESULT(WINAPI* _ScriptStringCPtoX)(SCRIPT_STRING_ANALYSIS, int, BOOL, int*);
	static                  HRESULT(WINAPI* _ScriptStringXtoCP)(SCRIPT_STRING_ANALYSIS, int, int*, int*);
	static                  HRESULT(WINAPI* _ScriptStringFree)(SCRIPT_STRING_ANALYSIS*);
	static const SCRIPT_LOGATTR* (WINAPI*_ScriptString_pLogAttr)(SCRIPT_STRING_ANALYSIS);
	static const int* (WINAPI*_ScriptString_pcOutChars)(SCRIPT_STRING_ANALYSIS);

	static HINSTANCE s_hDll;  // Uniscribe DLL handle

};

//-----------------------------------------------------------------------------
// EditBox control
//-----------------------------------------------------------------------------
class GLUFEditBox : public GLUFControl
{
public:
	GLUFEditBox(GLUFDialog* pDialog = NULL);
	virtual         ~GLUFEditBox();

	virtual bool    HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual bool    HandleMouse(UINT uMsg, GLUFPoint pt, WPARAM wParam, LPARAM lParam);
	virtual bool    MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void    UpdateGLUFRects();
	virtual bool    CanHaveFocus()
	{
		return (m_bVisible && m_bEnabled);
	}
	virtual void    Render(float fElapsedTime);
	virtual void    OnFocusIn();

	void            SetText(LPCWSTR wszText, bool bSelected = false);
	LPCWSTR         GetText()
	{
		return m_Buffer.GetBuffer();
	}
	int             GetTextLength()
	{
		return m_Buffer.GetTextSize();
	}  // Returns text length in chars excluding NULL.
	HRESULT         GetTextCopy(__out_ecount(bufferCount) LPWSTR strDest,
		UINT bufferCount);
	void            ClearText();
	virtual void    SetTextColor(Color Color)
	{
		m_TextColor = Color;
	}  // Text color
	void            SetSelectedTextColor(Color Color)
	{
		m_SelTextColor = Color;
	}  // Selected text color
	void            SetSelectedBackColor(Color Color)
	{
		m_SelBkColor = Color;
	}  // Selected background color
	void            SetCaretColor(Color Color)
	{
		m_CaretColor = Color;
	}  // Caret color
	void            SetBorderWidth(int nBorder)
	{
		m_nBorder = nBorder; UpdateGLUFRects();
	}  // Border of the window
	void            SetSpacing(int nSpacing)
	{
		m_nSpacing = nSpacing; UpdateGLUFRects();
	}
	void            ParseFloatArray(float* pNumbers, int nCount);
	void            SetTextFloatArray(const float* pNumbers, int nCount);

protected:
	void            PlaceCaret(int nCP);
	void            DeleteSelectionText();
	void            ResetCaretBlink();
	void            CopyToClipboard();
	void            PasteFromClipboard();

	CUniBuffer m_Buffer;     // Buffer to hold text
	int m_nBorder;      // Border of the window
	int m_nSpacing;     // Spacing between the text and the edge of border
	GLUFRect m_rcText;       // Bounding GLUFRectangle for the text
	GLUFRect            m_rcRender[9];  // Convenient GLUFRectangles for rendering elements
	double m_dfBlink;      // Caret blink time in milliseconds
	double m_dfLastBlink;  // Last timestamp of caret blink
	bool m_bCaretOn;     // Flag to indicate whether caret is currently visible
	int m_nCaret;       // Caret position, in characters
	bool m_bInsertMode;  // If true, control is in insert mode. Else, overwrite mode.
	int m_nSelStart;    // Starting position of the selection. The caret marks the end.
	int m_nFirstVisible;// First visible character in the edit control
	Color m_TextColor;    // Text color
	Color m_SelTextColor; // Selected text color
	Color m_SelBkColor;   // Selected background color
	Color m_CaretColor;   // Caret color

	// Mouse-specific
	bool m_bMouseDrag;       // True to indicate drag in progress

	// Static
	static bool s_bHideCaret;   // If true, we don't render the caret.
};



////////////////////////////////////////////////////////////////////////////////////////////
//
//Text Controls Below
//
//



class GLUFTextHelper
{
public:
	GLUFTextHelper(GLUFDialogResourceManager* pManager, int nLineHeight);
	~GLUFTextHelper();

	void Init(int nLineHeight = 15);

	void SetInsertionPos(int x, int y)
	{
		m_pt.x = x;
		m_pt.y = y;
	}
	void SetForegroundColor(glm::vec4 clr) { m_clr = clr; }

	void    Begin();
	HRESULT DrawFormattedTextLine(const WCHAR* strMsg, ...);
	HRESULT DrawTextLine(const WCHAR* strMsg);
	HRESULT DrawFormattedTextLine(const GLUFRect& rc, _In_ DWORD dwFlags, const WCHAR* strMsg, ...);
	HRESULT DrawTextLine(_In_ const GLUFRect& rc, _In_ DWORD dwFlags, _In_z_ const WCHAR* strMsg);
	void    End();

protected:
	glm::vec4 m_clr;
	GLUFPoint m_pt;
	int m_nLineHeight;

	// D3D11 font 
	GLUFDialogResourceManager* m_pManager;
};
