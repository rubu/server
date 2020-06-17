#include "StdAfx.h"

#include "accelerator.h"

#include "cpu/image/image_mixer.h"
#include "ogl/image/image_mixer.h"
#include "ogl/util/device.h"

#include <common/env.h>

#include <core/mixer/image/image_mixer.h>

#include <tbb/mutex.h>

#include <memory>

namespace caspar { namespace accelerator {
	
struct accelerator::impl
{
	const std::wstring				path_;
	tbb::mutex						mutex_;
	std::shared_ptr<ogl::device>	ogl_device_;
	const core::video_format_repository format_repository_;

	impl(const std::wstring& path, const core::video_format_repository format_repository)
		: path_(path), format_repository_(format_repository)
	{
	}

	std::unique_ptr<core::image_mixer> create_image_mixer(const int channel_id)
	{
		try
		{
			if(path_ == L"gpu" || path_ == L"ogl" || path_ == L"auto" || path_ == L"default")
			{
				tbb::mutex::scoped_lock lock(mutex_);

				if(!ogl_device_)
					ogl_device_.reset(new ogl::device());

				return std::make_unique<ogl::image_mixer>(
						spl::make_shared_ptr(ogl_device_),
						env::properties().get(L"configuration.mixer.blend-modes", false),
						env::properties().get(L"configuration.mixer.straight-alpha", false),
						channel_id,
						format_repository_.get_max_video_format_size());
			}
		}
		catch(...)
		{
			if(path_ == L"gpu" || path_ == L"ogl")
				CASPAR_LOG_CURRENT_EXCEPTION();
		}
		return std::make_unique<cpu::image_mixer>(channel_id);
	}
};

accelerator::accelerator(const std::wstring& path, const core::video_format_repository format_repository)
	: impl_(new impl(path, format_repository))
{
}

accelerator::~accelerator()
{
}

std::unique_ptr<core::image_mixer> accelerator::create_image_mixer(const int channel_id)
{
	return impl_->create_image_mixer(channel_id);
}

std::shared_ptr<ogl::device> accelerator::get_ogl_device() const
{
	return impl_->ogl_device_;
}

}}
