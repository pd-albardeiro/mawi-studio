#include "workspace.hpp"
#include "project.hpp"
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace fs=std::filesystem;
namespace {
std::string timestamp() {
    auto now=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm{}; localtime_r(&now,&tm);
    char buf[32];std::strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S",&tm);return buf;
}
bool valid_name(const std::string& s) {
    if(s.empty()||s.size()>80||s=="."||s=="..")return false;
    for(unsigned char c:s)if(c<32||c=='/'||c=='\\')return false;
    return true;
}
bool manifest(const fs::path& dir,WorkspaceProject& out,std::string& error){
    std::ifstream in(dir/"project.meta");std::string tag;int version=0;
    if(!(in>>tag>>version)||tag!="MAWI_WORKSPACE"||version!=1){error="Missing or unsupported project.meta";return false;}
    WorkspaceProject p;p.directory=dir;
    if(!(in>>std::quoted(p.name)>>std::quoted(p.created))||!valid_name(p.name)){
        error="Invalid project metadata";return false;
    }
    out=std::move(p);return true;
}
}

fs::path recent_file(){
    const char* xdg=std::getenv("XDG_CONFIG_HOME");
    if(xdg&&*xdg)return fs::path(xdg)/"mawi-studio"/"recent.txt";
    const char* home=std::getenv("HOME");
    return home&&*home?fs::path(home)/".config"/"mawi-studio"/"recent.txt":fs::path{};
}

std::vector<WorkspaceProject> read_recent(){
    std::vector<WorkspaceProject> list;fs::path file=recent_file();if(file.empty())return list;
    std::ifstream in(file);std::string header;int version=0;
    if(!(in>>header>>version)||header!="MAWI_RECENT"||version!=1)return list;
    std::string path,last;while(in>>std::quoted(path)>>std::quoted(last)){
        WorkspaceProject p;std::string error;
        if(manifest(path,p,error)){p.lastOpened=last;list.push_back(std::move(p));}
        if(list.size()>=12)break;
    }
    return list;
}

bool remember_project(const WorkspaceProject& p,std::string& error){
    fs::path file=recent_file();if(file.empty()){error="Cannot locate Linux config directory";return false;}
    auto list=read_recent();std::error_code ec;fs::path key=fs::weakly_canonical(p.directory,ec);
    fs::create_directories(file.parent_path(),ec);if(ec){error="Cannot create config directory: "+ec.message();return false;}
    std::ofstream out(file,std::ios::trunc);if(!out){error="Cannot write recent projects";return false;}
    out<<"MAWI_RECENT 1\n"<<std::quoted(key.string())<<' '<<std::quoted(timestamp())<<'\n';
    for(const auto& old:list){if(old.directory==key)continue;
        out<<std::quoted(old.directory.string())<<' '<<std::quoted(old.lastOpened)<<'\n';}
    return true;
}

bool forget_project(const fs::path& directory,std::string& error){
    fs::path file=recent_file();if(file.empty()){error="Cannot locate Linux config directory";return false;}
    auto list=read_recent();std::ofstream out(file,std::ios::trunc);
    if(!out){error="Cannot update recent projects";return false;}
    out<<"MAWI_RECENT 1\n";
    for(const auto& p:list)if(p.directory!=directory)
        out<<std::quoted(p.directory.string())<<' '<<std::quoted(p.lastOpened)<<'\n';
    return static_cast<bool>(out);
}

bool create_workspace(const fs::path& parent,const std::string& name,WorkspaceProject& project,Scene& scene,std::string& error){
    if(!valid_name(name)){error="Project name must be 1-80 characters without / or \\";return false;}
    std::error_code ec;if(!fs::is_directory(parent,ec)){error="Destination folder does not exist";return false;}
    fs::path dir=parent/name;if(fs::exists(dir,ec)){error="Destination already exists; choose another name";return false;}
    if(!fs::create_directory(dir,ec)){error="Cannot create project directory: "+ec.message();return false;}
    fs::create_directory(dir/"assets",ec);if(ec){error="Cannot create assets directory: "+ec.message();return false;}
    fs::create_directory(dir/"scenes",ec);if(ec){error="Cannot create scenes directory: "+ec.message();return false;}
    Scene fresh=make_default_scene();
    if(!save_project(fresh,(dir/"scenes"/"main.mawi").string(),error))return false;
    WorkspaceProject result{fs::canonical(dir,ec),name,timestamp(),{}};
    std::ofstream out(dir/"project.meta");if(!out){error="Cannot write project.meta";return false;}
    out<<"MAWI_WORKSPACE 1\n"<<std::quoted(result.name)<<' '<<std::quoted(result.created)<<'\n';
    out.close();if(!out){error="Cannot finish project.meta";return false;}
    scene=std::move(fresh);project=std::move(result);return true;
}

bool open_workspace(const fs::path& directory,WorkspaceProject& project,Scene& scene,std::string& error){
    std::error_code ec;fs::path dir=fs::canonical(directory,ec);
    if(ec||!fs::is_directory(dir)){error="Project directory does not exist";return false;}
    WorkspaceProject loaded;if(!manifest(dir,loaded,error))return false;
    Scene fresh;if(!load_project(fresh,(dir/"scenes"/"main.mawi").string(),error))return false;
    scene=std::move(fresh);project=std::move(loaded);return true;
}

bool save_workspace(const WorkspaceProject& project,const Scene& scene,std::string& error){
    if(project.directory.empty()){error="No project is open";return false;}
    return save_project(scene,(project.directory/"scenes"/"main.mawi").string(),error);
}
