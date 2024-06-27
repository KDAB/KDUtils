#include <KDFoundation/core_application.h>

#include <kdbindings/signal.h>

#include <thread>

using namespace KDUtils;

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_SUITE("Signal")
{
    TEST_CASE("Multiple Signals with Evaluator")
    {
        int val = 4;

        KDFoundation::CoreApplication app;
        KDBindings::Signal<int> signal1;
        KDBindings::Signal<int> signal2;

        std::thread thread1([&] {
            signal1.connectDeferred(app.connectionEvaluator(), [&val](int value) {
                val += value;
            });
        });

        std::thread thread2([&] {
            signal2.connectDeferred(app.connectionEvaluator(), [&val](int value) {
                val += value;
            });
        });

        thread1.join();
        thread2.join();

        signal1.emit(2);
        signal2.emit(3);
        CHECK(val == 4); // val not changing immediately after emit

        app.connectionEvaluator()->evaluateDeferredConnections();

        CHECK(val == 9);
    }

    TEST_CASE("Emit Multiple Signals with Evaluator")
    {
        KDFoundation::CoreApplication app;
        KDBindings::Signal<int> signal1;
        KDBindings::Signal<int> signal2;

        int val1 = 4;
        int val2 = 4;

        signal1.connectDeferred(app.connectionEvaluator(), [&val1](int value) {
            val1 += value;
        });

        signal2.connectDeferred(app.connectionEvaluator(), [&val2](int value) {
            val2 += value;
        });

        std::thread thread1([&] {
            signal1.emit(2);
        });

        std::thread thread2([&] {
            signal2.emit(3);
        });

        thread1.join();
        thread2.join();

        CHECK(val1 == 4);
        CHECK(val2 == 4);

        app.connectionEvaluator()->evaluateDeferredConnections();

        CHECK(val1 == 6);
        CHECK(val2 == 7);
    }

    TEST_CASE("Connect, Emit, Disconnect, and Evaluate")
    {
        KDFoundation::CoreApplication app;
        KDBindings::Signal<int> signal;
        int val = 4;

        auto connection = signal.connectDeferred(app.connectionEvaluator(), [&val](int value) {
            val += value;
        });

        CHECK(connection.isActive());

        signal.emit(2);
        CHECK(val == 4);

        connection.disconnect();

        app.connectionEvaluator()->evaluateDeferredConnections();

        CHECK(val == 4);
    }
}
