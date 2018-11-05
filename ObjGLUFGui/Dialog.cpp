#include "Dialog.h"

//--------------------------------------------------------------------------------------
Dialog::Dialog()
{
#ifdef _DEBUG
    //TODO: get a more graceful way to test this
    //This is to make sure all dialogs are being destroyed
    printf("Dialog Created\n");
#endif
}

double Dialog::sTimeRefresh = _60HZ;
ControlPtr Dialog::sControlFocus = nullptr;
ControlPtr Dialog::sControlPressed = nullptr;

//--------------------------------------------------------------------------------------
Dialog::~Dialog()
{
#ifdef _DEBUG
    //TODO: get a more graceful way to test this
    //This is to make sure all dialogs are being destroyed
    printf("Dialog Destroyed\n");
#endif

    RemoveAllControls();
}


//--------------------------------------------------------------------------------------
void Dialog::Init(DialogResourceManagerPtr& manager, bool registerDialog)
{
    if (g_ControlTextureResourceManLocation == -1)
    {
        g_ControlTextureResourceManLocation = manager->AddTexture(g_pControlTexturePtr);
    }

    Init(manager, registerDialog, g_ControlTextureResourceManLocation);
}


//--------------------------------------------------------------------------------------
void Dialog::Init(DialogResourceManagerPtr& manager, bool registerDialog, TextureIndex textureIndex)
{
    if (manager == nullptr)
        throw std::invalid_argument("Nullptr DRM");

    mDialogManager = manager;
    if (registerDialog)
        mDialogManager->RegisterDialog(shared_from_this());

    SetTexture(0, textureIndex);//this will always be the first one in our buffer of indices
    InitDefaultElements();
}


//--------------------------------------------------------------------------------------

/*void Dialog::Init(DialogResourceManager* pManager, bool bRegisterDialog, LPCWSTR szControlTextureResourceName, HMODULE hControlTextureResourceModule)
{
    m_pManager = pManager;
    if (bRegisterDialog)
        pManager->RegisterDialog(this);

    SetTexture(0, szControlTextureResourceName, hControlTextureResourceModule);
    InitDefaultElements();
}*/


//--------------------------------------------------------------------------------------
void Dialog::SetCallback(EventCallbackFuncPtr callback, EventCallbackReceivablePtr userContext) noexcept
{
    // If this assert triggers, you need to call Dialog::Init() first.  This change
    // was made so that the 's GUI could become separate and optional from 's core.  The
    // creation and interfacing with DialogResourceManager is now the responsibility
    // of the application if it wishes to use 's GUI.
    assert(mDialogManager && L"To fix call Dialog::Init() first.  See comments for details.");

    mCallbackEvent = callback;
    mCallbackContext = userContext;
}


//--------------------------------------------------------------------------------------
void Dialog::RemoveControl(ControlIndex ID)
{
    auto it = mControls.find(ID);

    if (it != mControls.end())
    {
        // Clean focus first
        ClearFocus();

        // Clear references to this control
        if (sControlFocus == it->second)
            sControlFocus = nullptr;
        if (sControlPressed == it->second)
            sControlPressed = nullptr;
        if (mControlMouseOver == it->second)
            mControlMouseOver = nullptr;

        mControls.erase(it);

    }
}


//--------------------------------------------------------------------------------------
void Dialog::RemoveAllControls() noexcept
{
    if (sControlFocus && &sControlFocus->mDialog == this)
        sControlFocus = nullptr;
    if (sControlPressed && &sControlPressed->mDialog == this)
        sControlPressed = nullptr;
    mControlMouseOver = nullptr;

    mControls.clear();
}


//--------------------------------------------------------------------------------------
void Dialog::Refresh() noexcept
{
    if (sControlFocus)
        sControlFocus->OnFocusOut();

    if (mControlMouseOver)
        mControlMouseOver->OnMouseLeave();

    sControlFocus = nullptr;
    sControlPressed = nullptr;
    mControlMouseOver = nullptr;

    for (auto it : mControls)
    {
        it.second->Refresh();
    }

    if (mKeyboardInput)
        FocusDefaultControl();
}


//--------------------------------------------------------------------------------------
void Dialog::OnRender(float elapsedTime) noexcept
{
    // If this assert triggers, you need to call DialogResourceManager::On*Device() from inside
    // the application's device callbacks.  See the SDK samples for an example of how to do this.
    //assert(m_pManager->GetD3D11Device() &&
    //    L"To fix hook up DialogResourceManager to device callbacks.  See comments for details");
    //no need for "devices", this is all handled by GLFW

    // See if the dialog needs to be refreshed
    if (mTimePrevRefresh < sTimeRefresh)
    {
        mTimePrevRefresh = GetTime();
        Refresh();
    }

    // For invisible dialog, out now.
    if (!mVisible ||
        (mMinimized && !mCaptionEnabled))
        return;

    // Enable depth test
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_CLAMP);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    mDialogManager->BeginSprites();

    if (!mMinimized)
    {
        // Convert the draw rectangle from screen coordinates to clip space coordinates.(where the origin is in the middle of the screen, and the edges are 1, or negative 1
        Rect windowCoords = { 0, GetHeight(), GetWidth(), 0 };
        //windowCoords = ScreenToClipspace(windowCoords);

        DrawSprite(mDlgElement, windowCoords, -0.99f, false);
    }

    // Sort depth back to front
    Text::BeginText(mDialogManager->GetOrthoMatrix());


    //m_pManager->ApplyRenderUI();
    // If the dialog is minimized, skip rendering
    // its controls.
    if (!mMinimized)
    {
        for (auto it : mControls)
        {
            // Focused control is drawn last
            if (it.second == sControlFocus)
                continue;

            it.second->Render(elapsedTime);
        }

        if (sControlFocus && &sControlFocus->mDialog == this)
            sControlFocus->Render(elapsedTime);
    }

    // Render the caption if it's enabled.
    if (mCaptionEnabled)
    {
        // DrawSprite will offset the rect down by
        // m_nCaptionHeight, so adjust the rect higher
        // here to negate the effect.

        mCapElement.mTextureColor.SetCurrent(STATE_NORMAL);
        mCapElement.mFontColor.SetCurrent(STATE_NORMAL);
        Rect rc = { 0, 0, GetWidth(), -mCaptionHeight };

        mDialogManager->ApplyRenderUIUntex();
        DrawSprite(mCapElement, rc, -0.99f, false);

        rc.left += 5; // Make a left margin

        if (mMinimized)
        {
            std::wstringstream wss;
            wss << mCaptionText;
            wss << L" (Minimized)";
            DrawText(wss.str(), mCapElement, rc);
        }
        else
            DrawText(mCaptionText, mCapElement, rc);
    }

    // End sprites
    /*if (m_bCaption)
    {
        m_pManager->EndSprites();
        EndText();
    }*/
    //m_pManager->RestoreD3D11State(pd3dDeviceContext);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_CLAMP);//set this back because it is the default
}


//--------------------------------------------------------------------------------------
void Dialog::SendEvent(Event ctrlEvent, bool triggeredByUser, ControlPtr control) noexcept
{
    // If no callback has been registered there's nowhere to send the event to
    if (!mCallbackEvent)
        return;

    // Discard events triggered programatically if these types of events haven't been
    // enabled
    if (!triggeredByUser && !mNonUserEvents)
        return;

    mCallbackEvent(ctrlEvent, control, mCallbackContext);
}


//--------------------------------------------------------------------------------------
void Dialog::SetFont(FontIndex index, FontIndex resManFontIndex)
{
    // If this assert triggers, you need to call Dialog::Init() first.  This change
    // was made so that the 's GUI could become separate and optional from 's core.  The
    // creation and interfacing with DialogResourceManager is now the responsibility
    // of the application if it wishes to use 's GUI.
    assert(mDialogManager && L"To fix call Dialog::Init() first.  See comments for details.");
    //_Analysis_assume_(m_pManager);


    //call this to trigger an exception if the font index does not exist
    mDialogManager->GetFontNode(resManFontIndex);

    mFonts[index] = resManFontIndex;
}


//--------------------------------------------------------------------------------------
FontNodePtr Dialog::GetFont(FontIndex index) const
{
    if (!mDialogManager)
        return nullptr;
    return mDialogManager->GetFontNode(index);
}


//--------------------------------------------------------------------------------------
void Dialog::SetTexture(TextureIndex index, TextureIndex resManTexIndex)
{
    // If this assert triggers, you need to call Dialog::Init() first.  This change
    // was made so that the 's GUI could become separate and optional from 's core.  The
    // creation and interfacing with DialogResourceManager is now the responsibility
    // of the application if it wishes to use 's GUI.
    assert(mDialogManager && L"To fix this, call Dialog::Init() first.  See comments for details.");
    //_Analysis_assume_(m_pManager);

    //call this to trigger an exception if the texture index does not exist
    mDialogManager->GetTextureNode(resManTexIndex);

    mTextures[index] = resManTexIndex;
}

//--------------------------------------------------------------------------------------
TextureNodePtr Dialog::GetTexture(TextureIndex index) const
{
    if (!mDialogManager)
        return nullptr;
    return mDialogManager->GetTextureNode(index);
}

//--------------------------------------------------------------------------------------
bool Dialog::MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept
{
    if (mFirstTime)
        mFirstTime = false;
    else
        mMousePositionOld = mMousePosition;

    mDialogManager->MsgProc(_PASS_CALLBACK_PARAM);

    if (msg == UNICODE_CHAR)
    {
        int i = 0;
    }

    //first, even if we are not going to use it, snatch up the cursor position just in case it moves in the time it takes to do this
    double x, y;
    glfwGetCursorPos(g_pGLFWWindow, &x, &y);
    mMousePosition = Point(static_cast<long>(x), g_WndHeight - static_cast<long>(y));

    //this gets broken when window is too big
    mMousePositionDialogSpace.x = mMousePosition.x - mRegion.x;
    mMousePositionDialogSpace.y = mMousePosition.y - mRegion.y - mCaptionHeight;//TODO: fix

    //if (m_bCaption)
    //    m_MousePositionDialogSpace.y -= m_nCaptionHeight;

    bool bHandled = false;


    //if it is a resize method, refresh ALL components
    if (msg == RESIZE || msg == FRAMEBUFFER_SIZE)
    {
        for (auto it = mControls.begin(); it != mControls.end(); ++it)
        {
            it->second->MsgProc(msg, param1, param2, param3, param4);
        }
    }

    // For invisible dialog, do not handle anything.
    if (!mVisible)
        return false;

    if (!mKeyboardInput && (msg == KEY || msg == UNICODE_CHAR))
        return false;

    // If caption is enable, check for clicks in the caption area.
    if (mCaptionEnabled && !mLocked)
    {
        static Point totalDelta;

        if (((msg == MB) == true) &&
            ((param1 == GLFW_MOUSE_BUTTON_LEFT) == true) &&
            ((param2 == GLFW_PRESS) == true) )
        {

            if (mMousePositionDialogSpace.x >= 0 && mMousePositionDialogSpace.x < RectWidth(mRegion) &&
                mMousePositionDialogSpace.y >= -mCaptionHeight && mMousePositionDialogSpace.y < 0)
            {
                mDrag = true;
                mDragged = false;
                //SetCapture(GetHWND());
                return true;
            }
            else if (!mMinimized && mGrabAnywhere && !GetControlAtPoint(mMousePositionDialogSpace))
            {
                //ONLY allow this if it is not on top of a control
                mDrag = true;
                return true;
            }
        }
        else if ((msg == MB) == true &&
                 (param1 == GLFW_MOUSE_BUTTON_LEFT) == true &&
                 (param2 == GLFW_RELEASE) == true &&
                 (mDrag))
        {
            if (mMousePositionDialogSpace.x >= 0 && mMousePositionDialogSpace.x < RectWidth(mRegion) &&
                mMousePositionDialogSpace.y >= -mCaptionHeight && mMousePositionDialogSpace.y < 0)
            {
                //ReleaseCapture();

                mDrag = false;

                //only minimize if the dialog WAS NOT moved
                if (!mDragged)
                {
                    //reset this when it passes its threshhold, which is when m_bDragged is toggled
                    totalDelta = { 0L, 0L };
                    mMinimized = !mMinimized;
                }

                return true;
            }
            else if (!mMinimized && mGrabAnywhere)
            {
                mDrag = false;
                return true;
            }
        }
        else if ((msg == CURSOR_POS))
        {
            //is it over the caption?
            if (glfwGetMouseButton(g_pGLFWWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            {
                if (mDrag)
                {
                    //if (m_MousePosition.x < 0.0f || m_MousePosition.y < 0.0f)
                    //{
                    //    glfwSetCursorPos(g_pGLFWWindow, (m_MousePosition.x < 0.0f) ? 0 : param1, (m_MousePosition.y < 0.0f) ? param2 : 0);
                    //}


                    Point delta = mMousePosition - mMousePositionOld;
                    totalDelta = { totalDelta.x + delta.x, totalDelta.y + delta.y };

                    RepositionRect(mRegion,
                                   std::clamp(delta.x + mRegion.x, 0L, static_cast<long>(g_WndWidth) - RectWidth(mRegion)),
                                   std::clamp(delta.y + mRegion.y, 0L, static_cast<long>(g_WndHeight) - mCaptionHeight));


                    //give a threshhold, because sometimes when a use clicks, the user will move the mouse a bit
                    if (totalDelta.x > 3 || totalDelta.y > 3)
                        mDragged = true;

                    return true;
                }
            }
        }
    }

    //this is important, if the window is resized, then make sure to reclamp the dialog position
    if (mAutoClamp && msg == RESIZE)
    {
        ClampToScreen();
    }

    // If the dialog is minimized, don't send any messages to controls.
    if (mMinimized)
        return false;

    // If a control is in focus, it belongs to this dialog, and it's enabled, then give
    // it the first chance at handling the message.
    if (sControlFocus &&
        &sControlFocus->mDialog == this &&
        sControlFocus->GetEnabled())
    {
        // If the control MsgProc handles it, then we don't.
        if (sControlFocus->MsgProc(msg, param1, param2, param3, param4))
            return true;
    }

    switch (msg)
    {
        case RESIZE:
        case POS:
        {
            // Handle sizing and moving messages so that in case the mouse cursor is moved out
            // of an UI control because of the window adjustment, we can properly
            // unhighlight the highlighted control.
            Point pt =
                    {
                            -1, -1
                    };
            OnMouseMove(pt);
            break;
        }

        case FOCUS:
            // Call OnFocusIn()/OnFocusOut() of the control that currently has the focus
            // as the application is activated/deactivated.  This matches the Windows
            // behavior.
            if (sControlFocus &&
                &sControlFocus->mDialog == this &&
                sControlFocus->GetEnabled())
            {
                if (param1 == GL_TRUE)
                    sControlFocus->OnFocusIn();
                else
                    sControlFocus->OnFocusOut();
            }
            break;

            // Keyboard messages
        case KEY:
            //case WM_SYSKEYDOWN:
            //case WM_KEYUP:
            //case WM_SYSKEYUP:
        {
            // If a control is in focus, it belongs to this dialog, and it's enabled, then give
            // it the first chance at handling the message.
            /*if (sControlFocus &&
                &sControlFocus->mDialog == this &&
                sControlFocus->GetEnabled())
                for (auto it : mControls)     --> Not Quite sure what this loop was here for
                {
                    if (sControlFocus->MsgProc(msg, param1, param2, param3, param4))
                        return true;
                }*/

            // Not yet handled, see if this matches a control's hotkey
            // Activate the hotkey if the focus doesn't belong to an
            // edit box.
            if (param3 == GLFW_PRESS && (!sControlFocus ||
                                         (sControlFocus->GetType() != CONTROL_EDITBOX
                                          && sControlFocus->GetType() != CONTROL_IMEEDITBOX)))
            {
                for (auto it : mControls)
                {
                    if (it.second->GetHotkey() == param1)
                    {
                        it.second->OnHotkey();
                        return true;
                    }
                }
            }

            // Not yet handled, check for focus messages
            if (param3 == GLFW_PRESS)
            {
                // If keyboard input is not enabled, this message should be ignored
                if (!mKeyboardInput)
                    return false;

                switch (param1)
                {
                    case GLFW_KEY_RIGHT:
                    case GLFW_KEY_DOWN:
                        if (sControlFocus)
                        {
                            return OnCycleFocus(true);
                        }
                        break;

                    case GLFW_KEY_LEFT:
                    case GLFW_KEY_UP:
                        if (sControlFocus)
                        {
                            return OnCycleFocus(false);
                        }
                        break;

                    case GLFW_KEY_TAB:
                    {
                        bool bShiftDown =(glfwGetKey(g_pGLFWWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ||
                                         (glfwGetKey(g_pGLFWWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
                        return OnCycleFocus(!bShiftDown);
                    }
                }
            }

            break;
        }


            // Mouse messages
            //case WM_MOUSEMOVE:
            //case WM_LBUTTONDOWN:
            //case WM_LBUTTONUP:
            //case WM_MBUTTONDOWN:
            //case WM_MBUTTONUP:
            //case WM_RBUTTONDOWN:
            //case WM_RBUTTONUP:
            //case WM_XBUTTONDOWN:
            //case WM_XBUTTONUP:
            //case WM_LBUTTONDBLCLK:
            //case WM_MBUTTONDBLCLK:
            //case WM_RBUTTONDBLCLK:
            //case WM_XBUTTONDBLCLK:
            //case WM_MOUSEWHEEL:
        case MB:
        case SCROLL:
        case CURSOR_POS:
        {
            // If not accepting mouse input, return false to indicate the message should still
            // be handled by the application (usually to move the camera).
            if (!mMouseInput)
                return false;


            // If caption is enabled, offset the Y coordinate by its height.
            //if (m_bCaption)
            //    m_MousePosition.y += m_nCaptionHeight;

            // If a control is in focus, it belongs to this dialog, and it's enabled, then give
            // it the first chance at handling the message.
            if (sControlFocus &&
                &sControlFocus->mDialog == this &&
                sControlFocus->GetEnabled())
            {
                if (sControlFocus->MsgProc(msg, param1, param2, param3, param4))
                    return true;
            }

            // Not yet handled, see if the mouse is over any controls
            ControlPtr pControl = GetControlAtPoint(mMousePositionDialogSpace);
            if (pControl && pControl->GetEnabled())
            {
                bHandled = pControl->MsgProc(msg, param1, param2, param3, param4);
                if (bHandled)
                    return true;
            }
            else
            {
                // Mouse not over any controls in this dialog, if there was a control
                // which had focus it just lost it
                if (param1 == GLFW_MOUSE_BUTTON_LEFT &&
                    param2 == GLFW_PRESS &&
                    sControlFocus &&
                    &sControlFocus->mDialog == this)
                {
                    sControlFocus->OnFocusOut();
                    sControlFocus = nullptr;
                }
            }

            // Still not handled, hand this off to the dialog. Return false to indicate the
            // message should still be handled by the application (usually to move the camera).
            switch (msg)
            {
                case CURSOR_POS:
                    OnMouseMove(mMousePositionDialogSpace);
                    return false;
            }

            break;
        }

        case CURSOR_ENTER:
        {
            // The application has lost mouse capture.
            // The dialog object may not have received
            // a WM_MOUSEUP when capture changed. Reset
            // mDrag so that the dialog does not mistakenly
            // think the mouse button is still held down.
            if (param1 == GL_FALSE)
                mDrag = false;
        }
    }

    return false;
}


//--------------------------------------------------------------------------------------
void Dialog::ClampToScreen() noexcept
{
    mRegion.x = std::clamp(mRegion.x, 0L, static_cast<long>(g_WndWidth) - RectWidth(mRegion));
    mRegion.y = std::clamp(mRegion.y, 0L, static_cast<long>(g_WndHeight) - mCaptionHeight);
}

//--------------------------------------------------------------------------------------
ControlPtr Dialog::GetControlAtPoint(const Point& pt) const noexcept
{
    // Search through all child controls for the first one which
    // contains the mouse point
    for (auto it : mControls)
    {
        if (!it.second)
        {
            continue;
        }

        // We only return the current control if it is visible
        // and enabled.  Because GetControlAtPoint() is used to do mouse
        // hittest, it makes sense to perform this filtering.
        if (it.second->ContainsPoint(pt) && it.second->GetEnabled() && it.second->GetVisible())
        {
            return it.second;
        }
    }

    return nullptr;
}


//--------------------------------------------------------------------------------------
bool Dialog::GetControlEnabled(ControlIndex ID) const
{
    ControlPtr pControl = GetControl<Control>(ID);
    if (!pControl)
        return false;

    return pControl->GetEnabled();
}



//--------------------------------------------------------------------------------------
void Dialog::SetControlEnabled(ControlIndex ID, bool bEnabled)
{
    ControlPtr pControl = GetControl<Control>(ID);
    if (!pControl)
        return;

    pControl->SetEnabled(bEnabled);
}


//--------------------------------------------------------------------------------------
void Dialog::OnMouseUp(const Point& pt) noexcept
{
    //TODO: do something here?
    GLUF_UNREFERENCED_PARAMETER(pt);
    sControlPressed = nullptr;
    mControlMouseOver = nullptr;
}


//--------------------------------------------------------------------------------------
void Dialog::OnMouseMove(const Point& pt) noexcept
{
    // Figure out which control the mouse is over now
    ControlPtr pControl = GetControlAtPoint(pt);

    // If the mouse is still over the same control, nothing needs to be done
    if (pControl == mControlMouseOver)
        return;

    // Handle mouse leaving the old control
    if (mControlMouseOver)
    {
        mControlMouseOver->OnMouseLeave();
        mControlMouseOver = nullptr;
    }

    // Handle mouse entering the new control
    mControlMouseOver = pControl;
    if (pControl)
    {
        mControlMouseOver->OnMouseEnter();
    }
}


//--------------------------------------------------------------------------------------
void Dialog::SetDefaultElement(ControlType controlType, ElementIndex elementIndex, const Element& element)
{
    // If this Element type already exist in the list, simply update the stored Element
    for (auto it : mDefaultElements)
    {
        if (it->mControlType == controlType && it->mElementIndex == elementIndex)
        {
            it->mElement = element;
            return;
        }
    }

    // Otherwise, add a new entry
    ElementHolderPtr pNewHolder = std::make_shared<ElementHolder>();

    pNewHolder->mControlType = controlType;
    pNewHolder->mElementIndex = elementIndex;
    pNewHolder->mElement = element;

    mDefaultElements.push_back(pNewHolder);
}


//--------------------------------------------------------------------------------------
Element Dialog::GetDefaultElement(ControlType controlType, ElementIndex elementIndex) const
{
    for (auto it : mDefaultElements)
    {
        if (it->mControlType == controlType && it->mElementIndex == elementIndex)
        {
            return it->mElement;
        }
    }

    GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("GetDefaultElement: elementIndex could not be found within controlType"));

    return Element();
}

//--------------------------------------------------------------------------------------
void Dialog::AddStatic(ControlIndex ID, const std::wstring& strText, const Rect& region, Bitfield textFlags, bool isDefault, std::shared_ptr<StaticPtr> ctrlPtr)
{
    auto pStatic = CreateStatic(textFlags, *this);

    if (ctrlPtr)
        *ctrlPtr = pStatic;

    // Set the ID and list index
    pStatic->SetID(ID);
    pStatic->SetText(strText);
    pStatic->SetRegion(region);
    pStatic->mIsDefault = isDefault;

    AddControl(std::dynamic_pointer_cast<Control>(pStatic));
}


//--------------------------------------------------------------------------------------
void Dialog::AddButton(ControlIndex ID, const std::wstring& strText, const Rect& region, int hotkey, bool isDefault, std::shared_ptr<ButtonPtr> ctrlPtr)
{
    auto pButton = CreateButton(*this);

    if (ctrlPtr)
        *ctrlPtr = pButton;

    // Set the ID and list index
    pButton->SetID(ID);
    pButton->SetText(strText);
    pButton->SetRegion(region);
    pButton->SetHotkey(hotkey);
    pButton->mIsDefault = isDefault;

    AddControl(std::dynamic_pointer_cast<Control>(pButton));
}


//--------------------------------------------------------------------------------------
void Dialog::AddCheckBox(ControlIndex ID, const std::wstring& strText, const Rect& region, bool checked , int hotkey, bool isDefault, std::shared_ptr<CheckBoxPtr> ctrlPtr)
{
    auto pCheckBox = CreateCheckBox(checked, *this);

    if (ctrlPtr)
        *ctrlPtr = pCheckBox;

    // Set the ID and list index
    pCheckBox->SetID(ID);
    pCheckBox->SetText(strText);
    pCheckBox->SetRegion(region);
    pCheckBox->SetHotkey(hotkey);
    pCheckBox->mIsDefault = isDefault;
    pCheckBox->SetChecked(checked);

    AddControl(std::dynamic_pointer_cast<Control>(pCheckBox));
}


//--------------------------------------------------------------------------------------
void Dialog::AddRadioButton(ControlIndex ID, RadioButtonGroup buttonGroup, const std::wstring& strText, const Rect& region, bool checked, int hotkey, bool isDefault, std::shared_ptr<RadioButtonPtr> ctrlPtr)
{
    auto pRadioButton = CreateRadioButton(*this);

    if (ctrlPtr)
        *ctrlPtr = pRadioButton;

    // Set the ID and list index
    pRadioButton->SetID(ID);
    pRadioButton->SetText(strText);
    pRadioButton->SetButtonGroup(buttonGroup);
    pRadioButton->SetRegion(region);
    pRadioButton->SetHotkey(hotkey);
    pRadioButton->SetChecked(checked);
    pRadioButton->mIsDefault = isDefault;
    pRadioButton->SetChecked(checked);

    AddControl(std::dynamic_pointer_cast<Control>(pRadioButton));
}


//--------------------------------------------------------------------------------------
void Dialog::AddComboBox(ControlIndex ID, const Rect& region, int hotKey, bool isDefault, std::shared_ptr<ComboBoxPtr> ctrlPtr)
{
    auto pComboBox = CreateComboBox(*this);

    if (ctrlPtr)
        *ctrlPtr = pComboBox;

    // Set the ID and list index
    pComboBox->SetID(ID);
    pComboBox->SetRegion(region);
    pComboBox->SetHotkey(hotKey);
    pComboBox->mIsDefault = isDefault;

    AddControl(std::dynamic_pointer_cast<Control>(pComboBox));
}


//--------------------------------------------------------------------------------------
void Dialog::AddSlider(ControlIndex ID, const Rect& region, long min, long max, long value, bool isDefault, std::shared_ptr<SliderPtr> ctrlPtr)
{
    auto pSlider = CreateSlider(*this);

    if (ctrlPtr)
        *ctrlPtr = pSlider;

    // Set the ID and list index
    pSlider->SetID(ID);
    pSlider->SetRegion(region);
    pSlider->mIsDefault = isDefault;
    pSlider->SetRange(min, max);
    pSlider->SetValue(value);
    pSlider->UpdateRects();

    AddControl(std::dynamic_pointer_cast<Control>(pSlider));
}


//--------------------------------------------------------------------------------------
void Dialog::AddEditBox(ControlIndex ID, const std::wstring& strText, const Rect& region, Charset charset, GLbitfield textFlags, bool isDefault, std::shared_ptr<EditBoxPtr> ctrlPtr)
{
    auto pEditBox = CreateEditBox(*this, (textFlags & GT_MULTI_LINE) == GT_MULTI_LINE);

    if (ctrlPtr)
        *ctrlPtr = pEditBox;

    pEditBox->GetElement(0).mTextFormatFlags = textFlags;

    // Set the ID and position
    pEditBox->SetID(ID);
    pEditBox->SetRegion(region);
    pEditBox->mIsDefault = isDefault;
    pEditBox->SetCharset(charset);
    pEditBox->SetText(strText);

    AddControl(std::dynamic_pointer_cast<Control>(pEditBox));
}


//--------------------------------------------------------------------------------------
void Dialog::AddListBox(ControlIndex ID, const Rect& region, Bitfield style, std::shared_ptr<ListBoxPtr> ctrlPtr)
{
    auto pListBox = CreateListBox(*this);

    if (ctrlPtr)
        *ctrlPtr = pListBox;

    // Set the ID and position
    pListBox->SetID(ID);
    pListBox->SetRegion(region);
    pListBox->SetStyle(style);

    AddControl(std::dynamic_pointer_cast<Control>(pListBox));
}


//--------------------------------------------------------------------------------------
void Dialog::AddControl(ControlPtr pControl)
{
    InitControl(pControl);

    if (!pControl)
        return;

    // Add to the list
    mControls[pControl->mID] = pControl;
}


//--------------------------------------------------------------------------------------
void Dialog::InitControl(ControlPtr pControl)
{
    //Result hr;

    if (!pControl)
        return;

    pControl->mIndex = static_cast<unsigned int>(mControls.size());

    // Look for a default Element entry
    for (auto it : mDefaultElements)
    {
        if (it->mControlType == pControl->GetType())
            pControl->SetElement(it->mElementIndex, it->mElement);
    }

    pControl->OnInit();
}


//--------------------------------------------------------------------------------------
ControlPtr Dialog::GetControl(ControlIndex ID, ControlType controlType) const
{
    // Try to find the control with the given ID
    for (auto it : mControls)
    {
        if (it.second->GetID() == ID && it.second->GetType() == controlType)
        {
            return it.second;
        }
    }

    GLUF_NON_CRITICAL_EXCEPTION(std::invalid_argument("Control ID Not Found"));

    // Not found
    return nullptr;
}


//--------------------------------------------------------------------------------------
ControlPtr Dialog::GetNextControl(ControlPtr control)
{
    Dialog& dialog = control->mDialog;


    auto indexIt = dialog.mControls.find(control->mID);

    //get the 'next' one
    ++indexIt;

    //is this still a valid control?
    if (indexIt != dialog.mControls.end())
        return indexIt->second;//yes

    //if not, get the next dialog
    DialogPtr nextDlg = dialog.mNextDialog;

    //in the event that there is only one control, or the dialogs were not hooked up correctly
    if (!nextDlg)
        return control;

    auto nextDialogIndexIt = nextDlg->mControls.begin();

    //keep going through dialogs until one with a control is found, but prevent looping back through
    while (nextDialogIndexIt == nextDlg->mControls.end())
    {
        nextDlg = nextDlg->mNextDialog;
        nextDialogIndexIt = nextDlg->mControls.begin();

        //if the same dialog is looped back through, return the current control
        if (nextDlg.get() == &dialog)
        {
            return control;
        }
    }

    //a control was found before the dialog chain was looped through
    return nextDialogIndexIt->second;
}


//--------------------------------------------------------------------------------------
ControlPtr Dialog::GetPrevControl(ControlPtr control)
{
    Dialog& dialog = control->mDialog;

    auto indexIt = dialog.mControls.find(control->mID);

    //get the 'previous' one
    --indexIt;

    //is this still a valid control?
    if (indexIt != dialog.mControls.end())
        return indexIt->second;

    //if not get the previous dialog
    DialogPtr prevDlg = dialog.mPrevDialog;

    //in the event that there is only one control, or the dialogs were not hooked up correctly
    if (!prevDlg)
        return control;

    auto prevDialogIndexIt = prevDlg->mControls.rbegin();

    //keep going through the dialogs until one with a control is found, but prevent looping back through
    while (prevDialogIndexIt == prevDlg->mControls.rend())
    {
        prevDlg = prevDlg->mPrevDialog;
        prevDialogIndexIt = prevDlg->mControls.rbegin();

        //if the same dialog is looped back through, return the current control
        if (prevDlg.get() == &dialog)
        {
            return control;
        }
    }

    //a control was found before the dialog chain was looped through
    return prevDialogIndexIt->second;
}


//--------------------------------------------------------------------------------------
void Dialog::ClearRadioButtonGroup(RadioButtonGroup buttonGroup)
{
    // Find all radio buttons with the given group number
    for (auto it : mControls)
    {
        if (it.second->GetType() == CONTROL_RADIOBUTTON)
        {
            RadioButtonPtr radioButton = std::dynamic_pointer_cast<RadioButton>(it.second);

            if (radioButton->GetButtonGroup() == buttonGroup)
                radioButton->SetChecked(false, false);
        }
    }
}


//--------------------------------------------------------------------------------------
std::vector<RadioButtonPtr> Dialog::GetRadioButtonGroup(RadioButtonGroup buttonGroup)
{
    std::vector<RadioButtonPtr> ret;

    // Find all radio buttons with the given group number
    for (auto it : mControls)
    {
        if (it.second->GetType() == CONTROL_RADIOBUTTON)
        {
            RadioButtonPtr radioButton = std::dynamic_pointer_cast<RadioButton>(it.second);

            if (radioButton->GetButtonGroup() == buttonGroup)
                ret.push_back(radioButton);
        }
    }

    return ret;
}


//--------------------------------------------------------------------------------------
void Dialog::ClearComboBox(ControlIndex ID)
{
    ComboBoxPtr comboBox = GetControl<ComboBox>(ID);
    if (!comboBox)
        return;

    comboBox->RemoveAllItems();
}


//--------------------------------------------------------------------------------------
void Dialog::RequestFocus(ControlPtr control)
{
    if (sControlFocus == control)
        return;

    if (!control->CanHaveFocus())
        return;

    if (sControlFocus)
        sControlFocus->OnFocusOut();

    control->OnFocusIn();
    sControlFocus = control;
}


//--------------------------------------------------------------------------------------
void Dialog::DrawRect(const Rect& rect, const Color& color, bool transform)
{
    Rect rcScreen = rect;

    if (transform)
        ScreenSpaceToGLSpace(rcScreen);

    //if (m_bCaption)
    //    OffsetRect(rcScreen, 0, m_nCaptionHeight);

    //rcScreen = ScreenToClipspace(rcScreen);

    auto thisSprite = SpriteVertexStruct::MakeMany(4);
    thisSprite[0] =
            {
                    glm::vec3(rcScreen.left, rcScreen.top, _NEAR_BUTTON_DEPTH),
                    ColorToFloat(color),
                    glm::vec2()
            };

    thisSprite[1] =
            {
                    glm::vec3(rcScreen.right, rcScreen.top, _NEAR_BUTTON_DEPTH),
                    ColorToFloat(color),
                    glm::vec2()
            };

    thisSprite[2] =
            {
                    glm::vec3(rcScreen.left, rcScreen.bottom, _NEAR_BUTTON_DEPTH),
                    ColorToFloat(color),
                    glm::vec2()
            };

    thisSprite[3] =
            {
                    glm::vec3(rcScreen.right, rcScreen.bottom, _NEAR_BUTTON_DEPTH),
                    ColorToFloat(color),
                    glm::vec2()
            };

    mDialogManager->mSpriteBuffer.BufferData(thisSprite);

    // Why are we drawing the sprite every time?  This is very inefficient, but the sprite workaround doesn't have support for sorting now, so we have to
    // draw a sprite every time to keep the order correct between sprites and text.
    mDialogManager->EndSprites(nullptr, false);//render in untextured mode
}


//--------------------------------------------------------------------------------------

void Dialog::DrawSprite(const Element& element, const Rect& rect, float depth, bool textured)
{
    // No need to draw fully transparent layers
    if (element.mTextureColor.GetCurrent().a == 0)
        return;

    /*if (element->mTextureColor.GetCurrent() == element->mTextureColor.mStates[STATE_HIDDEN])
        return;*/


    Rectf uvRect = element.mUVRect;

    Rect rcScreen = rect;

    OffsetRect(rcScreen, mRegion.x - long(g_WndWidth / 2), mCaptionHeight + mRegion.y - long(g_WndHeight / 2));

    /*TextureNodePtr textureNode = GetTexture(pElement->iTexture);
    if (!textureNode)
        return;*/

    auto thisSprite = SpriteVertexStruct::MakeMany(4);

    thisSprite[0] =
            {
                    glm::vec3(rcScreen.left, rcScreen.top, depth),
                    ColorToFloat(element.mTextureColor.GetCurrent()),
                    glm::vec2(uvRect.left, uvRect.top)
            };

    thisSprite[1] =
            {
                    glm::vec3(rcScreen.right, rcScreen.top, depth),
                    ColorToFloat(element.mTextureColor.GetCurrent()),
                    glm::vec2(uvRect.right, uvRect.top)
            };

    thisSprite[2] =
            {
                    glm::vec3(rcScreen.left, rcScreen.bottom, depth),
                    ColorToFloat(element.mTextureColor.GetCurrent()),
                    glm::vec2(uvRect.left, uvRect.bottom)
            };

    thisSprite[3] =
            {
                    glm::vec3(rcScreen.right, rcScreen.bottom, depth),
                    ColorToFloat(element.mTextureColor.GetCurrent()),
                    glm::vec2(uvRect.right, uvRect.bottom)
            };

    mDialogManager->mSpriteBuffer.BufferData(thisSprite);

    // Why are we drawing the sprite every time?  This is very inefficient, but the sprite workaround doesn't have support for sorting now, so we have to
    // draw a sprite every time to keep the order correct between sprites and text.
    mDialogManager->EndSprites(&element, textured);
}


//--------------------------------------------------------------------------------------
void Dialog::DrawText(const std::wstring& text, const Element& element, const Rect& rect, bool shadow, bool hardRect)
{
    // No need to draw fully transparent layers
    if (element.mFontColor.GetCurrent().a == 0)
        return;

    Rect screen = rect;
    OffsetRect(screen, mRegion.x, mRegion.y);


    OffsetRect(screen, 0, mCaptionHeight);

    /*if (bShadow)
    {
        Rect rcShadow = rcScreen;
        OffsetRect(rcShadow, 1 / m_pManager->GetWindowSize().x, 1 / m_pManager->GetWindowSize().y);

        Color vShadowColor(0, 0, 0, 255);
        DrawText(*m_pManager->GetFontNode(pElement->mFontIndex), strText, rcShadow, vShadowColor, bCenter, bHardRect);

    }*/

    Color vFontColor = element.mFontColor.GetCurrent();
    Text::DrawText(mDialogManager->GetFontNode(element.mFontIndex), text, screen, element.mFontColor.GetCurrent(), element.mTextFormatFlags, hardRect);
}


//--------------------------------------------------------------------------------------
void Dialog::CalcTextRect(const std::wstring& text, const Element& element, Rect& rect) const
{
    FontNodePtr pFontNode = GetFont(element.mFontIndex);
    if (!pFontNode)
        return;

    GLUF_UNREFERENCED_PARAMETER(text);
    GLUF_UNREFERENCED_PARAMETER(element);
    GLUF_UNREFERENCED_PARAMETER(rect);
    // TODO -

}

//--------------------------------------------------------------------------------------
void Dialog::SetNextDialog(DialogPtr nextDialog) noexcept
{
    if (!nextDialog)
        mNextDialog = shared_from_this();
    else
    {
        mNextDialog = nextDialog;
        nextDialog->mPrevDialog = shared_from_this();
    }
}


//--------------------------------------------------------------------------------------
void Dialog::ClearFocus() noexcept
{
    if (sControlFocus)
    {
        sControlFocus->OnFocusOut();
        sControlFocus = nullptr;
    }
}


//--------------------------------------------------------------------------------------
void Dialog::FocusDefaultControl() noexcept
{
    // Check for default control in this dialog
    for (auto it : mControls)
    {
        if (it.second->mIsDefault)
        {
            // Remove focus from the current control
            ClearFocus();

            // Give focus to the default control
            sControlFocus = it.second;
            sControlFocus->OnFocusIn();
            return;
        }
    }
}

void Dialog::ScreenSpaceToGLSpace(Rect& rc) noexcept
{
    OffsetRect(rc, mRegion.left - long(g_WndWidth / 2), mRegion.bottom - long(g_WndHeight / 2));
}

void Dialog::ScreenSpaceToGLSpace(Point& pt) noexcept
{
    pt.x += mRegion.left - long(g_WndWidth / 2);
    pt.y += mRegion.bottom - long(g_WndHeight / 2);
}


//--------------------------------------------------------------------------------------
bool Dialog::OnCycleFocus(bool forward) noexcept
{
    ControlPtr pControl = nullptr;
    DialogPtr pDialog = nullptr; // pDialog and pLastDialog are used to track wrapping of
    DialogPtr pLastDialog;    // focus from first control to last or vice versa.

    if (!sControlFocus)
    {
        // If sControlFocus is nullptr, we focus the first control of first dialog in
        // the case that bForward is true, and focus the last control of last dialog when
        // bForward is false.
        //
        if (forward)
        {
            // Search for the first control from the start of the dialog
            // array.
            for (auto it : mDialogManager->mDialogs)
            {
                pDialog = pLastDialog = it;
                if (pDialog && !pDialog->mControls.empty())
                {
                    pControl = pDialog->mControls[0];
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
            for (auto it = mDialogManager->mDialogs.crbegin(); it != mDialogManager->mDialogs.crend(); ++it)
            {
                pDialog = pLastDialog = *it;
                if (pDialog && !pDialog->mControls.empty())
                {
                    pControl = pDialog->mControls[pDialog->mControls.size() - 1];
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
    else if (&sControlFocus->mDialog != this)
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

        //this is safe to assume that the dialog is 'this' because of the line 'else if (&sControlFocus->mDialog != this)'
        pLastDialog = shared_from_this();

        pControl = (forward) ? GetNextControl(sControlFocus) : GetPrevControl(sControlFocus);

        //this is kind of inefficient
        if (!(pDialog = mDialogManager->GetDialogPtrFromRef(pControl->mDialog)))
            pDialog = shared_from_this();//not sure if this is what to do if the dialog is not found, but its the best thing I could think of
    }

    assert(pControl != 0);

    // If we just wrapped from last control to first or vice versa,
    // set the focused control to nullptr. This state, where no control
    // has focus, allows the camera to work.
    int nLastDialogIndex = -1;
    for (uint32_t i = 0; i < mDialogManager->mDialogs.size(); ++i)
    {
        if (mDialogManager->mDialogs[i] == pLastDialog)
        {
            nLastDialogIndex = i;
            break;
        }
    }

    int nDialogIndex = -1;
    for (uint32_t i = 0; i < mDialogManager->mDialogs.size(); ++i)
    {
        if (mDialogManager->mDialogs[i] == pDialog)
        {
            nDialogIndex = i;
            break;
        }
    }

    if ((!forward && nLastDialogIndex < nDialogIndex) ||
        (forward && nDialogIndex < nLastDialogIndex))
    {
        if (sControlFocus)
            sControlFocus->OnFocusOut();
        sControlFocus = nullptr;
        return true;
    }

    // If we've gone in a full circle then focus doesn't change
    if (pControl == sControlFocus)
        return true;

    // If the dialog accepts keybord input and the control can have focus then
    // move focus
    if (pControl->mDialog.mKeyboardInput && pControl->CanHaveFocus())
    {
        if (sControlFocus)
            sControlFocus->OnFocusOut();
        sControlFocus = pControl;
        if (sControlFocus)
            sControlFocus->OnFocusIn();
        return true;
    }

    pLastDialog = pDialog;
    pControl = (forward) ? GetNextControl(pControl) : GetPrevControl(pControl);
    if (!(pDialog = mDialogManager->GetDialogPtrFromRef(pControl->mDialog)))
        pDialog = shared_from_this();

    // If we reached this point, the chain of dialogs didn't form a complete loop
    GLUF_ERROR("Dialog: Multiple dialogs are improperly chained together");
    return false;
}

FontPtr g_ArialDefault = nullptr;
//--------------------------------------------------------------------------------------
void Dialog::InitDefaultElements()
{
    //this makes it more efficient
    int fontIndex = 0;
    if (g_DefaultFont == nullptr)
    {
        if (g_ArialDefault == nullptr)
        {

            std::vector<char> rawData;
            LoadFileIntoMemory("Arial.ttf", rawData);
            LoadFont(g_ArialDefault, rawData, 15L);
        }

        fontIndex = mDialogManager->AddFont(g_ArialDefault, 20, FONT_WEIGHT_NORMAL);
    }
    else
    {
        fontIndex = mDialogManager->AddFont(g_DefaultFont, 20, FONT_WEIGHT_NORMAL);
    }

    SetFont(0, fontIndex);

    Element Element;
    Rectf rcTexture;

    //-------------------------------------
    // Element for the caption
    //-------------------------------------
    //mCapElement = Element();
    mCapElement.SetFont(0);
    SetRect(rcTexture, 0.0f, 0.078125f, 0.4296875f, 0.0f);//blank part of the texture
    mCapElement.SetTexture(0, rcTexture);
    mCapElement.mTextureColor.Init({ 255, 255, 255, 255 });
    mCapElement.mFontColor.Init({ 255, 255, 255, 255 });
    mCapElement.SetFont(0, { 0, 0, 0, 255 }, GT_LEFT | GT_VCENTER);
    // Pre-blend as we don't need to transition the state
    mCapElement.mTextureColor.Blend(STATE_NORMAL, 10.0f);
    mCapElement.mFontColor.Blend(STATE_NORMAL, 10.0f);

    //mDlgElement = Element()
    mDlgElement.SetFont(0);
    SetRect(rcTexture, 0.0f, 0.078125f, 0.4296875f, 0.0f);//blank part of the texture
    //SetRect(rcTexture, 0.0f, 1.0f, 1.0f, 0.0f);//blank part of the texture
    mDlgElement.SetTexture(0, rcTexture);
    mDlgElement.mTextureColor.Init({ 255, 0, 0, 128 });
    mDlgElement.mFontColor.Init({ 0, 0, 0, 255 });
    mDlgElement.SetFont(0, { 0, 0, 0, 255 }, GT_LEFT | GT_VCENTER);
    // Pre-blend as we don't need to transition the state
    mDlgElement.mTextureColor.Blend(STATE_NORMAL, 10.0f);
    mDlgElement.mFontColor.Blend(STATE_NORMAL, 10.0f);

    /*

    Streamline the control blending

    */
    Element.mFontColor.SetState(STATE_NORMAL, {0, 0, 0, 255});
    Element.mFontColor.SetState(STATE_DISABLED, {0, 0, 0, 128});
    Element.mFontColor.SetState(STATE_HIDDEN, {0, 0, 0, 0});
    Element.mFontColor.SetState(STATE_FOCUS, {0, 0, 0, 255});
    Element.mFontColor.SetState(STATE_MOUSEOVER, {0, 0, 0, 255});
    Element.mFontColor.SetState(STATE_PRESSED, {0, 0, 0, 255});
    Element.mFontColor.SetCurrent(STATE_NORMAL);

    Element.mTextureColor.SetState(STATE_NORMAL, { 180, 180, 180, 255 });
    Element.mTextureColor.SetState(STATE_DISABLED, { 128, 128, 128, 128 });
    Element.mTextureColor.SetState(STATE_HIDDEN, { 0, 0, 0, 0 });
    Element.mTextureColor.SetState(STATE_FOCUS, { 200, 200, 200, 255 });
    Element.mTextureColor.SetState(STATE_MOUSEOVER, { 255, 255, 255, 255 });
    Element.mTextureColor.SetState(STATE_PRESSED, { 200, 200, 200, 255 });
    Element.mTextureColor.SetCurrent(STATE_NORMAL);

    Element.mFontIndex = 0;
    Element.mTextureIndex = 0;

    //-------------------------------------
    // Static
    //-------------------------------------
    Element.mTextFormatFlags = GT_LEFT | GT_VCENTER;

    // Assign the Element
    SetDefaultElement(CONTROL_STATIC, 0, Element);


    //-------------------------------------
    // Button - Button
    //-------------------------------------
    SetRect(rcTexture, 0.0f, 1.0f, 0.53125f, 0.7890625f);
    Element.mUVRect = rcTexture;

    // Assign the Element
    SetDefaultElement(CONTROL_BUTTON, 0, Element);


    //-------------------------------------
    // Button - Fill layer
    //-------------------------------------
    SetRect(rcTexture, 0.53125f, 1.0f, 0.984375f, 0.7890625f);
    Element.mUVRect = rcTexture;

    // Assign the Element
    SetDefaultElement(CONTROL_BUTTON, 1, Element);


    //-------------------------------------
    // CheckBox - Box
    //-------------------------------------
    SetRect(rcTexture, 0.0f, 0.7890625f, 0.10546875f, 0.68359375f);
    Element.mUVRect = rcTexture;

    // Assign the Element
    SetDefaultElement(CONTROL_CHECKBOX, 0, Element);


    //-------------------------------------
    // CheckBox - Check
    //-------------------------------------
    SetRect(rcTexture, 0.10546875f, 0.7890625f, 0.2109375f, 0.68359375f);
    Element.mUVRect = rcTexture;

    // Assign the Element
    SetDefaultElement(CONTROL_CHECKBOX, 1, Element);


    //-------------------------------------
    // RadioButton - Box
    //-------------------------------------
    SetRect(rcTexture, 0.2109375f, 0.7890625f, 0.31640625f, 0.68359375f);
    Element.mUVRect = rcTexture;

    // Assign the Element
    SetDefaultElement(CONTROL_RADIOBUTTON, 0, Element);


    //-------------------------------------
    // RadioButton - Check
    //-------------------------------------
    SetRect(rcTexture, 0.31640625f, 0.7890625f, 0.421875f, 0.68359375f);
    Element.mUVRect = rcTexture;

    // Assign the Element
    SetDefaultElement(CONTROL_RADIOBUTTON, 1, Element);


    //-------------------------------------
    // ComboBox - Main
    //-------------------------------------
    //SetRect(rcTexture, 0.02734375f, 0.5234375f, 0.96484375f, 0.3671875f);
    SetRect(rcTexture, 0.05078125f, 0.5234375f, 0.96484375f, 0.3671875f);
    Element.mUVRect = rcTexture;


    // Assign the Element
    SetDefaultElement(CONTROL_COMBOBOX, 0, Element);


    //-------------------------------------
    // ComboBox - Button
    //-------------------------------------
    SetRect(rcTexture, 0.3828125f, 0.26171875f, 0.58984375f, 0.0703125f);
    Element.mUVRect = rcTexture;

    // Assign the Element
    SetDefaultElement(CONTROL_COMBOBOX, 1, Element);


    //-------------------------------------
    // ComboBox - Dropdown
    //-------------------------------------
    SetRect(rcTexture, 0.05078125f, 0.51953125f, 0.94140625f, 0.37109375f);
    Element.mUVRect = rcTexture;
    Element.mTextFormatFlags = GT_LEFT | GT_TOP;

    // Assign the Element
    SetDefaultElement(CONTROL_COMBOBOX, 2, Element);


    //-------------------------------------
    // ComboBox - Selection
    //-------------------------------------
    SetRect(rcTexture, 0.046875f, 0.36328125f, 0.93359375f, 0.28515625f);
    Element.mUVRect = rcTexture;

    // Assign the Element
    SetDefaultElement(CONTROL_COMBOBOX, 3, Element);


    //-------------------------------------
    // Slider - Track
    //-------------------------------------
    SetRect(rcTexture, 0.00390625f, 0.26953125f, 0.36328125f, 0.109375f);
    Element.mUVRect = rcTexture;

    // Assign the Element
    SetDefaultElement(CONTROL_SLIDER, 0, Element);

    //-------------------------------------
    // Slider - Button
    //-------------------------------------
    SetRect(rcTexture, 0.58984375f, 0.24609375f, 0.75f, 0.0859375f);
    Element.mUVRect = rcTexture;

    // Assign the Element
    SetDefaultElement(CONTROL_SLIDER, 1, Element);

    //-------------------------------------
    // ScrollBar - Track
    //-------------------------------------
    float nScrollBarStartX = 0.76470588f;
    float nScrollBarStartY = 0.046875f;
    SetRect(rcTexture, nScrollBarStartX + 0.0f, nScrollBarStartY + 0.12890625f, nScrollBarStartX + 0.09076287f, nScrollBarStartY + 0.125f);
    Element.mUVRect = rcTexture;

    // Assign the Element
    SetDefaultElement(CONTROL_SCROLLBAR, 0, Element);

    //-------------------------------------
    // ScrollBar - Down Arrow
    //-------------------------------------
    SetRect(rcTexture, nScrollBarStartX + 0.0f, nScrollBarStartY + 0.08203125f, nScrollBarStartX + 0.09076287f, nScrollBarStartY + 0.00390625f);
    Element.mUVRect = rcTexture;


    // Assign the Element
    SetDefaultElement(CONTROL_SCROLLBAR, 2, Element);

    //-------------------------------------
    // ScrollBar - Up Arrow
    //-------------------------------------
    SetRect(rcTexture, nScrollBarStartX + 0.0f, nScrollBarStartY + 0.20703125f, nScrollBarStartX + 0.09076287f, nScrollBarStartY + 0.125f);
    Element.mUVRect = rcTexture;


    // Assign the Element
    SetDefaultElement(CONTROL_SCROLLBAR, 1, Element);

    //-------------------------------------
    // ScrollBar - Button
    //-------------------------------------
    SetRect(rcTexture, 0.859375f, 0.25f, 0.9296875f, 0.0859375f);
    Element.mUVRect = rcTexture;

    // Assign the Element
    SetDefaultElement(CONTROL_SCROLLBAR, 3, Element);

    //-------------------------------------
    // EditBox
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

    //TODO: this
    // Assign the style
    SetRect(rcTexture, 0.0507812f, 0.6484375f, 0.9375f, 0.55859375f);
    Element.mUVRect = rcTexture;
    SetDefaultElement(CONTROL_EDITBOX, 0, Element);

    SetRect(rcTexture, 0.03125f, 0.6796875f, 0.0546875f, 0.6484375f);
    Element.mUVRect = rcTexture;
    SetDefaultElement(CONTROL_EDITBOX, 1, Element);

    SetRect(rcTexture, 0.0546875f, 0.6796875f, 0.94140625f, 0.6484375f);
    Element.mUVRect = rcTexture;
    SetDefaultElement(CONTROL_EDITBOX, 2, Element);

    SetRect(rcTexture, 0.94140625f, 0.6796875f, 0.9609375f, 0.6484375f);
    Element.mUVRect = rcTexture;
    SetDefaultElement(CONTROL_EDITBOX, 3, Element);

    SetRect(rcTexture, 0.03125f, 0.6484375f, 0.0546875f, 0.55859375f);
    Element.mUVRect = rcTexture;
    SetDefaultElement(CONTROL_EDITBOX, 4, Element);

    SetRect(rcTexture, 0.94140625f, 0.6484375f, 0.9609375f, 0.55859375f);
    Element.mUVRect = rcTexture;
    SetDefaultElement(CONTROL_EDITBOX, 5, Element);

    SetRect(rcTexture, 0.03125f, 0.55859375f, 0.0546875f, 0.52734375f);
    Element.mUVRect = rcTexture;
    SetDefaultElement(CONTROL_EDITBOX, 6, Element);

    SetRect(rcTexture, 0.0546875f, 0.55859375f, 0.94140625f, 0.52734375f);
    Element.mUVRect = rcTexture;
    SetDefaultElement(CONTROL_EDITBOX, 7, Element);

    SetRect(rcTexture, 0.94140625f, 0.55859375f, 0.9609375f, 0.52734375f);
    Element.mUVRect = rcTexture;
    SetDefaultElement(CONTROL_EDITBOX, 8, Element);

    //-------------------------------------
    // ListBox - Main
    //-------------------------------------
    SetRect(rcTexture, 0.05078125f, 0.51953125f, 0.94140625f, 0.375f);
    Element.mUVRect = rcTexture;

    // Assign the Element
    SetDefaultElement(CONTROL_LISTBOX, 0, Element);

    //-------------------------------------
    // ListBox - Selection
    //-------------------------------------

    SetRect(rcTexture, 0.0625f, 0.3515625f, 0.9375f, 0.28515625f);
    Element.mUVRect = rcTexture;

    // Assign the Element
    SetDefaultElement(CONTROL_LISTBOX, 1, Element);
}

