#include "mesh_io.hpp"
#include "project.hpp"
#undef NDEBUG
#include <cassert>
#include <filesystem>
#include <fstream>
#include <unistd.h>

int main(){
    char pattern[]="/tmp/mawi-mesh-test-XXXXXX";char* temp=mkdtemp(pattern);assert(temp);
    std::filesystem::path root(temp);
    Scene scene=make_default_scene();auto& object=scene.objects.front();object.name="Test Cube";
    object.pivot={.1f,.2f,.3f};object.rotation={10,20,30};
    assert(object.mesh.vertices.size()==8&&object.mesh.faces.size()==6);
    assert(extrude_face(object.mesh,3,.5f));
    assert(object.mesh.vertices.size()==12&&object.mesh.faces.size()==10);
    assert(inset_face(object.mesh,3,.25f));
    assert(object.mesh.vertices.size()==16&&object.mesh.faces.size()==14);
    assert(subdivide_face(object.mesh,3));
    assert(object.mesh.vertices.size()==17&&object.mesh.faces.size()==17);
    std::string error;auto project=root/"scene.mawi";
    assert(save_project(scene,project.string(),error));
    Scene loaded;assert(load_project(loaded,project.string(),error));
    assert(loaded.objects.front().mesh.vertices.size()==17);
    assert(loaded.objects.front().mesh.faces.size()==17);
    assert(loaded.objects.front().name=="Test Cube");
    assert(loaded.objects.front().pivot.x==.1f);
    auto obj=root/"mesh.obj";assert(export_obj(obj,loaded.objects.front(),error));
    Scene imported=make_default_scene();int selected=0;
    assert(import_obj(obj,imported,selected,error));assert(imported.objects.size()==2);
    assert(imported.objects.back().mesh.faces.size()==17&&selected==imported.objects.back().id);
    auto count=imported.objects.size();
    assert(!import_obj(root/"fake.mdl",imported,selected,error));assert(imported.objects.size()==count);
    std::ofstream legacy(root/"legacy.mawi");
    legacy<<"MAWI_PROJECT 1\nOBJECT 1 Cube Part 0 0.5 0 0 0 0 1 1 1 0.5 0.5 0.5 0\n";legacy.close();
    Scene old;assert(load_project(old,(root/"legacy.mawi").string(),error));
    assert(old.objects.front().mesh.faces.size()==6);
    std::filesystem::remove_all(root);
}
