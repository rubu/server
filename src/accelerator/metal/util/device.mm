#import "device.h"

#import <common/metal/metal_check.h>
#import <common/assert.h>

#import <Metal/Metal.h>

namespace caspar { namespace accelerator { namespace metal {

struct device::impl : public std::enable_shared_from_this<impl>
{
    id<MTLDevice> device_;

    impl()
        : device_(MTLCreateSystemDefaultDevice())
    {
         CASPAR_LOG(info) << L"Initializing Metal device.";
        
        if (device_ == NULL) {
            CASPAR_THROW_EXCEPTION(caspar::metal::metal_exception() << msg_info("Failed to initialize Metal device"));
        }
        
        CASPAR_LOG(info) << L"Initialized Metal device " << [device_.name UTF8String];
    }
};

device::device() : impl_(new impl)
{
}

device::~device()
{
}

std::shared_ptr<class texture> device::create_texture(int width, int height)
{
    CASPAR_VERIFY(width > 0 && height > 0);

    MTLTextureDescriptor *desc =
    [
        MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm                                                                                       width:width
                                                         height:height
                                                      mipmapped:NO
    ];
    return std::make_shared<texture>(reinterpret_cast<void*>([impl_->device_ newTextureWithDescriptor:desc]));
}

boost::property_tree::wptree device::info() const
{
}

std::future<void> device::gc()
{
}

}}}
