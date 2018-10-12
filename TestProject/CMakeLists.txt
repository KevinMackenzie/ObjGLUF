cmake_minimum_required (VERSION 2.6)
project (TestProject)

add_executable (TestProject TestProject.cpp)
target_link_libraries(TestProject glfw ${GLFW_LIBRARIES} assimp glad objglf)
