#include "workspace.hpp"
#undef NDEBUG
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <unistd.h>

int main(){
    char pattern[]="/tmp/mawi-workspace-test-XXXXXX";char* raw=mkdtemp(pattern);assert(raw);
    std::filesystem::path root(raw);setenv("XDG_CONFIG_HOME",(root/"config").c_str(),1);
    WorkspaceProject made,opened;Scene scene,reloaded;std::string error;
    assert(create_workspace(root,"Test World",made,scene,error));
    assert(std::filesystem::exists(made.directory/"project.meta"));
    assert(std::filesystem::exists(made.directory/"assets"));
    assert(std::filesystem::exists(made.directory/"scenes"/"main.mawi"));
    assert(!made.created.empty());const auto created=made.created;
    assert(!create_workspace(root,"Test World",opened,reloaded,error));
    assert(open_workspace(made.directory,opened,reloaded,error));
    assert(opened.created==created&&reloaded.objects.size()==1);
    assert(remember_project(opened,error));assert(remember_project(opened,error));
    auto recent=read_recent();assert(recent.size()==1&&recent[0].name=="Test World");
    assert(recent[0].created==created&&!recent[0].lastOpened.empty());
    assert(forget_project(made.directory,error));assert(read_recent().empty());
    assert(!open_workspace(root/"missing",opened,reloaded,error));
    assert(reloaded.objects.size()==1);
    std::filesystem::remove_all(root);
    std::cout<<"workspace tests passed\n";
}
