// This file is distributed under the MIT license.
// See the LICENSE file for details.

#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include "cfile.h"
#include "model.h"
#include "sg.h"
#include "vsnray_loader.h"

namespace visionaray
{

//-------------------------------------------------------------------------------------------------
// Function declarations
//

void parse_children(std::shared_ptr<sg::node> parent, rapidjson::Value const& entries);

template <typename Object>
std::shared_ptr<sg::node> parse_camera(Object const& obj);

template <typename Object>
std::shared_ptr<sg::node> parse_point_light(Object const& obj);

template <typename Object>
std::shared_ptr<sg::node> parse_reference(Object const& obj);

template <typename Object>
std::shared_ptr<sg::node> parse_transform(Object const& obj);

template <typename Object>
std::shared_ptr<sg::node> parse_surface_properties(Object const& obj);

template <typename Object>
std::shared_ptr<sg::node> parse_triangle_mesh(Object const& obj);

template <typename Object>
std::shared_ptr<sg::node> parse_indexed_triangle_mesh(Object const& obj);

//-------------------------------------------------------------------------------------------------
// Parse nodes
//

void parse_children(std::shared_ptr<sg::node> parent, rapidjson::Value const& entries)
{
    parent->children().resize(entries.MemberCount());

    size_t i = 0;
    for (auto const& c : entries.GetArray())
    {
        auto const& obj = c.GetObject();

        if (obj.HasMember("type"))
        {
            auto const& type_string = obj["type"];
            if (strncmp(type_string.GetString(), "camera", 6) == 0)
            {
                parent->children().at(i++) = parse_camera(obj);
            }
            else if (strncmp(type_string.GetString(), "point_light", 11) == 0)
            {
                parent->children().at(i++) = parse_point_light(obj);
            }
            else if (strncmp(type_string.GetString(), "reference", 9) == 0)
            {
                parent->children().at(i++) = parse_reference(obj);
            }
            else if (strncmp(type_string.GetString(), "transform", 9) == 0)
            {
                parent->children().at(i++) = parse_transform(obj);
            }
            else if (strncmp(type_string.GetString(), "surface_properties", 18) == 0)
            {
                parent->children().at(i++) = parse_surface_properties(obj);
            }
            else if (strncmp(type_string.GetString(), "triangle_mesh", 13) == 0)
            {
                parent->children().at(i++) = parse_triangle_mesh(obj);
            }
            else if (strncmp(type_string.GetString(), "indexed_triangle_mesh", 21) == 0)
            {
                parent->children().at(i++) = parse_indexed_triangle_mesh(obj);
            }
            else
            {
                throw std::runtime_error("");
            }
        }
        else
        {
            throw std::runtime_error("");
        }
    }

    if (i != entries.MemberCount())
    {
        throw std::runtime_error("");
    }
}

template <typename Object>
std::shared_ptr<sg::node> parse_camera(Object const& obj)
{
    auto cam = std::make_shared<sg::camera>();

    vec3 eye(0.0f);
    if (obj.HasMember("eye"))
    {
        auto const& cam_eye = obj["eye"];

        int i = 0;
        for (auto const& item : cam_eye.GetArray())
        {
            eye[i++] = item.GetFloat();
        }

        if (i != 3)
        {
            throw std::runtime_error("");
        }
    }

    vec3 center(0.0f);
    if (obj.HasMember("center"))
    {
        auto const& cam_center = obj["center"];

        int i = 0;
        for (auto const& item : cam_center.GetArray())
        {
            center[i++] = item.GetFloat();
        }

        if (i != 3)
        {
            throw std::runtime_error("");
        }
    }

    vec3 up(0.0f);
    if (obj.HasMember("up"))
    {
        auto const& cam_up = obj["up"];

        int i = 0;
        for (auto const& item : cam_up.GetArray())
        {
            up[i++] = item.GetFloat();
        }

        if (i != 3)
        {
            throw std::runtime_error("");
        }
    }

    float fovy = 0.0f;
    if (obj.HasMember("fovy"))
    {
        fovy = obj["fovy"].GetFloat();
    }

    float znear = 0.0f;
    if (obj.HasMember("znear"))
    {
        znear = obj["znear"].GetFloat();
    }

    float zfar = 0.0f;
    if (obj.HasMember("zfar"))
    {
        zfar = obj["zfar"].GetFloat();
    }

    recti viewport;
    if (obj.HasMember("viewport"))
    {
        auto const& cam_viewport = obj["viewport"];

        int i = 0;
        for (auto const& item : cam_viewport.GetArray())
        {
            viewport.data()[i++] = item.GetInt();
        }

        if (i != 4)
        {
            throw std::runtime_error("");
        }
    }

    float lens_radius = 0.0f;
    if (obj.HasMember("lens_radius"))
    {
        lens_radius = obj["lens_radius"].GetFloat();
    }

    float focal_distance = 0.0f;
    if (obj.HasMember("focal_distance"))
    {
        focal_distance = obj["focal_distance"].GetFloat();
    }

    float aspect = viewport.w > 0 && viewport.h > 0
                 ? viewport.w / static_cast<float>(viewport.h)
                 : 1;

    cam->perspective(fovy * constants::degrees_to_radians<float>(), aspect, znear, zfar);
    cam->set_viewport(viewport);
    cam->set_lens_radius(lens_radius);
    cam->set_focal_distance(focal_distance);
    cam->look_at(eye, center, up);

    if (obj.HasMember("children"))
    {
        rapidjson::Value const& children = obj["children"];
        parse_children(cam, children);
    }

    return cam;
}

template <typename Object>
std::shared_ptr<sg::node> parse_point_light(Object const& obj)
{
    auto light = std::make_shared<sg::point_light>();

    vec3 cl(1.0f);
    if (obj.HasMember("cl"))
    {
        auto const& color = obj["cl"];

        int i = 0;
        for (auto const& item : color.GetArray())
        {
            cl[i++] = item.GetFloat();
        }

        if (i != 3)
        {
            throw std::runtime_error("");
        }
    }

    float kl = 1.0f;
    if (obj.HasMember("kl"))
    {
        kl = obj["kl"].GetFloat();
    }

    vec3 position(0.0f);
    if (obj.HasMember("position"))
    {
        auto const& pos = obj["position"];

        int i = 0;
        for (auto const& item : pos.GetArray())
        {
            position[i++] = item.GetFloat();
        }

        if (i != 3)
        {
            throw std::runtime_error("");
        }
    }

    float constant_attenuation = 1.0f;
    if (obj.HasMember("constant_attenuation"))
    {
        constant_attenuation = obj["constant_attenuation"].GetFloat();
    }

    float linear_attenuation = 0.0f;
    if (obj.HasMember("linear_attenuation"))
    {
        linear_attenuation = obj["linear_attenuation"].GetFloat();
    }

    float quadratic_attenuation = 0.0f;
    if (obj.HasMember("quadratic_attenuation"))
    {
        quadratic_attenuation = obj["quadratic_attenuation"].GetFloat();
    }

    light->set_cl(cl);
    light->set_kl(kl);
    light->set_position(position);
    light->set_constant_attenuation(constant_attenuation);
    light->set_linear_attenuation(linear_attenuation);
    light->set_quadratic_attenuation(quadratic_attenuation);

    if (obj.HasMember("children"))
    {
        rapidjson::Value const& children = obj["children"];
        parse_children(light, children);
    }

    return light;
}

template <typename Object>
std::shared_ptr<sg::node> parse_reference(Object const& obj)
{
    return std::make_shared<sg::node>();
}

template <typename Object>
std::shared_ptr<sg::node> parse_transform(Object const& obj)
{
    auto transform = std::make_shared<sg::transform>();

    if (obj.HasMember("matrix"))
    {
        auto const& mat = obj["matrix"];

        int i = 0;
        for (auto const& item : mat.GetArray())
        {
            transform->matrix().data()[i++] = item.GetFloat();
            assert(i <= 16);
        }
    }

    if (obj.HasMember("children"))
    {
        rapidjson::Value const& children = obj["children"];
        parse_children(transform, children);
    }

    return transform;
}

template <typename Object>
std::shared_ptr<sg::node> parse_surface_properties(Object const& obj)
{
    auto props = std::make_shared<sg::surface_properties>();

    if (obj.HasMember("material"))
    {
        auto const& mat = obj["material"];

        if (mat.HasMember("type"))
        {
            auto const& type_string = mat["type"];
            if (strncmp(type_string.GetString(), "obj", 3) == 0)
            {
                auto obj = std::make_shared<sg::obj_material>();

                if (mat.HasMember("ca"))
                {
                    auto const& ca = mat["ca"];

                    vec3 clr;
                    int i = 0;
                    for (auto const& item : ca.GetArray())
                    {
                        clr[i++] = item.GetFloat();
                    }

                    if (i != 3)
                    {
                        throw std::runtime_error("");
                    }

                    obj->ca = clr;
                }

                if (mat.HasMember("cd"))
                {
                    auto const& cd = mat["cd"];

                    vec3 clr;
                    int i = 0;
                    for (auto const& item : cd.GetArray())
                    {
                        clr[i++] = item.GetFloat();
                    }

                    if (i != 3)
                    {
                        throw std::runtime_error("");
                    }

                    obj->cd = clr;
                }

                if (mat.HasMember("cs"))
                {
                    auto const& cs = mat["cs"];

                    vec3 clr;
                    int i = 0;
                    for (auto const& item : cs.GetArray())
                    {
                        clr[i++] = item.GetFloat();
                    }

                    if (i != 3)
                    {
                        throw std::runtime_error("");
                    }

                    obj->cs = clr;
                }

                if (mat.HasMember("ce"))
                {
                    auto const& ce = mat["ce"];

                    vec3 clr;
                    int i = 0;
                    for (auto const& item : ce.GetArray())
                    {
                        clr[i++] = item.GetFloat();
                    }

                    if (i != 3)
                    {
                        throw std::runtime_error("");
                    }

                    obj->ce = clr;
                }

                props->material() = obj;
            }
            else
            {
                throw std::runtime_error("");
            }
        }
        else
        {
            throw std::runtime_error("");
        }
    }
    else
    {
        // Set default material (wavefront obj)
        auto obj = std::make_shared<sg::obj_material>();
        props->material() = obj;
    }

    if (obj.HasMember("diffuse"))
    {
        // TODO: load from file
#if 1
        vector<4, unorm<8>> dummy_texel(1.0f, 1.0f, 1.0f, 1.0f);
        auto tex = std::make_shared<sg::texture2d<vector<4, unorm<8>>>>();
        tex->resize(1, 1);
        tex->set_address_mode(Wrap);
        tex->set_filter_mode(Nearest);
        tex->reset(&dummy_texel);

        props->add_texture(tex);
#endif
    }
    else
    {
        // Set a dummy texture
        vector<4, unorm<8>> dummy_texel(1.0f, 1.0f, 1.0f, 1.0f);
        auto tex = std::make_shared<sg::texture2d<vector<4, unorm<8>>>>();
        tex->resize(1, 1);
        tex->set_address_mode(Wrap);
        tex->set_filter_mode(Nearest);
        tex->reset(&dummy_texel);

        props->add_texture(tex);
    }

    if (obj.HasMember("children"))
    {
        rapidjson::Value const& children = obj["children"];
        parse_children(props, children);
    }

    return props;
}

template <typename Object>
std::shared_ptr<sg::node> parse_triangle_mesh(Object const& obj)
{
    auto mesh = std::make_shared<sg::triangle_mesh>();

    if (obj.HasMember("vertices"))
    {
        auto const& verts = obj["vertices"];

        vec3 v;
        int i = 0;
        for (auto const& item : verts.GetArray())
        {
            v[i++ % 3] = item.GetFloat();

            if (i % 3 == 0)
            {
                mesh->vertices.emplace_back(v);
            }
        }
    }

    if (obj.HasMember("normals"))
    {
        auto const& normals = obj["normals"];

        vec3 n;
        int i = 0;
        for (auto const& item : normals.GetArray())
        {
            n[i++ % 3] = item.GetFloat();

            if (i % 3 == 0)
            {
                mesh->normals.emplace_back(n);
            }
        }
    }
    else
    {
        for (size_t i = 0; i < mesh->vertices.size(); i += 3)
        {
            vec3 v1 = mesh->vertices[i];
            vec3 v2 = mesh->vertices[i + 1];
            vec3 v3 = mesh->vertices[i + 2];

            vec3 gn = normalize(cross(v2 - v1, v3 - v1));

            mesh->normals.emplace_back(gn);
            mesh->normals.emplace_back(gn);
            mesh->normals.emplace_back(gn);
        }
    }

    if (obj.HasMember("tex_coords"))
    {
        auto const& tex_coords = obj["tex_coords"];

        vec3 tc;
        int i = 0;
        for (auto const& item : tex_coords.GetArray())
        {
            tc[i++ % 2] = item.GetFloat();

            if (i % 2 == 0)
            {
                mesh->tex_coords.emplace_back(tc);
            }
        }
    }
    else
    {
        for (size_t i = 0; i < mesh->vertices.size(); ++i)
        {
            mesh->tex_coords.emplace_back(0.0f, 0.0f);
        }
    }

    if (obj.HasMember("colors"))
    {
        auto const& colors = obj["colors"];

        vector<3, unorm<8>> c;
        int i = 0;
        for (auto const& item : colors.GetArray())
        {
            c[i++ % 3] = item.GetFloat();

            if (i % 3 == 0)
            {
                mesh->colors.emplace_back(c);
            }
        }
    }
    else
    {
        for (size_t i = 0; i < mesh->vertices.size(); ++i)
        {
            mesh->colors.emplace_back(1.0f);
        }
    }

    if (obj.HasMember("children"))
    {
        rapidjson::Value const& children = obj["children"];
        parse_children(mesh, children);
    }

    return mesh;
}

template <typename Object>
std::shared_ptr<sg::node> parse_indexed_triangle_mesh(Object const& obj)
{
    auto mesh = std::make_shared<sg::indexed_triangle_mesh>();

    if (obj.HasMember("indices"))
    {
        auto const& indices = obj["indices"];

        for (auto const& item : indices.GetArray())
        {
            mesh->indices.push_back(item.GetInt());
        }
    }

    if (obj.HasMember("vertices"))
    {
        auto const& verts = obj["vertices"];

        vec3 v;
        int i = 0;
        for (auto const& item : verts.GetArray())
        {
            v[i++ % 3] = item.GetFloat();

            if (i % 3 == 0)
            {
                mesh->vertices.emplace_back(v);
            }
        }
    }

    if (obj.HasMember("normals"))
    {
        auto const& normals = obj["normals"];

        vec3 n;
        int i = 0;
        for (auto const& item : normals.GetArray())
        {
            n[i++ % 3] = item.GetFloat();

            if (i % 3 == 0)
            {
                mesh->normals.emplace_back(n);
            }
        }
    }
    else
    {
        for (size_t i = 0; i < mesh->vertices.size(); i += 3)
        {
            vec3 v1 = mesh->vertices[i];
            vec3 v2 = mesh->vertices[i + 1];
            vec3 v3 = mesh->vertices[i + 2];

            vec3 gn = normalize(cross(v2 - v1, v3 - v1));

            mesh->normals.emplace_back(gn);
            mesh->normals.emplace_back(gn);
            mesh->normals.emplace_back(gn);
        }
    }

    if (obj.HasMember("tex_coords"))
    {
        auto const& tex_coords = obj["tex_coords"];

        vec3 tc;
        int i = 0;
        for (auto const& item : tex_coords.GetArray())
        {
            tc[i++ % 2] = item.GetFloat();

            if (i % 2 == 0)
            {
                mesh->tex_coords.emplace_back(tc);
            }
        }
    }
    else
    {
        for (size_t i = 0; i < mesh->vertices.size(); ++i)
        {
            mesh->tex_coords.emplace_back(0.0f, 0.0f);
        }
    }

    if (obj.HasMember("colors"))
    {
        auto const& colors = obj["colors"];

        vector<3, unorm<8>> c;
        int i = 0;
        for (auto const& item : colors.GetArray())
        {
            c[i++ % 3] = item.GetFloat();

            if (i % 3 == 0)
            {
                mesh->colors.emplace_back(c);
            }
        }
    }
    else
    {
        for (size_t i = 0; i < mesh->vertices.size(); ++i)
        {
            mesh->colors.emplace_back(1.0f);
        }
    }

    if (obj.HasMember("children"))
    {
        rapidjson::Value const& children = obj["children"];
        parse_children(mesh, children);
    }

    return mesh;
}


//-------------------------------------------------------------------------------------------------
// Interface
//

void load_vsnray(std::string const& filename, model& mod)
{
    std::vector<std::string> filenames(1);

    filenames[0] = filename;

    load_vsnray(filenames, mod);
}

void load_vsnray(std::vector<std::string> const& filenames, model& mod)
{
    auto root = std::make_shared<sg::node>();

    for (auto filename : filenames)
    {
        cfile file(filename, "r");
        if (!file.good())
        {
            std::cerr << "Cannot open " << filename << '\n';
            return;
        }

        char buffer[65536];
        rapidjson::FileReadStream frs(file.get(), buffer, sizeof(buffer));
        rapidjson::Document doc;
        doc.ParseStream(frs);


        if (doc.HasMember("children"))
        {
            rapidjson::Value const& children = doc["children"];
            parse_children(root, children);
        }
    }

    if (mod.scene_graph == nullptr)
    {
        mod.scene_graph = std::make_shared<sg::node>();
    }

    mod.scene_graph->add_child(root);
}

} // visionaray