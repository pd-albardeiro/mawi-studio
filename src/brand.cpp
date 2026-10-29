#include "brand.hpp"

void draw_mawi_mark(ImVec2 pos,float size){
    auto* dl=ImGui::GetWindowDrawList();const ImU32 light=IM_COL32(216,220,227,255),dark=IM_COL32(32,34,38,255);
    auto p=[&](float x,float y){return ImVec2(pos.x+x*size,pos.y+y*size);};
    dl->AddQuadFilled(p(.5f,.03f),p(.97f,.5f),p(.5f,.97f),p(.03f,.5f),light);
    dl->AddQuadFilled(p(.27f,.38f),p(.38f,.38f),p(.38f,.70f),p(.27f,.70f),dark);
    dl->AddQuadFilled(p(.62f,.38f),p(.73f,.38f),p(.73f,.70f),p(.62f,.70f),dark);
    dl->AddTriangleFilled(p(.27f,.38f),p(.39f,.38f),p(.5f,.54f),dark);
    dl->AddTriangleFilled(p(.61f,.38f),p(.73f,.38f),p(.5f,.54f),dark);
    dl->AddTriangleFilled(p(.38f,.70f),p(.38f,.55f),p(.5f,.68f),dark);
    dl->AddTriangleFilled(p(.62f,.70f),p(.62f,.55f),p(.5f,.68f),dark);
}
