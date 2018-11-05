//
// Created by donald on 11/4/18.
//

#ifndef OBJGLUF_SPRITEVERTEXSTRUCT_H
#define OBJGLUF_SPRITEVERTEXSTRUCT_H

/*
SpriteVertexStruct

    Note:
        This is derived from the vertex struct class for use on AoS data packing

    Data Members:
        'mPos': a position
        'mColor': a color
        'mTexCoords': a uv coord

*/
struct SpriteVertexStruct : public VertexStruct
{
    glm::vec3 mPos;
    Color4f mColor;
    glm::vec2 mTexCoords;

    SpriteVertexStruct(){}
    SpriteVertexStruct(const glm::vec3& pos, const Color4f& color, const glm::vec2& texCoords) :
            mPos(pos), mColor(color), mTexCoords(texCoords)
    {}

    virtual char* get_data() const override
    {
        char* ret = new char[size()];

        memcpy(ret, &mPos[0], 12);
        memcpy(ret + 12, &mColor[0], 16);
        memcpy(ret + 28, &mTexCoords[0], 8);

        return ret;
    }

    virtual size_t size() const override
    {
        return 36; // sizeof(mPos) + sizeof(mColor) + sizeof(mTexCoords);
    }

    virtual size_t n_elem_size(size_t element)
    {
        switch (element)
        {
            case 0:
                return 12;
            case 1:
                return 16;
            case 2:
                return 8;
            default:
                return 0;//if it is too big, just return 0; not worth an exception
        }
    }

    virtual void buffer_element(void* data, size_t element) override
    {
        switch (element)
        {
            case 0:
                mPos = static_cast<glm::vec3*>(data)[0];
            case 1:
                mColor = static_cast<Color4f*>(data)[0];
            case 2:
                mTexCoords = static_cast<glm::vec2*>(data)[0];
            default:
                break;
        }
    }

    static GLVector<SpriteVertexStruct> MakeMany(size_t howMany)
    {
        GLVector<SpriteVertexStruct> ret;
        ret.resize(howMany);

        return ret;
    }
};
#endif //OBJGLUF_SPRITEVERTEXSTRUCT_H
