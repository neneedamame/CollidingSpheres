cmake_minimum_required(VERSION 3.28)
project(CollidingSpheres)

find_package(glm CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(CGAL CONFIG REQUIRED)

add_library(Geometry STATIC)
target_compile_features(Geometry PRIVATE cxx_std_20)
file(GLOB GeometryModuleFiles "Geometry/*.ixx")
#file(GLOB GeometryModuleImpl "Geometry/*.cpp")
target_sources(Geometry PUBLIC FILE_SET CXX_MODULES FILES ${GeometryModuleFiles})
target_sources(Geometry PUBLIC ${GeometryModuleImpl})
target_link_libraries(Geometry PUBLIC glm::glm)
target_link_libraries(Geometry PUBLIC glad::glad)
target_link_libraries(Geometry PUBLIC glfw)
target_link_libraries(Geometry PUBLIC CGAL::CGAL)

add_executable(CollidingSpheres)
target_sources(CollidingSpheres PRIVATE "main.cpp")
target_compile_features(CollidingSpheres PRIVATE cxx_std_20)
target_link_libraries(CollidingSpheres PUBLIC Geometry)

