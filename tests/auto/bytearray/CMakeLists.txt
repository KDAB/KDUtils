add_executable(tst_bytearray tst_bytearray.cpp)
target_link_libraries(tst_bytearray PRIVATE KDUtils doctest::doctest)
doctest_discover_tests(tst_bytearray ADD_LABELS 1 PROPERTIES LABELS "KDUtils")
