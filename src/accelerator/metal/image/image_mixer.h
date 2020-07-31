#pragma once

#include <common/array.h>
#include <common/memory.h>

#include <core/frame/frame.h>
#include <core/mixer/image/image_mixer.h>
#include <core/video_format.h>

#include <future>

namespace caspar { namespace accelerator { namespace metal {

class image_mixer final : public core::image_mixer
{
  public:
    image_mixer(const spl::shared_ptr<class device>& ogl, int channel_id);
    image_mixer(const image_mixer&) = delete;

    ~image_mixer();

    image_mixer& operator=(const image_mixer&) = delete;

    std::future<array<const std::uint8_t>> operator()(const core::video_format_desc& format_desc) override;
    core::mutable_frame                    create_frame(const void* tag, const core::pixel_format_desc& desc) override;

    // core::image_mixer

    void push(const core::frame_transform& frame) override;
    void visit(const core::const_frame& frame) override;
    void pop() override;

  private:
    struct impl;
    std::shared_ptr<impl> impl_;
};

}}} // namespace caspar::accelerator::metal
