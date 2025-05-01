include_directories(.)
include_directories(${CMAKE_SOURCE_DIR}/include)

set_property(DIRECTORY APPEND PROPERTY
    COMPILE_DEFINITIONS $<$<CONFIG:Debug>:_DEBUG> $<$<NOT:$<CONFIG:Debug>>:NDEBUG>)

if (MSVC)
    set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE STRING "" FORCE)

    add_definitions(-DNOMINMAX)

    add_definitions(-DWIN32_LEAN_AND_MEAN)
else()
    if (APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL Clang)
        add_compile_options("-stdlib=libc++")
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR MINGW)
        add_definitions(-D_FILE_OFFSET_BITS=64)
    endif()
      # Add -O3 optimization flag for non-MSVC compilers
      add_compile_options(-O3)
        # Add -march=native and -mavx flags for non-MSVC compilers
      add_compile_options(-march=native -mavx)
      # Add -fPIC flag for position-independent code
      add_compile_options(-fPIC)
endif()

add_subdirectory(numpy)

add_library(numpy_random STATIC
    "numpy_random.cpp"
    "pcg64/pcg64.c"
    "pcg64/pcg64.orig.c"
    "mt19937/mt19937.c"
    "mt19937/mt19937-jump.c"
)


# Combine both static libraries
add_library(numpyrandom SHARED $<TARGET_OBJECTS:numpy> $<TARGET_OBJECTS:numpy_random>)
target_link_libraries(numpyrandom PUBLIC numpy numpy_random)

create_target_directory_groups(numpy_random)