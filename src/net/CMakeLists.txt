add_library(net STATIC
    networking.cpp
)

target_include_directories(net PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Placeholder: no dependencies yet
