#include "renderer.hpp"
#include <epoxy/gl.h>
#include <algorithm>
#include <cmath>

void Renderer::orbit(float dx, float dy) { yaw_ += dx*.45f; pitch_=std::clamp(pitch_+dy*.45f,-85.f,85.f); }
void Renderer::zoom(float amount) { distance_=std::clamp(distance_+amount,3.f,30.f); }

void Renderer::cube(const SceneObject& o, bool selected) const {
    const float v[8][3]={{-.5f,-.5f,-.5f},{.5f,-.5f,-.5f},{.5f,.5f,-.5f},{-.5f,.5f,-.5f},{-.5f,-.5f,.5f},{.5f,-.5f,.5f},{.5f,.5f,.5f},{-.5f,.5f,.5f}};
    const int faces[6][4]={{0,1,2,3},{4,7,6,5},{0,4,5,1},{3,2,6,7},{1,5,6,2},{0,3,7,4}};
    glPushMatrix(); glTranslatef(o.position.x,o.position.y,o.position.z); glRotatef(o.rotation.y,0,1,0); glScalef(o.scale.x,o.scale.y,o.scale.z);
    glBegin(GL_QUADS); for(int f=0;f<6;++f){ float shade=.72f+.055f*f; glColor3f(o.color.r*shade,o.color.g*shade,o.color.b*shade); for(int j=0;j<4;++j) glVertex3fv(v[faces[f][j]]); } glEnd();
    if(selected){ glColor3f(.89f,.9f,.92f); glLineWidth(2); for(int f=0;f<6;++f){glBegin(GL_LINE_LOOP);for(int j=0;j<4;++j)glVertex3fv(v[faces[f][j]]);glEnd();}glLineWidth(1); }
    glPopMatrix();
}

void Renderer::draw(const Scene& scene, int selectedId, int x, int y, int width, int height) {
    if(width<1||height<1)return;
    glViewport(x,y,width,height); glEnable(GL_SCISSOR_TEST);glScissor(x,y,width,height);
    glClearColor(.105f,.11f,.12f,1); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);glDisable(GL_BLEND);glDisable(GL_CULL_FACE);glShadeModel(GL_FLAT);
    glMatrixMode(GL_PROJECTION);glLoadIdentity();
    const double aspect=double(width)/height; glFrustum(-.75*aspect,.75*aspect,-.75,.75,1,100);
    glMatrixMode(GL_MODELVIEW);glLoadIdentity();glTranslatef(0,-.7f,-distance_);glRotatef(pitch_,1,0,0);glRotatef(yaw_,0,1,0);
    glColor3f(.22f,.23f,.25f);glBegin(GL_LINES);for(int i=-10;i<=10;++i){glVertex3f(i,0,-10);glVertex3f(i,0,10);glVertex3f(-10,0,i);glVertex3f(10,0,i);}glEnd();
    glLineWidth(2);glBegin(GL_LINES);glColor3f(.62f,.34f,.34f);glVertex3f(0,.02f,0);glVertex3f(3,.02f,0);glColor3f(.35f,.62f,.4f);glVertex3f(0,.02f,0);glVertex3f(0,3,0);glColor3f(.35f,.48f,.7f);glVertex3f(0,.02f,0);glVertex3f(0,.02f,3);glEnd();glLineWidth(1);
    for(const auto& o:scene.objects)cube(o,o.id==selectedId);
    glDisable(GL_SCISSOR_TEST);glDisable(GL_DEPTH_TEST);
}

int Renderer::pick(const Scene& scene,int x,int y,int width,int height) const {
    if(width<1||height<1)return 0;
    // Small scenes: project each object's center with the same camera transform.
    const double ay=yaw_*3.141592653589793/180, ap=pitch_*3.141592653589793/180;
    int closest=0;double best=1e9;
    for(const auto& o:scene.objects){double px=o.position.x,py=o.position.y,pz=o.position.z;
        double rx=std::cos(ay)*px+std::sin(ay)*pz, rz=-std::sin(ay)*px+std::cos(ay)*pz;
        double ry=std::cos(ap)*py-std::sin(ap)*rz-.7, depth=distance_-(std::sin(ap)*py+std::cos(ap)*rz);
        if(depth<=1)continue;
        double sx=width*.5+rx*height/(1.5*depth), sy=height*.5-ry*height/(1.5*depth);
        double d=std::hypot(x-sx,y-sy);if(d<45&&d<best){best=d;closest=o.id;}
    }
    return closest;
}
