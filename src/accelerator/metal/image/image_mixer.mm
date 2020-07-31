#import "image_mixer.h"

namespace caspar { namespace accelerator { namespace metal {

image_mixer::image_mixer(const spl::shared_ptr<class device>& ogl, int channel_id)
{
}

image_mixer::~image_mixer()
{
}

std::future<array<const std::uint8_t>> image_mixer::operator()(const core::video_format_desc& format_desc)
{
}

core::mutable_frame image_mixer::create_frame(const void* tag, const core::pixel_format_desc& desc)
{
}

void image_mixer::push(const core::frame_transform& frame)
{
}

void image_mixer::visit(const core::const_frame& frame)
{
}

void image_mixer::pop()
{
}

}}} // namespace caspar::accelerator::metal
