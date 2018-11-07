cmake_minimum_required(VERSION 2.6)
project(objgluf)

set(objgluf_SOURCES
        ObjGLUF.cpp
        Ext/Assimp.cpp
        Shader/Shader.cpp
        Texture/Texture.cpp
        VertexBuffer/VertexBuffer.cpp
        )
set(objgluf_HEADERS
        ObjGLUF.h
        Exports.h
        ObjGLUFTemplates.inl
        Ext/Assimp.h
        Shader/Shader.h
        Texture/Texture.h
        VertexBuffer/VertexBuffer.h
        VertexBuffer/GLVector.h
        VertexBuffer/GLVector.inl
        VertexBuffer/VertexBuffer.inl
        GLExtensions.h
        Exceptions.h
        GLExtensions.cpp Types.h Types.cpp)

add_definitions(-DOBJGLUF_EXPORTS)

add_library(objgluf ${objgluf_SOURCES} ${objgluf_HEADERS})

# TODO: better support optional use of Assimp (b/c not everyone wants it)
target_link_libraries(objgluf glfw ${GLFW_LIBRARIES} freetype assimp glad)