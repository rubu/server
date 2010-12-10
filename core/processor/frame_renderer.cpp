#include "../StdAfx.h"

#include "frame_renderer.h"

#include "frame_shader.h"
#include "write_frame.h"
#include "read_frame.h"

#include "../format/video_format.h"

#include "../../common/exception/exceptions.h"
#include "../../common/gl/utility.h"
#include "../../common/gl/frame_buffer_object.h"

#include <Glee.h>

#include <tbb/concurrent_queue.h>

#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/range/algorithm.hpp>

#include <functional>

namespace caspar { namespace core {
	
struct frame_renderer::implementation : boost::noncopyable
{	
	implementation(const video_format_desc& format_desc) : shader_(format_desc), format_desc_(format_desc),
		reading_(new read_frame(0, 0)), writing_(new write_frame(pixel_format_desc())), drawing_(new write_frame(pixel_format_desc())), fbo_(format_desc.width, format_desc.height)
	{	
		GL(glEnable(GL_POLYGON_STIPPLE));
		GL(glEnable(GL_TEXTURE_2D));
		GL(glEnable(GL_BLEND));
		GL(glDisable(GL_DEPTH_TEST));
		GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));			
		GL(glViewport(0, 0, format_desc.width, format_desc.height));
	}
				
	consumer_frame render(const gpu_frame_ptr& frame)
	{
		if(frame == nullptr)
			return nullptr;

		read_frame_ptr result;
		try
		{
			drawing_ = writing_;
			writing_ = frame;
						
			writing_->begin_write(); // Note: end_write is done when returned to pool, write_frame::reset();
						
			reading_->end_read();
			result = reading_; 
						
			GL(glClear(GL_COLOR_BUFFER_BIT));
						
			drawing_->draw(shader_);
				
			reading_ = create_output_frame();
			
			reading_->begin_read();
			reading_->audio_data() = std::move(drawing_->audio_data());
						
			drawing_ = nullptr;
		}
		catch(...)
		{
			CASPAR_LOG_CURRENT_EXCEPTION();
		}

		return result;
	}

	read_frame_ptr create_output_frame()
	{
		read_frame_ptr frame;
		if(!frame_pool_.try_pop(frame))		
			frame.reset(new read_frame(format_desc_.width, format_desc_.height));		
		return read_frame_ptr(frame.get(), [=](read_frame*){frame_pool_.push(frame);});
	}

	tbb::concurrent_bounded_queue<read_frame_ptr> frame_pool_;

	common::gl::frame_buffer_object fbo_;

	read_frame_ptr	reading_;	
	gpu_frame_ptr	writing_;
	gpu_frame_ptr	drawing_;
	
	frame_shader shader_;

	video_format_desc format_desc_;
};
	
frame_renderer::frame_renderer(const video_format_desc& format_desc) : impl_(new implementation(format_desc)){}
consumer_frame frame_renderer::render(const gpu_frame_ptr& frame)
{
	return impl_->render(frame);
}
}}