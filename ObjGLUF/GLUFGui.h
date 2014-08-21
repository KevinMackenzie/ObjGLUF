#pragma once

#include "ObjGLUF.h"

//--------------------------------------------------------------------------------------
// Macro definitions
//--------------------------------------------------------------------------------------
#define GT_CENTER	0x000000001
#define GT_LEFT		0x000000002
#define GT_RIGHT    0x000000003

#define GT_VCENTER	0x000000010
#define GT_TOP      0x000000020
#define GT_BOTTOM   0x000000030



//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
class OBJGLUF_API GLUFDialogResourceManager;
class OBJGLUF_API GLUFControl;
class OBJGLUF_API GLUFButton;
class OBJGLUF_API GLUFStatic;
class OBJGLUF_API GLUFCheckBox;
class OBJGLUF_API GLUFRadioButton;
class OBJGLUF_API GLUFComboBox;
class OBJGLUF_API GLUFSlider;
class OBJGLUF_API GLUFEditBox;
class OBJGLUF_API GLUFListBox;
class OBJGLUF_API GLUFScrollBar;
class OBJGLUF_API GLUFElement;
class OBJGLUF_API GLUFFont;
class OBJGLUF_API GLUFDialog;
struct GLUFElementHolder;
struct GLUFTextureNode;
struct GLUFFontNode;


enum GLUF_MESSAGE_TYPE
{
	GM_MB = 0,
	GM_CURSOR_POS,
	GM_CURSOR_ENTER,
	GM_SCROLL, //since i am using 32 bit integers as input, this value will be multiplied by 1000 to preserver any relevant decimal points
	GM_KEY,//don't' support joysticks yet
	GM_UNICODE_CHAR,
	GM_RESIZE,
	GM_POS,
	GM_CLOSE,
	GM_REFRESH,
	GM_FOCUS,
	GM_ICONIFY,
	GM_FRAMEBUFFER_SIZE
};

//--------------------------------------------------------------------------------------
// Funcs and Enums for using fonts
//--------------------------------------------------------------------------------------

#define GLUF_POINTS_PER_PIXEL 1.333333f
#define GLUF_POINTS_TO_PIXELS(points) (GLUFFontSize)((float)points * GLUF_POINTS_PER_PIXEL)

//this is because when rendered the actual font height will be cut in half.  Use this when using font in NDC space
#define GLUF_FONT_HEIGHT_NDC(size) (size * 2) 

typedef bool(*PGLUFCALLBACK)(GLUF_MESSAGE_TYPE, int, int, int, int);

#define GLUF_GUI_CALLBACK_PARAM GLUF_MESSAGE_TYPE msg, int param1, int param2, int param3, int param4
#define GLUF_PASS_CALLBACK_PARAM msg, param1, param2, param3, param4

//this must be called AFTER GLUFInitOpenGLExtentions();  callbackFunc may do whatever it pleases, however;
// callbackFunc must explicitly call the callback methods of the dialog manager and the dialog classes (and whatever else)
// for full documentation on what each parameter is, consult the glfw input/window documentation.  For each value,
// it will be truncated into a 32 bit integer (from a double if necessary) and put in order into the procedure.  If
// a callback does not use the parameter, it will be 0, but this does not mean 0 is an invalid parameter for callbacks
// that use it.  Other notes: when specifying hotkeys, always use the GLFW macros for specifying them.  Consult the GLFW
// input documentation for more information.
bool OBJGLUF_API GLUFInitGui(GLFWwindow* pInitializedGLFWWindow, PGLUFCALLBACK callbackFunc, GLuint controltex);

typedef std::shared_ptr<GLUFFont> GLUFFontPtr;
typedef float GLUFFontSize;//this is in normalized screencoords

//NOTE: i am no longer using freetype, it is just more than is necisary right now
OBJGLUF_API GLUFFontPtr GLUFLoadFont(char* rawData, uint64_t rawSize, float fontHeight);
OBJGLUF_API GLUFFontSize GLUFGetFontHeight(GLUFFontPtr font);

//NOTE: not all fonts support all of these weights! the closest available will be chosen (ALSO this does not work well with preinstalled fonts)
enum GLUF_FONT_WEIGHT
{
	FONT_WEIGHT_HAIRLINE = 0,
	FONT_WEIGHT_THIN,
	FONT_WEIGHT_ULTRA_LIGHT,
	FONT_WEIGHT_EXTRA_LIGHT,
	FONT_WEIGHT_LIGHT,
	FONT_WEIGHT_BOOK,
	FONT_WEIGHT_NORMAL,
	FONT_WEIGHT_MEDIUM,
	FONT_WEIGHT_SEMI_BOLD,
	FONT_WEIGHT_BOLD,
	FONT_WEIGHT_EXTRA_BOLD,
	FONT_WEIGHT_HEAVY,
	FONT_WEIGHT_BLACK,
	FONT_WEIGHT_EXTRA_BLACK,
	FONT_WEIGHT_ULTRA_BLACK
};


//extern OBJGLUF_API GLUFProgramPtr g_UIProgram;


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

//WIP
enum GLUF_EVENT
{
	GLUF_EVENT_BUTTON_CLICKED = 0,
	GLUF_EVENT_COMBOBOX_SELECTION_CHANGED,
	GLUF_EVENT_RADIOBUTTON_CHANGED,
	GLUF_EVENT_CHECKBOXCHANGED,
	GLUF_EVENT_SLIDER_VALUE_CHANGED,
	GLUF_EVENT_SLIDER_VALUE_CHANGED_UP,
	GLUF_EVENT_EDITBOX_STRING,
	GLUF_EVENT_EDITBOX_CHANGE,//when the listbox contents change due to user input
	GLUF_EVENT_LISTBOX_ITEM_DBLCLK,
	GLUF_EVENT_LISTBOX_SELECTION,//when the selection changes in a single selection list box
	GLUF_EVENT_LISTBOX_SELECTION_END,
};




typedef void (*PCALLBACKGLUFGUIEVENT)(GLUF_EVENT nEvent, int nControlID, GLUFControl* pControl);


#define MAX_CONTROL_STATES 6

struct GLUFBlendColor
{
	void        Init(Color defaultColor, Color disabledColor = Color(200, 128, 128, 100), Color hiddenColor = Color(0, 0, 0, 0));
	void        Blend(GLUF_CONTROL_STATE iState, float fElapsedTime, float fRate = 0.7f);
	void		SetCurrent(Color current);
	void		SetCurrent(GLUF_CONTROL_STATE state);

	Color		States[MAX_CONTROL_STATES]; // Modulate colors for all possible control states
	Color		Current;
};

typedef unsigned int GLUFTextureIndex;
typedef unsigned int GLUFFontIndex;

//-----------------------------------------------------------------------------
// Contains all the display tweakables for a sub-control
//-----------------------------------------------------------------------------
class GLUFElement
{
public:
	void    SetTexture(GLUFTextureIndex iTexture, GLUFRect* prcTexture, Color defaultTextureColor = Color(255, 255, 255, 0));
	void    SetFont(GLUFFontIndex iFont, Color defaultFontColor = Color(0, 0, 0, 255), unsigned int dwTextFormat = GT_CENTER | GT_VCENTER);

	void    Refresh();

	GLUFTextureIndex iTexture;			// Index of the texture for this Element 
	GLUFFontIndex iFont;				// Index of the font for this Element
	unsigned int dwTextFormat;			// The format of the text

	GLUFRect rcTexture;					// Bounding Rect of this element on the composite texture(NOTE: this are given in normalized coordinates just like openGL textures)

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
	void                Init(GLUFDialogResourceManager* pManager, bool bRegisterDialog, unsigned int iTexture);

	// message handler (this can handle any message type, where GLUF_MESSAGE_TYPE is what command it is actually responding from
	bool                MsgProc(GLUF_MESSAGE_TYPE msg, int param1, int param2, int param3, int param4);

	// Control creation (all coordinates are based on normalzied device space)
	GLUFResult             AddStatic(int ID, std::wstring strText, float x, float y, float width, float height, bool bIsDefault = false, GLUFStatic** ppCreated = NULL);
	GLUFResult             AddButton(int ID, std::wstring strText, float x, float y, float width, float height, int nHotkey = 0, bool bIsDefault = false, GLUFButton** ppCreated = NULL);
	GLUFResult             AddCheckBox(int ID, std::wstring strText, float x, float y, float width, float height, bool bChecked = false, int nHotkey = 0, bool bIsDefault = false, GLUFCheckBox** ppCreated = NULL);
	GLUFResult             AddRadioButton(int ID, unsigned int nButtonGroup, std::wstring strText, float x, float y, float width, float height, bool bChecked = false, int nHotkey = 0, bool bIsDefault = false, GLUFRadioButton** ppCreated = NULL);
	GLUFResult             AddComboBox(int ID, float x, float y, float width, float height, int nHotKey = 0, bool bIsDefault = false, GLUFComboBox** ppCreated = NULL);
	GLUFResult             AddSlider(int ID, float x, float y, float width, float height, float min = 0.0f, float max = 0.25f, float value = 0.125f, bool bIsDefault = false, GLUFSlider** ppCreated = NULL);
	GLUFResult             AddEditBox(int ID, std::wstring strText, float x, float y, float width, float height, bool bIsDefault = false, GLUFEditBox** ppCreated = NULL);
	GLUFResult             AddListBox(int ID, float x, float y, float width, float height, unsigned long dwStyle = 0, GLUFListBox** ppCreated = NULL);
	GLUFResult             AddControl(GLUFControl* pControl);
	GLUFResult             InitControl(GLUFControl* pControl);

	// Control retrieval
	GLUFStatic*			GetStatic(int ID)		{ return (GLUFStatic*)GetControl(ID, GLUF_CONTROL_STATIC);				}
	GLUFButton*			GetButton(int ID)		{ return (GLUFButton*)GetControl(ID, GLUF_CONTROL_BUTTON);				}
	GLUFCheckBox*		GetCheckBox(int ID)		{ return (GLUFCheckBox*)GetControl(ID, GLUF_CONTROL_CHECKBOX);			}
	GLUFRadioButton*	GetRadioButton(int ID)	{ return (GLUFRadioButton*)GetControl(ID, GLUF_CONTROL_RADIOBUTTON);	}
	GLUFComboBox*		GetComboBox(int ID)		{ return (GLUFComboBox*)GetControl(ID, GLUF_CONTROL_COMBOBOX);			}
	GLUFSlider*			GetSlider(int ID)		{ return (GLUFSlider*)GetControl(ID, GLUF_CONTROL_SLIDER);				}
	GLUFEditBox*		GetEditBox(int ID)		{ return (GLUFEditBox*)GetControl(ID, GLUF_CONTROL_EDITBOX);			}
	GLUFListBox*		GetListBox(int ID)		{ return (GLUFListBox*)GetControl(ID, GLUF_CONTROL_LISTBOX);			}

	GLUFControl* GetControl(int ID);
	GLUFControl* GetControl(int ID, GLUF_CONTROL_TYPE nControlType);
	GLUFControl* GetControlAtPoint(GLUFPoint pt);

	bool                GetControlEnabled(int ID);
	void                SetControlEnabled(int ID, bool bEnabled);

	void                ClearRadioButtonGroup(unsigned int nGroup);
	void                ClearComboBox(int ID);

	// Access the default display Elements used when adding new controls
	GLUFResult          SetDefaultElement(GLUF_CONTROL_TYPE nControlType, unsigned int iElement, GLUFElement* pElement);
	GLUFElement*		GetDefaultElement(GLUF_CONTROL_TYPE nControlType, unsigned int iElement);

	// Methods called by controls
	void                SendEvent(GLUF_EVENT nEvent, bool bTriggeredByUser, GLUFControl* pControl);
	void                RequestFocus(GLUFControl* pControl);

	// Render helpers
	GLUFResult          DrawRect(GLUFRect pGLUFRect, Color color);
	GLUFResult          DrawPolyLine(GLUFPoint* apPoints, unsigned int nNumPoints, Color color);
	GLUFResult          DrawSprite(GLUFElement* pElement, GLUFRect prcDest, float fDepth, bool textured = true);
	GLUFResult          CalcTextRect(std::wstring strText, GLUFElement* pElement, GLUFRect prcDest, int nCount = -1);
	GLUFResult          DrawText(std::wstring strText, GLUFElement* pElement, GLUFRect prcDest, bool bShadow = false,
		bool bCenter = false, bool bHardRect = false);

	// Attributes
	void                SetBackgroundColors(Color colorTopLeft, Color colorTopRight, Color colorBottomLeft,	Color colorBottomRight);
	bool                GetVisible()								{ return m_bVisible;																		}
	void                SetVisible(bool bVisible)					{ m_bVisible = bVisible;																	}
	bool                GetMinimized()								{ return m_bMinimized;																		}
	void                SetMinimized(bool bMinimized)				{ m_bMinimized = bMinimized;																}
	void                SetBackgroundColors(Color colorAllCorners)	{ SetBackgroundColors(colorAllCorners, colorAllCorners, colorAllCorners, colorAllCorners);	}
	void                EnableCaption(bool bEnable)					{ m_bCaption = bEnable;																		}
	float               GetCaptionHeight() const					{ return m_nCaptionHeight; }
	void                SetCaptionHeight(float nHeight)				{ m_nCaptionHeight = nHeight;																}
	void                SetCaptionText(std::wstring pwszText)		{ m_wszCaption = pwszText;																	}
	void                GetLocation(GLUFPoint& Pt) const			{ Pt.x = m_x; Pt.y = m_y;																	}
	void                SetLocation(float x, float y)				{ m_x = x; m_y = y; }
	void                SetSize(float width, float height)			{ m_width = width; m_height = height; }
	float               GetWidth()									{ return m_width; }
	float               GetHeight()									{ return m_height; }

	void LockPosition(bool lock = true){ m_bPosLocked = lock; }

	static void			SetRefreshTime(float fTime)					{ s_fTimeRefresh = fTime;																	}

	static GLUFControl* GetNextControl(GLUFControl* pControl);
	static GLUFControl* GetPrevControl(GLUFControl* pControl);

	void                RemoveControl(int ID);
	void                RemoveAllControls();

	// Sets the callback used to notify the app of control events
	void                SetCallback(PCALLBACKGLUFGUIEVENT pCallback, void* pUserContext = NULL);
	void                EnableNonUserEvents(bool bEnable)	{ m_bNonUserEvents = bEnable;	}
	void                EnableKeyboardInput(bool bEnable)	{ m_bKeyboardInput = bEnable;	}
	void                EnableMouseInput(bool bEnable)		{ m_bMouseInput = bEnable;		}
	bool                IsKeyboardInputEnabled() const		{ return m_bKeyboardInput;		}

	// Device state notification
	void                Refresh();
	GLUFResult          OnRender(float fElapsedTime);

	// Shared resource access. Fonts and textures are shared among
	// all the controls.
	GLUFResult          SetFont(GLUFFontIndex iFontIndex, GLUFFontIndex resManFontIndex); //NOTE: this requires the font already existing on the resource manager
	GLUFFontNode*		GetFont(unsigned int index) const;

	GLUFResult          SetTexture(GLUFTextureIndex iTexIndex, GLUFTextureIndex resManTexIndex); //NOTE: this requries the texture to already exist in the resource manager
	GLUFTextureNode*	GetTexture(GLUFFontIndex index) const;

	GLUFDialogResourceManager* GetManager(){ return m_pManager; }

	static void			ClearFocus();
	void                FocusDefaultControl();

	bool m_bNonUserEvents;
	bool m_bKeyboardInput;
	bool m_bMouseInput;

	//this is the current mouse cursor position
	GLUFPoint m_MousePosition;
	GLUFPoint m_MousePositionDialogSpace;


	GLUFPoint m_MousePositionOld;

private:
	bool firstTime = true;

	bool m_bPosLocked = true;
	bool m_bDragged = false;

	int m_nDefaultControlID;

	//HRESULT             OnRender9(float fElapsedTime);
	//HRESULT             OnRender10(float fElapsedTime);
	//HRESULT             OnRender11(float fElapsedTime);

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
	std::wstring        m_wszCaption;

	//these are assumed to be based on the origin (bottom left) AND normalized
	float m_x;
	float m_y;
	float m_width;
	float m_height;
	float m_nCaptionHeight;

	Color m_colorTopLeft;
	Color m_colorTopRight;
	Color m_colorBottomLeft;
	Color m_colorBottomRight;

	GLUFDialogResourceManager* m_pManager;
	PCALLBACKGLUFGUIEVENT m_pCallbackEvent;
	void* m_pCallbackEventUserContext;

	std::vector <int> m_Textures;	// Textures
	std::vector <int> m_Fonts;		// Fonts

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
	GLuint m_pTextureElement;
};

//WIP, support more font options eg. stroke, italics, variable leading, etc.
struct GLUFFontNode
{
	//GLUFFontSize mSize;
	GLUF_FONT_WEIGHT mWeight;
	GLUFFontPtr m_pFontType;
};

struct GLUFSpriteVertexArray
{
private:
	std::vector<glm::vec3> vPos;
	std::vector<Color4f>   vColor;
	std::vector<glm::vec2> vTex;
public:

	glm::vec3* data_pos()  { if (size() > 0) return &vPos[0]; else return nullptr; }
	Color4f*   data_color(){ if (size() > 0) return &vColor[0]; else return nullptr; }
	glm::vec2* data_tex()  { if (size() > 0) return &vTex[0]; else return nullptr; }

	void push_back(glm::vec3 pos, Color color, glm::vec2 tex)
	{
		vPos.push_back(pos);	vColor.push_back(GLUFColorToFloat(color));	vTex.push_back(tex);
	}

	void clear(){ vPos.clear(); vColor.clear(); vTex.clear(); }
	unsigned long size(){ return vPos.size(); }
};

//-----------------------------------------------------------------------------
// Manages shared resources of dialogs
//-----------------------------------------------------------------------------
class GLUFDialogResourceManager
{
public:
	GLUFDialogResourceManager();
	~GLUFDialogResourceManager();

	bool    MsgProc(GLUF_MESSAGE_TYPE msg, int param1, int param2, int param3, int param4);

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
	void	EndSprites(bool textured);
	/*ID3D11Device* GetD3D11Device()
	{
	return m_pd3d11Device;
	}
	ID3D11DeviceContext* GetD3D11DeviceContext()
	{
	return m_pd3d11DeviceContext;
	}*/

	GLUFFontNode* GetFontNode(int iIndex)		{ return m_FontCache[iIndex];		}
	GLUFTextureNode* GetTextureNode(int iIndex)	{ return m_TextureCache[iIndex];	}

	int		GetTextureCount(){ return m_TextureCache.size(); }
	int     GetFontCount()   { return m_FontCache.size(); }

	int     AddFont(GLUFFontPtr font, GLUF_FONT_WEIGHT weight);
	int     AddTexture(GLuint texture);

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
	//GLUFProgramPtr m_pShader;

	// States (these might be unnecessary)
	//GLbitfield m_pDepthStencilStateUI11;
	//ID3D11RasterizerState* m_pRasterizerStateUI11;
	//GLbitfield m_pBlendStateUI11;
	//GLbitfield m_pSamplerStateUI11;

	// Stored states (again, might be unnecessary)
	//GLbitfield* m_pDepthStencilStateStored;
	//UINT m_StencilRefStored11;
	//ID3D11RasterizerState* m_pRasterizerStateStored;
	//GLbitfield* m_pBlendStateStored;
	//float m_BlendFactorStored[4];
	//UINT m_SampleMaskStored11;
	//GLbitfield m_pSamplerStateStored;


	//ID3D11InputLayout* m_pInputLayout;
	GLuint m_pVBScreenQuadVAO;
	GLuint m_pVBScreenQuadIndicies;
	GLuint m_pVBScreenQuadPositions;
	GLuint m_pVBScreenQuadColor;
	GLuint m_pVBScreenQuadUVs;
	GLuint m_pSamplerLocation;

	// Sprite workaround (what does this mean? and why do I need it?)
	//ID3D11Buffer* m_pSpriteBuffer;
	GLuint m_SpriteBufferVao;
	GLuint m_SpriteBufferPos;
	GLuint m_SpriteBufferColors;
	GLuint m_SpriteBufferTexCoords;
	GLuint m_SpriteBufferIndices;
	GLUFSpriteVertexArray m_SpriteVertices;//unfortunately, we have to do a SoA, instead of an AoS

	//unsigned int m_nBackBufferWidth;
	//unsigned int m_nBackBufferHeight;

	GLUFPoint GetWindowSize();

	std::vector <GLUFDialog*> m_Dialogs;            // Dialogs registered

	GLUFPoint GetOrthoPoint();
	glm::mat4 GetOrthoMatrix();

protected:
	void ApplyOrtho();

	// D3D9 specific
	/*IDiGLUFRect3DDevice9* m_pd3d9Device;
	HRESULT CreateFont9( UINT index );
	HRESULT CreateTexture9( UINT index );*/

	// D3D11 specific
	//ID3D11Device* m_pd3d11Device;
	//ID3D11DeviceContext* m_pd3d11DeviceContext;
	//GLUFResult CreateFont_(GLUFFontIndex index);
	//GLUFResult CreateTexture(GLUFTextureIndex index);

	GLUFPoint m_WndSize;

	std::vector <GLUFTextureNode*> m_TextureCache;   // Shared textures
	std::vector <GLUFFontNode*> m_FontCache;         // Shared fonts
};

void BeginText(glm::mat4 orthoMatrix);

//NOTE: this only supports char values, yes newlines, and only ASCII characters
void DrawTextGLUF(GLUFFontNode font, std::wstring strText, GLUFRect rcScreen, Color vFontColor, bool bCenter, bool bHardRect = false);
void EndText(GLUFFontPtr font);

//-----------------------------------------------------------------------------
// Base class for controls
//-----------------------------------------------------------------------------
class GLUFControl
{
public:
	GLUFControl(GLUFDialog* pDialog = NULL);
	virtual         ~GLUFControl();

	virtual GLUFResult OnInit(){ return GR_SUCCESS; }
	virtual void    Refresh();
	virtual void    Render(float fElapsedTime){};

	// message handler
	virtual bool    MsgProc(GLUF_MESSAGE_TYPE msg, int param1, int param2, int param3, int param4){ return false; }

	//these will be all handled by MsgProc
	/*virtual bool    HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return false;
	}
	virtual bool    HandleMouse(UINT uMsg, GLUFPoint pt, WPARAM wParam, LPARAM lParam)
	{
		return false;
	}*/

	virtual bool		CanHaveFocus()					{ return false;												}
	virtual void		OnFocusIn()						{ m_bHasFocus = true;										}
	virtual void		OnFocusOut()					{ m_bHasFocus = false;										}
	virtual void		OnMouseEnter()					{ m_bMouseOver = true;										}
	virtual void		OnMouseLeave()					{ m_bMouseOver = false;										}
	virtual void		OnHotkey()						{															}
	virtual bool		ContainsPoint(GLUFPoint pt)		{ return GLUFPtInRect(m_rcBoundingBox, pt);					}
	virtual void		SetEnabled(bool bEnabled)		{ m_bEnabled = bEnabled;									}
	virtual bool		GetEnabled()					{ return m_bEnabled;										}
	virtual void		SetVisible(bool bVisible)		{ m_bVisible = bVisible;									}
	virtual bool		GetVisible()					{ return m_bVisible;										}
	int					GetID() const					{ return m_ID;												}
	void				SetID(int ID)					{ m_ID = ID;												}
	void				SetLocation(float x, float y)	{ m_x = x; m_y = y; UpdateRects();							}
	void				SetSize(float width, float height){ m_width = width; m_height = height; UpdateRects();		}
	void				SetHotkey(int nHotkey)			{ m_nHotkey = nHotkey;										}
	int					GetHotkey()						{ return m_nHotkey;											}
	void				SetUserData(void* pUserData)	{ m_pUserData = pUserData;									}
	void*				GetUserData() const				{ return m_pUserData;										}
	GLUF_CONTROL_TYPE	GetType() const					{ return m_Type;											}
	GLUFElement*		GetElement(unsigned int iElement){return m_Elements[iElement];								}

	GLUFResult          SetElement(unsigned int iElement, GLUFElement* pElement);
	virtual void    SetTextColor(Color Color);

	bool m_bVisible;                // Shown/hidden flag
	bool m_bMouseOver;              // Mouse pointer is above control
	bool m_bHasFocus;               // Control has input focus
	bool m_bIsDefault;              // Is the default control

	// Size, scale, and positioning members
	float m_x, m_y;
	float m_width, m_height;

	// These members are set by the container
	GLUFDialog* m_pDialog;    // Parent container
	unsigned int m_Index;     // Index within the control list

	std::vector <GLUFElement*> m_Elements;  // All display elements

protected:
	virtual void    UpdateRects();

	int m_ID;                 // ID number
	GLUF_CONTROL_TYPE m_Type;  // Control type, set once in constructor  
	int  m_nHotkey;            // Virtual key code for this control's hotkey (hotkeys are represented by GLFW's keycodes
	void* m_pUserData;         // Data associated with this control that is set by user.

	bool m_bEnabled;           // Enabled/disabled flag

	GLUFRect m_rcBoundingBox;      // GLUFRectangle defining the active region of the control
};


//-----------------------------------------------------------------------------
// Contains all the display information for a given control type
//-----------------------------------------------------------------------------
struct GLUFElementHolder
{
	GLUF_CONTROL_TYPE nControlType;
	unsigned int iElement; //index of element

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
	virtual bool    ContainsPoint(GLUFPoint pt){ return false; }

	GLUFResult      GetTextCopy(std::wstring& strDest, unsigned int bufferCount);
	std::wstring     GetText(){ return m_strText; }
	GLUFResult      SetText(std::wstring strText);


protected:
	std::wstring     m_strText;      // Window text  
};


//-----------------------------------------------------------------------------
// Button control
//-----------------------------------------------------------------------------
class GLUFButton : public GLUFStatic
{
public:
	GLUFButton(GLUFDialog* pDialog = NULL);

	virtual bool MsgProc(GLUF_MESSAGE_TYPE msg, int param1, int param2, int param3, int param4);
	//virtual bool    HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);
	//virtual bool    HandleMouse(UINT uMsg, GLUFPoint pt, WPARAM wParam, LPARAM lParam);
	virtual void    OnHotkey(){ if (m_pDialog->IsKeyboardInputEnabled()) 
									m_pDialog->RequestFocus(this);	
								m_pDialog->SendEvent(GLUF_EVENT_BUTTON_CLICKED, true, this); }

	virtual bool    ContainsPoint(GLUFPoint pt)	{ return GLUFPtInRect(m_rcBoundingBox, pt); }
	virtual bool    CanHaveFocus()				{ return (m_bVisible && m_bEnabled);		}

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

	virtual bool MsgProc(GLUF_MESSAGE_TYPE msg, int param1, int param2, int param3, int param4);

	//virtual bool    HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);
	//virtual bool    HandleMouse(UINT uMsg, GLUFPoint pt, WPARAM wParam, LPARAM lParam);
	virtual void    OnHotkey(){	if (m_pDialog->IsKeyboardInputEnabled()) 
									m_pDialog->RequestFocus(this);
								SetCheckedInternal(!m_bChecked, true);}

	virtual bool    ContainsPoint(GLUFPoint pt);
	virtual void    UpdateRects();

	virtual void    Render(float fElapsedTime);

	bool            GetChecked(){ return m_bChecked; }
	void            SetChecked(bool bChecked){ SetCheckedInternal(bChecked, false); }

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

	virtual bool MsgProc(GLUF_MESSAGE_TYPE msg, int param1, int param2, int param3, int param4);
	//virtual bool    HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);
	//virtual bool    HandleMouse(UINT uMsg, GLUFPoint pt, WPARAM wParam, LPARAM lParam);
	virtual void    OnHotkey()
	{
		if (m_pDialog->IsKeyboardInputEnabled()) 
			m_pDialog->RequestFocus(this);
		SetCheckedInternal(true, true, true);
	}

	void            SetChecked(bool bChecked, bool bClearGroup = true){ SetCheckedInternal(bChecked, bClearGroup, false); }
	void            SetButtonGroup(unsigned int nButtonGroup){ m_nButtonGroup = nButtonGroup; }
	unsigned int    GetButtonGroup(){ return m_nButtonGroup; }

protected:
	virtual void    SetCheckedInternal(bool bChecked, bool bClearGroup, bool bFromInput);
	unsigned int m_nButtonGroup;
};


//-----------------------------------------------------------------------------
// Scrollbar control
//-----------------------------------------------------------------------------
class GLUFScrollBar : public GLUFControl
{
public:
	GLUFScrollBar(GLUFDialog* pDialog = NULL);
	virtual         ~GLUFScrollBar();

	//virtual bool    HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);
	//virtual bool    HandleMouse(UINT uMsg, GLUFPoint pt, WPARAM wParam, LPARAM lParam);
	virtual bool    MsgProc(GLUF_MESSAGE_TYPE msg, int param1, int param2, int param3, int param4);

	virtual void    Render(float fElapsedTime);
	virtual void    UpdateRects();

	void            SetTrackRange(int nStart, int nEnd);
	int             GetTrackPos()				{ return m_nPosition;									}
	void            SetTrackPos(int nPosition)	{ m_nPosition = nPosition; Cap(); UpdateThumbRect(); }
	int             GetPageSize()				{ return m_nPageSize;									 }
	void            SetPageSize(int nPageSize)	{ m_nPageSize = nPageSize; Cap(); UpdateThumbRect(); }

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

	void            UpdateThumbRect();
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
	std::wstring strText;
	void* pData;

	bool bVisible;
	GLUFRect rcActive;
	//bool bSelected;
};

class GLUFListBox : public GLUFControl
{
public:
	GLUFListBox(GLUFDialog* pDialog = NULL);
	virtual         ~GLUFListBox();

	virtual GLUFResult OnInit()		{ return m_pDialog->InitControl(&m_ScrollBar);	}
	virtual bool    CanHaveFocus()	{ return (m_bVisible && m_bEnabled);			}
	//virtual bool    HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);
	//virtual bool    HandleMouse(UINT uMsg, GLUFPoint pt, WPARAM wParam, LPARAM lParam);
	virtual bool    MsgProc(GLUF_MESSAGE_TYPE msg, int param1, int param2, int param3, int param4);

	virtual void    Render(float fElapsedTime);
	virtual void    UpdateRects();

	long            GetStyle() const					{ return m_dwStyle;							}
	int             GetSize() const						{ return m_Items.size();					}
	void            SetStyle(long dwStyle)				{ m_dwStyle = dwStyle;						}
	float           GetScrollBarWidth() const			{ return m_fSBWidth; }
	void            SetScrollBarWidth(float nWidth)		{ m_fSBWidth = nWidth; UpdateRects(); }
	//This should be in PIXELS
	void            SetBorderPixels(int nBorder, int nMargin);
	void            SetBorder(float fBorder, float fMargin){ m_fBorder = fBorder; m_fMargin = fMargin; }
	GLUFResult      AddItem(std::wstring wszText, void* pData);
	GLUFResult      InsertItem(int nIndex, std::wstring wszText, void* pData);
	void            RemoveItem(int nIndex);
	void            RemoveAllItems();

	GLUFListBoxItem* GetItem(int nIndex);
	int              GetSelectedIndex(int nPreviousSelected = -1);
	GLUFListBoxItem* GetSelectedItem(int nPreviousSelected = -1){ return GetItem(GetSelectedIndex(nPreviousSelected)); }
	void            SelectItem(int nNewIndex);

	enum STYLE
	{
		MULTISELECTION = 1
	};

	virtual bool ContainsPoint(GLUFPoint pt){ return GLUFControl::ContainsPoint(pt) || m_ScrollBar.ContainsPoint(pt); }

protected:
	GLUFRect m_rcText;      // Text rendering bound
	GLUFRect m_rcSelection; // Selection box bound
	GLUFScrollBar m_ScrollBar;
	float m_fSBWidth;
	float m_fBorder; //top / bottom
	float m_fMargin;	//left / right
	float m_fTextHeight;  // Height of a single line of text
	long m_dwStyle;     // List box style
	std::vector<int> m_Selected;//this has a size of 1 for single selected boxes
	bool m_bDrag;       // Whether the user is dragging the mouse to select

	std::vector <GLUFListBoxItem*> m_Items;

	void UpdateItemRects();
};


//-----------------------------------------------------------------------------
// ComboBox control
//-----------------------------------------------------------------------------
struct GLUFComboBoxItem
{
	std::wstring strText;
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
	virtual GLUFResult OnInit(){ return m_pDialog->InitControl(&m_ScrollBar); }

	virtual bool MsgProc(GLUF_MESSAGE_TYPE msg, int param1, int param2, int param3, int param4);

	//virtual bool    HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);
	//virtual bool    HandleMouse(UINT uMsg, GLUFPoint pt, WPARAM wParam, LPARAM lParam);
	virtual void    OnHotkey();

	virtual bool    CanHaveFocus(){ return (m_bVisible && m_bEnabled); }
	virtual void    OnFocusOut();
	virtual void    Render(float fElapsedTime);

	virtual void    UpdateRects();

	GLUFResult      AddItem(std::wstring strText, void* pData);
	void            RemoveAllItems();
	void            RemoveItem(unsigned int index);
	bool            ContainsItem(std::wstring strText, unsigned int iStart = 0);
	int             FindItem(std::wstring strText, unsigned int iStart = 0);
	void*			GetItemData(std::wstring strText);
	void*			GetItemData(int nIndex);
	void            SetDropHeight(float nHeight)			{ m_fDropHeight = nHeight; UpdateRects();		}
	float           GetScrollBarWidth() const				{ return m_fSBWidth;							}
	void            SetScrollBarWidth(float nWidth)			{ m_fSBWidth = nWidth; UpdateRects();			}

	int             GetSelectedIndex() const				{ return m_iSelected;							}
	void* GetSelectedData();
	GLUFComboBoxItem* GetSelectedItem();

	unsigned int      GetNumItems()							{ return m_Items.size();						}
	GLUFComboBoxItem* GetItem(unsigned int index)			{ return m_Items[index];						}

	GLUFResult         SetSelectedByIndex(unsigned int index);
	GLUFResult         SetSelectedByText(std::wstring strText);
	GLUFResult         SetSelectedByData(void* pData);

protected:
	void UpdateItemRects();

	int m_iSelected;
	int m_iFocused;
	float m_fDropHeight;//normalized
	GLUFScrollBar m_ScrollBar;
	float m_fSBWidth;

	bool m_bOpened;

	GLUFRect m_rcText;
	GLUFRect m_rcButton;
	GLUFRect m_rcDropdown;
	GLUFRect m_rcDropdownText;


	std::vector <GLUFComboBoxItem*> m_Items;
};


//-----------------------------------------------------------------------------
// Slider control TODO: support horizontal sliders as well
//-----------------------------------------------------------------------------
class GLUFSlider : public GLUFControl
{
public:
	GLUFSlider(GLUFDialog* pDialog = NULL);

	virtual bool    ContainsPoint(GLUFPoint pt);
	virtual bool    CanHaveFocus(){ return (m_bVisible && m_bEnabled); }
	//virtual bool    HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);
	//virtual bool    HandleMouse(UINT uMsg, GLUFPoint pt, WPARAM wParam, LPARAM lParam);

	virtual bool	MsgProc(GLUF_MESSAGE_TYPE msg, int param1, int param2, int param3, int param4);

	virtual void    UpdateRects();

	virtual void    Render(float fElapsedTime);

	void            SetValue(float nValue){ SetValueInternal(nValue, false); }
	float           GetValue() const{ return m_fValue; };

	void            GetRange(float& nMin, float& nMax) const{ nMin = m_fMin; nMax = m_fMax; }
	void            SetRange(float nMin, float nMax);

protected:
	void            SetValueInternal(float nValue, bool bFromInput);
	float           ValueFromPos(float x);

	float m_fValue;

	float m_fMin;
	float m_fMax;

	float m_fDragX;      // Mouse position at start of drag
	float m_fDragOffset; // Drag offset from the center of the button
	float m_fButtonX;

	bool m_bPressed;
	GLUFRect m_rcButton;
};

//-----------------------------------------------------------------------------
// EditBox control TODO: make text insertion AND fix things
//-----------------------------------------------------------------------------
class GLUFEditBox : public GLUFControl
{
public:
	GLUFEditBox(GLUFDialog* pDialog = NULL);
	virtual         ~GLUFEditBox();

	//virtual bool    HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam);
	//virtual bool    HandleMouse(UINT uMsg, GLUFPoint pt, WPARAM wParam, LPARAM lParam);
	virtual bool    MsgProc(GLUF_MESSAGE_TYPE msg, int param1, int param2, int param3, int param4);
	virtual void    UpdateRects();
	virtual bool    CanHaveFocus(){ return (m_bVisible && m_bEnabled); }
	virtual void    Render(float fElapsedTime);
	virtual void    OnFocusIn();
	virtual GLUFResult OnInit(){ return m_pDialog->InitControl(&m_ScrollBar); }

	void            SetText(std::wstring wszText, bool bSelected = false);
	std::wstring     GetText(){ return m_strBuffer; }
	int             GetTextLength(){ return m_strBuffer.length(); }  // Returns text length in chars excluding NULL.
	std::wstring     GetTextClamped();//this gets the text, but clamped to the bounding box, (NOTE: this will overflow off the bottom);
	void            ClearText();
	virtual void    SetTextColor(Color Color){ m_TextColor = Color; }  // Text color
	void            SetSelectedTextColor(Color Color){ m_SelTextColor = Color; }  // Selected text color
	void            SetSelectedBackColor(Color Color){ m_SelBkColor = Color; }  // Selected background color
	void            SetCaretColor(Color Color){ m_CaretColor = Color; }  // Caret color
	void            SetBorderWidth(float fBorderX, float fBorderY){ m_fBorderX = fBorderX; m_fBorderY = fBorderY; UpdateRects(); m_bAnalyseRequired = true; }  // Border of the window
	void            SetSpacing(float fSpacingX, float fSpacingY){ m_fSpacingX = fSpacingX; m_fSpacingY = fSpacingY; UpdateRects(); m_bAnalyseRequired = true; }
	//void            ParseFloatArray(float* pNumbers, int nCount);
	//void            SetTextFloatArray(const float* pNumbers, int nCount);


protected:
	void            PlaceCaret(int nCP);//input m_strBuffer
	void			PlaceCaretRndBuffer(int nRndCp);//input m_strRenderBuffer
	void            DeleteSelectionText();
	void            ResetCaretBlink();
	void            CopyToClipboard();
	void            PasteFromClipboard();

	//NOTE: nCP must be the index from m_strRenderBuffer;
	int             GetLineNumberFromCharPos(unsigned int nCP);//the value returned is the line number within the box, so even if it is scrolled, the top line is still 0

	int				GetStrIndexFromStrRenderIndex(int strRenderIndex);//this is used to convert an index of an object that was clicked on the screen to the index of the real string
	int             GetStrRenderIndexFromStrIndex(int strIndex);//just the opposite

	//former CUniBuffer Methods
	void Analyse();
	
	//NOTE: input the cursor position in m_strRenderBuffer space
	bool CPtoRC(int nCP, bool bTrail, GLUFRect *pPt);

	//NOTE: outputs the cursor position in m_strRenderBuffer space
	bool PttoCP(GLUFPoint pt, int* pCP, bool* bTrail);
	
	//NOTE: all methods referencing a position within the edit box will be done IN STRING  SPACE and will be converted appropriately to make it so

	void InsertString(int pos, std::wstring str);
	void InsertChar(int pos, wchar_t ch);

	void RemoveString(int pos, int len);
	void RemoveChar(int pos);

	void GetNextItemPos(int pos, int& next);
	void GetPriorItemPos(int pos, int& prior);

	int GetNumNewlines();

	//GLUFUniBuffer m_Buffer;     // Buffer to hold text

	std::wstring m_strBuffer; //buffer to hold the text
	std::wstring m_strRenderBuffer;//this stores the string that will be rendered, this inclues newlines inserted at the end of the rect
	unsigned int m_strRenderBufferOffset;//the distance between the start of the string buffer, and the start of the render string buffer
	std::vector<size_t> m_strInsertedNewlineLocations;//the location of all of the newlines that were inserted into the render string
	std::vector<size_t> m_nAdditionalInsertedCharLocations;//the number of chars inserted NOTE: these are the locations within the renderspace string

	std::vector<GLUFRect> m_CharBoundingBoxes;//a buffer to hold all of the rects of the chars. the origin is the botom left of the text region also, this is based on m_strRenderBuffer, NOT m_strBuffer
	bool m_bAnalyseRequired;            // True if the string has changed since last analysis.

	float m_fBorderX, m_fBorderY;      // Border of the window
	float m_fSpacingX, m_fSpacingY;     // Spacing between the text and the edge of border
	GLUFRect m_rcText;       // Bounding GLUFRectangle for the text
	GLUFRect            m_rcRender[9];  // Convenient Rectangles for rendering elements
	double m_dfBlink;      // Caret blink time in milliseconds
	double m_dfLastBlink;  // Last timestamp of caret blink
	bool m_bCaretOn;     // Flag to indicate whether caret is currently visible
	int m_nCaret;       // Caret position, in characters (of the original string buffer)
	bool m_bInsertMode;  // If true, control is in insert mode. Else, overwrite mode.
	int m_nSelStart;    // Starting position of the selection. The caret marks the end.
	int m_nFirstVisible;// First visible character in the edit control
	Color m_TextColor;    // Text color
	Color m_SelTextColor; // Selected text color
	Color m_SelBkColor;   // Selected background color
	Color m_CaretColor;   // Caret color
	//std::vector<int> m_CharLineBreaks;//a vector of the positions of line breaks

	GLUFScrollBar m_ScrollBar;//TODO: impliment
	float m_fSBWidth;

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
	GLUFTextHelper(GLUFDialogResourceManager* pManager, GLUFFontSize fLineHeight);
	~GLUFTextHelper();

	//line height = leadding
	void Init(GLUFFontSize fLineHeight = 0.025f);

	void SetInsertionPos(GLUFPoint pt)
	{
		m_pt = pt;
	}
	void SetForegroundColor(Color clr) { m_clr = clr; }

	void       Begin(GLUFFontIndex fontToUse, GLUFFontSize size, GLUF_FONT_WEIGHT weight);
	GLUFResult DrawFormattedTextLine(const wchar_t* strMsg, size_t strLen, ...);
	GLUFResult DrawTextLine(const wchar_t* strMsg, size_t strLen);
	//GLUFResult DrawFormattedTextLine(const GLUFRect& rc, _In_ DWORD dwFlags, const WCHAR* strMsg, ...);
	//GLUFResult DrawTextLine(_In_ const GLUFRect& rc, _In_ DWORD dwFlags, _In_z_ const WCHAR* strMsg);
	GLUFResult DrawFormattedTextLine(const GLUFRect& rc, unsigned int dwFlags, const wchar_t* strMsg, size_t strLen, ...);
	GLUFResult DrawTextLine(const GLUFRect& rc, unsigned int dwFlags, const wchar_t* strMsg, size_t strLen);
	void    End();

protected:
	Color m_clr;
	GLUFPoint m_pt;
	GLUFFontSize m_fLineHeight;


	GLUFDialogResourceManager* m_pManager;

	GLUFFontIndex m_nFont;
	GLUFFontSize m_fFontSize;
	GLUF_FONT_WEIGHT m_Weight;
};
