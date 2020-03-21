// This file is distributed under the MIT license.
// See the LICENSE file for details.

#include <common/config.h>

#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#if VSNRAY_COMMON_HAVE_GLEW
#include <GL/glew.h>
#endif

#include <imgui.h>

#include <Support/CmdLine.h>
#include <Support/CmdLineUtil.h>

#include "input/key_event.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include "input/mouse_event.h"
#include "input/space_mouse.h"
#include "manip/camera_manipulator.h"
#include "inifile.h"
#include "viewer_base.h"

using namespace support;
using namespace visionaray;

using manipulators      = std::vector<std::shared_ptr<camera_manipulator>>;
using cmdline_options   = std::vector<std::shared_ptr<cl::OptionBase>>;

struct viewer_base::impl
{
    static viewer_base* viewer;

    manipulators        manips;
    cmdline_options     options;
    cl::CmdLine         cmd;
    bool                allow_unknown_args = false;

    bool                full_screen        = false;
    int                 width              = 512;
    int                 height             = 512;
    char const*         window_title       = "";
    vec3                bgcolor            = { 0.1f, 0.4f, 1.0f };

    GLuint              imgui_font_texture;

    impl(int width, int height, char const* window_title);

    void init(int argc, char** argv);

    void parse_inifile(std::set<std::string> const& filenames);

    void parse_cmd_line(int argc, char** argv);

    // Space mouse callbacks
    static void space_mouse_move_func(space_mouse_event const& event);
    static void space_mouse_button_press_func(space_mouse_event const& event);
};

viewer_base* viewer_base::impl::viewer = nullptr;
    

viewer_base::impl::impl(int width, int height, char const* window_title)
    : width(width)
    , height(height)
    , window_title(window_title)
{
    // add default options (-fullscreen, -width, -height, -bgcolor)

    options.emplace_back( cl::makeOption<bool&>(
        cl::Parser<>(),
        "fullscreen",
        cl::Desc("Full screen window"),
        cl::ArgDisallowed,
        cl::init(viewer_base::impl::full_screen)
        ) );

    options.emplace_back( cl::makeOption<int&>(
        cl::Parser<>(),
        "width",
        cl::Desc("Window width"),
        cl::ArgRequired,
        cl::init(viewer_base::impl::width)
        ) );

    options.emplace_back( cl::makeOption<int&>(
        cl::Parser<>(),
        "height",
        cl::Desc("Window height"),
        cl::ArgRequired,
        cl::init(viewer_base::impl::height)
        ) );

    options.emplace_back( cl::makeOption<vec3&, cl::ScalarType>(
        [&](StringRef name, StringRef /*arg*/, vec3& value)
        {
            cl::Parser<>()(name + "-r", cmd.bump(), value.x);
            cl::Parser<>()(name + "-g", cmd.bump(), value.y);
            cl::Parser<>()(name + "-b", cmd.bump(), value.z);
        },
        "bgcolor",
        cl::Desc("Background color"),
        cl::ArgDisallowed,
        cl::init(viewer_base::impl::bgcolor)
        ) );
}

void viewer_base::impl::init(int argc, char** argv)
{
    try
    {
        parse_cmd_line(argc, argv);
    }
    catch (...)
    {
        std::cout << cmd.help(argv[0]) << '\n';
        throw;
    }
}


//-------------------------------------------------------------------------------------------------
// Parse ini file
//

void viewer_base::impl::parse_inifile(std::set<std::string> const& filenames)
{
    // Process the first (if any) valid inifile
    for (auto filename : filenames)
    {
        inifile ini(filename);

        if (ini.good())
        {
            inifile::error_code err = inifile::Ok;

            // Full screen
            bool fs = full_screen;
            err = ini.get_bool("fullscreen", fs);
            if (err == inifile::Ok)
            {
                full_screen = fs;
            }

            // Window width
            int32_t w = width;
            err = ini.get_int32("width", w);
            if (err == inifile::Ok)
            {
                width = w;
            }

            // Window height
            int32_t h = height;
            err = ini.get_int32("height", h);
            if (err == inifile::Ok)
            {
                height = h;
            }

            // Background color
            vec3 bg = bgcolor;
            err = ini.get_vec3f("bgcolor", bg.x, bg.y, bg.z);
            if (err == inifile::Ok)
            {
                bgcolor = bg;
            }

            // Don't consider other files
            break;
        }
    }
}


//-------------------------------------------------------------------------------------------------
// Parse default command line options
//

void viewer_base::impl::parse_cmd_line(int argc, char** argv)
{
    for (auto& opt : options)
    {
        cmd.add(*opt);
    }

    auto args = std::vector<std::string>(argv + 1, argv + argc);
    cl::expandWildcards(args);
    cl::expandResponseFiles(args, cl::TokenizeUnix());

    cmd.parse(args, allow_unknown_args);
}


//-------------------------------------------------------------------------------------------------
// Static space mouse callbacks
//

void viewer_base::impl::space_mouse_move_func(space_mouse_event const& event)
{
    viewer->on_space_mouse_move(event);
}

void viewer_base::impl::space_mouse_button_press_func(space_mouse_event const& event)
{
    viewer->on_space_mouse_button_press(event);
}


viewer_base::viewer_base(
        int width,
        int height,
        char const* window_title
        )
    : impl_(new impl(width, height, window_title))
{
    viewer_base::impl::viewer = this;

    if (space_mouse::init())
    {
        space_mouse::register_event_callback(space_mouse::Button, &impl::space_mouse_button_press_func);
        space_mouse::register_event_callback(space_mouse::Rotation, &impl::space_mouse_move_func);
        space_mouse::register_event_callback(space_mouse::Translation, &impl::space_mouse_move_func);
    }
}

viewer_base::~viewer_base()
{
    space_mouse::cleanup();
}

void viewer_base::init(int argc, char** argv)
{
    impl_->init(argc, argv);
}

void viewer_base::parse_inifile(std::set<std::string> const& filenames)
{
    impl_->parse_inifile(filenames);
}

void viewer_base::add_manipulator( std::shared_ptr<camera_manipulator> manip )
{
    impl_->manips.push_back(manip);
}

void viewer_base::add_cmdline_option( std::shared_ptr<cl::OptionBase> option )
{
    impl_->options.emplace_back(option);
}

char const* viewer_base::window_title() const
{
    return impl_->window_title;
}

bool viewer_base::full_screen() const
{
    return impl_->full_screen;
}

int viewer_base::width() const
{
    return impl_->width;
}

int viewer_base::height() const
{
    return impl_->height;
}

vec3 viewer_base::background_color() const
{
    return impl_->bgcolor;
}

void viewer_base::set_allow_unknown_cmd_line_args(bool allow)
{
    impl_->allow_unknown_args = allow;
}

cl::CmdLine& viewer_base::cmd_line_inst()
{
    return impl_->cmd;
}

void viewer_base::set_background_color(vec3 color)
{
    impl_->bgcolor = color;
}

void viewer_base::event_loop()
{
}

void viewer_base::resize(int width, int height)
{
    impl_->width = width;
    impl_->height = height;
}

void viewer_base::swap_buffers()
{
}

void viewer_base::toggle_full_screen()
{
    impl_->full_screen = !impl_->full_screen;
}

void viewer_base::quit()
{
}

bool viewer_base::have_imgui_support()
{
    return false;
}


//-------------------------------------------------------------------------------------------------
// Event handlers
//

void viewer_base::on_close()
{
}

void viewer_base::on_display()
{
}

void viewer_base::on_idle()
{
}

void viewer_base::on_key_press(visionaray::key_event const& event)
{
    if (event.key() == keyboard::F5)
    {
        toggle_full_screen();
    }

    if (event.key() == keyboard::Escape && impl_->full_screen)
    {
        toggle_full_screen();
    }

    if (event.key() == keyboard::q)
    {
        quit();
    }

    for (auto& manip : impl_->manips)
    {
        manip->handle_key_press(event);
    }
}

void viewer_base::on_key_release(visionaray::key_event const& event)
{
    for (auto& manip : impl_->manips)
    {
        manip->handle_key_release(event);
    }
}

void viewer_base::on_mouse_move(visionaray::mouse_event const& event)
{
    for (auto& manip : impl_->manips)
    {
        manip->handle_mouse_move(event);
    }
}

void viewer_base::on_mouse_down(visionaray::mouse_event const& event)
{
    for (auto& manip : impl_->manips)
    {
        manip->handle_mouse_down(event);
    }
}

void viewer_base::on_mouse_up(visionaray::mouse_event const& event)
{
    for (auto& manip : impl_->manips)
    {
        manip->handle_mouse_up(event);
    }
}

void viewer_base::on_space_mouse_move(visionaray::space_mouse_event const& event)
{
    for (auto& manip : impl_->manips)
    {
        manip->handle_space_mouse_move(event);
    }
}

void viewer_base::on_space_mouse_button_press(visionaray::space_mouse_event const& event)
{
    for (auto& manip : impl_->manips)
    {
        manip->handle_space_mouse_button_press(event);
    }
}

void viewer_base::on_resize(int w, int h)
{
    impl_->width = w;
    impl_->height = h;

    glViewport(0, 0, w, h);
}

void viewer_base::imgui_draw_opengl2(ImDrawData* draw_data)
{
#if VSNRAY_COMMON_HAVE_GLEW
    ImGuiIO& io = ImGui::GetIO();

    int width = static_cast<int>(draw_data->DisplaySize.x * io.DisplayFramebufferScale.x);
    int height = static_cast<int>(draw_data->DisplaySize.y * io.DisplayFramebufferScale.y);

    if (width == 0 || height == 0)
    {
        return;
    }

    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // Store OpenGL state
    GLint prev_texture = 0;
    GLint prev_polygon_mode[2] = {};
    GLint prev_viewport[4] = {};
    GLint prev_scissor_box[4] = {};
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev_texture);
    glGetIntegerv(GL_POLYGON_MODE, prev_polygon_mode);
    glGetIntegerv(GL_VIEWPORT, prev_viewport);
    glGetIntegerv(GL_SCISSOR_BOX, prev_scissor_box); 

    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
    glEnable(GL_SCISSOR_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //glUseProgram(0);

    glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(
        draw_data->DisplayPos.x,
        draw_data->DisplayPos.x + draw_data->DisplaySize.x,
        draw_data->DisplayPos.y + draw_data->DisplaySize.y,
        draw_data->DisplayPos.y,
        -1.0f,
        1.0f
        );
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    ImVec2 pos = draw_data->DisplayPos;
    for (int n = 0; n < draw_data->CmdListsCount; ++n)
    {
        ImDrawList const* cmd_list = draw_data->CmdLists[n];
        ImDrawVert const* vtx_buffer = cmd_list->VtxBuffer.Data;
        ImDrawIdx const* idx_buffer = cmd_list->IdxBuffer.Data;

        glVertexPointer(
            2,
            GL_FLOAT,
            sizeof(ImDrawVert),
            static_cast<GLvoid const*>(reinterpret_cast<char const*>(vtx_buffer) + IM_OFFSETOF(ImDrawVert, pos))
            );

        glTexCoordPointer(
            2,
            GL_FLOAT,
            sizeof(ImDrawVert),
            static_cast<GLvoid const*>(reinterpret_cast<char const*>(vtx_buffer) + IM_OFFSETOF(ImDrawVert, uv))
            );

        glColorPointer(
            4,
            GL_UNSIGNED_BYTE,
            sizeof(ImDrawVert),
            static_cast<GLvoid const*>(reinterpret_cast<char const*>(vtx_buffer) + IM_OFFSETOF(ImDrawVert, col))
            );

        for (int i = 0; i < cmd_list->CmdBuffer.Size; ++i)
        {
            ImDrawCmd const* pcmd = &cmd_list->CmdBuffer[i];

            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                ImVec4 clip_rect(
                    pcmd->ClipRect.x - pos.x,
                    pcmd->ClipRect.y - pos.y,
                    pcmd->ClipRect.z - pos.x,
                    pcmd->ClipRect.w - pos.y
                    );

                if (clip_rect.x < width && clip_rect.y < height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
                {
                    glScissor(
                        static_cast<int>(clip_rect.x),
                        static_cast<int>(height - clip_rect.w),
                        static_cast<int>(clip_rect.z - clip_rect.x),
                        static_cast<int>(clip_rect.w - clip_rect.y)
                        );

                    glBindTexture(
                        GL_TEXTURE_2D,
                        static_cast<GLuint>(reinterpret_cast<intptr_t>(pcmd->TextureId))
                        );

                    glDrawElements(
                        GL_TRIANGLES,
                        static_cast<GLsizei>(pcmd->ElemCount),
                        sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                        idx_buffer
                        );
                }
            }

            idx_buffer += pcmd->ElemCount;
        }
    }

    // Restore OpenGL state
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(prev_texture));
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
    glPolygonMode(GL_FRONT, static_cast<GLenum>(prev_polygon_mode[0]));
    glPolygonMode(GL_BACK,  static_cast<GLenum>(prev_polygon_mode[1]));
    glViewport(
        prev_viewport[0],
        prev_viewport[1],
        static_cast<GLsizei>(prev_viewport[2]),
        static_cast<GLsizei>(prev_viewport[3])
        );
    glScissor(
        prev_scissor_box[0],
        prev_scissor_box[1],
        static_cast<GLsizei>(prev_scissor_box[2]),
        static_cast<GLsizei>(prev_scissor_box[3])
        );
#endif
}

void viewer_base::imgui_create_font_texture_opengl2()
{
#if VSNRAY_COMMON_HAVE_GLEW
    ImGuiIO& io = ImGui::GetIO();

    unsigned char* pixels = nullptr;
    int width = 0;
    int height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    GLint prev_tex = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev_tex);
    glGenTextures(1, &impl_->imgui_font_texture);
    glBindTexture(GL_TEXTURE_2D, impl_->imgui_font_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    io.Fonts->TexID = reinterpret_cast<ImTextureID>(
            static_cast<intptr_t>(impl_->imgui_font_texture)
            );

    glBindTexture(GL_TEXTURE_2D, prev_tex);
#endif
}

void viewer_base::imgui_destroy_font_texture_opengl2()
{
#if VSNRAY_COMMON_HAVE_GLEW
    assert(impl_->imgui_font_texture);

    ImGuiIO& io = ImGui::GetIO();

    glDeleteTextures(1, &impl_->imgui_font_texture);
    io.Fonts->TexID = nullptr;
    impl_->imgui_font_texture = 0;
#endif
}
