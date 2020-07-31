#pragma once

#include "../screen_consumer.h"

namespace caspar { namespace screen { namespace metal {

struct metal_screen_consumer : screen_consumer
{
    metal_screen_consumer(const screen_consumer&) = delete;
    metal_screen_consumer& operator=(const screen_consumer&) = delete;

    metal_screen_consumer(const configuration& config, const core::video_format_desc& format_desc, int channel_index);
    ~metal_screen_consumer();

    std::future<bool> send(const core::const_frame& frame);
};

}}} // namespace caspar::screen::metal