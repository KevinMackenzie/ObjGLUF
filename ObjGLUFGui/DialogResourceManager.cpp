#include "DialogResourceManager.h"

namespace GLUF {

//--------------------------------------------------------------------------------------
DialogResourceManager::DialogResourceManager() :
        mSpriteBuffer(GL_TRIANGLES, GL_STREAM_DRAW)//use stream draw because it will be changed every frame
{
    //glGenVertexArrayBindVertexArray(&m_pVBScreenQuadVAO);
    //glGenBuffers(1, &m_pVBScreenQuadIndicies);
    //glGenBuffers(1, &m_pVBScreenQuadPositions);
    //glGenBuffers(1, &m_pVBScreenQuadColor);
    //glGenBuffers(1, &m_pVBScreenQuadUVs);

    /*glGenBufferBindBuffer(GL_ARRAY_BUFFER, &m_pVBScreenQuadPositions);
    glVertexAttribPointer(g_UIShaderLocations.position, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glGenBufferBindBuffer(GL_ARRAY_BUFFER, &m_pVBScreenQuadColor);
    glVertexAttribPointer(g_UIShaderLocations.color, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

    glGenBufferBindBuffer(GL_ARRAY_BUFFER, &m_pVBScreenQuadUVs);
    glVertexAttribPointer(g_UIShaderLocations.uv, 2, GL_FLOAT, GL_FALSE, 0, nullptr);*/

    //this is static
    //glGenBufferBindBuffer(GL_ELEMENT_ARRAY_BUFFER, &m_pVBScreenQuadIndicies);

    //GLubyte indices[6] = {    2, 1, 0,
    //                        2, 3, 1};
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLubyte), indices, GL_STATIC_DRAW);


    //glGenVertexArrayBindVertexArray(&m_SpriteBufferVao);
    //glBindVertexArray(m_SpriteBufferVao);

    //glGenBuffers(1, &m_SpriteBufferPos);
    //glGenBuffers(1, &m_SpriteBufferColors);
    //glGenBuffers(1, &m_SpriteBufferTexCoords);
    //glGenBuffers(1, &m_SpriteBufferIndices);

    mSpriteBuffer.AddVertexAttrib({4, 3, g_UIShaderLocations.position, GL_FLOAT, 0}, 0);
    mSpriteBuffer.AddVertexAttrib({4, 4, g_UIShaderLocations.color, GL_FLOAT, 0}, 12);
    mSpriteBuffer.AddVertexAttrib({4, 2, g_UIShaderLocations.uv, GL_FLOAT, 0}, 28);

    //this is static
    //glGenBufferBindBuffer(GL_ELEMENT_ARRAY_BUFFER, &m_SpriteBufferIndices);

    mSpriteBuffer.BufferIndices(
            {
                    2, 1, 0,
                    2, 3, 1
            });

    //GLubyte indicesS[6] = { 2, 1, 0,
    //                        2, 3, 1 };
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLubyte), indicesS, GL_STATIC_DRAW);


    GetWindowSize();
}


//--------------------------------------------------------------------------------------
DialogResourceManager::~DialogResourceManager() {
    //mFontCache.clear();
    //mTextureCache.clear();

    //TODO: make this with a class in the buffer sections
    /*glBindVertexArray(m_pVBScreenQuadVAO);
    glDeleteBuffers(1, &m_pVBScreenQuadPositions);
    glDeleteBuffers(1, &m_pVBScreenQuadColor);
    glDeleteBuffers(1, &m_pVBScreenQuadUVs);
    glDeleteVertexArrays(1, &m_pVBScreenQuadVAO);

    glBindVertexArray(m_SpriteBufferVao);
    glDeleteBuffers(1, &m_SpriteBufferPos);
    glDeleteBuffers(1, &m_SpriteBufferColors);
    glDeleteBuffers(1, &m_SpriteBufferTexCoords);
    glDeleteVertexArrays(1, &m_SpriteBufferVao);
    glBindVertexArray(0);*/
}


//--------------------------------------------------------------------------------------
bool DialogResourceManager::MsgProc(MessageType msg, int32_t param1, int32_t param2, int32_t param3, int32_t param4) noexcept {
    GLUF_UNREFERENCED_PARAMETER(msg);
    //GLUF_UNREFERENCED_PARAMETER(param1);
    //GLUF_UNREFERENCED_PARAMETER(param2);
    GLUF_UNREFERENCED_PARAMETER(param3);
    GLUF_UNREFERENCED_PARAMETER(param4);

    switch (msg) {
        case RESIZE:
            mWndSize.width = 0L;
            mWndSize.height = 0L;
            GetWindowSize();

            //refresh the fonts to the new window size
            /*for (auto it : m_FontCache)
                if (it != nullptr)
                it->mFontType->Refresh();*/
    }

    return false;
}

//--------------------------------------------------------------------------------------
void DialogResourceManager::ApplyRenderUI() noexcept {
    // Shaders
    /*glEnableVertexAttribArray(g_UIShaderLocations.position);
    glEnableVertexAttribArray(g_UIShaderLocations.color);
    glEnableVertexAttribArray(g_UIShaderLocations.uv);*/
    SHADERMANAGER.UseProgram(g_UIProgram);

    ApplyOrtho();
}


//--------------------------------------------------------------------------------------
void DialogResourceManager::ApplyRenderUIUntex() noexcept {
    /*glEnableVertexAttribArray(g_UIShaderLocationsUntex.position);
    glEnableVertexAttribArray(g_UIShaderLocationsUntex.color);*/
    SHADERMANAGER.UseProgram(g_UIProgramUntex);

    ApplyOrtho();
}

glm::mat4 DialogResourceManager::GetOrthoMatrix() noexcept {
    Point pt = GetWindowSize();
    float x2 = (float) pt.x / 2.0f;
    float y2 = (float) pt.y / 2.0f;
    return glm::ortho((float) -x2, (float) x2, (float) -y2, (float) y2);
}

DialogPtr DialogResourceManager::GetDialogPtrFromRef(const Dialog &ref) noexcept {
    for (auto it = mDialogs.cbegin(); it != mDialogs.cend(); ++it) {
        if (it->get() == &ref) {
            return *it;
        }
    }

    return nullptr;
}

void DialogResourceManager::ApplyOrtho() noexcept {
    glm::mat4 mat = GetOrthoMatrix();
    SHADERMANAGER.GLUniformMatrix4f(g_UIShaderLocations.ortho, mat);
}

//--------------------------------------------------------------------------------------
void DialogResourceManager::BeginSprites() noexcept {
}


//--------------------------------------------------------------------------------------

void DialogResourceManager::EndSprites(const Element *element, bool textured) {
    /*if (textured)
    {
        mSpriteBuffer.EnableVertexAttribute(2);
    }
    else
    {
        mSpriteBuffer.DisableVertexAttribute(2);
    }*/


    if (textured && element) {
        ApplyRenderUI();

        TextureNodePtr pTexture = GetTextureNode(element->mTextureIndex);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, pTexture->mTextureElement);
        glUniform1i(g_UIShaderLocations.sampler, 0);
    } else {
        ApplyRenderUIUntex();
    }


    mSpriteBuffer.Draw();
}


//--------------------------------------------------------------------------------------
void DialogResourceManager::RegisterDialog(const DialogPtr &dialog) noexcept {
    if (!dialog)
        return;

    // Check that the dialog isn't already registered.
    for (auto it : mDialogs) {
        if (it == dialog)
            return;
    }

    // Add to the list.
    mDialogs.push_back(dialog);

    // Set up next and prev pointers.
    if (mDialogs.size() > 1)
        mDialogs[mDialogs.size() - 2]->SetNextDialog(dialog);
    mDialogs[mDialogs.size() - 1]->SetNextDialog(mDialogs[0]);
}


//--------------------------------------------------------------------------------------
void DialogResourceManager::UnregisterDialog(const DialogPtr &pDialog) {
    // Search for the dialog in the list.
    for (size_t i = 0; i < mDialogs.size(); ++i) {
        if (mDialogs[i] == pDialog) {
            mDialogs.erase(mDialogs.begin() + i);
            if (!mDialogs.empty()) {
                int l, r;

                if (0 == i)
                    l = int(mDialogs.size() - 1);
                else
                    l = int(i) - 1;

                if (mDialogs.size() == i)
                    r = 0;
                else
                    r = int(i);

                mDialogs[l]->SetNextDialog(mDialogs[r]);
            }
            return;
        }
    }
}


//--------------------------------------------------------------------------------------
void DialogResourceManager::EnableKeyboardInputForAllDialogs() noexcept {
    // Enable keyboard input for all registered dialogs
    for (auto it : mDialogs)
        it->EnableKeyboardInput(true);
}

//--------------------------------------------------------------------------------------
Point DialogResourceManager::GetWindowSize() {
    if (mWndSize.x == 0L || mWndSize.y == 0L) {
        int w, h;
        glfwGetWindowSize(g_pGLFWWindow, &w, &h);
        mWndSize.width = (long) w;
        mWndSize.height = (long) h;
        g_WndHeight = (unsigned short) h;
        g_WndWidth = (unsigned short) w;
    }
    return mWndSize;
}

//--------------------------------------------------------------------------------------
FontIndex DialogResourceManager::AddFont(const FontPtr &font, FontSize leading, FontWeight weight) noexcept {
    // See if this font already exists (this is simple)
    for (size_t i = 0; i < mFontCache.size(); ++i) {
        FontNodePtr node = mFontCache[i];
        if (node->mFontType == font && node->mWeight == weight && node->mLeading == leading)
            return i;
    }

    // Add a new font and try to create it
    auto newFontNode = std::make_shared<FontNode>();

    //wcscpy_s(pNewFontNode->strFace, MAX_PATH, strFaceName);
    newFontNode->mFontType = font;
    newFontNode->mLeading = leading;
    //pNewFontNode->mSize = height;
    newFontNode->mWeight = weight;
    mFontCache.push_back(newFontNode);

    return mFontCache.size() - 1;
}


//--------------------------------------------------------------------------------------
TextureIndex DialogResourceManager::AddTexture(GLuint texture) noexcept {
    // See if this texture already exists
    for (size_t i = 0; i < mTextureCache.size(); ++i) {
        TextureNodePtr pTextureNode = mTextureCache[i];
        if (texture == pTextureNode->mTextureElement)
            return i;
    }

    // Add a new texture and try to create it
    auto newTextureNode = std::make_shared<TextureNode>();

    newTextureNode->mTextureElement = texture;
    mTextureCache.push_back(newTextureNode);

    return mTextureCache.size() - 1;
}

}
