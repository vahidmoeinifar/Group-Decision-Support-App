# Decision engine: plain C++17, no Qt. Can be reused in any C++ program.
add_library(gdcore STATIC
    include/groupdecision/Types.h
    include/groupdecision/GroupProblem.h
    include/groupdecision/Methods.h
    include/groupdecision/Statistics.h
    include/groupdecision/Engine.h
    include/groupdecision/ConsensusFeedback.h
    include/groupdecision/Sensitivity.h
    src/GroupProblem.cpp
    src/Methods.cpp
    src/Statistics.cpp
    src/Engine.cpp
    src/ConsensusFeedback.cpp
    src/Sensitivity.cpp
)

target_include_directories(gdcore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(gdcore PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(gdcore PRIVATE /W4)
else()
    target_compile_options(gdcore PRIVATE -Wall -Wextra -Wpedantic)
endif()
