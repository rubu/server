#import "metal_screen_consumer.h"

namespace caspar { namespace screen { namespace metal {

metal_screen_consumer::metal_screen_consumer(const configuration& config, const core::video_format_desc& format_desc, int channel_index)
    : screen_consumer(config, format_desc, channel_index)
{
}

metal_screen_consumer::~metal_screen_consumer()
{
}

std::future<bool> metal_screen_consumer::send(const core::const_frame& frame)
{
    std::promise<bool> result;
    result.set_value(false);
    return result.get_future();
}

}}} // namespace caspar::screen::metal
