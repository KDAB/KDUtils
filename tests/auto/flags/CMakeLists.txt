add_executable(tst_flags tst_flags.cpp)
target_link_libraries(tst_flags PRIVATE KDUtils doctest::doctest)
doctest_discover_tests(tst_flags ADD_LABELS 1 PROPERTIES LABELS "KDUtils")
