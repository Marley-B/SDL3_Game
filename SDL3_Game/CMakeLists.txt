# CMakeList.txt : CMake project for SDL3_Game, include source and define
# project specific logic here.
#
find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(glm REQUIRED)
find_package(SDL3_mixer REQUIRED)
find_package(SDL3_ttf REQUIRED)

# Add source to this project's executable.
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR NOT CMAKE_BUILD_TYPE)
  add_executable(SDL3_Game WIN32 "SDL3_Game.cpp" "SDL3_Game.h" "timer.h" "tmx.cpp" "tmx.h" "animation.h" "ext/tinyxml2/tinyxml2.cpp" 
      "object_state.cpp" "game_status.cpp" "game_status.h" "resources.cpp" "level_state.cpp" "game_state.cpp" "ui.cpp")
else()
  add_executable(SDL3_Game "SDL3_Game.cpp" "SDL3_Game.h" "timer.h" "tmx.cpp" "tmx.h" "animation.h" "ext/tinyxml2/tinyxml2.cpp" 
      "object_state.cpp" "game_status.cpp" "game_status.h" "resources.cpp" "level_state.cpp" "game_state.cpp" "ui.cpp")
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SDL3_Game PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
target_link_libraries(SDL3_Game PRIVATE SDL3::SDL3 SDL3_image::SDL3_image glm::glm SDL3_mixer::SDL3_mixer SDL3_ttf::SDL3_ttf)
target_include_directories(SDL3_Game PRIVATE "ext/")