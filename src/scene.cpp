#include "scene.hpp"
#include <cmath>

Mesh cube_mesh(){
    Mesh m;
    m.vertices={{-.5f,-.5f,-.5f},{.5f,-.5f,-.5f},{.5f,.5f,-.5f},{-.5f,.5f,-.5f},
                {-.5f,-.5f,.5f},{.5f,-.5f,.5f},{.5f,.5f,.5f},{-.5f,.5f,.5f}};
    m.faces={{0,1,2,3},{4,7,6,5},{0,4,5,1},{3,2,6,7},{1,5,6,2},{0,3,7,4}};
    return m;
}

bool extrude_face(Mesh& mesh,size_t faceIndex,float distance){
    if(faceIndex>=mesh.faces.size()||!std::isfinite(distance)||std::abs(distance)<.0001f)return false;
    const auto face=mesh.faces[faceIndex];if(face.size()<3||face.size()>128)return false;
    for(int index:face)if(index<0||size_t(index)>=mesh.vertices.size())return false;
    Vec3 a=mesh.vertices[face[0]],b=mesh.vertices[face[1]],c=mesh.vertices[face[2]];
    float ux=b.x-a.x,uy=b.y-a.y,uz=b.z-a.z,vx=c.x-a.x,vy=c.y-a.y,vz=c.z-a.z;
    Vec3 n{uy*vz-uz*vy,uz*vx-ux*vz,ux*vy-uy*vx};
    float length=std::sqrt(n.x*n.x+n.y*n.y+n.z*n.z);if(length<.00001f)return false;
    n.x*=distance/length;n.y*=distance/length;n.z*=distance/length;
    const int base=int(mesh.vertices.size());std::vector<int> top;
    for(int index:face){auto v=mesh.vertices[index];mesh.vertices.push_back({v.x+n.x,v.y+n.y,v.z+n.z});top.push_back(base+int(top.size()));}
    for(size_t i=0;i<face.size();++i){size_t next=(i+1)%face.size();mesh.faces.push_back({face[i],face[next],top[next],top[i]});}
    mesh.faces[faceIndex]=std::move(top);
    return true;
}

bool inset_face(Mesh& mesh,size_t faceIndex,float factor){
    if(faceIndex>=mesh.faces.size()||factor<=0||factor>=1)return false;
    const auto face=mesh.faces[faceIndex];if(face.size()<3||face.size()>128)return false;
    Vec3 center{};for(int index:face){if(index<0||size_t(index)>=mesh.vertices.size())return false;
        const auto& v=mesh.vertices[index];center.x+=v.x;center.y+=v.y;center.z+=v.z;}
    const float inverse=1.f/face.size();center.x*=inverse;center.y*=inverse;center.z*=inverse;
    const int base=int(mesh.vertices.size());std::vector<int> inner;
    for(int index:face){const auto& v=mesh.vertices[index];mesh.vertices.push_back({v.x+(center.x-v.x)*factor,
        v.y+(center.y-v.y)*factor,v.z+(center.z-v.z)*factor});inner.push_back(base+int(inner.size()));}
    for(size_t i=0;i<face.size();++i){size_t next=(i+1)%face.size();mesh.faces.push_back({face[i],face[next],inner[next],inner[i]});}
    mesh.faces[faceIndex]=std::move(inner);return true;
}

bool subdivide_face(Mesh& mesh,size_t faceIndex){
    if(faceIndex>=mesh.faces.size())return false;
    const auto face=mesh.faces[faceIndex];if(face.size()<3||face.size()>128)return false;
    Vec3 center{};for(int index:face){if(index<0||size_t(index)>=mesh.vertices.size())return false;
        const auto& v=mesh.vertices[index];center.x+=v.x;center.y+=v.y;center.z+=v.z;}
    float inverse=1.f/face.size();center.x*=inverse;center.y*=inverse;center.z*=inverse;
    int middle=int(mesh.vertices.size());mesh.vertices.push_back(center);
    mesh.faces[faceIndex]={face[0],face[1],middle};
    for(size_t i=1;i<face.size();++i)mesh.faces.push_back({face[i],face[(i+1)%face.size()],middle});
    return true;
}

Scene make_default_scene() {
    Scene scene;
    scene.objects.push_back(SceneObject{});
    scene.objects.back().mesh=cube_mesh();
    return scene;
}

SceneObject* selected_object(Scene& scene, int id) {
    for (auto& object : scene.objects) if (object.id == id) return &object;
    return nullptr;
}
