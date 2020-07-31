#pragma once

#include "../screen_consumer.h"

#include <GL/glew.h>
#include <SFML/Window.hpp>

namespace caspar { namespace screen { namespace ogl {

struct frame
{
    GLuint pbo   = 0;
    GLuint tex   = 0;
    char*  ptr   = nullptr;
    GLsync fence = nullptr;
};

struct ogl_screen_consumer : public screen_consumer
{
    std::vector<frame> frames_;
  
    sf::Window window_;

    std::unique_ptr<accelerator::ogl::shader> shader_;
    GLuint                                    vao_;
    GLuint                                    vbo_;

    ogl_screen_consumer(const ogl_screen_consumer&) = delete;
    ogl_screen_consumer& operator=(const ogl_screen_consumer&) = delete;

  public:
    ogl_screen_consumer(const configuration& config, const core::video_format_desc& format_desc, int channel_index);
    ~ogl_screen_consumer();

    bool poll();
    void tick();
    std::future<bool> send(const core::const_frame& frame);
    std::wstring channel_and_format() const;
    std::wstring print() const;
    void calculate_aspect();
    std::pair<float, float> none();
    std::pair<float, float> uniform();
    std::pair<float, float> Fill();
    std::pair<float, float> uniform_to_fill();
};

}}} // namespace caspar::screen::ogl


