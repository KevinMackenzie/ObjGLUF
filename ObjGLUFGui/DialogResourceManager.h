#ifndef OBJGLUF_DIALOGRESOURCEMANAGER_H
#define OBJGLUF_DIALOGRESOURCEMANAGER_H

/*
DialogResourceManager

    Note:
        Manages Resources Shared by the Dialog

    Data Members:
        'mWndSize': the size of the window
        'mSpriteBuffer': the vertex array for drawing sprites; use 'VertexArray'
            because it automatically handles OpenGL version restrictions
        'mDialogs': the list of registered dialogs
        'mTextureCache': a list of shared textures
        'mFontCache': a list of shared fonts

*/
class DialogResourceManager
{

    Point mWndSize;

    VertexArray mSpriteBuffer;
    std::vector<DialogPtr> mDialogs;
    std::vector<TextureNodePtr> mTextureCache;
    std::vector<FontNodePtr>    mFontCache;

    friend Dialog;
public:
    DialogResourceManager();
    ~DialogResourceManager();

    /*
    MsgProg

        Note:
            This only cares about window resize messages in order to keep 'mWndSize' accurate

        Parameters:
            see 'EventCallbackFuncPtr' for details

        Throws:
            no-throw guarantee
    */
    bool MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept;

    /*
    ApplyRenderUI(Untex)

        Note:
            This sets up the global UI shader
            if 'Untex', it does not enable texture coordinates

        Throws:
            no-throw guarantee
    */
    void ApplyRenderUI() noexcept;
    void ApplyRenderUIUntex() noexcept;

    /*
    BeginSprites

        Note:
            Clears the sprite vertices; call before rendering anything

        Throws:
            no-throw guarantee
    */
    void BeginSprites() noexcept;

    /*
    EndSprites

        Note:
            This buffers and draws the data from the sprites
            This is generally called to render every element individually for flexibility

        Parameters:
            'element': an element whose formatting/texture data is used
            'textured': are the sprites textured

        Throws:
            'std::out_of_range': if element texture index is out of range
    */
    void EndSprites(const Element* element, bool textured);

    /*
    GetFont/TextureNode

        Parameters:
            'index': the index of the font node within the DRM

        Throws:
            'std::out_of_range': if 'index' is larger than the size of the font/texture cache
    */
    FontNodePtr    GetFontNode(FontIndex index) const        { return mFontCache[index];        }
    TextureNodePtr GetTextureNode(TextureIndex index) const    { return mTextureCache[index];    }


    /*
    GetTexture/FontCount

        Returns:
            the number of fonts/textures

        Throws:
            no-throw guarantee

    */
    Size GetTextureCount() const noexcept   { return static_cast<Size>(mTextureCache.size()); }
    Size GetFontCount() const noexcept      { return static_cast<Size>(mFontCache.size());    }

    /*
    AddFont

        Parameters:
            'font': a pointer to the typeface
            'leading': the vertical distance between lines of text
            'weight': the weight to use

        Returns:
            the index of the created font

        Throws:
            no-throw guarantee

    */
    FontIndex AddFont(const FontPtr& font, FontSize leading, FontWeight weight) noexcept;

    /*
    AddTexture

        Note:
            TODO: when  supports a more advanced texture system, pass one of those,
                but for now just require an openGL texture id

        Parameters:
            'texture': the OpenGL Texture Id to use

        Returns:
            the index of the created texture

        Throws:
            no-throw guarantee
    */
    TextureIndex AddTexture(GLuint texture) noexcept;

    /*
    RegisterDialog

        Note:
            This sets up the dialogs' focus order for tabbing

        Parameters:
            'dialog': the dialog to register

        Throws:
            no-throw guarantee

    */
    void RegisterDialog(const DialogPtr& pDialog) noexcept;

    /*
    UnregisterDialog

        Parameters:
            'dialog': the dialog to remove

        Throws:
            'std::invalid_argument': if 'dialog' == nullptr or if 'dialog' is not a registered dialog in _DEBUG mode

    */
    void UnregisterDialog(const DialogPtr& dialog);

    /*
    EnableKeyboardInputForAllDialogs

        Note:
            Pretty self explanitory

        Throws:
            no-throw guarantee
    */
    void EnableKeyboardInputForAllDialogs() noexcept;


    /*
    GetWindowSize()

        Note:
            This can also be used to update mWndSize; This function does not simply return the size, it actually
                Gets the window size from GLFW

        Returns:
            The size of the window in pixels

    */
    Point GetWindowSize();

    /*
    GetOrthoMatrix

        Returns:
            the Orthographic projection matrix for the current window

        Throws:
            no-throw guarantee

    */
    glm::mat4 GetOrthoMatrix() noexcept;


    /*
    GetDialogPtrFromRef

        Returns:
            the std::shared_ptr version of the reference give, nullptr if not found

        Parameters:
            'ref': a reference to a dialog

    */
    DialogPtr GetDialogPtrFromRef(const Dialog& ref) noexcept;
protected:
    /*
    ApplyOrtho

        Note:
            Gets the Ortho Matrix, then Applies it to the current draw call; used internally

        Throws:
            no-throw guarantee
    */
    void ApplyOrtho() noexcept;

};

#endif //OBJGLUF_DIALOGRESOURCEMANAGER_H
