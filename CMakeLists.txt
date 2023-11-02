cmake_minimum_required(VERSION 3.20)
project(SoundTest VERSION 0.0.1 LANGUAGES CXX)

add_subdirectory(deps)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    OpenAL
    dr_libs
)

target_compile_features(${PROJECT_NAME} 
  PUBLIC 
    cxx_std_11
)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)  

# If we are building in windows we should copy required dll's to our binary dir
if (WIN32)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_BINARY_DIR}/deps/openal-soft/OpenAL32.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>
  )
endif()
