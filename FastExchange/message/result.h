#ifndef FAST_EXCHANGE_RESULT_H
#define FAST_EXCHANGE_RESULT_H
#include "types.h"

namespace Message::Result {
    struct Result {
        virtual ~Result() = default;
    };
}
#endif //FAST_EXCHANGE_RESULT_H
