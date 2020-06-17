#include "../thread.h"
#include "../../utf.h"

#include <pthread.h>

namespace caspar {

void set_thread_name(const std::wstring& name)
{
#if defined(__APPLE__)
    pthread_setname_np(u8(name).c_str());
#else
    pthread_setname_np(pthread_self(), u8(name).c_str());
#endif
}

} // namespace caspar