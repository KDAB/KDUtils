#ifndef KDUTILS_SIGNAL_H
#define KDUTILS_SIGNAL_H

#include <kdbindings/signal.h>

using namespace KDBindings;

namespace KDUtils {

template<typename... Args>
class Signal : public KDBindings::Signal<Args...>
{
public:
    // New method to connect a slot through the CoreApplication's ConnectionEvaluator
    ConnectionHandle connect(const std::shared_ptr<ConnectionEvaluator> &evaluator, std::function<void(Args...)> const &slot)
    {
        if (!evaluator) {
            throw std::runtime_error("ConnectionEvaluator is not available.");
        }
        return this->connectDeferred(evaluator, slot);
    }
};

} // namespace KDUtils

#endif