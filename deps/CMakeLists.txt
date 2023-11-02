add_subdirectory(openal-soft)

set(DRWAV_LIB dr_libs)
set(DRWAV_SOURCES
  dr_libs/dr_wav.h
  dr_libs/dr_mp3.h
  dr_libs/dr_flac.h
)

add_library(${DRWAV_LIB}
  INTERFACE
    ${DRWAV_SOURCES}
)

target_include_directories(${DRWAV_LIB}
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/dr_libs
)
