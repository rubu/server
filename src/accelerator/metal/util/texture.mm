#import "texture.h"

#import <Metal/Metal.h>

namespace caspar { namespace accelerator { namespace metal {


struct texture::impl
{
    id<MTLTexture> texture_;
    
    impl(id<MTLTexture> texture)
        : texture_(texture)
    {
    }
};

texture::texture(void *metal_texture)
    : impl_(new impl(reinterpret_cast<id<MTLTexture>>(metal_texture)))
{
}

texture::~texture()
{
}

}}} // namespace caspar::accelerator::metal
