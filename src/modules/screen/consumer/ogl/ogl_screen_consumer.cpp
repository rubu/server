#include "ogl_screen_consumer.h"

#if defined(_MSC_VER)
#include <windows.h>

#pragma warning(push)
#pragma warning(disable : 4244)
#else
#include "../util/x11_util.h"
#endif

#include "consumer_screen_fragment.h"
#include "consumer_screen_vertex.h"
#include <accelerator/ogl/util/shader.h>

namespace caspar { namespace screen { namespace ogl {

std::unique_ptr<accelerator::ogl::shader> get_shader()
{
    return std::make_unique<accelerator::ogl::shader>(std::string(vertex_shader), std::string(fragment_shader));
}

ogl_screen_consumer::ogl_screen_consumer(const configuration& config, const core::video_format_desc& format_desc, int channel_index)
    : screen_consumer(config, format_desc, channel_index)
{
    thread_ = std::thread([this] {
        try {
            const auto window_style = config_.borderless ? sf::Style::None
                                                            : config_.windowed ? sf::Style::Resize | sf::Style::Close
                                                                            : sf::Style::Fullscreen;
            sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
            sf::VideoMode mode(
                config_.sbs_key ? screen_width_ * 2 : screen_width_, screen_height_, desktop.bitsPerPixel);
            window_.create(mode,
                            u8(print()),
                            window_style,
                            sf::ContextSettings(0, 0, 0, 4, 5, sf::ContextSettings::Attribute::Core));
            window_.setPosition(sf::Vector2i(screen_x_, screen_y_));
            window_.setMouseCursorVisible(config_.interactive);
            window_.setActive(true);

            if (config_.always_on_top) {
#ifdef _MSC_VER
                HWND hwnd = window_.getSystemHandle();
                SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
#elif defined(__APPLE__)
                // TODO: implement
#else
                window_always_on_top(window_);
#endif
            }

            if (glewInit() != GLEW_OK) {
                CASPAR_THROW_EXCEPTION(gl::ogl_exception() << msg_info("Failed to initialize GLEW."));
            }

            if (!GLEW_VERSION_4_5 && (glewIsSupported("GL_ARB_sync GL_ARB_shader_objects GL_ARB_multitexture "
                                                        "GL_ARB_direct_state_access GL_ARB_texture_barrier") == 0u)) {
                CASPAR_THROW_EXCEPTION(not_supported() << msg_info(
                                            "Your graphics card does not meet the minimum hardware requirements "
                                            "since it does not support OpenGL 4.5 or higher."));
            }

            GL(glGenVertexArrays(1, &vao_));
            GL(glGenBuffers(1, &vbo_));
            GL(glBindVertexArray(vao_));
            GL(glBindBuffer(GL_ARRAY_BUFFER, vbo_));

            shader_ = get_shader();
            shader_->use();
            shader_->set("background", 0);
            shader_->set("window_width", screen_width_);

            for (int n = 0; n < 2; ++n) {
                screen::frame frame;
                auto          flags = GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT;
                GL(glCreateBuffers(1, &frame.pbo));
                GL(glNamedBufferStorage(frame.pbo, format_desc_.size, nullptr, flags));
                frame.ptr =
                    reinterpret_cast<char*>(GL2(glMapNamedBufferRange(frame.pbo, 0, format_desc_.size, flags)));

                GL(glCreateTextures(GL_TEXTURE_2D, 1, &frame.tex));
                GL(glTextureParameteri(frame.tex,
                                        GL_TEXTURE_MIN_FILTER,
                                        (config_.colour_space == configuration::colour_spaces::datavideo_full ||
                                        config_.colour_space == configuration::colour_spaces::datavideo_limited)
                                            ? GL_NEAREST
                                            : GL_LINEAR));
                GL(glTextureParameteri(frame.tex,
                                        GL_TEXTURE_MAG_FILTER,
                                        (config_.colour_space == configuration::colour_spaces::datavideo_full ||
                                        config_.colour_space == configuration::colour_spaces::datavideo_limited)
                                            ? GL_NEAREST
                                            : GL_LINEAR));
                GL(glTextureParameteri(frame.tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
                GL(glTextureParameteri(frame.tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
                GL(glTextureStorage2D(frame.tex, 1, GL_RGBA8, format_desc_.width, format_desc_.height));
                GL(glClearTexImage(frame.tex, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr));

                frames_.push_back(frame);
            }

            GL(glDisable(GL_DEPTH_TEST));
            GL(glClearColor(0.0, 0.0, 0.0, 0.0));
            GL(glViewport(
                0, 0, config_.sbs_key ? format_desc_.width * 2 : format_desc_.width, format_desc_.height));

            calculate_aspect();

            window_.setVerticalSyncEnabled(config_.vsync);
            if (config_.vsync) {
                CASPAR_LOG(info) << print() << " Enabled vsync.";
            }

            shader_->set("colour_space", config_.colour_space);
            if (config_.colour_space == configuration::colour_spaces::datavideo_full ||
                config_.colour_space == configuration::colour_spaces::datavideo_limited) {
                CASPAR_LOG(info) << print() << " Enabled colours conversion for DataVideo TC-100/TC-200 "
                                    << (config_.colour_space == configuration::colour_spaces::datavideo_full
                                            ? "(Full Range)."
                                            : "(Limited Range).");
            }
            while (is_running_) {
                tick();
            }
        } catch (tbb::user_abort&) {
            // Do nothing
        } catch (...) {
            CASPAR_LOG_CURRENT_EXCEPTION();
            is_running_ = false;
        }
        for (auto frame : frames_) {
            GL(glUnmapNamedBuffer(frame.pbo));
            glDeleteBuffers(1, &frame.pbo);
            glDeleteTextures(1, &frame.tex);
        }

        shader_.reset();
        GL(glDeleteVertexArrays(1, &vao_));
        GL(glDeleteBuffers(1, &vbo_));

        window_.close();
    });
}

ogl_screen_consumer::~ogl_screen_consumer()
{
    is_running_ = false;
    frame_buffer_.abort();
    thread_.join();
}

bool ogl_screen_consumer::poll()
{
    int       count = 0;
    sf::Event e;
    while (window_.pollEvent(e)) {
        count++;
        if (e.type == sf::Event::Resized) {
            calculate_aspect();
        } else if (e.type == sf::Event::Closed) {
            is_running_ = false;
        }
    }
    return count > 0;
}

void ogl_screen_consumer::tick()
{
    core::const_frame in_frame;

    while (!frame_buffer_.try_pop(in_frame) && is_running_) {
        // TODO (fix)
        if (!poll()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }

    if (!in_frame) {
        return;
    }

    // Upload
    {
        auto& frame = frames_.front();

        while (frame.fence != nullptr) {
            auto wait = glClientWaitSync(frame.fence, 0, 0);
            if (wait == GL_ALREADY_SIGNALED || wait == GL_CONDITION_SATISFIED) {
                glDeleteSync(frame.fence);
                frame.fence = nullptr;
            }
            if (!poll()) {
                // TODO (fix)
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        }

        std::memcpy(frame.ptr, in_frame.image_data(0).begin(), format_desc_.size);

        GL(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, frame.pbo));
        GL(glTextureSubImage2D(
            frame.tex, 0, 0, 0, format_desc_.width, format_desc_.height, GL_BGRA, GL_UNSIGNED_BYTE, nullptr));
        GL(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));

        frame.fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }

    // Display
    {
        auto& frame = frames_.back();

        GL(glClear(GL_COLOR_BUFFER_BIT));

        GL(glActiveTexture(GL_TEXTURE0));
        GL(glBindTexture(GL_TEXTURE_2D, frame.tex));

        GL(glBufferData(GL_ARRAY_BUFFER,
                        static_cast<GLsizeiptr>(sizeof(core::frame_geometry::coord)) * draw_coords_.size(),
                        draw_coords_.data(),
                        GL_STATIC_DRAW));

        auto stride = static_cast<GLsizei>(sizeof(core::frame_geometry::coord));

        auto vtx_loc = shader_->get_attrib_location("Position");
        auto tex_loc = shader_->get_attrib_location("TexCoordIn");

        GL(glEnableVertexAttribArray(vtx_loc));
        GL(glEnableVertexAttribArray(tex_loc));

        GL(glVertexAttribPointer(vtx_loc, 2, GL_DOUBLE, GL_FALSE, stride, nullptr));
        GL(glVertexAttribPointer(tex_loc, 4, GL_DOUBLE, GL_FALSE, stride, (GLvoid*)(2 * sizeof(GLdouble))));

        shader_->set("window_width", screen_width_);

        if (config_.sbs_key) {
            auto coords_size = static_cast<GLsizei>(draw_coords_.size());

            // First half fill
            shader_->set("key_only", false);
            GL(glDrawArrays(GL_TRIANGLES, 0, coords_size / 2));

            // Second half key
            shader_->set("key_only", true);
            GL(glDrawArrays(GL_TRIANGLES, coords_size / 2, coords_size / 2));
        } else {
            shader_->set("key_only", config_.key_only);
            GL(glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(draw_coords_.size())));
        }

        GL(glDisableVertexAttribArray(vtx_loc));
        GL(glDisableVertexAttribArray(tex_loc));

        GL(glBindTexture(GL_TEXTURE_2D, 0));
    }

    window_.display();

    std::rotate(frames_.begin(), frames_.begin() + 1, frames_.end());

    graph_->set_value("tick-time", tick_timer_.elapsed() * format_desc_.fps * 0.5);
    tick_timer_.restart();
}

std::future<bool> ogl_screen_consumer::send(const core::const_frame& frame)
{
    if (!frame_buffer_.try_push(frame)) {
        graph_->set_tag(diagnostics::tag_severity::WARNING, "dropped-frame");
    }
    return make_ready_future(is_running_.load());
}

void ogl_screen_consumer::calculate_aspect()
{
    if (config_.windowed) {
        screen_height_ = window_.getSize().y;
        screen_width_  = window_.getSize().x;
    }

    GL(glViewport(0, 0, screen_width_, screen_height_));

    std::pair<float, float> target_ratio = none();
    if (config_.stretch == screen::stretch::fill) {
        target_ratio = Fill();
    } else if (config_.stretch == screen::stretch::uniform) {
        target_ratio = uniform();
    } else if (config_.stretch == screen::stretch::uniform_to_fill) {
        target_ratio = uniform_to_fill();
    }

    if (config_.sbs_key) {
        draw_coords_ = {
            // First half fill
            {-target_ratio.first, target_ratio.second, 0.0, 0.0}, // upper left
            {0, target_ratio.second, 1.0, 0.0},                   // upper right
            {0, -target_ratio.second, 1.0, 1.0},                  // lower right

            {-target_ratio.first, target_ratio.second, 0.0, 0.0},  // upper left
            {0, -target_ratio.second, 1.0, 1.0},                   // lower right
            {-target_ratio.first, -target_ratio.second, 0.0, 1.0}, // lower left

            // Second half key
            {0, target_ratio.second, 0.0, 0.0},                   // upper left
            {target_ratio.first, target_ratio.second, 1.0, 0.0},  // upper right
            {target_ratio.first, -target_ratio.second, 1.0, 1.0}, // lower right

            {0, target_ratio.second, 0.0, 0.0},                   // upper left
            {target_ratio.first, -target_ratio.second, 1.0, 1.0}, // lower right
            {0, -target_ratio.second, 0.0, 1.0}                   // lower left
        };
    } else {
        draw_coords_ = {
            //    vertex    texture
            {-target_ratio.first, target_ratio.second, 0.0, 0.0}, // upper left
            {target_ratio.first, target_ratio.second, 1.0, 0.0},  // upper right
            {target_ratio.first, -target_ratio.second, 1.0, 1.0}, // lower right

            {-target_ratio.first, target_ratio.second, 0.0, 0.0}, // upper left
            {target_ratio.first, -target_ratio.second, 1.0, 1.0}, // lower right
            {-target_ratio.first, -target_ratio.second, 0.0, 1.0} // lower left
        };
    }
}

std::pair<float, float> ogl_screen_consumer::none()
{
    float width =
        static_cast<float>(config_.sbs_key ? square_width_ * 2 : square_width_) / static_cast<float>(screen_width_);
    float height = static_cast<float>(square_height_) / static_cast<float>(screen_height_);

    return std::make_pair(width, height);
}

std::pair<float, float> ogl_screen_consumer::uniform()
{
    float aspect = static_cast<float>(config_.sbs_key ? square_width_ * 2 : square_width_) /
                    static_cast<float>(square_height_);
    float width  = std::min(1.0f, static_cast<float>(screen_height_) * aspect / static_cast<float>(screen_width_));
    float height = static_cast<float>(screen_width_ * width) / static_cast<float>(screen_height_ * aspect);

    return std::make_pair(width, height);
}

std::pair<float, float> ogl_screen_consumer::Fill()
{
    return std::make_pair(1.0f, 1.0f);
}

std::pair<float, float> ogl_screen_consumer::uniform_to_fill()
{
    float wr =
        static_cast<float>(config_.sbs_key ? square_width_ * 2 : square_width_) / static_cast<float>(screen_width_);
    float hr    = static_cast<float>(square_height_) / static_cast<float>(screen_height_);
    float r_inv = 1.0f / std::min(wr, hr);

    float width  = wr * r_inv;
    float height = hr * r_inv;

    return std::make_pair(width, height);
}

}}} // namespace caspar::screen::ogl