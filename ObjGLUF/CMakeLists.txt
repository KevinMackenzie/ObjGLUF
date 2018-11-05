cmake_minimum_required(VERSION 2.6)
project(objgluf)

set(objgluf_SOURCES ObjGLUF.cpp)
set(objgluf_HEADERS ObjGLUF.h ObjGLUFTemplates.inl Shader/Shader.h Shader/Shader.cpp Exports.h Texture/Texture.h VertexBuffer/VertexBuffer.h Ext/Assimp.h)

add_definitions(-DOBJGLUF_EXPORTS)

add_library(objgluf ${objgluf_SOURCES} ${objgluf_HEADERS})

# TODO: better support optional use of Assimp (b/c not everyone wants it)
target_link_libraries(objgluf glfw ${GLFW_LIBRARIES} freetype assimp glad)