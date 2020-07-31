#pragma once

#import "texture.h"

#include <accelerator/accelerator.h>
#include <common/array.h>

#include <functional>
#include <future>

namespace caspar { namespace accelerator { namespace metal {

class device final
    : public std::enable_shared_from_this<device>
    , public accelerator_device
{
  public:
    device();
    ~device();

    device(const device&) = delete;

    device& operator=(const device&) = delete;

    std::shared_ptr<texture> create_texture(int width, int height);

    boost::property_tree::wptree info() const;
    std::future<void>            gc();

  private:
    void dispatch(std::function<void()> func);
    struct impl;
    std::shared_ptr<impl> impl_;
};

}}} // namespace caspar::accelerator::metal
