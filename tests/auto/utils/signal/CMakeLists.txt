

add_executable(
    tst_signal
    tst_signal.cpp
)
target_link_libraries(
    tst_signal
    PRIVATE KDUtils KDUtils::KDFoundation doctest::doctest
)
doctest_discover_tests(
    tst_signal
    ADD_LABELS
    1
    PROPERTIES
    LABELS
    "KDUtils"
)
