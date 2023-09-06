/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/platform/linux/linux_platform_event_loop.h>
#include <KDFoundation/file_descriptor_notifier.h>
#include <KDFoundation/object.h>
#include <KDFoundation/postman.h>

#include <KDFoundation/logging.h>

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <numeric>
#include <string>
#include <thread>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDFoundation;

static_assert(std::is_destructible<LinuxPlatformEventLoop>{});
static_assert(std::is_default_constructible<LinuxPlatformEventLoop>{});
static_assert(!std::is_copy_constructible<LinuxPlatformEventLoop>{});
static_assert(!std::is_copy_assignable<LinuxPlatformEventLoop>{});
static_assert(!std::is_move_constructible<LinuxPlatformEventLoop>{});
static_assert(!std::is_move_assignable<LinuxPlatformEventLoop>{});

TEST_CASE("Register and unregister for events")
{
    spdlog::set_level(spdlog::level::debug);

    SUBCASE("can create a linux event loop")
    {
        LinuxPlatformEventLoop loop;
        REQUIRE(loop.epollHandle() != -1);
    }

    SUBCASE("can register a file descriptor for read notifications")
    {
        LinuxPlatformEventLoop loop;
        bool result = loop.registerFileDescriptor(0, FileDescriptorNotifier::NotificationType::Read);

        REQUIRE(result == true);
    }

    SUBCASE("can unregister a file descriptor for read notifications")
    {
        LinuxPlatformEventLoop loop;
        loop.registerFileDescriptor(0, FileDescriptorNotifier::NotificationType::Read);
        bool result = loop.unregisterFileDescriptor(0, FileDescriptorNotifier::NotificationType::Read);

        REQUIRE(result == true);
    }

    SUBCASE("can re-register a file descriptor for read notifications")
    {
        LinuxPlatformEventLoop loop;
        loop.registerFileDescriptor(0, FileDescriptorNotifier::NotificationType::Read);
        loop.unregisterFileDescriptor(0, FileDescriptorNotifier::NotificationType::Read);
        bool result = loop.registerFileDescriptor(0, FileDescriptorNotifier::NotificationType::Read);

        REQUIRE(result == true);
    }

    SUBCASE("can register a file descriptor for write notifications")
    {
        LinuxPlatformEventLoop loop;
        bool result = loop.registerFileDescriptor(0, FileDescriptorNotifier::NotificationType::Write);

        REQUIRE(result == true);
    }

    SUBCASE("can unregister a file descriptor for write notifications")
    {
        LinuxPlatformEventLoop loop;
        loop.registerFileDescriptor(0, FileDescriptorNotifier::NotificationType::Write);
        bool result = loop.unregisterFileDescriptor(0, FileDescriptorNotifier::NotificationType::Write);

        REQUIRE(result == true);
    }

    SUBCASE("can re-register a file descriptor for write notifications")
    {
        LinuxPlatformEventLoop loop;
        loop.registerFileDescriptor(0, FileDescriptorNotifier::NotificationType::Write);
        loop.unregisterFileDescriptor(0, FileDescriptorNotifier::NotificationType::Write);
        bool result = loop.registerFileDescriptor(0, FileDescriptorNotifier::NotificationType::Write);

        REQUIRE(result == true);
    }

    SUBCASE("can register a file descriptor for exception notifications")
    {
        LinuxPlatformEventLoop loop;
        bool result = loop.registerFileDescriptor(0, FileDescriptorNotifier::NotificationType::Exception);

        REQUIRE(result == true);
    }

    SUBCASE("can unregister a file descriptor for exception notifications")
    {
        LinuxPlatformEventLoop loop;
        loop.registerFileDescriptor(0, FileDescriptorNotifier::NotificationType::Exception);
        bool result = loop.unregisterFileDescriptor(0, FileDescriptorNotifier::NotificationType::Exception);

        REQUIRE(result == true);
    }

    SUBCASE("can re-register a file descriptor for exception notifications")
    {
        LinuxPlatformEventLoop loop;
        loop.registerFileDescriptor(0, FileDescriptorNotifier::NotificationType::Exception);
        loop.unregisterFileDescriptor(0, FileDescriptorNotifier::NotificationType::Exception);
        bool result = loop.registerFileDescriptor(0, FileDescriptorNotifier::NotificationType::Exception);

        REQUIRE(result == true);
    }

    SUBCASE("can register a notifier for read notifications")
    {
        LinuxPlatformEventLoop loop;
        auto notifier = std::make_unique<FileDescriptorNotifier>(0, FileDescriptorNotifier::NotificationType::Read);
        bool result = loop.registerNotifier(notifier.get());

        REQUIRE(result == true);
        REQUIRE(loop.registeredFileDescriptorCount() == 1);
    }

    SUBCASE("can unregister a notifier")
    {
        LinuxPlatformEventLoop loop;
        auto notifier = std::make_unique<FileDescriptorNotifier>(0, FileDescriptorNotifier::NotificationType::Read);
        loop.registerNotifier(notifier.get());
        bool result = loop.unregisterNotifier(notifier.get());

        REQUIRE(result == true);
        REQUIRE(loop.registeredFileDescriptorCount() == 0);
    }

    SUBCASE("can register multiple notifiers for read")
    {
        LinuxPlatformEventLoop loop;
        int pipe1[2];
        int pipe2[2];
        int pipe3[2];
        if (pipe(pipe1) == -1) {
            spdlog::debug("Failed to create pipe. errno = {}", errno);
            REQUIRE(false);
        }
        if (pipe(pipe2) == -1) {
            spdlog::debug("Failed to create pipe. errno = {}", errno);
            REQUIRE(false);
        }
        if (pipe(pipe3) == -1) {
            spdlog::debug("Failed to create pipe. errno = {}", errno);
            REQUIRE(false);
        }

        auto notifier1 = std::make_unique<FileDescriptorNotifier>(pipe1[0], FileDescriptorNotifier::NotificationType::Read);
        auto notifier2 = std::make_unique<FileDescriptorNotifier>(pipe2[0], FileDescriptorNotifier::NotificationType::Read);
        auto notifier3 = std::make_unique<FileDescriptorNotifier>(pipe3[0], FileDescriptorNotifier::NotificationType::Read);

        bool result = loop.registerNotifier(notifier1.get());
        result &= loop.registerNotifier(notifier2.get());
        result &= loop.registerNotifier(notifier3.get());

        REQUIRE(result == true);
        REQUIRE(loop.registeredFileDescriptorCount() == 3);

        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);
        close(pipe3[0]);
        close(pipe3[1]);
    }

    SUBCASE("can register same fd for multiple events")
    {
        LinuxPlatformEventLoop loop;
        int pipe1[2];
        if (pipe(pipe1) == -1) {
            spdlog::debug("Failed to create pipe. errno = {}", errno);
            REQUIRE(false);
        }

        auto notifier1 = std::make_unique<FileDescriptorNotifier>(pipe1[0], FileDescriptorNotifier::NotificationType::Read);
        auto notifier2 = std::make_unique<FileDescriptorNotifier>(pipe1[0], FileDescriptorNotifier::NotificationType::Write);

        bool result = loop.registerNotifier(notifier1.get());
        result &= loop.registerNotifier(notifier2.get());

        REQUIRE(result == true);
        REQUIRE(loop.registeredFileDescriptorCount() == 1);

        close(pipe1[0]);
        close(pipe1[1]);
    }

    SUBCASE("can register same fd for multiple events then unregister")
    {
        LinuxPlatformEventLoop loop;
        int pipe1[2];
        if (pipe(pipe1) == -1) {
            spdlog::debug("Failed to create pipe. errno = {}", errno);
            REQUIRE(false);
        }

        auto notifier1 = std::make_unique<FileDescriptorNotifier>(pipe1[0], FileDescriptorNotifier::NotificationType::Read);
        auto notifier2 = std::make_unique<FileDescriptorNotifier>(pipe1[0], FileDescriptorNotifier::NotificationType::Write);

        loop.registerNotifier(notifier1.get());
        loop.registerNotifier(notifier2.get());

        bool result = loop.unregisterNotifier(notifier2.get());
        REQUIRE(result == true);
        REQUIRE(loop.registeredFileDescriptorCount() == 1);

        result = loop.unregisterNotifier(notifier1.get());
        REQUIRE(result == true);
        REQUIRE(loop.registeredFileDescriptorCount() == 0);

        close(pipe1[0]);
        close(pipe1[1]);
    }
}

TEST_CASE("Wait for events")
{
    spdlog::set_level(spdlog::level::debug);

    SUBCASE("can poll for events (0ms timeout)")
    {
        LinuxPlatformEventLoop loop;
        loop.waitForEvents(0);
    }

    SUBCASE("can wait for events (100 ms timeout)")
    {
        LinuxPlatformEventLoop loop;
        loop.waitForEvents(100);
    }

    SUBCASE("can wait until an event occurs on a watched fd")
    {
        LinuxPlatformEventLoop loop;
        int pipe1[2];
        if (pipe(pipe1) == -1) {
            spdlog::debug("Failed to create pipe. errno = {}", errno);
            REQUIRE(false);
        }

        // Register a notifier to listen to the read end of the pipe
        auto notifier1 = std::make_unique<FileDescriptorNotifier>(pipe1[0], FileDescriptorNotifier::NotificationType::Read);
        loop.registerNotifier(notifier1.get());

        // Spawn a thread to write to the write-end of the pipe
        std::mutex mutex;
        std::condition_variable cond;
        bool ready = false;
        auto writeToPipe = [&mutex, &cond, &ready](int fd) {
            spdlog::info("Launched helper thread");
            std::unique_lock lock(mutex);
            cond.wait(lock, [&ready] { return ready == true; });
            spdlog::info("Thread going to sleep before writing to pipe");

            std::this_thread::sleep_for(std::chrono::milliseconds(300));

            int n = 42;
            int written = write(fd, &n, sizeof(n));
        };
        std::thread t1(writeToPipe, pipe1[1]);

        // Kick the thread off to fire an event in 1s then wait for up to 10s for an event
        {
            spdlog::info("Waking up helper thread");
            std::unique_lock lock(mutex);
            ready = true;
            cond.notify_all();
        }

        const auto startTime = std::chrono::steady_clock::now();
        loop.waitForEvents(10000);
        const auto endTime = std::chrono::steady_clock::now();

        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        spdlog::info("elapsedTime = {}", elapsedTime);
        REQUIRE(elapsedTime < 10000);

        // Be nice!
        t1.join();
    }

    SUBCASE("can wake up by calling wakeUp from another thread")
    {
        LinuxPlatformEventLoop loop;

        // Spawn a thread to write to the write-end of the pipe
        std::mutex mutex;
        std::condition_variable cond;
        bool ready = false;
        auto callWakeUp = [&mutex, &cond, &ready, &loop]() {
            spdlog::info("Launched helper thread");
            std::unique_lock lock(mutex);
            cond.wait(lock, [&ready] { return ready == true; });
            spdlog::info("Thread going to sleep before writing to pipe");

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            loop.wakeUp();
        };
        std::thread t1(callWakeUp);

        // Kick the thread off to fire an event in 1s then wait for up to 10s for an event
        {
            spdlog::info("Waking up helper thread");
            std::unique_lock lock(mutex);
            ready = true;
            cond.notify_all();
        }

        const auto startTime = std::chrono::steady_clock::now();
        loop.waitForEvents(10000);
        const auto endTime = std::chrono::steady_clock::now();

        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        spdlog::info("elapsedTime = {}", elapsedTime);
        REQUIRE(elapsedTime < 10000);

        // Be nice!
        t1.join();
    }
}

TEST_CASE("Notifies about events")
{
    spdlog::set_level(spdlog::level::debug);

    SUBCASE("can wait until an event occurs on a watched fd")
    {
        LinuxPlatformEventLoop loop;
        auto postman = std::make_unique<Postman>();
        loop.setPostman(postman.get());
        int pipe1[2];
        if (pipe(pipe1) == -1) {
            spdlog::debug("Failed to create pipe. errno = {}", errno);
            REQUIRE(false);
        }

        // Register a notifier to listen to the read end of the pipe
        auto notifier1 = std::make_unique<FileDescriptorNotifier>(pipe1[0], FileDescriptorNotifier::NotificationType::Read);
        bool notified = false;
        notifier1->triggered.connect([&notified](const int &) {
            spdlog::info("Notifier has been triggered");
            notified = true;
        });
        loop.registerNotifier(notifier1.get());

        // Spawn a thread to write to the write-end of the pipe
        std::mutex mutex;
        std::condition_variable cond;
        bool ready = false;
        auto writeToPipe = [&mutex, &cond, &ready](int fd) {
            spdlog::info("Launched helper thread");
            std::unique_lock lock(mutex);
            cond.wait(lock, [&ready] { return ready == true; });
            spdlog::info("Thread going to sleep before writing to pipe");

            std::this_thread::sleep_for(std::chrono::milliseconds(300));

            int n = 42;
            int written = write(fd, &n, sizeof(n));
        };
        std::thread t1(writeToPipe, pipe1[1]);

        // Kick the thread off to fire an event in 1s then wait for up to 10s for an event
        {
            spdlog::info("Waking up helper thread");
            std::unique_lock lock(mutex);
            ready = true;
            cond.notify_all();
        }

        const auto startTime = std::chrono::steady_clock::now();
        loop.waitForEvents(10000);
        const auto endTime = std::chrono::steady_clock::now();

        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        spdlog::info("elapsedTime = {}", elapsedTime);
        REQUIRE(elapsedTime < 10000);
        REQUIRE(notified == true);

        // Be nice!
        t1.join();
    }
}
