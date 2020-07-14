add_executable(convert_model
  ../common/logger.cpp
  ../graphics/model/model.cpp
  ../graphics/model/model_mesh.cpp
  ../graphics/model/model_input.cpp
  ../graphics/model/model_output.cpp
  convert_model.cpp)
target_link_libraries(convert_model PRIVATE Boost::headers)
target_include_directories(convert_model PRIVATE . ..)
