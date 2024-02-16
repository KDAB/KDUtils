#include <KDUtils/signal.h>
#include <KDFoundation/core_application.h>
#include <thread>

using namespace KDUtils;

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_SUITE("Signal")
{
    TEST_CASE("Multiple Signals with Evaluator")
    {
        int val = 4;

        auto *appInstance = new KDFoundation::CoreApplication;
        KDUtils::Signal<int> signal1;
        KDUtils::Signal<int> signal2;

        std::thread thread1([&] {
            signal1.connect(appInstance->connectionEvaluator(), [&val](int value) {
                val += value;
            });
        });

        std::thread thread2([&] {
            signal2.connect(appInstance->connectionEvaluator(), [&val](int value) {
                val += value;
            });
        });

        thread1.join();
        thread2.join();

        signal1.emit(2);
        signal2.emit(3);
        CHECK(val == 4); // val not changing immediately after emit

        appInstance->connectionEvaluator()->evaluateDeferredConnections();

        CHECK(val == 9);
    }
}
