#include "project.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

bool save_project(const Scene& scene, const std::string& path, std::string& error) {
    std::ofstream out(path);
    if (!out) { error = "Could not write " + path; return false; }
    out << "MAWI_PROJECT 2\n";
    for (const auto& o : scene.objects) {
        out << "OBJECT " << o.id << ' ' << std::quoted(o.name) << ' ' << std::quoted(o.type) << ' '
            << o.position.x << ' ' << o.position.y << ' ' << o.position.z << ' '
            << o.rotation.x << ' ' << o.rotation.y << ' ' << o.rotation.z << ' '
            << o.scale.x << ' ' << o.scale.y << ' ' << o.scale.z << ' '
            << o.color.r << ' ' << o.color.g << ' ' << o.color.b << ' ' << o.parent << ' '
            << o.pivot.x << ' ' << o.pivot.y << ' ' << o.pivot.z << '\n';
        out << "MESH " << o.mesh.vertices.size() << ' ' << o.mesh.faces.size() << '\n';
        for(const auto& v:o.mesh.vertices)out << "V " << v.x << ' ' << v.y << ' ' << v.z << '\n';
        for(const auto& face:o.mesh.faces){out << "F " << face.size();for(int index:face)out << ' ' << index;out << '\n';}
    }
    if(!out){error="Could not finish writing "+path;return false;}return true;
}

bool load_project(Scene& scene, const std::string& path, std::string& error) {
    std::ifstream in(path);
    if (!in) { error = "Could not open " + path; return false; }
    std::string header; int version = 0;
    if (!(in >> header >> version) || header != "MAWI_PROJECT" || (version != 1 && version != 2)) {
        error = "Invalid or unsupported MAWI project"; return false;
    }
    Scene loaded; std::string tag;
    while (in >> tag) {
        if (tag != "OBJECT") { error = "Invalid project record"; return false; }
        SceneObject o;
        bool ok=version==1?static_cast<bool>(in >> o.id >> o.name >> o.type):static_cast<bool>(in >> o.id >> std::quoted(o.name) >> std::quoted(o.type));
        if (!ok || !(in >> o.position.x >> o.position.y >> o.position.z
              >> o.rotation.x >> o.rotation.y >> o.rotation.z >> o.scale.x >> o.scale.y >> o.scale.z
              >> o.color.r >> o.color.g >> o.color.b >> o.parent)) {
            error = "Incomplete object record"; return false;
        }
        if(version==2){
            if(!(in>>o.pivot.x>>o.pivot.y>>o.pivot.z)){error="Invalid pivot";return false;}
            size_t vertices=0,faces=0;if(!(in>>tag>>vertices>>faces)||tag!="MESH"||vertices>100000||faces>100000){error="Invalid mesh size";return false;}
            o.mesh.vertices.reserve(vertices);o.mesh.faces.reserve(faces);
            for(size_t i=0;i<vertices;++i){Vec3 v;
                if(!(in>>tag>>v.x>>v.y>>v.z)||tag!="V"||!std::isfinite(v.x)||!std::isfinite(v.y)||!std::isfinite(v.z)){error="Invalid mesh vertex";return false;}
                o.mesh.vertices.push_back(v);}
            for(size_t i=0;i<faces;++i){size_t count=0;
                if(!(in>>tag>>count)||tag!="F"||count<3||count>128){error="Invalid mesh face";return false;}
                std::vector<int> face;face.reserve(count);
                for(size_t j=0;j<count;++j){int index=-1;if(!(in>>index)||index<0||size_t(index)>=vertices){error="Invalid mesh index";return false;}face.push_back(index);}
                o.mesh.faces.push_back(std::move(face));}
        }else o.mesh=cube_mesh();
        loaded.nextId = std::max(loaded.nextId, o.id + 1);
        loaded.objects.push_back(o);
    }
    if (loaded.objects.empty()) { error = "Project contains no objects"; return false; }
    scene = std::move(loaded);
    return true;
}
