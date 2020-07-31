#pragma once

#include <memory>

namespace caspar { namespace accelerator { namespace metal {

class device;

class texture final
{
  public:
    texture(void *metal_texture /* id<MTLTexture> */);
    texture(texture&& other) = default;
    ~texture();

    void copy_from(class buffer& source);
    void copy_to(class buffer& dest);
   
  private:
    struct impl;
    std::unique_ptr<impl> impl_;
};

}}} // namespace caspar::accelerator::ogl
