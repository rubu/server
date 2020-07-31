/*
 * Copyright (c) 2011 Sveriges Television AB <info@casparcg.com>
 *
 * This file is part of CasparCG (www.casparcg.com).
 *
 * CasparCG is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * CasparCG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CasparCG. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Robert Nagy, ronag89@gmail.com
 */

#include "screen_consumer.h"

#if defined(__APPLE__)
#include "metal/metal_screen_consumer.h"
#else
#include "ogl/ogl_screen_consumer.h"
#endif

namespace caspar { namespace screen {


screen_consumer::screen_consumer(const configuration& config, const core::video_format_desc& format_desc, int channel_index)
    : config_(config)
    , format_desc_(format_desc)
    , channel_index_(channel_index)
{
    if (format_desc_.format == core::video_format::ntsc &&
        config_.aspect == configuration::aspect_ratio::aspect_4_3) {
        // Use default values which are 4:3.
    } else {
        if (config_.aspect == configuration::aspect_ratio::aspect_16_9) {
            square_width_ = format_desc.height * 16 / 9;
        } else if (config_.aspect == configuration::aspect_ratio::aspect_4_3) {
            square_width_ = format_desc.height * 4 / 3;
        }
    }

    frame_buffer_.set_capacity(1);

    graph_->set_color("tick-time", diagnostics::color(0.0f, 0.6f, 0.9f));
    graph_->set_color("frame-time", diagnostics::color(0.1f, 1.0f, 0.1f));
    graph_->set_color("dropped-frame", diagnostics::color(0.3f, 0.6f, 0.3f));
    graph_->set_text(print());
    diagnostics::register_graph(graph_);

#if defined(_MSC_VER)
    DISPLAY_DEVICE              d_device = {sizeof(d_device), 0};
    std::vector<DISPLAY_DEVICE> displayDevices;
    for (int n = 0; EnumDisplayDevices(nullptr, n, &d_device, NULL); ++n) {
        displayDevices.push_back(d_device);
    }

    if (config_.screen_index >= displayDevices.size()) {
        CASPAR_LOG(warning) << print() << L" Invalid screen-index: " << config_.screen_index;
    }

    DEVMODE devmode = {};
    if (!EnumDisplaySettings(displayDevices[config_.screen_index].DeviceName, ENUM_CURRENT_SETTINGS, &devmode)) {
        CASPAR_LOG(warning) << print() << L" Could not find display settings for screen-index: "
                            << config_.screen_index;
    }

    screen_x_      = devmode.dmPosition.x;
    screen_y_      = devmode.dmPosition.y;
    screen_width_  = devmode.dmPelsWidth;
    screen_height_ = devmode.dmPelsHeight;
#else
    if (config_.screen_index > 1) {
        CASPAR_LOG(warning) << print() << L" Screen-index is not supported on linux";
    }
#endif

    if (config.windowed) {
        screen_x_ += config.screen_x;
        screen_y_ += config.screen_y;

        if (config.screen_width > 0 && config.screen_height > 0) {
            screen_width_  = config.screen_width;
            screen_height_ = config.screen_height;
        } else if (config.screen_width > 0) {
            screen_width_  = config.screen_width;
            screen_height_ = square_height_ * config.screen_width / square_width_;
        } else if (config.screen_height > 0) {
            screen_height_ = config.screen_height;
            screen_width_  = square_width_ * config.screen_height / square_height_;
        } else {
            screen_width_  = square_width_;
            screen_height_ = square_height_;
        }
    }
}

std::wstring screen_consumer::channel_and_format() const
{
    return L"[" + std::to_wstring(channel_index_) + L"|" + format_desc_.name + L"]";
}

std::wstring screen_consumer::print() const
{
    return config_.name + L" " + channel_and_format();
}

struct screen_consumer_proxy : public core::frame_consumer
{
#if defined(__APPLE__)
    using screen_consumer = metal::metal_screen_consumer;
#else
    using screen_consumer = ogl::ogl_screen_consumer;
#endif

    const configuration              config_;
    std::unique_ptr<screen_consumer> consumer_;

  public:
    explicit screen_consumer_proxy(configuration config)
        : config_(std::move(config))
    {
    }

    // frame_consumer

    void initialize(const core::video_format_desc& format_desc, int channel_index) override
    {
        consumer_.reset();
        consumer_ = std::make_unique<screen_consumer>(config_, format_desc, channel_index);
    }

    std::future<bool> send(core::const_frame frame) override { return consumer_->send(frame); }

    std::wstring print() const override { return consumer_ ? consumer_->print() : L"[screen_consumer]"; }

    std::wstring name() const override { return L"screen"; }

    bool has_synchronization_clock() const override { return false; }

    int index() const override { return 600 + (config_.key_only ? 10 : 0) + config_.screen_index; }
};

spl::shared_ptr<core::frame_consumer> create_consumer(const std::vector<std::wstring>&                         params,
                                                      const std::vector<spl::shared_ptr<core::video_channel>>& channels)
{
    if (params.empty() || !boost::iequals(params.at(0), L"SCREEN")) {
        return core::frame_consumer::empty();
    }

    configuration config;

    if (params.size() > 1) {
        try {
            config.screen_index = std::stoi(params.at(1));
        } catch (...) {
        }
    }

    config.windowed    = !contains_param(L"FULLSCREEN", params);
    config.key_only    = contains_param(L"KEY_ONLY", params);
    config.sbs_key     = contains_param(L"SBS_KEY", params);
    config.interactive = !contains_param(L"NON_INTERACTIVE", params);
    config.borderless  = contains_param(L"BORDERLESS", params);

    if (contains_param(L"NAME", params)) {
        config.name = get_param(L"NAME", params);
    }

    if (config.sbs_key && config.key_only) {
        CASPAR_LOG(warning) << L" Key-only not supported with configuration of side-by-side fill and key. Ignored.";
        config.key_only = false;
    }

    return spl::make_shared<screen_consumer_proxy>(config);
}

spl::shared_ptr<core::frame_consumer>
create_preconfigured_consumer(const boost::property_tree::wptree&                      ptree,
                              const std::vector<spl::shared_ptr<core::video_channel>>& channels)
{
    configuration config; 
    config.name          = ptree.get(L"name", config.name);
    config.screen_index  = ptree.get(L"device", config.screen_index + 1) - 1;
    config.screen_x      = ptree.get(L"x", config.screen_x);
    config.screen_y      = ptree.get(L"y", config.screen_y);
    config.screen_width  = ptree.get(L"width", config.screen_width);
    config.screen_height = ptree.get(L"height", config.screen_height);
    config.windowed      = ptree.get(L"windowed", config.windowed);
    config.key_only      = ptree.get(L"key-only", config.key_only);
    config.sbs_key       = ptree.get(L"sbs-key", config.sbs_key);
    config.vsync         = ptree.get(L"vsync", config.vsync);
    config.interactive   = ptree.get(L"interactive", config.interactive);
    config.borderless    = ptree.get(L"borderless", config.borderless);
    config.always_on_top = ptree.get(L"always-on-top", config.always_on_top);

    auto colour_space_value = ptree.get(L"colour-space", L"RGB");
    config.colour_space     = configuration::colour_spaces::RGB;
    if (colour_space_value == L"datavideo-full")
        config.colour_space = configuration::colour_spaces::datavideo_full;
    else if (colour_space_value == L"datavideo-limited")
        config.colour_space = configuration::colour_spaces::datavideo_limited;

    if (config.sbs_key && config.key_only) {
        CASPAR_LOG(warning) << L" Key-only not supported with configuration of side-by-side fill and key. Ignored.";
        config.key_only = false;
    }

    if ((config.colour_space == configuration::colour_spaces::datavideo_full ||
         config.colour_space == configuration::colour_spaces::datavideo_limited) &&
        config.sbs_key) {
        CASPAR_LOG(warning) << L" Side-by-side fill and key not supported for DataVideo TC100/TC200. Ignored.";
        config.sbs_key = false;
    }

    if ((config.colour_space == configuration::colour_spaces::datavideo_full ||
         config.colour_space == configuration::colour_spaces::datavideo_limited) &&
        config.key_only) {
        CASPAR_LOG(warning) << L" Key only not supported for DataVideo TC100/TC200. Ignored.";
        config.key_only = false;
    }

    auto stretch_str = ptree.get(L"stretch", L"fill");
    if (stretch_str == L"none") {
        config.stretch = screen::stretch::none;
    } else if (stretch_str == L"uniform") {
        config.stretch = screen::stretch::uniform;
    } else if (stretch_str == L"uniform_to_fill") {
        config.stretch = screen::stretch::uniform_to_fill;
    }

    auto aspect_str = ptree.get(L"aspect-ratio", L"default");
    if (aspect_str == L"16:9") {
        config.aspect = configuration::aspect_ratio::aspect_16_9;
    } else if (aspect_str == L"4:3") {
        config.aspect = configuration::aspect_ratio::aspect_4_3;
    }

    return spl::make_shared<screen_consumer_proxy>(config);
}

}} // namespace caspar::screen
