#include "stdafx.h"
#include "GLUFGui.h"
#include "ft2build.h"
#include FT_FREETYPE_H
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//IMPORTANT///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//																						//////////////////////////////////////
//																						//////////////////////////////////////
//	I WILL NOT be using the api declared in ObjGLUF.h in this file UNTIL i get it		//////////////////////////////////////
//		to a more reliable state														//////////////////////////////////////
//																						//////////////////////////////////////
//																						//////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//======================================================================================
// Various Structs that are used exclusively for UI
//======================================================================================

struct GLUFScreenVertex
{
	glm::vec3 pos;
	Color     color;
	glm::vec2 uv;
};

struct GLUFScreenVertexUntex
{
	glm::vec3 pos;
	Color     color;
};

//======================================================================================
// Initialization and globals
//======================================================================================


GLUFProgramPtr g_UIProgram;
GLUFProgramPtr g_TextProgram;

const char* g_UIShaderVert =
"#version 430 core" \
"";

const char* g_UIShaderFrag =
"#version 430 core" \
"";


const char* g_TextShaderVert =
"#version 430 core" \
"";

const char* g_TextShaderFrag =
"#version 430 core" \
"";


//the freetype library
FT_Library ft;

void GLUFInitGui()
{
	GLUF_ASSERT(FT_Init_FreeType(&ft));

	//load the ui shader
	GLUFShaderSourceList sources;
	sources.insert(std::pair<GLUFShaderType, const char*>(SH_VERTEX_SHADER, g_UIShaderVert));
	sources.insert(std::pair<GLUFShaderType, const char*>(SH_FRAGMENT_SHADER, g_UIShaderFrag));
	g_UIProgram = GLUFSHADERMANAGER.CreateProgram(sources);
	sources.clear();

	sources.insert(std::pair<GLUFShaderType, const char*>(SH_VERTEX_SHADER, g_TextShaderVert));
	sources.insert(std::pair<GLUFShaderType, const char*>(SH_FRAGMENT_SHADER, g_TextShaderFrag));
	g_TextProgram = GLUFSHADERMANAGER.CreateProgram(sources);
}

//======================================================================================
// GLUFFont
//======================================================================================


class GLUFFont
{
	FT_Face mFontFace;
	GLUFFontSize mFontSize;
public:

	bool Init(unsigned char* data, uint64_t rawSize);

	void SetFontSize(GLUFFontSize fontSize);
	GLUFFontSize GetFontSize(){ return mFontSize; }

	//font properties


	operator FT_Face*(){ return &mFontFace; }
};

bool GLUFFont::Init(unsigned char* data, uint64_t rawSize)
{
	FT_Error err = FT_New_Memory_Face(ft, data, rawSize, 0, &mFontFace);
	if (err)
	{
		GLUF_ERROR("Error loading font Error code: " + err);
		return false;
	}

	return true;
}

void GLUFFont::SetFontSize(GLUFFontSize fontSize)
{
	//convert from points to pixels (4/3s)
	FT_Set_Pixel_Sizes(mFontFace, 0, GLUF_POINTS_TO_PIXELS(fontSize));
	mFontSize = GLUF_POINTS_TO_PIXELS(fontSize);
}


GLUFFontPtr GLUFLoadFont(unsigned char* rawData, uint64_t rawSize)
{
	GLUFFontPtr ret(new GLUFFont());
	ret->Init(rawData, rawSize);
}

//======================================================================================
// GLUFBlendColor
//======================================================================================

//--------------------------------------------------------------------------------------
void GLUFBlendColor::Init(Color defaultColor, Color disabledColor, Color hiddenColor)
{
	for (int i = 0; i < MAX_CONTROL_STATES; i++)
	{
		States[i] = defaultColor;
	}

	States[GLUF_STATE_DISABLED] = disabledColor;
	States[GLUF_STATE_HIDDEN] = hiddenColor;
	Current = hiddenColor;//start hidden
}


//--------------------------------------------------------------------------------------
void GLUFBlendColor::Blend(GLUF_CONTROL_STATE iState, float fElapsedTime, float fRate)
{
	//this is quite condensed, this basically interpolates from the current state to the destination state based on the time
	Current = glm::mix(Current, States[iState], 1.0f - powf(fRate, 30 * fElapsedTime));
}

void GLUFBlendColor::SetCurrent(Color current)
{
	Current = current;
}

void GLUFBlendColor::SetCurrent(GLUF_CONTROL_STATE state)
{
	Current = States[state];
}

//======================================================================================
// GLUFElement
//======================================================================================

//--------------------------------------------------------------------------------------

void GLUFElement::SetTexture(unsigned int iTexture, GLUFRect* prcTexture, Color defaultTextureColor)
{
	this->iTexture = iTexture;

	if (prcTexture)
		rcTexture = *prcTexture;
	else
		GLUFSetRectEmpty(rcTexture);

	TextureColor.Init(defaultTextureColor);
}


//--------------------------------------------------------------------------------------

void GLUFElement::SetFont(unsigned int iFont, Color defaultFontColor, unsigned int dwTextFormat)
{
	this->iFont = iFont;
	this->dwTextFormat = dwTextFormat;

	FontColor.Init(defaultFontColor);
}


//--------------------------------------------------------------------------------------
void GLUFElement::Refresh()
{
	TextureColor.SetCurrent(GLUF_STATE_HIDDEN);
	FontColor.SetCurrent(GLUF_STATE_HIDDEN);
}



//======================================================================================
// GLUFDialog class
//======================================================================================

GLUFDialog::GLUFDialog() :
m_x(0),
m_y(0),
m_width(0),
m_height(0),
m_pManager(nullptr),
m_bVisible(true),
m_bCaption(false),
m_bMinimized(false),
m_bDrag(false),
m_nCaptionHeight(18),
m_colorTopLeft(0),
m_colorTopRight(0),
m_colorBottomLeft(0),
m_colorBottomRight(0),
m_pCallbackEvent(nullptr),
m_pCallbackEventUserContext(nullptr),
m_fTimeLastRefresh(0),
m_pControlMouseOver(nullptr),
m_nDefaultControlID(0xffff),
m_bNonUserEvents(false),
m_bKeyboardInput(false),
m_bMouseInput(true)
{
	m_wszCaption[0] = L'\0';

	m_pNextDialog = this;
	m_pPrevDialog = this;
}


//--------------------------------------------------------------------------------------
GLUFDialog::~GLUFDialog()
{
	RemoveAllControls();

	m_Fonts.clear();
	m_Textures.clear();

	for (auto it = m_DefaultElements.begin(); it != m_DefaultElements.end(); ++it)
	{
		GLUF_SAFE_DELETE(*it);
	}

	m_DefaultElements.clear();
}


//--------------------------------------------------------------------------------------

void GLUFDialog::Init(GLUFDialogResourceManager* pManager, bool bRegisterDialog)
{
	m_pManager = pManager;
	if (bRegisterDialog)
		pManager->RegisterDialog(this);

	SetTexture(0);
	InitDefaultElements();
}


//--------------------------------------------------------------------------------------

void GLUFDialog::Init(GLUFDialogResourceManager* pManager, bool bRegisterDialog, unsigned int iTexture)
{
	m_pManager = pManager;
	if (bRegisterDialog)
		pManager->RegisterDialog(this);
	SetTexture(iTexture);
	InitDefaultElements();
}


//--------------------------------------------------------------------------------------

/*void GLUFDialog::Init(GLUFDialogResourceManager* pManager, bool bRegisterDialog, LPCWSTR szControlTextureResourceName, HMODULE hControlTextureResourceModule)
{
	m_pManager = pManager;
	if (bRegisterDialog)
		pManager->RegisterDialog(this);

	SetTexture(0, szControlTextureResourceName, hControlTextureResourceModule);
	InitDefaultElements();
}*/


//--------------------------------------------------------------------------------------

void GLUFDialog::SetCallback(PCALLBACKGLUFGUIEVENT pCallback, void* pUserContext)
{
	// If this assert triggers, you need to call GLUFDialog::Init() first.  This change
	// was made so that the GLUF's GUI could become seperate and optional from GLUF's core.  The 
	// creation and interfacing with GLUFDialogResourceManager is now the responsibility 
	// of the application if it wishes to use GLUF's GUI.
	GLUF_ASSERT(m_pManager && L"To fix call GLUFDialog::Init() first.  See comments for details.");

	m_pCallbackEvent = pCallback;
	m_pCallbackEventUserContext = pUserContext;
}


//--------------------------------------------------------------------------------------
void GLUFDialog::RemoveControl(int ID)
{
	for (auto it = m_Controls.begin(); it != m_Controls.end(); ++it)
	{
		if ((*it)->GetID() == ID)
		{
			// Clean focus first
			ClearFocus();

			// Clear references to this control
			if (s_pControlFocus == (*it))
				s_pControlFocus = nullptr;
			if (s_pControlPressed == (*it))
				s_pControlPressed = nullptr;
			if (m_pControlMouseOver == (*it))
				m_pControlMouseOver = nullptr;

			GLUF_SAFE_DELETE((*it));
			m_Controls.erase(it);

			return;
		}
	}
}


//--------------------------------------------------------------------------------------
void GLUFDialog::RemoveAllControls()
{
	if (s_pControlFocus && s_pControlFocus->m_pDialog == this)
		s_pControlFocus = nullptr;
	if (s_pControlPressed && s_pControlPressed->m_pDialog == this)
		s_pControlPressed = nullptr;
	m_pControlMouseOver = nullptr;

	for (auto it = m_Controls.begin(); it != m_Controls.end(); ++it)
	{
		GLUF_SAFE_DELETE(*it);
	}

	m_Controls.clear();
}


//--------------------------------------------------------------------------------------
void GLUFDialog::Refresh()
{
	if (s_pControlFocus)
		s_pControlFocus->OnFocusOut();

	if (m_pControlMouseOver)
		m_pControlMouseOver->OnMouseLeave();

	s_pControlFocus = nullptr;
	s_pControlPressed = nullptr;
	m_pControlMouseOver = nullptr;

	for (auto it = m_Controls.begin(); it != m_Controls.end(); ++it)
	{
		(*it)->Refresh();
	}

	if (m_bKeyboardInput)
		FocusDefaultControl();
}


//--------------------------------------------------------------------------------------
GLUFResult GLUFDialog::OnRender(float fElapsedTime)
{
	// If this assert triggers, you need to call GLUFDialogResourceManager::On*Device() from inside
	// the application's device callbacks.  See the SDK samples for an example of how to do this.
	//GLUF_ASSERT(m_pManager->GetD3D11Device() &&
	//	L"To fix hook up GLUFDialogResourceManager to device callbacks.  See comments for details");
	//no need for "devices", this is all handled by GLFW

	// See if the dialog needs to be refreshed
	if (m_fTimeLastRefresh < s_fTimeRefresh)
	{
		m_fTimeLastRefresh = GLUFGetTime();
		Refresh();
	}

	// For invisible dialog, out now.
	if (!m_bVisible ||
		(m_bMinimized && !m_bCaption))
		return GR_SUCCESS;

	//ID3D11Device* pd3dDevice = m_pManager->GetD3D11Device();
	//ID3D11DeviceContext* pd3dDeviceContext = m_pManager->GetD3D11DeviceContext();

	// Set up a state block here and restore it when finished drawing all the controls
	//m_pManager->StoreD3D11State(pd3dDeviceContext);

	//if any of them are visible, then draw
	bool bBackgroundIsVisible = (m_colorTopLeft.a > 0.0f || m_colorTopRight.a > 0.0f || m_colorBottomRight.a > 0.0f || m_colorBottomLeft.a > 0.0f);
	if (!m_bMinimized && bBackgroundIsVisible)
	{
		// Convert the draw rectangle from screen coordinates to clip space coordinates.
		float Left, Right, Top, Bottom;
		Left = m_x * 2.0f / m_pManager->m_nBackBufferWidth - 1.0f;
		Right = (m_x + m_width) * 2.0f / m_pManager->m_nBackBufferWidth - 1.0f;
		Top = 1.0f - m_y * 2.0f / m_pManager->m_nBackBufferHeight;
		Bottom = 1.0f - (m_y + m_height) * 2.0f / m_pManager->m_nBackBufferHeight;

		/*glm::vec3 vertices[4] =
		{
			glm::vec3(Left, Top, 0.5f), m_colorTopLeft, glm::vec2(0.0f, 0.0f),
			glm::vec3(Right, Top, 0.5f), m_colorTopRight, glm::vec2(1.0f, 0.0f),
			glm::vec3(Left, Bottom, 0.5f), m_colorBottomLeft, glm::vec2(0.0f, 1.0f),
			glm::vec3(Right, Bottom, 0.5f), m_colorBottomRight, glm::vec2(1.0f, 1.0f)
		};*/

		glm::vec3 positions[4] = 
		{ 
			glm::vec3(Left, Top, 0.5f), 
			glm::vec3(Right, Top, 0.5f), 
			glm::vec3(Left, Bottom, 0.5f), 
			glm::vec3(Right, Bottom, 0.5f) 
		};

		Color colors[4] = 
		{
			m_colorBottomLeft,
			m_colorTopRight,
			m_colorBottomLeft,
			m_colorBottomRight
		};

		glm::vec2 texCoords[4] = 
		{
			glm::vec2(0.0f, 0.0f),
			glm::vec2(1.0f, 0.0f),
			glm::vec2(0.0f, 1.0f),
			glm::vec2(1.0f, 1.0f) 
		};

		//GLUF_SCREEN_VERTEX_10 *pVB;
		/*D3D11_MAPPED_SUBRESOURCE MappedData;
		if (SUCCEEDED(pd3dDeviceContext->Map(m_pManager->m_pVBScreenQuad11, 0, D3D11_MAP_WRITE_DISCARD,
			0, &MappedData)))
		{
			memcpy(MappedData.pData, vertices, sizeof(vertices));
			pd3dDeviceContext->Unmap(m_pManager->m_pVBScreenQuad11, 0);
		}*/

		glBindVertexArray(m_pManager->m_pVBScreenQuadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_pManager->m_pVBScreenQuadPositions);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec3), positions, GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, m_pManager->m_pVBScreenQuadColor);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Color), colors, GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, m_pManager->m_pVBScreenQuadUVs);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec2), texCoords, GL_STREAM_DRAW);
		

		// Set the quad VB as current
		/*UINT stride = sizeof(GLUF_SCREEN_VERTEX_10);
		UINT offset = 0;
		pd3dDeviceContext->IASetVertexBuffers(0, 1, &m_pManager->m_pVBScreenQuad11, &stride, &offset);
		pd3dDeviceContext->IASetInputLayout(m_pManager->m_pInputLayout11);
		pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);*/

		// Setup for rendering
		//m_pManager->ApplyRenderUIUntex11(pd3dDeviceContext);
		//pd3dDeviceContext->Draw(4, 0);


		glEnableVertexAttribArray(0);//Positions
		glEnableVertexAttribArray(1);//Colors
		glEnableVertexAttribArray(2);//UVs

		GLUFSHADERMANAGER.UseProgram(g_UIProgram);

		//well we are drawing a square (would there be any reason to make this GL_TRIANGLES?)
		glDrawArrays(GL_QUADS, 0, 4);
	}

	GLUFTextureNode* pTextureNode = GetTexture(0);
	//pd3dDeviceContext->PSSetShaderResources(0, 1, &pTextureNode->pTexResView11);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pTextureNode->m_pTextureElement);
	glUniform1f(m_pManager->m_pSamplerLocation, 0);/*0 corresponds to GL_TEXTURE0*/


	// Sort depth back to front
	//m_pManager->BeginSprites11();
	BeginText();

	//m_pManager->ApplyRenderUI11(pd3dDeviceContext);
	//TODO: ENDED HERE
	// Render the caption if it's enabled.
	if (m_bCaption)
	{
		// DrawSprite will offset the rect down by
		// m_nCaptionHeight, so adjust the rect higher
		// here to negate the effect.
		GLUFRect rc = { 0, -m_nCaptionHeight, m_width, 0 };
		DrawSprite(&m_CapElement, &rc, 0.99f);
		rc.left += 5; // Make a left margin
		if (m_bMinimized)
			DrawText("(Minimized)", &m_CapElement, &rc, true);
		DrawText(m_wszCaption, &m_CapElement, &rc, true);
	}

	// If the dialog is minimized, skip rendering
	// its controls.
	if (!m_bMinimized)
	{
		for (auto it = m_Controls.cbegin(); it != m_Controls.cend(); ++it)
		{
			// Focused control is drawn last
			if (*it == s_pControlFocus)
				continue;

			(*it)->Render(fElapsedTime);
		}

		if (s_pControlFocus && s_pControlFocus->m_pDialog == this)
			s_pControlFocus->Render(fElapsedTime);
	}

	// End sprites
	if (m_bCaption)
	{
		//m_pManager->EndSprites11(pd3dDevice, pd3dDeviceContext);
		EndText();
	}
	m_pManager->RestoreD3D11State(pd3dDeviceContext);

	return GR_SUCCESS;
}


//--------------------------------------------------------------------------------------

VOID GLUFDialog::SendEvent(UINT nEvent, bool bTriggeredByUser, CGLUFControl* pControl)
{
	// If no callback has been registered there's nowhere to send the event to
	if (!m_pCallbackEvent)
		return;

	// Discard events triggered programatically if these types of events haven't been
	// enabled
	if (!bTriggeredByUser && !m_bNonUserEvents)
		return;

	m_pCallbackEvent(nEvent, pControl->GetID(), pControl, m_pCallbackEventUserContext);
}


//--------------------------------------------------------------------------------------

GLUFResult GLUFDialog::SetFont(UINT index, LPCWSTR strFaceName, LONG height, LONG weight)
{
	// If this assert triggers, you need to call GLUFDialog::Init() first.  This change
	// was made so that the GLUF's GUI could become seperate and optional from GLUF's core.  The 
	// creation and interfacing with GLUFDialogResourceManager is now the responsibility 
	// of the application if it wishes to use GLUF's GUI.
	assert(m_pManager && L"To fix call GLUFDialog::Init() first.  See comments for details.");
	_Analysis_assume_(m_pManager);

	// Make sure the list is at least as large as the index being set
	for (size_t i = m_Fonts.size(); i <= index; i++)
	{
		m_Fonts.push_back(-1);
	}

	int iFont = m_pManager->AddFont(strFaceName, height, weight);
	m_Fonts[index] = iFont;

	return GR_SUCCESS;
}


//--------------------------------------------------------------------------------------
GLUFFontNode* GLUFDialog::GetFont(UINT index) const
{
	if (!m_pManager)
		return nullptr;
	return m_pManager->GetFontNode(m_Fonts[index]);
}


//--------------------------------------------------------------------------------------

GLUFResult GLUFDialog::SetTexture(UINT index, LPCWSTR strFilename)
{
	// If this assert triggers, you need to call GLUFDialog::Init() first.  This change
	// was made so that the GLUF's GUI could become seperate and optional from GLUF's core.  The 
	// creation and interfacing with GLUFDialogResourceManager is now the responsibility 
	// of the application if it wishes to use GLUF's GUI.
	assert(m_pManager && L"To fix this, call GLUFDialog::Init() first.  See comments for details.");
	_Analysis_assume_(m_pManager);

	// Make sure the list is at least as large as the index being set
	for (size_t i = m_Textures.size(); i <= index; i++)
	{
		m_Textures.push_back(-1);
	}

	int iTexture = m_pManager->AddTexture(strFilename);

	m_Textures[index] = iTexture;
	return GR_SUCCESS;
}


//--------------------------------------------------------------------------------------

GLUFResult GLUFDialog::SetTexture(UINT index, LPCWSTR strResourceName, HMODULE hResourceModule)
{
	// If this assert triggers, you need to call GLUFDialog::Init() first.  This change
	// was made so that the GLUF's GUI could become seperate and optional from GLUF's core.  The 
	// creation and interfacing with GLUFDialogResourceManager is now the responsibility 
	// of the application if it wishes to use GLUF's GUI.
	assert(m_pManager && L"To fix this, call GLUFDialog::Init() first.  See comments for details.");
	_Analysis_assume_(m_pManager);

	// Make sure the list is at least as large as the index being set
	for (size_t i = m_Textures.size(); i <= index; i++)
	{
		m_Textures.push_back(-1);
	}

	int iTexture = m_pManager->AddTexture(strResourceName, hResourceModule);

	m_Textures[index] = iTexture;
	return GR_SUCCESS;
}


//--------------------------------------------------------------------------------------
GLUFTextureNode* GLUFDialog::GetTexture(UINT index) const
{
	if (!m_pManager)
		return nullptr;
	return m_pManager->GetTextureNode(m_Textures[index]);
}


//--------------------------------------------------------------------------------------

bool GLUFDialog::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	bool bHandled = false;

	// For invisible dialog, do not handle anything.
	if (!m_bVisible)
		return false;

	// If automation command-line switch is on, enable this dialog's keyboard input
	// upon any key press or mouse click.
	if (GLUFGetAutomation() &&
		(WM_LBUTTONDOWN == uMsg || WM_LBUTTONDBLCLK == uMsg || WM_KEYDOWN == uMsg))
	{
		m_pManager->EnableKeyboardInputForAllDialogs();
	}

	// If caption is enable, check for clicks in the caption area.
	if (m_bCaption)
	{
		if (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK)
		{
			POINT mousePoint =
			{
				short(LOWORD(lParam)), short(HIWORD(lParam))
			};

			if (mousePoint.x >= m_x && mousePoint.x < m_x + m_width &&
				mousePoint.y >= m_y && mousePoint.y < m_y + m_nCaptionHeight)
			{
				m_bDrag = true;
				SetCapture(GLUFGetHWND());
				return true;
			}
		}
		else if (uMsg == WM_LBUTTONUP && m_bDrag)
		{
			POINT mousePoint =
			{
				short(LOWORD(lParam)), short(HIWORD(lParam))
			};

			if (mousePoint.x >= m_x && mousePoint.x < m_x + m_width &&
				mousePoint.y >= m_y && mousePoint.y < m_y + m_nCaptionHeight)
			{
				ReleaseCapture();
				m_bDrag = false;
				m_bMinimized = !m_bMinimized;
				return true;
			}
		}
	}

	// If the dialog is minimized, don't send any messages to controls.
	if (m_bMinimized)
		return false;

	// If a control is in focus, it belongs to this dialog, and it's enabled, then give
	// it the first chance at handling the message.
	if (s_pControlFocus &&
		s_pControlFocus->m_pDialog == this &&
		s_pControlFocus->GetEnabled())
	{
		// If the control MsgProc handles it, then we don't.
		if (s_pControlFocus->MsgProc(uMsg, wParam, lParam))
			return true;
	}

	switch (uMsg)
	{
	case WM_SIZE:
	case WM_MOVE:
	{
		// Handle sizing and moving messages so that in case the mouse cursor is moved out
		// of an UI control because of the window adjustment, we can properly
		// unhighlight the highlighted control.
		POINT pt =
		{
			-1, -1
		};
		OnMouseMove(pt);
		break;
	}

	case WM_ACTIVATEAPP:
		// Call OnFocusIn()/OnFocusOut() of the control that currently has the focus
		// as the application is activated/deactivated.  This matches the Windows
		// behavior.
		if (s_pControlFocus &&
			s_pControlFocus->m_pDialog == this &&
			s_pControlFocus->GetEnabled())
		{
			if (wParam)
				s_pControlFocus->OnFocusIn();
			else
				s_pControlFocus->OnFocusOut();
		}
		break;

		// Keyboard messages
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		// If a control is in focus, it belongs to this dialog, and it's enabled, then give
		// it the first chance at handling the message.
		if (s_pControlFocus &&
			s_pControlFocus->m_pDialog == this &&
			s_pControlFocus->GetEnabled())
			for (auto it = m_Controls.cbegin(); it != m_Controls.cend(); ++it)
			{
			if (s_pControlFocus->HandleKeyboard(uMsg, wParam, lParam))
				return true;
			}

		// Not yet handled, see if this matches a control's hotkey
		// Activate the hotkey if the focus doesn't belong to an
		// edit box.
		if (uMsg == WM_KEYDOWN && (!s_pControlFocus ||
			(s_pControlFocus->GetType() != GLUF_CONTROL_EDITBOX
			&& s_pControlFocus->GetType() != GLUF_CONTROL_IMEEDITBOX)))
		{
			for (auto it = m_Controls.begin(); it != m_Controls.end(); ++it)
			{
				if ((*it)->GetHotkey() == wParam)
				{
					(*it)->OnHotkey();
					return true;
				}
			}
		}

		// Not yet handled, check for focus messages
		if (uMsg == WM_KEYDOWN)
		{
			// If keyboard input is not enabled, this message should be ignored
			if (!m_bKeyboardInput)
				return false;

			switch (wParam)
			{
			case VK_RIGHT:
			case VK_DOWN:
				if (s_pControlFocus)
				{
					return OnCycleFocus(true);
				}
				break;

			case VK_LEFT:
			case VK_UP:
				if (s_pControlFocus)
				{
					return OnCycleFocus(false);
				}
				break;

			case VK_TAB:
			{
				bool bShiftDown = ((GetKeyState(VK_SHIFT) & 0x8000) != 0);
				return OnCycleFocus(!bShiftDown);
			}
			}
		}

		break;
	}


		// Mouse messages
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_XBUTTONDBLCLK:
	case WM_MOUSEWHEEL:
	{
		// If not accepting mouse input, return false to indicate the message should still 
		// be handled by the application (usually to move the camera).
		if (!m_bMouseInput)
			return false;

		POINT mousePoint =
		{
			short(LOWORD(lParam)), short(HIWORD(lParam))
		};
		mousePoint.x -= m_x;
		mousePoint.y -= m_y;

		// If caption is enabled, offset the Y coordinate by the negative of its height.
		if (m_bCaption)
			mousePoint.y -= m_nCaptionHeight;

		// If a control is in focus, it belongs to this dialog, and it's enabled, then give
		// it the first chance at handling the message.
		if (s_pControlFocus &&
			s_pControlFocus->m_pDialog == this &&
			s_pControlFocus->GetEnabled())
		{
			if (s_pControlFocus->HandleMouse(uMsg, mousePoint, wParam, lParam))
				return true;
		}

		// Not yet handled, see if the mouse is over any controls
		CGLUFControl* pControl = GetControlAtPoint(mousePoint);
		if (pControl && pControl->GetEnabled())
		{
			bHandled = pControl->HandleMouse(uMsg, mousePoint, wParam, lParam);
			if (bHandled)
				return true;
		}
		else
		{
			// Mouse not over any controls in this dialog, if there was a control
			// which had focus it just lost it
			if (uMsg == WM_LBUTTONDOWN &&
				s_pControlFocus &&
				s_pControlFocus->m_pDialog == this)
			{
				s_pControlFocus->OnFocusOut();
				s_pControlFocus = nullptr;
			}
		}

		// Still not handled, hand this off to the dialog. Return false to indicate the
		// message should still be handled by the application (usually to move the camera).
		switch (uMsg)
		{
		case WM_MOUSEMOVE:
			OnMouseMove(mousePoint);
			return false;
		}

		break;
	}

	case WM_CAPTURECHANGED:
	{
		// The application has lost mouse capture.
		// The dialog object may not have received
		// a WM_MOUSEUP when capture changed. Reset
		// m_bDrag so that the dialog does not mistakenly
		// think the mouse button is still held down.
		if ((HWND)lParam != hWnd)
			m_bDrag = false;
	}
	}

	return false;
}


//--------------------------------------------------------------------------------------
CGLUFControl* GLUFDialog::GetControlAtPoint(const POINT& pt) const
{
	// Search through all child controls for the first one which
	// contains the mouse point
	for (auto it = m_Controls.cbegin(); it != m_Controls.cend(); ++it)
	{
		if (!*it)
		{
			continue;
		}

		// We only return the current control if it is visible
		// and enabled.  Because GetControlAtPoint() is used to do mouse
		// hittest, it makes sense to perform this filtering.
		if ((*it)->ContainsPoint(pt) && (*it)->GetEnabled() && (*it)->GetVisible())
		{
			return *it;
		}
	}

	return nullptr;
}


//--------------------------------------------------------------------------------------
bool GLUFDialog::GetControlEnabled(int ID) const
{
	CGLUFControl* pControl = GetControl(ID);
	if (!pControl)
		return false;

	return pControl->GetEnabled();
}



//--------------------------------------------------------------------------------------
void GLUFDialog::SetControlEnabled(int ID, bool bEnabled)
{
	CGLUFControl* pControl = GetControl(ID);
	if (!pControl)
		return;

	pControl->SetEnabled(bEnabled);
}


//--------------------------------------------------------------------------------------
void GLUFDialog::OnMouseUp(const POINT& pt)
{
	UNREFERENCED_PARAMETER(pt);
	s_pControlPressed = nullptr;
	m_pControlMouseOver = nullptr;
}


//--------------------------------------------------------------------------------------
void GLUFDialog::OnMouseMove(const POINT& pt)
{
	// Figure out which control the mouse is over now
	CGLUFControl* pControl = GetControlAtPoint(pt);

	// If the mouse is still over the same control, nothing needs to be done
	if (pControl == m_pControlMouseOver)
		return;

	// Handle mouse leaving the old control
	if (m_pControlMouseOver)
		m_pControlMouseOver->OnMouseLeave();

	// Handle mouse entering the new control
	m_pControlMouseOver = pControl;
	if (pControl)
		m_pControlMouseOver->OnMouseEnter();
}


//--------------------------------------------------------------------------------------

GLUFResult GLUFDialog::SetDefaultElement(UINT nControlType, UINT iElement, CGLUFElement* pElement)
{
	// If this Element type already exist in the list, simply update the stored Element
	for (auto it = m_DefaultElements.begin(); it != m_DefaultElements.end(); ++it)
	{
		if ((*it)->nControlType == nControlType &&
			(*it)->iElement == iElement)
		{
			(*it)->Element = *pElement;
			return GR_SUCCESS;
		}
	}

	// Otherwise, add a new entry
	GLUFElementHolder* pNewHolder;
	pNewHolder = new (std::nothrow) GLUFElementHolder;
	if (!pNewHolder)
		return E_OUTOFMEMORY;

	pNewHolder->nControlType = nControlType;
	pNewHolder->iElement = iElement;
	pNewHolder->Element = *pElement;

	m_DefaultElements.push_back(pNewHolder);

	return GR_SUCCESS;
}


//--------------------------------------------------------------------------------------

CGLUFElement* GLUFDialog::GetDefaultElement(UINT nControlType, UINT iElement) const
{
	for (auto it = m_DefaultElements.cbegin(); it != m_DefaultElements.cend(); ++it)
	{
		if ((*it)->nControlType == nControlType &&
			(*it)->iElement == iElement)
		{
			return &(*it)->Element;
		}
	}

	return nullptr;
}


//--------------------------------------------------------------------------------------

GLUFResult GLUFDialog::AddStatic(int ID, LPCWSTR strText, int x, int y, int width, int height, bool bIsDefault,
CGLUFStatic** ppCreated)
{
	GLUFResult hr = GR_SUCCESS;

	CGLUFStatic* pStatic = new (std::nothrow) CGLUFStatic(this);

	if (ppCreated)
		*ppCreated = pStatic;

	if (!pStatic)
		return E_OUTOFMEMORY;

	hr = AddControl(pStatic);
	if (FAILED(hr))
		return hr;

	// Set the ID and list index
	pStatic->SetID(ID);
	pStatic->SetText(strText);
	pStatic->SetLocation(x, y);
	pStatic->SetSize(width, height);
	pStatic->m_bIsDefault = bIsDefault;

	return GR_SUCCESS;
}


//--------------------------------------------------------------------------------------

GLUFResult GLUFDialog::AddButton(int ID, LPCWSTR strText, int x, int y, int width, int height, UINT nHotkey,
bool bIsDefault, CGLUFButton** ppCreated)
{
	GLUFResult hr = GR_SUCCESS;

	CGLUFButton* pButton = new (std::nothrow) CGLUFButton(this);

	if (ppCreated)
		*ppCreated = pButton;

	if (!pButton)
		return E_OUTOFMEMORY;

	hr = AddControl(pButton);
	if (FAILED(hr))
		return hr;

	// Set the ID and list index
	pButton->SetID(ID);
	pButton->SetText(strText);
	pButton->SetLocation(x, y);
	pButton->SetSize(width, height);
	pButton->SetHotkey(nHotkey);
	pButton->m_bIsDefault = bIsDefault;

	return GR_SUCCESS;
}


//--------------------------------------------------------------------------------------

GLUFResult GLUFDialog::AddCheckBox(int ID, LPCWSTR strText, int x, int y, int width, int height, bool bChecked,
UINT nHotkey, bool bIsDefault, CGLUFCheckBox** ppCreated)
{
	GLUFResult hr = GR_SUCCESS;

	CGLUFCheckBox* pCheckBox = new (std::nothrow) CGLUFCheckBox(this);

	if (ppCreated)
		*ppCreated = pCheckBox;

	if (!pCheckBox)
		return E_OUTOFMEMORY;

	hr = AddControl(pCheckBox);
	if (FAILED(hr))
		return hr;

	// Set the ID and list index
	pCheckBox->SetID(ID);
	pCheckBox->SetText(strText);
	pCheckBox->SetLocation(x, y);
	pCheckBox->SetSize(width, height);
	pCheckBox->SetHotkey(nHotkey);
	pCheckBox->m_bIsDefault = bIsDefault;
	pCheckBox->SetChecked(bChecked);

	return GR_SUCCESS;
}


//--------------------------------------------------------------------------------------

GLUFResult GLUFDialog::AddRadioButton(int ID, UINT nButtonGroup, LPCWSTR strText, int x, int y, int width, int height,
bool bChecked, UINT nHotkey, bool bIsDefault, CGLUFRadioButton** ppCreated)
{
	GLUFResult hr = GR_SUCCESS;

	CGLUFRadioButton* pRadioButton = new (std::nothrow) CGLUFRadioButton(this);

	if (ppCreated)
		*ppCreated = pRadioButton;

	if (!pRadioButton)
		return E_OUTOFMEMORY;

	hr = AddControl(pRadioButton);
	if (FAILED(hr))
		return hr;

	// Set the ID and list index
	pRadioButton->SetID(ID);
	pRadioButton->SetText(strText);
	pRadioButton->SetButtonGroup(nButtonGroup);
	pRadioButton->SetLocation(x, y);
	pRadioButton->SetSize(width, height);
	pRadioButton->SetHotkey(nHotkey);
	pRadioButton->SetChecked(bChecked);
	pRadioButton->m_bIsDefault = bIsDefault;
	pRadioButton->SetChecked(bChecked);

	return GR_SUCCESS;
}


//--------------------------------------------------------------------------------------

GLUFResult GLUFDialog::AddComboBox(int ID, int x, int y, int width, int height, UINT nHotkey, bool bIsDefault,
CGLUFComboBox** ppCreated)
{
	GLUFResult hr = GR_SUCCESS;

	CGLUFComboBox* pComboBox = new (std::nothrow) CGLUFComboBox(this);

	if (ppCreated)
		*ppCreated = pComboBox;

	if (!pComboBox)
		return E_OUTOFMEMORY;

	hr = AddControl(pComboBox);
	if (FAILED(hr))
		return hr;

	// Set the ID and list index
	pComboBox->SetID(ID);
	pComboBox->SetLocation(x, y);
	pComboBox->SetSize(width, height);
	pComboBox->SetHotkey(nHotkey);
	pComboBox->m_bIsDefault = bIsDefault;

	return GR_SUCCESS;
}


//--------------------------------------------------------------------------------------

GLUFResult GLUFDialog::AddSlider(int ID, int x, int y, int width, int height, int min, int max, int value,
bool bIsDefault, CGLUFSlider** ppCreated)
{
	GLUFResult hr = GR_SUCCESS;

	CGLUFSlider* pSlider = new (std::nothrow) CGLUFSlider(this);

	if (ppCreated)
		*ppCreated = pSlider;

	if (!pSlider)
		return E_OUTOFMEMORY;

	hr = AddControl(pSlider);
	if (FAILED(hr))
		return hr;

	// Set the ID and list index
	pSlider->SetID(ID);
	pSlider->SetLocation(x, y);
	pSlider->SetSize(width, height);
	pSlider->m_bIsDefault = bIsDefault;
	pSlider->SetRange(min, max);
	pSlider->SetValue(value);
	pSlider->UpdateRects();

	return GR_SUCCESS;
}


//--------------------------------------------------------------------------------------

GLUFResult GLUFDialog::AddEditBox(int ID, LPCWSTR strText, int x, int y, int width, int height, bool bIsDefault,
CGLUFEditBox** ppCreated)
{
	GLUFResult hr = GR_SUCCESS;

	CGLUFEditBox* pEditBox = new (std::nothrow) CGLUFEditBox(this);

	if (ppCreated)
		*ppCreated = pEditBox;

	if (!pEditBox)
		return E_OUTOFMEMORY;

	hr = AddControl(pEditBox);
	if (FAILED(hr))
		return hr;

	// Set the ID and position
	pEditBox->SetID(ID);
	pEditBox->SetLocation(x, y);
	pEditBox->SetSize(width, height);
	pEditBox->m_bIsDefault = bIsDefault;

	if (strText)
		pEditBox->SetText(strText);

	return GR_SUCCESS;
}


//--------------------------------------------------------------------------------------

GLUFResult GLUFDialog::AddListBox(int ID, int x, int y, int width, int height, DWORD dwStyle, CGLUFListBox** ppCreated)
{
	GLUFResult hr = GR_SUCCESS;
	CGLUFListBox* pListBox = new (std::nothrow) CGLUFListBox(this);

	if (ppCreated)
		*ppCreated = pListBox;

	if (!pListBox)
		return E_OUTOFMEMORY;

	hr = AddControl(pListBox);
	if (FAILED(hr))
		return hr;

	// Set the ID and position
	pListBox->SetID(ID);
	pListBox->SetLocation(x, y);
	pListBox->SetSize(width, height);
	pListBox->SetStyle(dwStyle);

	return GR_SUCCESS;
}


//--------------------------------------------------------------------------------------
GLUFResult GLUFDialog::InitControl(CGLUFControl* pControl)
{
	GLUFResult hr;

	if (!pControl)
		return E_INVALIDARG;

	pControl->m_Index = static_cast<UINT>(m_Controls.size());

	// Look for a default Element entries
	for (auto it = m_DefaultElements.begin(); it != m_DefaultElements.end(); ++it)
	{
		if ((*it)->nControlType == pControl->GetType())
			pControl->SetElement((*it)->iElement, &(*it)->Element);
	}

	V_RETURN(pControl->OnInit());

	return GR_SUCCESS;
}


//--------------------------------------------------------------------------------------
GLUFResult GLUFDialog::AddControl(CGLUFControl* pControl)
{
	GLUFResult hr = GR_SUCCESS;

	hr = InitControl(pControl);
	if (FAILED(hr))
		return DXTRACE_ERR(L"GLUFDialog::InitControl", hr);

	// Add to the list
	m_Controls.push_back(pControl);

	return GR_SUCCESS;
}


//--------------------------------------------------------------------------------------
CGLUFControl* GLUFDialog::GetControl(int ID) const
{
	// Try to find the control with the given ID
	for (auto it = m_Controls.cbegin(); it != m_Controls.cend(); ++it)
	{
		if ((*it)->GetID() == ID)
		{
			return *it;
		}
	}

	// Not found
	return nullptr;
}


//--------------------------------------------------------------------------------------
CGLUFControl* GLUFDialog::GetControl( int ID,  UINT nControlType) const
{
	// Try to find the control with the given ID
	for (auto it = m_Controls.cbegin(); it != m_Controls.cend(); ++it)
	{
		if ((*it)->GetID() == ID && (*it)->GetType() == nControlType)
		{
			return *it;
		}
	}

	// Not found
	return nullptr;
}


//--------------------------------------------------------------------------------------
CGLUFControl* GLUFDialog::GetNextControl(CGLUFControl* pControl)
{
	int index = pControl->m_Index + 1;

	GLUFDialog* pDialog = pControl->m_pDialog;

	// Cycle through dialogs in the loop to find the next control. Note
	// that if only one control exists in all looped dialogs it will
	// be the returned 'next' control.
	while (index >= (int)pDialog->m_Controls.size())
	{
		pDialog = pDialog->m_pNextDialog;
		index = 0;
	}

	return pDialog->m_Controls[index];
}


//--------------------------------------------------------------------------------------
CGLUFControl* GLUFDialog::GetPrevControl(CGLUFControl* pControl)
{
	int index = pControl->m_Index - 1;

	GLUFDialog* pDialog = pControl->m_pDialog;

	// Cycle through dialogs in the loop to find the next control. Note
	// that if only one control exists in all looped dialogs it will
	// be the returned 'previous' control.
	while (index < 0)
	{
		pDialog = pDialog->m_pPrevDialog;
		if (!pDialog)
			pDialog = pControl->m_pDialog;

		index = int(pDialog->m_Controls.size()) - 1;
	}

	return pDialog->m_Controls[index];
}


//--------------------------------------------------------------------------------------
void GLUFDialog::ClearRadioButtonGroup(UINT nButtonGroup)
{
	// Find all radio buttons with the given group number
	for (auto it = m_Controls.cbegin(); it != m_Controls.cend(); ++it)
	{
		if ((*it)->GetType() == GLUF_CONTROL_RADIOBUTTON)
		{
			CGLUFRadioButton* pRadioButton = (CGLUFRadioButton*)*it;

			if (pRadioButton->GetButtonGroup() == nButtonGroup)
				pRadioButton->SetChecked(false, false);
		}
	}
}


//--------------------------------------------------------------------------------------
void GLUFDialog::ClearComboBox(int ID)
{
	CGLUFComboBox* pComboBox = GetComboBox(ID);
	if (!pComboBox)
		return;

	pComboBox->RemoveAllItems();
}


//--------------------------------------------------------------------------------------
void GLUFDialog::RequestFocus(CGLUFControl* pControl)
{
	if (s_pControlFocus == pControl)
		return;

	if (!pControl->CanHaveFocus())
		return;

	if (s_pControlFocus)
		s_pControlFocus->OnFocusOut();

	pControl->OnFocusIn();
	s_pControlFocus = pControl;
}


//--------------------------------------------------------------------------------------

GLUFResult GLUFDialog::DrawRect(const RECT* pRect, DWORD color)
{
	UNREFERENCED_PARAMETER(pRect);
	UNREFERENCED_PARAMETER(color);
	// TODO -
	return E_FAIL;
}


//--------------------------------------------------------------------------------------

GLUFResult GLUFDialog::DrawSprite(CGLUFElement* pElement, const RECT* prcDest, float fDepth)
{
	// No need to draw fully transparent layers
	if (pElement->TextureColor.Current.w == 0)
		return GR_SUCCESS;

	RECT rcTexture = pElement->rcTexture;

	RECT rcScreen = *prcDest;
	OffsetRect(&rcScreen, m_x, m_y);

	// If caption is enabled, offset the Y position by its height.
	if (m_bCaption)
		OffsetRect(&rcScreen, 0, m_nCaptionHeight);

	GLUFTextureNode* pTextureNode = GetTexture(pElement->iTexture);
	if (!pTextureNode)
		return E_FAIL;

	float fBBWidth = (float)m_pManager->m_nBackBufferWidth;
	float fBBHeight = (float)m_pManager->m_nBackBufferHeight;
	float fTexWidth = (float)pTextureNode->dwWidth;
	float fTexHeight = (float)pTextureNode->dwHeight;

	float fRectLeft = rcScreen.left / fBBWidth;
	float fRectTop = 1.0f - rcScreen.top / fBBHeight;
	float fRectRight = rcScreen.right / fBBWidth;
	float fRectBottom = 1.0f - rcScreen.bottom / fBBHeight;

	fRectLeft = fRectLeft * 2.0f - 1.0f;
	fRectTop = fRectTop * 2.0f - 1.0f;
	fRectRight = fRectRight * 2.0f - 1.0f;
	fRectBottom = fRectBottom * 2.0f - 1.0f;

	float fTexLeft = rcTexture.left / fTexWidth;
	float fTexTop = rcTexture.top / fTexHeight;
	float fTexRight = rcTexture.right / fTexWidth;
	float fTexBottom = rcTexture.bottom / fTexHeight;

	// Add 6 sprite vertices
	GLUFSpriteVertex SpriteVertex;

	// tri1
	SpriteVertex.vPos = glm::vec3(fRectLeft, fRectTop, fDepth);
	SpriteVertex.vTex = glm::vec2(fTexLeft, fTexTop);
	SpriteVertex.vColor = pElement->TextureColor.Current;
	m_pManager->m_SpriteVertices.push_back(SpriteVertex);

	SpriteVertex.vPos = glm::vec3(fRectRight, fRectTop, fDepth);
	SpriteVertex.vTex = glm::vec2(fTexRight, fTexTop);
	SpriteVertex.vColor = pElement->TextureColor.Current;
	m_pManager->m_SpriteVertices.push_back(SpriteVertex);

	SpriteVertex.vPos = glm::vec3(fRectLeft, fRectBottom, fDepth);
	SpriteVertex.vTex = glm::vec2(fTexLeft, fTexBottom);
	SpriteVertex.vColor = pElement->TextureColor.Current;
	m_pManager->m_SpriteVertices.push_back(SpriteVertex);

	// tri2
	SpriteVertex.vPos = glm::vec3(fRectRight, fRectTop, fDepth);
	SpriteVertex.vTex = glm::vec2(fTexRight, fTexTop);
	SpriteVertex.vColor = pElement->TextureColor.Current;
	m_pManager->m_SpriteVertices.push_back(SpriteVertex);

	SpriteVertex.vPos = glm::vec3(fRectRight, fRectBottom, fDepth);
	SpriteVertex.vTex = glm::vec2(fTexRight, fTexBottom);
	SpriteVertex.vColor = pElement->TextureColor.Current;
	m_pManager->m_SpriteVertices.push_back(SpriteVertex);

	SpriteVertex.vPos = glm::vec3(fRectLeft, fRectBottom, fDepth);
	SpriteVertex.vTex = glm::vec2(fTexLeft, fTexBottom);
	SpriteVertex.vColor = pElement->TextureColor.Current;
	m_pManager->m_SpriteVertices.push_back(SpriteVertex);

	// Why are we drawing the sprite every time?  This is very inefficient, but the sprite workaround doesn't have support for sorting now, so we have to
	// draw a sprite every time to keep the order correct between sprites and text.
	m_pManager->EndSprites11(GLUFGetD3D11Device(), GLUFGetD3D11DeviceContext());

	return GR_SUCCESS;
}


//--------------------------------------------------------------------------------------

GLUFResult GLUFDialog::CalcTextRect(LPCWSTR strText, CGLUFElement* pElement, const RECT* prcDest, int nCount)
{
	GLUFFontNode* pFontNode = GetFont(pElement->iFont);
	if (!pFontNode)
		return E_FAIL;

	UNREFERENCED_PARAMETER(strText);
	UNREFERENCED_PARAMETER(prcDest);
	UNREFERENCED_PARAMETER(nCount);
	// TODO -

	return GR_SUCCESS;
}


//--------------------------------------------------------------------------------------

GLUFResult GLUFDialog::DrawText(LPCWSTR strText, CGLUFElement* pElement, const RECT* prcDest, bool bShadow, bool bCenter)
{
	// No need to draw fully transparent layers
	if (pElement->FontColor.Current.w == 0)
		return GR_SUCCESS;

	RECT rcScreen = *prcDest;
	OffsetRect(&rcScreen, m_x, m_y);

	// If caption is enabled, offset the Y position by its height.
	if (m_bCaption)
		OffsetRect(&rcScreen, 0, m_nCaptionHeight);

	float fBBWidth = (float)m_pManager->m_nBackBufferWidth;
	float fBBHeight = (float)m_pManager->m_nBackBufferHeight;

	auto pd3dDevice = m_pManager->GetD3D11Device();
	auto pd3d11DeviceContext = m_pManager->GetD3D11DeviceContext();

	if (bShadow)
	{
		RECT rcShadow = rcScreen;
		OffsetRect(&rcShadow, 1, 1);

		glm::vec4 vShadowColor(0, 0, 0, 1.0f);
		DrawText11GLUF(pd3dDevice, pd3d11DeviceContext,
			strText, rcShadow, vShadowColor,
			fBBWidth, fBBHeight, bCenter);

	}

	glm::vec4 vFontColor(pElement->FontColor.Current.x, pElement->FontColor.Current.y, pElement->FontColor.Current.z, 1.0f);
	DrawText11GLUF(pd3dDevice, pd3d11DeviceContext,
		strText, rcScreen, vFontColor,
		fBBWidth, fBBHeight, bCenter);

	return GR_SUCCESS;
}


//--------------------------------------------------------------------------------------

void GLUFDialog::SetBackgroundColors(DWORD colorTopLeft, DWORD colorTopRight, DWORD colorBottomLeft,
DWORD colorBottomRight)
{
	m_colorTopLeft = colorTopLeft;
	m_colorTopRight = colorTopRight;
	m_colorBottomLeft = colorBottomLeft;
	m_colorBottomRight = colorBottomRight;
}


//--------------------------------------------------------------------------------------
void GLUFDialog::SetNextDialog(GLUFDialog* pNextDialog)
{
	if (!pNextDialog)
		pNextDialog = this;

	m_pNextDialog = pNextDialog;
	if (pNextDialog)
		m_pNextDialog->m_pPrevDialog = this;
}


//--------------------------------------------------------------------------------------
void GLUFDialog::ClearFocus()
{
	if (s_pControlFocus)
	{
		s_pControlFocus->OnFocusOut();
		s_pControlFocus = nullptr;
	}

	ReleaseCapture();
}


//--------------------------------------------------------------------------------------
void GLUFDialog::FocusDefaultControl()
{
	// Check for default control in this dialog
	for (auto it = m_Controls.cbegin(); it != m_Controls.cend(); ++it)
	{
		if ((*it)->m_bIsDefault)
		{
			// Remove focus from the current control
			ClearFocus();

			// Give focus to the default control
			s_pControlFocus = *it;
			s_pControlFocus->OnFocusIn();
			return;
		}
	}
}


//--------------------------------------------------------------------------------------
bool GLUFDialog::OnCycleFocus(bool bForward)
{
	CGLUFControl* pControl = nullptr;
	GLUFDialog* pDialog = nullptr; // pDialog and pLastDialog are used to track wrapping of
	GLUFDialog* pLastDialog;    // focus from first control to last or vice versa.

	if (!s_pControlFocus)
	{
		// If s_pControlFocus is nullptr, we focus the first control of first dialog in
		// the case that bForward is true, and focus the last control of last dialog when
		// bForward is false.
		//
		if (bForward)
		{
			// Search for the first control from the start of the dialog
			// array.
			for (auto it = m_pManager->m_Dialogs.cbegin(); it != m_pManager->m_Dialogs.cend(); ++it)
			{
				pDialog = pLastDialog = *it;
				if (pDialog && !pDialog->m_Controls.empty())
				{
					pControl = pDialog->m_Controls[0];
					break;
				}
			}

			if (!pDialog || !pControl)
			{
				// No dialog has been registered yet or no controls have been
				// added to the dialogs. Cannot proceed.
				return true;
			}
		}
		else
		{
			// Search for the first control from the end of the dialog
			// array.
			for (auto it = m_pManager->m_Dialogs.crbegin(); it != m_pManager->m_Dialogs.crend(); ++it)
			{
				pDialog = pLastDialog = *it;
				if (pDialog && !pDialog->m_Controls.empty())
				{
					pControl = pDialog->m_Controls[pDialog->m_Controls.size() - 1];
					break;
				}
			}

			if (!pDialog || !pControl)
			{
				// No dialog has been registered yet or no controls have been
				// added to the dialogs. Cannot proceed.
				return true;
			}
		}
	}
	else if (s_pControlFocus->m_pDialog != this)
	{
		// If a control belonging to another dialog has focus, let that other
		// dialog handle this event by returning false.
		//
		return false;
	}
	else
	{
		// Focused control belongs to this dialog. Cycle to the
		// next/previous control.
		assert(pControl != 0);
		_Analysis_assume_(pControl != 0);
		pLastDialog = s_pControlFocus->m_pDialog;
		pControl = (bForward) ? GetNextControl(s_pControlFocus) : GetPrevControl(s_pControlFocus);
		pDialog = pControl->m_pDialog;
	}

	assert(pControl != 0);
	_Analysis_assume_(pControl != 0);

	for (int i = 0; i < 0xffff; i++)
	{
		// If we just wrapped from last control to first or vice versa,
		// set the focused control to nullptr. This state, where no control
		// has focus, allows the camera to work.
		int nLastDialogIndex = -1;
		auto fit = std::find(m_pManager->m_Dialogs.cbegin(), m_pManager->m_Dialogs.cend(), pLastDialog);
		if (fit != m_pManager->m_Dialogs.cend())
		{
			nLastDialogIndex = int(fit - m_pManager->m_Dialogs.begin());
		}

		int nDialogIndex = -1;
		fit = std::find(m_pManager->m_Dialogs.cbegin(), m_pManager->m_Dialogs.cend(), pDialog);
		if (fit != m_pManager->m_Dialogs.cend())
		{
			nDialogIndex = int(fit - m_pManager->m_Dialogs.begin());
		}

		if ((!bForward && nLastDialogIndex < nDialogIndex) ||
			(bForward && nDialogIndex < nLastDialogIndex))
		{
			if (s_pControlFocus)
				s_pControlFocus->OnFocusOut();
			s_pControlFocus = nullptr;
			return true;
		}

		// If we've gone in a full circle then focus doesn't change
		if (pControl == s_pControlFocus)
			return true;

		// If the dialog accepts keybord input and the control can have focus then
		// move focus
		if (pControl->m_pDialog->m_bKeyboardInput && pControl->CanHaveFocus())
		{
			if (s_pControlFocus)
				s_pControlFocus->OnFocusOut();
			s_pControlFocus = pControl;
			if (s_pControlFocus)
				s_pControlFocus->OnFocusIn();
			return true;
		}

		pLastDialog = pDialog;
		pControl = (bForward) ? GetNextControl(pControl) : GetPrevControl(pControl);
		pDialog = pControl->m_pDialog;
	}

	// If we reached this point, the chain of dialogs didn't form a complete loop
	DXTRACE_ERR(L"GLUFDialog: Multiple dialogs are improperly chained together", E_FAIL);
	return false;
}


//--------------------------------------------------------------------------------------
void GLUFDialog::InitDefaultElements()
{
	SetFont(0, L"Arial", 14, FW_NORMAL);

	CGLUFElement Element;
	RECT rcTexture;

	//-------------------------------------
	// Element for the caption
	//-------------------------------------
	m_CapElement.SetFont(0);
	SetRect(&rcTexture, 17, 269, 241, 287);
	m_CapElement.SetTexture(0, &rcTexture);
	m_CapElement.TextureColor.States[GLUF_STATE_NORMAL] = D3DCOLOR_ARGB(255, 255, 255, 255);
	m_CapElement.FontColor.States[GLUF_STATE_NORMAL] = D3DCOLOR_ARGB(255, 255, 255, 255);
	m_CapElement.SetFont(0, D3DCOLOR_ARGB(255, 255, 255, 255), DT_LEFT | DT_VCENTER);
	// Pre-blend as we don't need to transition the state
	m_CapElement.TextureColor.Blend(GLUF_STATE_NORMAL, 10.0f);
	m_CapElement.FontColor.Blend(GLUF_STATE_NORMAL, 10.0f);

	//-------------------------------------
	// CGLUFStatic
	//-------------------------------------
	Element.SetFont(0);
	Element.FontColor.States[GLUF_STATE_DISABLED] = D3DCOLOR_ARGB(200, 200, 200, 200);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_STATIC, 0, &Element);


	//-------------------------------------
	// CGLUFButton - Button
	//-------------------------------------
	SetRect(&rcTexture, 0, 0, 136, 54);
	Element.SetTexture(0, &rcTexture);
	Element.SetFont(0);
	Element.TextureColor.States[GLUF_STATE_NORMAL] = D3DCOLOR_ARGB(150, 255, 255, 255);
	Element.TextureColor.States[GLUF_STATE_PRESSED] = D3DCOLOR_ARGB(200, 255, 255, 255);
	Element.FontColor.States[GLUF_STATE_MOUSEOVER] = D3DCOLOR_ARGB(255, 0, 0, 0);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_BUTTON, 0, &Element);


	//-------------------------------------
	// CGLUFButton - Fill layer
	//-------------------------------------
	SetRect(&rcTexture, 136, 0, 252, 54);
	Element.SetTexture(0, &rcTexture, D3DCOLOR_ARGB(0, 255, 255, 255));
	Element.TextureColor.States[GLUF_STATE_MOUSEOVER] = D3DCOLOR_ARGB(160, 255, 255, 255);
	Element.TextureColor.States[GLUF_STATE_PRESSED] = D3DCOLOR_ARGB(60, 0, 0, 0);
	Element.TextureColor.States[GLUF_STATE_FOCUS] = D3DCOLOR_ARGB(30, 255, 255, 255);


	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_BUTTON, 1, &Element);


	//-------------------------------------
	// CGLUFCheckBox - Box
	//-------------------------------------
	SetRect(&rcTexture, 0, 54, 27, 81);
	Element.SetTexture(0, &rcTexture);
	Element.SetFont(0, D3DCOLOR_ARGB(255, 255, 255, 255), DT_LEFT | DT_VCENTER);
	Element.FontColor.States[GLUF_STATE_DISABLED] = D3DCOLOR_ARGB(200, 200, 200, 200);
	Element.TextureColor.States[GLUF_STATE_NORMAL] = D3DCOLOR_ARGB(150, 255, 255, 255);
	Element.TextureColor.States[GLUF_STATE_FOCUS] = D3DCOLOR_ARGB(200, 255, 255, 255);
	Element.TextureColor.States[GLUF_STATE_PRESSED] = D3DCOLOR_ARGB(255, 255, 255, 255);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_CHECKBOX, 0, &Element);


	//-------------------------------------
	// CGLUFCheckBox - Check
	//-------------------------------------
	SetRect(&rcTexture, 27, 54, 54, 81);
	Element.SetTexture(0, &rcTexture);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_CHECKBOX, 1, &Element);


	//-------------------------------------
	// CGLUFRadioButton - Box
	//-------------------------------------
	SetRect(&rcTexture, 54, 54, 81, 81);
	Element.SetTexture(0, &rcTexture);
	Element.SetFont(0, D3DCOLOR_ARGB(255, 255, 255, 255), DT_LEFT | DT_VCENTER);
	Element.FontColor.States[GLUF_STATE_DISABLED] = D3DCOLOR_ARGB(200, 200, 200, 200);
	Element.TextureColor.States[GLUF_STATE_NORMAL] = D3DCOLOR_ARGB(150, 255, 255, 255);
	Element.TextureColor.States[GLUF_STATE_FOCUS] = D3DCOLOR_ARGB(200, 255, 255, 255);
	Element.TextureColor.States[GLUF_STATE_PRESSED] = D3DCOLOR_ARGB(255, 255, 255, 255);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_RADIOBUTTON, 0, &Element);


	//-------------------------------------
	// CGLUFRadioButton - Check
	//-------------------------------------
	SetRect(&rcTexture, 81, 54, 108, 81);
	Element.SetTexture(0, &rcTexture);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_RADIOBUTTON, 1, &Element);


	//-------------------------------------
	// CGLUFComboBox - Main
	//-------------------------------------
	SetRect(&rcTexture, 7, 81, 247, 123);
	Element.SetTexture(0, &rcTexture);
	Element.SetFont(0);
	Element.TextureColor.States[GLUF_STATE_NORMAL] = D3DCOLOR_ARGB(150, 200, 200, 200);
	Element.TextureColor.States[GLUF_STATE_FOCUS] = D3DCOLOR_ARGB(170, 230, 230, 230);
	Element.TextureColor.States[GLUF_STATE_DISABLED] = D3DCOLOR_ARGB(70, 200, 200, 200);
	Element.FontColor.States[GLUF_STATE_MOUSEOVER] = D3DCOLOR_ARGB(255, 0, 0, 0);
	Element.FontColor.States[GLUF_STATE_PRESSED] = D3DCOLOR_ARGB(255, 0, 0, 0);
	Element.FontColor.States[GLUF_STATE_DISABLED] = D3DCOLOR_ARGB(200, 200, 200, 200);


	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_COMBOBOX, 0, &Element);


	//-------------------------------------
	// CGLUFComboBox - Button
	//-------------------------------------
	SetRect(&rcTexture, 98, 189, 151, 238);
	Element.SetTexture(0, &rcTexture);
	Element.TextureColor.States[GLUF_STATE_NORMAL] = D3DCOLOR_ARGB(150, 255, 255, 255);
	Element.TextureColor.States[GLUF_STATE_PRESSED] = D3DCOLOR_ARGB(255, 150, 150, 150);
	Element.TextureColor.States[GLUF_STATE_FOCUS] = D3DCOLOR_ARGB(200, 255, 255, 255);
	Element.TextureColor.States[GLUF_STATE_DISABLED] = D3DCOLOR_ARGB(70, 255, 255, 255);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_COMBOBOX, 1, &Element);


	//-------------------------------------
	// CGLUFComboBox - Dropdown
	//-------------------------------------
	SetRect(&rcTexture, 13, 123, 241, 160);
	Element.SetTexture(0, &rcTexture);
	Element.SetFont(0, D3DCOLOR_ARGB(255, 0, 0, 0), DT_LEFT | DT_TOP);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_COMBOBOX, 2, &Element);


	//-------------------------------------
	// CGLUFComboBox - Selection
	//-------------------------------------
	SetRect(&rcTexture, 12, 163, 239, 183);
	Element.SetTexture(0, &rcTexture);
	Element.SetFont(0, D3DCOLOR_ARGB(255, 255, 255, 255), DT_LEFT | DT_TOP);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_COMBOBOX, 3, &Element);


	//-------------------------------------
	// CGLUFSlider - Track
	//-------------------------------------
	SetRect(&rcTexture, 1, 187, 93, 228);
	Element.SetTexture(0, &rcTexture);
	Element.TextureColor.States[GLUF_STATE_NORMAL] = D3DCOLOR_ARGB(150, 255, 255, 255);
	Element.TextureColor.States[GLUF_STATE_FOCUS] = D3DCOLOR_ARGB(200, 255, 255, 255);
	Element.TextureColor.States[GLUF_STATE_DISABLED] = D3DCOLOR_ARGB(70, 255, 255, 255);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_SLIDER, 0, &Element);

	//-------------------------------------
	// CGLUFSlider - Button
	//-------------------------------------
	SetRect(&rcTexture, 151, 193, 192, 234);
	Element.SetTexture(0, &rcTexture);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_SLIDER, 1, &Element);

	//-------------------------------------
	// CGLUFScrollBar - Track
	//-------------------------------------
	int nScrollBarStartX = 196;
	int nScrollBarStartY = 191;
	SetRect(&rcTexture, nScrollBarStartX + 0, nScrollBarStartY + 21, nScrollBarStartX + 22, nScrollBarStartY + 32);
	Element.SetTexture(0, &rcTexture);
	Element.TextureColor.States[GLUF_STATE_DISABLED] = D3DCOLOR_ARGB(255, 200, 200, 200);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_SCROLLBAR, 0, &Element);

	//-------------------------------------
	// CGLUFScrollBar - Up Arrow
	//-------------------------------------
	SetRect(&rcTexture, nScrollBarStartX + 0, nScrollBarStartY + 1, nScrollBarStartX + 22, nScrollBarStartY + 21);
	Element.SetTexture(0, &rcTexture);
	Element.TextureColor.States[GLUF_STATE_DISABLED] = D3DCOLOR_ARGB(255, 200, 200, 200);


	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_SCROLLBAR, 1, &Element);

	//-------------------------------------
	// CGLUFScrollBar - Down Arrow
	//-------------------------------------
	SetRect(&rcTexture, nScrollBarStartX + 0, nScrollBarStartY + 32, nScrollBarStartX + 22, nScrollBarStartY + 53);
	Element.SetTexture(0, &rcTexture);
	Element.TextureColor.States[GLUF_STATE_DISABLED] = D3DCOLOR_ARGB(255, 200, 200, 200);


	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_SCROLLBAR, 2, &Element);

	//-------------------------------------
	// CGLUFScrollBar - Button
	//-------------------------------------
	SetRect(&rcTexture, 220, 192, 238, 234);
	Element.SetTexture(0, &rcTexture);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_SCROLLBAR, 3, &Element);


	//-------------------------------------
	// CGLUFEditBox
	//-------------------------------------
	// Element assignment:
	//   0 - text area
	//   1 - top left border
	//   2 - top border
	//   3 - top right border
	//   4 - left border
	//   5 - right border
	//   6 - lower left border
	//   7 - lower border
	//   8 - lower right border

	Element.SetFont(0, D3DCOLOR_ARGB(255, 0, 0, 0), DT_LEFT | DT_TOP);

	// Assign the style
	SetRect(&rcTexture, 14, 90, 241, 113);
	Element.SetTexture(0, &rcTexture);
	SetDefaultElement(GLUF_CONTROL_EDITBOX, 0, &Element);
	SetRect(&rcTexture, 8, 82, 14, 90);
	Element.SetTexture(0, &rcTexture);
	SetDefaultElement(GLUF_CONTROL_EDITBOX, 1, &Element);
	SetRect(&rcTexture, 14, 82, 241, 90);
	Element.SetTexture(0, &rcTexture);
	SetDefaultElement(GLUF_CONTROL_EDITBOX, 2, &Element);
	SetRect(&rcTexture, 241, 82, 246, 90);
	Element.SetTexture(0, &rcTexture);
	SetDefaultElement(GLUF_CONTROL_EDITBOX, 3, &Element);
	SetRect(&rcTexture, 8, 90, 14, 113);
	Element.SetTexture(0, &rcTexture);
	SetDefaultElement(GLUF_CONTROL_EDITBOX, 4, &Element);
	SetRect(&rcTexture, 241, 90, 246, 113);
	Element.SetTexture(0, &rcTexture);
	SetDefaultElement(GLUF_CONTROL_EDITBOX, 5, &Element);
	SetRect(&rcTexture, 8, 113, 14, 121);
	Element.SetTexture(0, &rcTexture);
	SetDefaultElement(GLUF_CONTROL_EDITBOX, 6, &Element);
	SetRect(&rcTexture, 14, 113, 241, 121);
	Element.SetTexture(0, &rcTexture);
	SetDefaultElement(GLUF_CONTROL_EDITBOX, 7, &Element);
	SetRect(&rcTexture, 241, 113, 246, 121);
	Element.SetTexture(0, &rcTexture);
	SetDefaultElement(GLUF_CONTROL_EDITBOX, 8, &Element);

	//-------------------------------------
	// CGLUFListBox - Main
	//-------------------------------------
	SetRect(&rcTexture, 13, 123, 241, 160);
	Element.SetTexture(0, &rcTexture);
	Element.SetFont(0, D3DCOLOR_ARGB(255, 0, 0, 0), DT_LEFT | DT_TOP);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_LISTBOX, 0, &Element);

	//-------------------------------------
	// CGLUFListBox - Selection
	//-------------------------------------

	SetRect(&rcTexture, 16, 166, 240, 183);
	Element.SetTexture(0, &rcTexture);
	Element.SetFont(0, D3DCOLOR_ARGB(255, 255, 255, 255), DT_LEFT | DT_TOP);

	// Assign the Element
	SetDefaultElement(GLUF_CONTROL_LISTBOX, 1, &Element);
}
