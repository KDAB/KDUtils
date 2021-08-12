add_executable(tst_flags tst_flags.cpp)
target_link_libraries(tst_flags PRIVATE KDCore doctest::doctest)
doctest_discover_tests(tst_flags ADD_LABELS 1 PROPERTIES LABELS "KDCore")
