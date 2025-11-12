/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: BogDan Vatra <bogdan@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "android_platform_event_loop.h"

#include <android_native_app_glue.h>

#include <KDFoundation/core_application.h>
#include <KDFoundation/event.h>
#include <KDFoundation/file_descriptor_notifier.h>
#include <KDFoundation/postman.h>

#include <android/log.h>

#include "android_platform_integration.h"

using namespace KDFoundation;
namespace KDGui {

void AndroidPlatformEventLoop::androidHandleCmd(android_app *app, int32_t cmd)
{
    switch (cmd) {
        //    case APP_CMD_INIT_WINDOW:
        //// TODO handle me
        //        break;
        //    case APP_CMD_TERM_WINDOW:
        //// TODO handle me
        //        break;
    case APP_CMD_WINDOW_RESIZED:
        reinterpret_cast<AndroidPlatformEventLoop *>(AndroidPlatformIntegration::s_androidApp->userData)->androidPlatformIntegration()->handleWindowResize();
        break;
    default:
        __android_log_print(ANDROID_LOG_INFO, "AndroidPlatformEventLoop", "event not handled: %d", cmd);
    }
}

int32_t AndroidPlatformEventLoop::androidHandleInputEvent(android_app *app, AInputEvent *event)
{
    switch (AInputEvent_getType(event)) {
    case AINPUT_EVENT_TYPE_KEY:
        if (AKeyEvent_getKeyCode(event) == AKEYCODE_BACK) {
            auto vm = app->activity->vm;
            JNIEnv *env = nullptr;
            int res = vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
            if (res == JNI_EDETACHED) {
                JavaVMAttachArgs aargs = { JNI_VERSION_1_6, "Serenity Thread", nullptr };
                vm->AttachCurrentThread(&env, &aargs);
            }
            const auto finish = env->GetMethodID(env->GetObjectClass(app->activity->clazz), "finish", "()V");
            env->CallVoidMethod(app->activity->clazz, finish, 0);
            return 1; // prevent default handler
        } else {
            auto platformIntegration = reinterpret_cast<AndroidPlatformEventLoop *>(AndroidPlatformIntegration::s_androidApp->userData)->androidPlatformIntegration();
            platformIntegration->handleKeyEvent(AKeyEvent_getAction(event), AKeyEvent_getKeyCode(event), AKeyEvent_getMetaState(event), AKeyEvent_getEventTime(event));
            return 0;
        }
        break;
    case AINPUT_EVENT_TYPE_MOTION: {
        auto platformIntegration = reinterpret_cast<AndroidPlatformEventLoop *>(AndroidPlatformIntegration::s_androidApp->userData)->androidPlatformIntegration();
        const auto timestamp = AMotionEvent_getEventTime(event);
        const auto action = AMotionEvent_getAction(event);
        // Only track the first touch event
        const auto xPos = static_cast<int64_t>(AMotionEvent_getX(event, 0));
        const auto yPos = static_cast<int64_t>(AMotionEvent_getY(event, 0));

        platformIntegration->handleTouchEvent(action, xPos, yPos, timestamp);
        break;
    }

    // TODO IMPLEMENT ME
    case AINPUT_EVENT_TYPE_FOCUS:
    case AINPUT_EVENT_TYPE_CAPTURE:
    case AINPUT_EVENT_TYPE_DRAG:
    case AINPUT_EVENT_TYPE_TOUCH_MODE:
        break;
    default:
        break;
    }
    return 0;
}

AndroidPlatformEventLoop::AndroidPlatformEventLoop(AndroidPlatformIntegration *androidPlatformInitegration)
{
    AndroidPlatformIntegration::s_androidApp->onAppCmd = androidHandleCmd;
    AndroidPlatformIntegration::s_androidApp->onInputEvent = androidHandleInputEvent;
    AndroidPlatformIntegration::s_androidApp->userData = this;
    m_androidPlatformInitegration = androidPlatformInitegration;
}

void AndroidPlatformEventLoop::waitForEventsImpl(int timeout)
{
    android_poll_source *source;
    if (ALooper_pollOnce(timeout, nullptr, nullptr, (void **)&source) >= 0) {
        if (source != nullptr)
            source->process(AndroidPlatformIntegration::s_androidApp, source);
    }
    if (AndroidPlatformIntegration::s_androidApp->destroyRequested) {
        __android_log_print(ANDROID_LOG_INFO, "AndroidPlatformEventLoop", "destroyRequested 1");
        if (auto instance = KDFoundation::CoreApplication::instance()) {
            __android_log_print(ANDROID_LOG_INFO, "AndroidPlatformEventLoop", "destroyRequested 2");
            instance->quit();
        }
    }
}

void AndroidPlatformEventLoop::wakeUp()
{
    ALooper_wake(AndroidPlatformIntegration::s_androidApp->looper);
}

bool AndroidPlatformEventLoop::registerNotifier(KDFoundation::FileDescriptorNotifier *notifier)
{
    if (!notifier)
        return false;

    auto &notifierSet = m_notifiers[notifier->fileDescriptor()];
    const auto eventType = static_cast<uint8_t>(notifier->type());
    if (notifierSet.events[eventType])
        return false;
    notifierSet.events[eventType] = notifier;
    if (-1 == ALooper_addFd(AndroidPlatformIntegration::s_androidApp->looper, notifier->fileDescriptor(), ALOOPER_POLL_CALLBACK, notifierSet.androidEvents(), ALooperCallback, this)) {
        notifierSet.events[eventType] = nullptr;
    }
    return true;
}

bool AndroidPlatformEventLoop::unregisterNotifier(KDFoundation::FileDescriptorNotifier *notifier)
{
    if (!notifier)
        return false;
    auto &notifierSet = m_notifiers[notifier->fileDescriptor()];
    const auto eventType = static_cast<uint8_t>(notifier->type());
    if (!notifierSet.events[eventType])
        return false;
    notifierSet.events[eventType] = nullptr;
    if (notifierSet.isEmpty()) {
        ALooper_removeFd(AndroidPlatformIntegration::s_androidApp->looper, notifier->fileDescriptor());
        m_notifiers.erase(notifier->fileDescriptor());
    } else {
        ALooper_addFd(AndroidPlatformIntegration::s_androidApp->looper, notifier->fileDescriptor(), ALOOPER_POLL_CALLBACK, notifierSet.androidEvents(), ALooperCallback, this);
    }
    return true;
}

AndroidPlatformIntegration *AndroidPlatformEventLoop::androidPlatformIntegration()
{
    return m_androidPlatformInitegration;
}

std::unique_ptr<KDFoundation::AbstractPlatformTimer>
AndroidPlatformEventLoop::createPlatformTimerImpl(KDFoundation::Timer * /*timer*/)
{
    return {};
}

int AndroidPlatformEventLoop::ALooperCallback(int fd, int events, void *data)
{
    auto thiz = reinterpret_cast<AndroidPlatformEventLoop *>(data);
    if (!thiz->m_postman)
        return 1;
    auto &notifierSet = thiz->m_notifiers[fd];
    if (events & ALOOPER_EVENT_INPUT && notifierSet.events[0]) {
        KDFoundation::NotifierEvent ev;
        thiz->m_postman->deliverEvent(notifierSet.events[0], &ev);
    }

    if (events & ALOOPER_EVENT_OUTPUT && notifierSet.events[1]) {
        KDFoundation::NotifierEvent ev;
        thiz->m_postman->deliverEvent(notifierSet.events[1], &ev);
    }

    if ((events & ALOOPER_EVENT_ERROR || events & ALOOPER_EVENT_HANGUP) && notifierSet.events[2]) {
        KDFoundation::NotifierEvent ev;
        thiz->m_postman->deliverEvent(notifierSet.events[2], &ev);
    }
    return 1;
}

int AndroidPlatformEventLoop::NotifierSet::androidEvents() const
{
    int res = 0;
    if (events[0])
        res |= ALOOPER_EVENT_INPUT;
    if (events[1])
        res |= ALOOPER_EVENT_OUTPUT;
    if (events[2])
        res |= ALOOPER_EVENT_ERROR | ALOOPER_EVENT_HANGUP;
    return res;
}

bool AndroidPlatformEventLoop::NotifierSet::isEmpty() const
{
    return events[0] == nullptr && events[1] == nullptr && events[2] == nullptr;
}

} // namespace KDGui
