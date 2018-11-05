#include "Assimp.h"

/*
=======================================================================================================================================================================================================
Assimp Utility Functions

*/

//--------------------------------------------------------------------------------------
std::shared_ptr<VertexArray> LoadVertexArrayFromScene(const aiScene* scene, GLuint meshNum)
{
    if (meshNum > scene->mNumMeshes)
        GLUF_CRITICAL_EXCEPTION(std::invalid_argument("\"meshNum\" is higher than number of meshes in \"scene\""));

    //const aiMesh* mesh = scene->mMeshes[meshNum];

    std::shared_ptr<VertexArray> arr = LoadVertexArrayFromScene(scene, g_stdAttrib, meshNum);

    /*if (mesh->HasPositions())
        vertexData->AddVertexAttrib(g_attribPOS);
    if (mesh->HasNormals())
        vertexData->AddVertexAttrib(g_attribNORM);
    if (mesh->HasTextureCoords(0))
        vertexData->AddVertexAttrib(g_attribUV0);
    if (mesh->HasTextureCoords(1))
        vertexData->AddVertexAttrib(g_attribUV1);
    if (mesh->HasTextureCoords(2))
        vertexData->AddVertexAttrib(g_attribUV2);
    if (mesh->HasTextureCoords(3))
        vertexData->AddVertexAttrib(g_attribUV3);
    if (mesh->HasTextureCoords(4))
        vertexData->AddVertexAttrib(g_attribUV4);
    if (mesh->HasTextureCoords(5))
        vertexData->AddVertexAttrib(g_attribUV5);
    if (mesh->HasTextureCoords(6))
        vertexData->AddVertexAttrib(g_attribUV6);
    if (mesh->HasTextureCoords(7))
        vertexData->AddVertexAttrib(g_attribUV7);

    if (mesh->HasVertexColors(0))
        vertexData->AddVertexAttrib(g_attribCOLOR0);
    if (mesh->HasVertexColors(1))
        vertexData->AddVertexAttrib(g_attribCOLOR1);
    if (mesh->HasVertexColors(2))
        vertexData->AddVertexAttrib(g_attribCOLOR2);
    if (mesh->HasVertexColors(3))
        vertexData->AddVertexAttrib(g_attribCOLOR3);
    if (mesh->HasVertexColors(4))
        vertexData->AddVertexAttrib(g_attribCOLOR4);
    if (mesh->HasVertexColors(5))
        vertexData->AddVertexAttrib(g_attribCOLOR5);
    if (mesh->HasVertexColors(6))
        vertexData->AddVertexAttrib(g_attribCOLOR6);
    if (mesh->HasVertexColors(7))
        vertexData->AddVertexAttrib(g_attribCOLOR7);
    if (mesh->HasTangentsAndBitangents())
    {
        vertexData->AddVertexAttrib(g_attribTAN);
        vertexData->AddVertexAttrib(g_attribBITAN);
    }


    if (mesh->HasPositions())
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_POSITION, mesh->mNumVertices, mesh->mVertices);
    if (mesh->HasNormals())
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_NORMAL, mesh->mNumVertices, mesh->mNormals);
    if (mesh->HasTextureCoords(0))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_UV0, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[0], mesh->mNumVertices));
    if (mesh->HasTextureCoords(1))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_UV1, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[1], mesh->mNumVertices));
    if (mesh->HasTextureCoords(2))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_UV2, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[2], mesh->mNumVertices));
    if (mesh->HasTextureCoords(3))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_UV3, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[3], mesh->mNumVertices));
    if (mesh->HasTextureCoords(4))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_UV4, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[4], mesh->mNumVertices));
    if (mesh->HasTextureCoords(5))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_UV5, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[5], mesh->mNumVertices));
    if (mesh->HasTextureCoords(6))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_UV6, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[6], mesh->mNumVertices));
    if (mesh->HasTextureCoords(7))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_UV7, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[7], mesh->mNumVertices));

    if (mesh->HasVertexColors(0))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_COLOR0, mesh->mNumVertices, mesh->mColors[0]);
    if (mesh->HasVertexColors(1))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_COLOR1, mesh->mNumVertices, mesh->mColors[1]);
    if (mesh->HasVertexColors(2))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_COLOR2, mesh->mNumVertices, mesh->mColors[2]);
    if (mesh->HasVertexColors(3))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_COLOR3, mesh->mNumVertices, mesh->mColors[3]);
    if (mesh->HasVertexColors(4))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_COLOR4, mesh->mNumVertices, mesh->mColors[4]);
    if (mesh->HasVertexColors(5))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_COLOR5, mesh->mNumVertices, mesh->mColors[5]);
    if (mesh->HasVertexColors(6))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_COLOR6, mesh->mNumVertices, mesh->mColors[6]);
    if (mesh->HasVertexColors(7))
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_COLOR7, mesh->mNumVertices, mesh->mColors[7]);
    if (mesh->HasTangentsAndBitangents())
    {
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_BITAN, mesh->mNumVertices, mesh->mBitangents);
    vertexData->BufferData(GLUF_VERTEX_ATTRIB_TAN, mesh->mNumVertices, mesh->mTangents);
    }

    std::vector<GLuint> indices;
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
    aiFace curr = mesh->mFaces[i];
    indices.push_back(curr.mIndices[0]);
    indices.push_back(curr.mIndices[1]);
    indices.push_back(curr.mIndices[2]);
    }
    vertexData->BufferIndices(&indices[0], indices.size());*/

return arr;
}

/*

AssimpVertexStruct


*/

//--------------------------------------------------------------------------------------
struct AssimpVertexStruct : public VertexStruct
{
    std::vector<aiVector2D> v2;
    std::vector<aiVector3D> v3;
    std::vector<aiColor4D> v4;

    AssimpVertexStruct(size_t vec2Cnt, size_t vec3Cnt, size_t vec4Cnt)
    {
        v2.resize(vec2Cnt);
        v3.resize(vec3Cnt);
        v4.resize(vec4Cnt);
    }

    AssimpVertexStruct(){};

    virtual char* get_data() const override
    {
        char* ret = new char[size()];

        size_t v2Size = 2 * v2.size() * 4;
        size_t v3Size = 3 * v3.size() * 4;
        size_t v4Size = 4 * v4.size() * 4;

        memcpy(ret, v2.data(), v2Size);
        memcpy(ret + v2Size, v3.data(), v3Size);
        memcpy(ret + v2Size + v3Size, v4.data(), v4Size);

        return ret;
    }

    virtual size_t size() const override
    {
        return 4 * (2 * v2.size() + 3 * v3.size() + 4 * v4.size());
    }

    virtual size_t n_elem_size(size_t element)
    {
        size_t i = 0;
        if (element < v2.size())
            return sizeof(aiVector2D);

        i += v2.size();

        if (element < i + v3.size())
            return sizeof(aiVector3D);

        i += v3.size();

        if (element < i + v4.size())
            return sizeof(aiColor4D);

        return 0;//if it is too big, just return 0; not worth an exception
    }

    virtual void buffer_element(void* data, size_t element) override
    {
        size_t i = 0;
        if (element < v2.size())
        {
            v2[element] = static_cast<aiVector2D*>(data)[0];
            return;
        }

        i += v2.size();

        if (element < i + v3.size())
        {
            v3[element - i] = static_cast<aiVector3D*>(data)[0];
            return;
        }

        i += v3.size();

        if (element < i + v4.size())
        {
            v4[element - i] = static_cast<aiColor4D*>(data)[0];
            return;
        }
    }

    static GLVector<AssimpVertexStruct> MakeMany(size_t howMany, size_t vec2Cnt, size_t vec3Cnt, size_t vec4Cnt)
    {
        GLVector<AssimpVertexStruct> ret;
        ret.reserve(howMany);

        //the individual struct object that will be cloned to fill the return vector
        AssimpVertexStruct clone{ vec2Cnt, vec3Cnt, vec4Cnt };
        
        //use 'fill' version of vector::insert for increased speed
        ret.insert(ret.begin(), howMany, clone);

        return ret;
    }
};


//--------------------------------------------------------------------------------------
std::shared_ptr<VertexArray> LoadVertexArrayFromScene(const aiScene* scene, const VertexAttribMap& inputs, GLuint meshNum)
{
    if (meshNum > scene->mNumMeshes)
        GLUF_CRITICAL_EXCEPTION(std::invalid_argument("\"meshNum\" is higher than the number of meshes in \"scene\""));

    const aiMesh* mesh = scene->mMeshes[meshNum];

    auto vertexData = std::make_shared<VertexArray>(GL_TRIANGLES, GL_STATIC_DRAW, mesh->HasFaces());

    //which vertex attributes go where in assimp loading vertex struct
    std::map<AttribLoc, int> vertexAttribLoc;

    //the number of each vec type
    unsigned char numVec2 = 0;
    unsigned char numVec3 = 0;
    unsigned char numVec4 = 0;

    auto itUV0 = inputs.find(GLUF_VERTEX_ATTRIB_UV0);
    auto it = itUV0;
    if (mesh->HasTextureCoords(0) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec2 * 8);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV0] = numVec2;
        ++numVec2;
    }

    auto itUV1 = inputs.find(GLUF_VERTEX_ATTRIB_UV1);
    it = itUV1;
    if (mesh->HasTextureCoords(1) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec2 * 8);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV1] = numVec2;
        ++numVec2;
    }

    auto itUV2 = inputs.find(GLUF_VERTEX_ATTRIB_UV2);
    it = itUV2;
    if (mesh->HasTextureCoords(2) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec2 * 8);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV2] = numVec2;
        ++numVec2;
    }

    auto itUV3 = inputs.find(GLUF_VERTEX_ATTRIB_UV3);
    it = itUV3;
    if (mesh->HasTextureCoords(3) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec2 * 8);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV3] = numVec2;
        ++numVec2;
    }

    auto itUV4 = inputs.find(GLUF_VERTEX_ATTRIB_UV4);
    it = itUV4;
    if (mesh->HasTextureCoords(4) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec2 * 8);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV4] = numVec2;
        ++numVec2;
    }

    auto itUV5 = inputs.find(GLUF_VERTEX_ATTRIB_UV5);
    it = itUV5;
    if (mesh->HasTextureCoords(5) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec2 * 8);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV5] = numVec2;
        ++numVec2;
    }

    auto itUV6 = inputs.find(GLUF_VERTEX_ATTRIB_UV6);
    it = itUV6;
    if (mesh->HasTextureCoords(6) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec2 * 8);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV6] = numVec2;
        ++numVec2;
    }

    auto itUV7 = inputs.find(GLUF_VERTEX_ATTRIB_UV7);
    it = itUV7;
    if (mesh->HasTextureCoords(7) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec2 * 8);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV7] = numVec2;
        ++numVec2;
    }


    unsigned int runningOffsetTotal = numVec2 * 8;


    //vec3's go between vec2's and vec4's
    auto itPos = inputs.find(GLUF_VERTEX_ATTRIB_POSITION);
    it = itPos;
    if (mesh->HasPositions() && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec3 * 12 + runningOffsetTotal);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_POSITION] = numVec3 + numVec2;
        ++numVec3;
    }

    auto itNorm = inputs.find(GLUF_VERTEX_ATTRIB_NORMAL);
    it = itNorm;
    if (mesh->HasNormals() && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec3 * 12 + runningOffsetTotal);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_NORMAL] = numVec3 + numVec2;
        ++numVec3;
    }


    auto itTan = inputs.find(GLUF_VERTEX_ATTRIB_TAN);
    auto itBitan = inputs.find(GLUF_VERTEX_ATTRIB_BITAN);
    if (mesh->HasTangentsAndBitangents() && itTan != inputs.end() && itBitan != inputs.end())
    {
        vertexData->AddVertexAttrib(itTan->second, numVec3 * 12 + runningOffsetTotal);
        vertexData->AddVertexAttrib(itBitan->second, (numVec3 + 1) * 12 + runningOffsetTotal);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_TAN] = numVec3 + numVec2;
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_BITAN] = numVec3 + 1 + numVec2;
        numVec3 += 2;
    }

    //to keep from doing unncessessary addition
    unsigned char runningTotal = numVec3 + numVec2;

    runningOffsetTotal += numVec3 * 12;

    auto itCol0 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR0);
    it = itCol0;
    if (mesh->HasVertexColors(0) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec4 * 16 + runningOffsetTotal);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR0] = numVec4 + runningTotal;
        ++numVec4;
    }

    auto itCol1 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR1);
    it = itCol1;
    if (mesh->HasVertexColors(1) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec4 * 16 + runningOffsetTotal);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR1] = numVec4 + runningTotal;
        ++numVec4;
    }

    auto itCol2 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR2);
    it = itCol2;
    if (mesh->HasVertexColors(2) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec4 * 16 + runningOffsetTotal);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR2] = numVec4 + runningTotal;
        ++numVec4;
    }

    auto itCol3 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR3);
    it = itCol3;
    if (mesh->HasVertexColors(3) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec4 * 16 + runningOffsetTotal);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR3] = numVec4 + runningTotal;
        ++numVec4;
    }

    auto itCol4 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR4);
    it = itCol4;
    if (mesh->HasVertexColors(4) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec4 * 16 + runningOffsetTotal);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR4] = numVec4 + runningTotal;
        ++numVec4;
    }
    
    auto itCol5 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR5);
    it = itCol5;
    if (mesh->HasVertexColors(5) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec4 * 16 + runningOffsetTotal);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR5] = numVec4 + runningTotal;
        ++numVec4;
    }

    auto itCol6 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR6);
    it = itCol6;
    if (mesh->HasVertexColors(6) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec4 * 16 + runningOffsetTotal);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR6] = numVec4 + runningTotal;
        ++numVec4;
    }

    auto itCol7 = inputs.find(GLUF_VERTEX_ATTRIB_COLOR7);
    it = itCol7;
    if (mesh->HasVertexColors(7) && it != inputs.end())
    {
        vertexData->AddVertexAttrib(it->second, numVec4 * 16 + runningOffsetTotal);
        vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR7] = numVec4 + runningTotal;
        ++numVec4;
    }



    //the custom vertex array
    GLVector<AssimpVertexStruct> vertexBuffer = AssimpVertexStruct::MakeMany(mesh->mNumVertices, numVec2, numVec3, numVec4);

    if (mesh->HasPositions() && itPos != inputs.end())
    {
        //positions will ALWAYS be the first vec3
        vertexBuffer.buffer_element(mesh->mVertices, vertexAttribLoc[GLUF_VERTEX_ATTRIB_POSITION]);
    }
    if (mesh->HasNormals() && itNorm != inputs.end())
    {
        vertexBuffer.buffer_element(mesh->mNormals, vertexAttribLoc[GLUF_VERTEX_ATTRIB_NORMAL]);
    }



    /*

    Instead of flipping the pixels when loading textures, UV's will be flipped instead

    Also, since we are using 2 dimmentional texture coordinates, make sure to convert to 2 element vectors

    */

    //note: data2D must be size in length
    const auto FlipAndConvertUVArray = [](aiVector3D* data, aiVector2D*& data2D, const unsigned int size)
    {
        for (unsigned int i = 0; i < size; ++i)
        {
            data2D[i].x = data[i].x;
            data2D[i].y = 1.0f - data[i].y;
        }
    };

    aiVector2D* data2D = nullptr;
    if (mesh->HasTextureCoords(0) && itUV0 != inputs.end())
    {
        //do not initialize data2D until here, because if there is not a first texture coord, then there wil be no others
        data2D = new aiVector2D[mesh->mNumVertices];

        FlipAndConvertUVArray(mesh->mTextureCoords[0], data2D, mesh->mNumVertices);
        vertexBuffer.buffer_element(data2D, vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV0]);
    }
    if (mesh->HasTextureCoords(1) && itUV1 != inputs.end())
    {
        FlipAndConvertUVArray(mesh->mTextureCoords[1], data2D, mesh->mNumVertices);
        vertexBuffer.buffer_element(data2D, vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV1]);
    }
    if (mesh->HasTextureCoords(2) && itUV2 != inputs.end())
    {
        FlipAndConvertUVArray(mesh->mTextureCoords[2], data2D, mesh->mNumVertices);
        vertexBuffer.buffer_element(data2D, vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV2]);
    }
    if (mesh->HasTextureCoords(3) && itUV3 != inputs.end())
    {
        FlipAndConvertUVArray(mesh->mTextureCoords[3], data2D, mesh->mNumVertices);
        vertexBuffer.buffer_element(data2D, vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV3]);
    }
    if (mesh->HasTextureCoords(4) && itUV4 != inputs.end())
    {
        FlipAndConvertUVArray(mesh->mTextureCoords[4], data2D, mesh->mNumVertices);
        vertexBuffer.buffer_element(data2D, vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV4]);
    }
    if (mesh->HasTextureCoords(5) && itUV5 != inputs.end())
    {
        FlipAndConvertUVArray(mesh->mTextureCoords[5], data2D, mesh->mNumVertices);
        vertexBuffer.buffer_element(data2D, vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV5]);
    }
    if (mesh->HasTextureCoords(6) && itUV6 != inputs.end())
    {
        FlipAndConvertUVArray(mesh->mTextureCoords[6], data2D, mesh->mNumVertices);
        vertexBuffer.buffer_element(data2D, vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV6]);
    }
    if (mesh->HasTextureCoords(7) && itUV7 != inputs.end())
    {
        FlipAndConvertUVArray(mesh->mTextureCoords[7], data2D, mesh->mNumVertices);
        vertexBuffer.buffer_element(data2D, vertexAttribLoc[GLUF_VERTEX_ATTRIB_UV7]);
    }
    delete[] data2D;


    if (mesh->HasVertexColors(0) && itCol0 != inputs.end())
    {
        vertexBuffer.buffer_element(mesh->mColors[0], vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR0]);
    }
    if (mesh->HasVertexColors(1) && itCol1 != inputs.end())
    {
        vertexBuffer.buffer_element(mesh->mColors[1], vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR1]);
    }
    if (mesh->HasVertexColors(2) && itCol2 != inputs.end())
    {
        vertexBuffer.buffer_element(mesh->mColors[2], vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR2]);
    }
    if (mesh->HasVertexColors(3) && itCol3 != inputs.end())
    {
        vertexBuffer.buffer_element(mesh->mColors[3], vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR3]);
    }
    if (mesh->HasVertexColors(4) && itCol4 != inputs.end())
    {
        vertexBuffer.buffer_element(mesh->mColors[4], vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR4]);
    }
    if (mesh->HasVertexColors(5) && itCol5 != inputs.end())
    {
        vertexBuffer.buffer_element(mesh->mColors[5], vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR5]);
    }
    if (mesh->HasVertexColors(6) && itCol6 != inputs.end())
    {
        vertexBuffer.buffer_element(mesh->mColors[6], vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR6]);
    }
    if (mesh->HasVertexColors(7) && itCol7 != inputs.end())
    {
        vertexBuffer.buffer_element(mesh->mColors[7], vertexAttribLoc[GLUF_VERTEX_ATTRIB_COLOR7]);
    }
    if (mesh->HasTangentsAndBitangents() && itBitan != inputs.end() && itTan != inputs.end())
    {
        vertexBuffer.buffer_element(mesh->mTangents, vertexAttribLoc[GLUF_VERTEX_ATTRIB_TAN]);
        vertexBuffer.buffer_element(mesh->mBitangents, vertexAttribLoc[GLUF_VERTEX_ATTRIB_BITAN]);
    }

    //don't forget to buffer the actual data :) (i actually forgot this part at first)
    vertexData->BufferData(vertexBuffer);


    std::vector<glm::u32vec3> indices;
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace curr = mesh->mFaces[i];
        indices.push_back({ curr.mIndices[0], curr.mIndices[1], curr.mIndices[2] });
    }
    vertexData->BufferIndices(indices);

    return vertexData;
}


//--------------------------------------------------------------------------------------
std::vector<std::shared_ptr<VertexArray>> LoadVertexArraysFromScene(const aiScene* scene, GLuint meshOffset, GLuint numMeshes)
{
    std::vector<std::shared_ptr<VertexArray>> arrays;

    if (numMeshes > scene->mNumMeshes)
    {
        arrays.push_back(nullptr);
        return arrays;
    }

    for(unsigned int cnt = 0; cnt < numMeshes; ++cnt)
    {
        arrays.push_back(LoadVertexArrayFromScene(scene, cnt));//new VertexArray(GL_TRIANGLES, GL_STATIC_DRAW, mesh->HasFaces());
    }
    return arrays;
}

/*
VertexArray* LoadVertexArrayFromFile(std::string path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);

    const aiMesh* mesh = scene->mMeshes[0];

    VertexArray vertexData(GL_TRIANGLES, GL_STATIC_DRAW, mesh->HasFaces());

    if (mesh->HasPositions())
        vertexData.AddVertexAttrib(g_attribPOS);
    if (mesh->HasNormals())
        vertexData.AddVertexAttrib(g_attribNORM);
    if (mesh->HasTextureCoords(0))
        vertexData.AddVertexAttrib(g_attribUV);
    if (mesh->HasTangentsAndBitangents())
    {
        vertexData.AddVertexAttrib(g_attribTAN);
        vertexData.AddVertexAttrib(g_attribBITAN);
    }
    if (mesh->HasVertexColors(0))
        vertexData.AddVertexAttrib(g_attribCOLOR);


    if (mesh->HasPositions())
        vertexData.BufferData(VERTEX_ATTRIB_POSITION, mesh->mNumVertices, mesh->mVertices);
    if (mesh->HasNormals())
        vertexData.BufferData(VERTEX_ATTRIB_NORMAL, mesh->mNumVertices, mesh->mNormals);
    if (mesh->HasTextureCoords(0))
        vertexData.BufferData(VERTEX_ATTRIB_UV, mesh->mNumVertices, AssimpToGlm3_2(mesh->mTextureCoords[0], mesh->mNumVertices));
    if (mesh->HasTangentsAndBitangents())
    {
        vertexData.BufferData(VERTEX_ATTRIB_BITAN, mesh->mNumVertices, mesh->mBitangents);
        vertexData.BufferData(VERTEX_ATTRIB_TAN, mesh->mNumVertices, mesh->mTangents);
    }
    if (mesh->HasVertexColors(0))
        vertexData.BufferData(VERTEX_ATTRIB_COLOR, mesh->mNumVertices, mesh->mColors[0]);

    std::vector<GLuint> indices;
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace curr = mesh->mFaces[i];
        indices.push_back(curr.mIndices[0]);
        indices.push_back(curr.mIndices[1]);
        indices.push_back(curr.mIndices[2]);
    }
    vertexData.BufferIndices(&indices[0], indices.size());

    return vertexData;
    return LoadVertexArray(scene);
}

VertexArray *LoadVertexArrayFromFile(unsigned int size, void* data)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFileFromMemory(data, size,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);

    return LoadVertexArray(scene);
}*/
