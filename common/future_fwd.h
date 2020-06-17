#pragma once

#if !defined(__APPLE__)
#include "forward.h"

FORWARD1(std, template<typename> class future);
FORWARD1(std, template<typename> class shared_future);
FORWARD1(std, template<typename> class promise);
#else
#include <future>
#endif