#include "project.hpp"
#include "renderer.hpp"
#include "home.hpp"
#include "workspace.hpp"
#include "settings.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_opengl2.h"
#include <epoxy/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <iostream>
#include <string>
#include <filesystem>
#include <chrono>
#include <thread>

static bool same_settings(const Settings& a,const Settings& b){
    return a.theme==b.theme&&a.orbitSensitivity==b.orbitSensitivity&&a.zoomSensitivity==b.zoomSensitivity
        &&a.invertOrbitY==b.invertOrbitY&&a.showGrid==b.showGrid&&a.showAxes==b.showAxes
        &&a.fpsLimit==b.fpsLimit&&a.renderScale==b.renderScale
        &&a.windowWidth==b.windowWidth&&a.windowHeight==b.windowHeight;
}

static void theme() {
    ImGui::StyleColorsDark(); ImGuiStyle& s=ImGui::GetStyle();
    s.WindowRounding=5;s.FrameRounding=4;s.ChildRounding=4;s.PopupRounding=4;s.GrabRounding=3;
    s.WindowPadding=ImVec2(12,10);s.FramePadding=ImVec2(9,6);s.ItemSpacing=ImVec2(8,7);
    s.WindowBorderSize=1;s.FrameBorderSize=1;s.TabRounding=4;
    auto set=[](ImGuiCol idx,int r,int g,int b){ImGui::GetStyle().Colors[idx]=ImVec4(r/255.f,g/255.f,b/255.f,1);};
    set(ImGuiCol_WindowBg,32,32,32);set(ImGuiCol_ChildBg,32,32,32);set(ImGuiCol_PopupBg,37,37,37);
    set(ImGuiCol_TitleBg,32,32,32);set(ImGuiCol_TitleBgActive,32,32,32);set(ImGuiCol_MenuBarBg,25,25,25);
    set(ImGuiCol_FrameBg,43,43,43);set(ImGuiCol_FrameBgHovered,56,56,56);set(ImGuiCol_FrameBgActive,66,66,66);
    set(ImGuiCol_Button,48,48,48);set(ImGuiCol_ButtonHovered,66,66,66);set(ImGuiCol_ButtonActive,78,78,78);
    set(ImGuiCol_Header,52,52,52);set(ImGuiCol_HeaderHovered,66,66,66);set(ImGuiCol_HeaderActive,78,78,78);
    set(ImGuiCol_Tab,37,37,37);set(ImGuiCol_TabHovered,58,58,58);set(ImGuiCol_TabActive,48,48,48);
    set(ImGuiCol_Border,56,56,56);set(ImGuiCol_Separator,56,56,56);set(ImGuiCol_ResizeGrip,70,70,70);
    set(ImGuiCol_Text,230,230,230);set(ImGuiCol_TextDisabled,150,150,150);set(ImGuiCol_CheckMark,190,190,190);
    set(ImGuiCol_DockingEmptyBg,23,23,23);set(ImGuiCol_DockingPreview,90,90,90);
}

static void dock_layout(ImGuiID root) {
    ImGui::DockBuilderRemoveNode(root);ImGui::DockBuilderAddNode(root,ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(root,ImGui::GetMainViewport()->WorkSize);
    ImGuiID center=root,right=0,bottom=0,properties=0;
    ImGui::DockBuilderSplitNode(center,ImGuiDir_Right,.25f,&right,&center);
    ImGui::DockBuilderSplitNode(center,ImGuiDir_Down,.19f,&bottom,&center);
    ImGui::DockBuilderSplitNode(right,ImGuiDir_Down,.48f,&properties,&right);
    ImGui::DockBuilderDockWindow("Viewport",center);ImGui::DockBuilderDockWindow("Explorer",right);
    ImGui::DockBuilderDockWindow("Properties",properties);ImGui::DockBuilderDockWindow("Console",bottom);
    ImGui::DockBuilderFinish(root);
}

int main(){
    Settings settings=load_settings();Settings savedSettings=settings;
    Display* d=XOpenDisplay(nullptr);if(!d){std::cerr<<"MAWI Studio: unable to open X11 display\n";return 1;}
    int screen=DefaultScreen(d);int attrs[]={GLX_RGBA,GLX_DOUBLEBUFFER,GLX_DEPTH_SIZE,24,None};
    XVisualInfo* v=glXChooseVisual(d,screen,attrs);if(!v){std::cerr<<"MAWI Studio: no GLX visual\n";XCloseDisplay(d);return 1;}
    Colormap cmap=XCreateColormap(d,RootWindow(d,screen),v->visual,AllocNone);XSetWindowAttributes wa{};wa.colormap=cmap;
    wa.event_mask=ExposureMask|StructureNotifyMask|KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask|LeaveWindowMask;
    Window win=XCreateWindow(d,RootWindow(d,screen),40,40,settings.windowWidth,settings.windowHeight,0,v->depth,InputOutput,v->visual,CWColormap|CWEventMask,&wa);
    XStoreName(d,win,"MAWI Studio");Atom close=XInternAtom(d,"WM_DELETE_WINDOW",False);XSetWMProtocols(d,win,&close,1);XMapWindow(d,win);
    GLXContext gl=glXCreateContext(d,v,nullptr,True);XFree(v);if(!gl||!glXMakeCurrent(d,win,gl)){std::cerr<<"MAWI Studio: GLX context failed\n";return 1;}
    std::cerr<<"OpenGL: "<<glGetString(GL_VERSION)<<" / "<<glGetString(GL_RENDERER)<<'\n';
    IMGUI_CHECKVERSION();ImGui::CreateContext();ImGuiIO& io=ImGui::GetIO();io.ConfigFlags|=ImGuiConfigFlags_DockingEnable;io.IniFilename=nullptr;
    if(!io.Fonts->AddFontFromFileTTF("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",14))io.Fonts->AddFontDefault();
    ImFontConfig iconConfig;iconConfig.MergeMode=true;iconConfig.PixelSnapH=true;
    static const ImWchar iconRanges[]={0xf013,0xf015,0xf02d,0xf02d,0xf067,0xf067,0xf07c,0xf07c,0xf0c7,0xf0c7,0xf1b2,0xf1b2,0};
    io.Fonts->AddFontFromFileTTF(MAWI_ICON_FONT_PATH,14,&iconConfig,iconRanges);
    theme();apply_theme(settings.theme);ImGui_ImplOpenGL2_Init();
    GLuint sceneTexture=0,sceneFbo=0,sceneDepth=0;int textureW=0,textureH=0;
    glGenTextures(1,&sceneTexture);glGenFramebuffers(1,&sceneFbo);glGenRenderbuffers(1,&sceneDepth);
    Scene scene=make_default_scene();Renderer renderer;int selected=scene.objects.front().id,tool=0,width=settings.windowWidth,height=settings.windowHeight;
    WorkspaceProject workspace;HomeState home;home.recent=read_recent();
    std::snprintf(home.parent,sizeof(home.parent),"%s",std::filesystem::current_path().string().c_str());
    bool editor=false;
    std::string status="Ready";bool running=true,consoleOpen=true,orbiting=false,dragging=false;int lastX=0,lastY=0;
    ImVec2 viewPos(0,0),viewSize(1,1);bool docked=false,applyWindowSize=false;
    auto lastChange=std::chrono::steady_clock::now(),lastFrame=lastChange;
    bool settingsDirty=false;
    auto save=[&](){std::string e;status=save_workspace(workspace,scene,e)?"Saved project scene":e;};
    auto open=[&](const std::string& path){std::string e;WorkspaceProject candidate;Scene loaded;
        if(open_workspace(path,candidate,loaded,e)){workspace=std::move(candidate);scene=std::move(loaded);
            selected=scene.objects.front().id;editor=true;status="Opened "+workspace.name;home.error.clear();
            if(!remember_project(workspace,e))status+=" (recent list: "+e+")";
            home.recent=read_recent();home.openDialog=false;home.newDialog=false;
        }else{home.error=e;status=e;}};
    while(running){
        auto frameStart=std::chrono::steady_clock::now();
        while(XPending(d)){XEvent ev;XNextEvent(d,&ev);
            if(ev.type==ClientMessage&&static_cast<Atom>(ev.xclient.data.l[0])==close)running=false;
            else if(ev.type==ConfigureNotify){width=std::max(1,ev.xconfigure.width);height=std::max(1,ev.xconfigure.height);}
            else if(ev.type==MotionNotify){io.AddMousePosEvent(ev.xmotion.x,ev.xmotion.y);
                if(orbiting)renderer.orbit((ev.xmotion.x-lastX)*settings.orbitSensitivity/.45f,
                    (ev.xmotion.y-lastY)*settings.orbitSensitivity/.45f*(settings.invertOrbitY?-1.f:1.f));
                if(dragging&&tool>0)if(auto* o=selected_object(scene,selected)){float dx=ev.xmotion.x-lastX,dy=ev.xmotion.y-lastY;
                    if(tool==1){o->position.x+=dx*.01f;o->position.y-=dy*.01f;}
                    if(tool==2)o->rotation.y+=dx*.5f;
                    if(tool==3){o->scale.x=std::max(.05f,o->scale.x+dx*.01f);o->scale.y=std::max(.05f,o->scale.y-dy*.01f);}}
                lastX=ev.xmotion.x;lastY=ev.xmotion.y;}
            else if(ev.type==LeaveNotify)io.AddMousePosEvent(-FLT_MAX,-FLT_MAX);
            else if(ev.type==ButtonPress||ev.type==ButtonRelease){bool down=ev.type==ButtonPress;int bx=ev.xbutton.x,by=ev.xbutton.y;io.AddMousePosEvent(bx,by);
                bool inside=editor&&bx>=viewPos.x&&by>=viewPos.y&&bx<viewPos.x+viewSize.x&&by<viewPos.y+viewSize.y;
                if(down&&inside&&ev.xbutton.button==3){orbiting=true;lastX=bx;lastY=by;}
                if(!down&&ev.xbutton.button==3)orbiting=false;
                if(down&&inside&&ev.xbutton.button==4)renderer.zoom(-settings.zoomSensitivity);
                if(down&&inside&&ev.xbutton.button==5)renderer.zoom(settings.zoomSensitivity);
                if(down&&inside&&ev.xbutton.button==1){selected=renderer.pick(scene,bx-int(viewPos.x),by-int(viewPos.y),int(viewSize.x),int(viewSize.y));dragging=selected!=0;lastX=bx;lastY=by;}
                if(!down&&ev.xbutton.button==1)dragging=false;
                if(ev.xbutton.button>=1&&ev.xbutton.button<=3)io.AddMouseButtonEvent(ev.xbutton.button-1,down);
                if(down&&(ev.xbutton.button==4||ev.xbutton.button==5))io.AddMouseWheelEvent(0,ev.xbutton.button==4?1.f:-1.f);
            }else if(ev.type==KeyPress||ev.type==KeyRelease){bool down=ev.type==KeyPress;KeySym key=XLookupKeysym(&ev.xkey,0);
                bool ctrl=(ev.xkey.state&ControlMask)!=0;
                if(key==XK_Control_L||key==XK_Control_R)ctrl=down;
                io.AddKeyEvent(ImGuiKey_ModCtrl,ctrl);
                ImGuiKey mapped=ImGuiKey_None;
                switch(key){
                    case XK_BackSpace:mapped=ImGuiKey_Backspace;break;
                    case XK_Delete:mapped=ImGuiKey_Delete;break;
                    case XK_Return:mapped=ImGuiKey_Enter;break;
                    case XK_Tab:mapped=ImGuiKey_Tab;break;
                    case XK_Left:mapped=ImGuiKey_LeftArrow;break;
                    case XK_Right:mapped=ImGuiKey_RightArrow;break;
                    case XK_Home:mapped=ImGuiKey_Home;break;
                    case XK_End:mapped=ImGuiKey_End;break;
                    case XK_a:case XK_A:mapped=ImGuiKey_A;break;
                    case XK_c:case XK_C:mapped=ImGuiKey_C;break;
                    case XK_v:case XK_V:mapped=ImGuiKey_V;break;
                    case XK_x:case XK_X:mapped=ImGuiKey_X;break;
                    default:break;
                }
                if(mapped!=ImGuiKey_None)io.AddKeyEvent(mapped,down);
                if(key==XK_Escape&&down)running=false;
                if(editor&&down&&(ev.xkey.state&ControlMask)&&key==XK_s)save();
                if(down&&(ev.xkey.state&ControlMask)&&key==XK_o){home.openDialog=true;editor=false;}
                if(down){char buf[32];KeySym k;int n=XLookupString(&ev.xkey,buf,sizeof(buf),&k,nullptr);if(!(ev.xkey.state&ControlMask))for(int i=0;i<n;++i)if(static_cast<unsigned char>(buf[i])>=32)io.AddInputCharacter(static_cast<unsigned char>(buf[i]));}
            }
        }
        io.DisplaySize=ImVec2(width,height);
        io.DeltaTime=std::max(.001f,std::chrono::duration<float>(frameStart-lastFrame).count());lastFrame=frameStart;
        ImGui_ImplOpenGL2_NewFrame();ImGui::NewFrame();
        if(editor){
        if(ImGui::BeginMainMenuBar()){
        ImGui::TextUnformatted("MAWI Studio");ImGui::Separator();ImGui::TextDisabled("%s",workspace.name.c_str());
        if(ImGui::BeginMenu("File")){if(ImGui::MenuItem("Home")){editor=false;home.recent=read_recent();}
            if(ImGui::MenuItem("Open Project","Ctrl+O")){editor=false;home.openDialog=true;}
            if(ImGui::MenuItem("Save","Ctrl+S"))save();
            ImGui::EndMenu();}
        if(ImGui::BeginMenu("Help")){if(ImGui::MenuItem("Help & Documentation"))home.help=true;ImGui::EndMenu();}
        if(ImGui::BeginMenu("Edit")){if(ImGui::MenuItem("Settings"))home.settings=true;ImGui::EndMenu();}
        if(ImGui::Button(u8"\uf0c7  Save"))save();
        ImGui::SameLine();ImGui::BeginDisabled();ImGui::Button("Undo");ImGui::SameLine();ImGui::Button("Redo");ImGui::EndDisabled();ImGui::SameLine();
        ImGui::BeginDisabled();ImGui::Button("Play");ImGui::SameLine();ImGui::Button("Stop");ImGui::EndDisabled();
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))ImGui::SetTooltip("Game runtime is not available yet");
        ImGui::EndMainMenuBar();}
        ImGuiID dockId=ImGui::GetID("MAWI Dockspace");
        ImGui::DockSpaceOverViewport(dockId,ImGui::GetMainViewport());
        if(!docked){dock_layout(dockId);docked=true;}
        ImGui::Begin("Viewport");
        const char* names[]={"Select","Move","Rotate","Scale"};
        for(int i=0;i<4;++i){if(i)ImGui::SameLine();bool active=i==tool;if(active)ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(.38f,.38f,.38f,1));
            if(ImGui::Button(names[i]))tool=i;
            if(active)ImGui::PopStyleColor();}
        ImGui::SameLine();ImGui::TextDisabled("Right drag: orbit  |  Wheel: zoom");ImGui::Separator();
        viewPos=ImGui::GetCursorScreenPos();viewSize=ImGui::GetContentRegionAvail();viewSize.x=std::max(1.f,viewSize.x);viewSize.y=std::max(1.f,viewSize.y);
        ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(sceneTexture)),viewSize,ImVec2(0,1),ImVec2(1,0));
        ImGui::End();
        ImGui::Begin("Explorer");if(ImGui::TreeNodeEx("Workspace",ImGuiTreeNodeFlags_DefaultOpen)){
            for(const auto& o:scene.objects)if(ImGui::Selectable(o.name.c_str(),o.id==selected))selected=o.id;
            ImGui::TreePop();}ImGui::End();
        ImGui::Begin("Properties");if(auto* o=selected_object(scene,selected)){
            ImGui::Text("%s  |  %s",o->name.c_str(),o->type.c_str());ImGui::Separator();
            ImGui::DragFloat3("Position",&o->position.x,.05f);ImGui::DragFloat3("Rotation",&o->rotation.x,.5f);
            ImGui::DragFloat3("Scale",&o->scale.x,.02f,.05f,50.f);ImGui::ColorEdit3("Color",&o->color.r);
        }else ImGui::TextDisabled("Select an object");ImGui::End();
        if(consoleOpen){ImGui::Begin("Console",&consoleOpen);ImGui::TextUnformatted(status.c_str());ImGui::End();}
        else {ImGui::Begin("Console toggle",nullptr,ImGuiWindowFlags_NoDecoration);if(ImGui::Button("Show console"))consoleOpen=true;ImGui::End();}
        }else{
            viewPos=ImVec2(-10000,-10000);viewSize=ImVec2(1,1);orbiting=false;dragging=false;
            draw_home(home);
            if(home.action==HomeAction::Create){home.action=HomeAction::Idle;
                WorkspaceProject created;Scene fresh;std::string e;
                if(create_workspace(home.parent,home.name,created,fresh,e)){workspace=std::move(created);scene=std::move(fresh);
                    selected=scene.objects.front().id;editor=true;status="Created "+workspace.name;home.error.clear();
                    if(!remember_project(workspace,e))status+=" (recent list: "+e+")";
                    home.recent=read_recent();home.newDialog=false;
                }else home.error=e;
            }else if(home.action==HomeAction::Open){home.action=HomeAction::Idle;open(home.chosenPath);}
        }
        draw_help(home.help);
        draw_settings(settings,home.settings,applyWindowSize);
        if(applyWindowSize){XResizeWindow(d,win,settings.windowWidth,settings.windowHeight);applyWindowSize=false;}
        if(!same_settings(settings,savedSettings)){
            bool themeChanged=settings.theme!=savedSettings.theme;
            savedSettings=settings;settingsDirty=true;lastChange=std::chrono::steady_clock::now();
            if(themeChanged){theme();apply_theme(settings.theme);}
        }
        if(settingsDirty&&std::chrono::steady_clock::now()-lastChange>std::chrono::milliseconds(500)){
            std::string error;if(save_settings(settings,error))settingsDirty=false;
            else status=error;
        }
        ImGui::Render();
        int targetW=std::max(1,int(viewSize.x*settings.renderScale)),targetH=std::max(1,int(viewSize.y*settings.renderScale));
        if(editor&&(targetW!=textureW||targetH!=textureH)){textureW=targetW;textureH=targetH;
            glBindTexture(GL_TEXTURE_2D,sceneTexture);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,textureW,textureH,0,GL_RGB,GL_UNSIGNED_BYTE,nullptr);
            glBindRenderbuffer(GL_RENDERBUFFER,sceneDepth);glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,textureW,textureH);
            glBindFramebuffer(GL_FRAMEBUFFER,sceneFbo);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,sceneTexture,0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,sceneDepth);
            if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE){std::cerr<<"MAWI Studio: framebuffer unavailable\n";running=false;}
        }
        if(running&&editor){glBindFramebuffer(GL_FRAMEBUFFER,sceneFbo);renderer.draw(scene,selected,0,0,targetW,targetH,settings.showGrid,settings.showAxes);}
        glBindFramebuffer(GL_FRAMEBUFFER,0);glViewport(0,0,width,height);glDisable(GL_SCISSOR_TEST);glClearColor(.09f,.09f,.09f,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glViewport(0,0,width,height);ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());glXSwapBuffers(d,win);
        if(settings.fpsLimit>0)std::this_thread::sleep_until(frameStart+std::chrono::microseconds(1000000/settings.fpsLimit));
    }
    if(settingsDirty){std::string error;if(!save_settings(settings,error))std::cerr<<error<<'\n';}
    glDeleteFramebuffers(1,&sceneFbo);glDeleteRenderbuffers(1,&sceneDepth);glDeleteTextures(1,&sceneTexture);
    ImGui_ImplOpenGL2_Shutdown();ImGui::DestroyContext();glXMakeCurrent(d,None,nullptr);glXDestroyContext(d,gl);XDestroyWindow(d,win);XFreeColormap(d,cmap);XCloseDisplay(d);
}
