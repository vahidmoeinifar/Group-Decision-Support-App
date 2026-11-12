# Run all tests with:  ctest --test-dir build --output-on-failure

qt_add_executable(tst_engine tst_engine.cpp)
target_link_libraries(tst_engine PRIVATE gdcore Qt6::Test)
add_test(NAME engine COMMAND tst_engine)

qt_add_executable(tst_service tst_service.cpp)
target_link_libraries(tst_service PRIVATE gdbackend Qt6::Test)
add_test(NAME service COMMAND tst_service)
