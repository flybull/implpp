add_executable(my_project main.cpp 1.cpp 2.cpp)

include_directories(${CMAKE_SOURCE_DIR}/../)
# 设置编译选项
set_target_properties(my_project PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED True
)
