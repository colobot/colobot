set(CONVERT_MODEL_SOURCES
  ../common/logger.cpp
  ../graphics/model/model.cpp
  ../graphics/model/model_mesh.cpp
  ../graphics/model/model_input.cpp
  ../graphics/model/model_output.cpp
  convert_model.cpp
)

include_directories(. ..)

include_directories(SYSTEM ${SDL_INCLUDE_DIR})

add_executable(convert_model ${CONVERT_MODEL_SOURCES})

