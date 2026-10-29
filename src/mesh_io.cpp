#include "mesh_io.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace {
bool obj_extension(const std::filesystem::path& path){
    auto ext=path.extension().string();std::transform(ext.begin(),ext.end(),ext.begin(),[](unsigned char c){return char(std::tolower(c));});
    return ext==".obj";
}
Vec3 transform(const SceneObject& o,Vec3 v){
    v.x-=o.pivot.x;v.y-=o.pivot.y;v.z-=o.pivot.z;
    v.x*=o.scale.x;v.y*=o.scale.y;v.z*=o.scale.z;
    const float rad=.01745329252f;
    auto rotate=[](float& a,float& b,float angle){float c=std::cos(angle),s=std::sin(angle),u=a,w=b;a=c*u-s*w;b=s*u+c*w;};
    rotate(v.y,v.z,o.rotation.x*rad);rotate(v.z,v.x,o.rotation.y*rad);rotate(v.x,v.y,o.rotation.z*rad);
    v.x+=o.position.x+o.pivot.x;v.y+=o.position.y+o.pivot.y;v.z+=o.position.z+o.pivot.z;return v;
}
}

bool import_obj(const std::filesystem::path& path,Scene& scene,int& selectedId,std::string& error){
    if(!obj_extension(path)){error="Only Wavefront .obj models are supported";return false;}
    std::ifstream in(path);if(!in){error="Cannot open OBJ file";return false;}
    Mesh mesh;std::string line;size_t lineNumber=0;
    while(std::getline(in,line)){
        ++lineNumber;if(line.size()>65536){error="OBJ line is too long";return false;}
        std::istringstream row(line);std::string tag;if(!(row>>tag)||tag[0]=='#')continue;
        if(tag=="v"){
            Vec3 v;if(!(row>>v.x>>v.y>>v.z)||!std::isfinite(v.x)||!std::isfinite(v.y)||!std::isfinite(v.z)){
                error="Invalid OBJ vertex at line "+std::to_string(lineNumber);return false;}
            if(mesh.vertices.size()>=100000){error="OBJ exceeds 100000 vertices";return false;}mesh.vertices.push_back(v);
        }else if(tag=="f"){
            std::vector<int> face;std::string token;
            while(row>>token){auto slash=token.find('/');std::string part=token.substr(0,slash);
                int index=0;try{size_t used=0;index=std::stoi(part,&used);if(used!=part.size())throw std::invalid_argument("index");}
                catch(...){error="Invalid OBJ face at line "+std::to_string(lineNumber);return false;}
                if(index==0){error="OBJ indices are 1-based";return false;}
                index=index>0?index-1:int(mesh.vertices.size())+index;
                if(index<0||size_t(index)>=mesh.vertices.size()){error="OBJ face references missing vertex";return false;}
                face.push_back(index);if(face.size()>128){error="OBJ face has too many vertices";return false;}}
            if(face.size()<3){error="OBJ face has fewer than 3 vertices";return false;}
            if(mesh.faces.size()>=100000){error="OBJ exceeds 100000 faces";return false;}mesh.faces.push_back(std::move(face));
        }
    }
    if(mesh.vertices.empty()||mesh.faces.empty()){error="OBJ contains no renderable mesh";return false;}
    SceneObject o;o.id=scene.nextId++;o.name=path.stem().string();if(o.name.empty())o.name="Imported Mesh";
    o.type="Mesh";o.position={0,0,0};o.mesh=std::move(mesh);selectedId=o.id;scene.objects.push_back(std::move(o));
    return true;
}

bool export_obj(const std::filesystem::path& path,const SceneObject& o,std::string& error){
    if(!obj_extension(path)){error="Export path must end in .obj";return false;}
    std::ofstream out(path);if(!out){error="Cannot write OBJ file";return false;}
    const Mesh fallback=cube_mesh();const Mesh& mesh=o.mesh.faces.empty()?fallback:o.mesh;
    out<<"# Exported by MAWI Studio\n"<<"o "<<o.name<<'\n';
    for(const auto& raw:mesh.vertices){Vec3 v=transform(o,raw);out<<"v "<<v.x<<' '<<v.y<<' '<<v.z<<'\n';}
    for(const auto& face:mesh.faces){out<<"f";for(int index:face)out<<' '<<index+1;out<<'\n';}
    if(!out){error="Could not finish OBJ export";return false;}return true;
}
