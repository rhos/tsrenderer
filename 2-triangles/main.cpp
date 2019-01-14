#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "geometry.h"
#include "model.h"

const TGAColor white = TGAColor(255, 255, 255, 255),
               red   = TGAColor(255, 0,   0,   255),
               green = TGAColor(0  , 255, 0,   255);



void line(Vec2i v0, Vec2i v1, TGAImage &image, TGAColor color) 
{
    bool steep = false;
    if (std::abs(v0.x-v1.x) < std::abs(v0.y-v1.y)) 
    {
        std::swap(v0.x, v0.y);
        std::swap(v1.x, v1.y);
        steep = true;
    }
    
    if (v0.x > v1.x) 
    {
        std::swap(v0, v1);
    }

    int dx = v1.x - v0.x;
    int dy = v1.y - v0.y;
    int yi = 1;
    if (dy < 0)
    {
        yi = -1;
        dy = -dy;
    }
    int D = 2 * dy - dx;
    int y = v0.y;

    for(int x = v0.x; x <= v1.x; ++x)
    {
        if (steep) image.set(y, x, color);
        else image.set(x, y, color);
        if (D > 0)
        {
            y = y + yi;
            D = D - 2*dx;
        }
        D = D + 2*dy;
    }
}

Vec3f barycentric(Vec2i* t, Vec2i p)
{
    Vec2i v0 = t[2]-t[0];
    Vec2i v1 = t[2]-t[1];
    Vec2i v2 = p-t[2];
    auto uv = cross(Vec3i(v0.x, v1.x, v2.x) , Vec3i(v0.y, v1.y, v2.y));
    if (std::abs(uv.z)!= 0)
    {
        return Vec3f(1.f-(uv.x+uv.y)/(float)uv.z, uv.y/(float)uv.z, uv.x/(float)uv.z);
    }
    return Vec3f(-1,1,1);
}

void parallel_triangle(Vec2i* t, TGAImage &image, TGAColor color)
{
    Vec2i lb(image.get_width()-1,  image.get_height()-1); 
    Vec2i rt(0, 0); 
    Vec2i clamp(image.get_width()-1, image.get_height()-1); 
    for (int i=0; i<3; i++) 
        for (int j=0; j<2; j++) 
        { 
            lb[j] = std::max(0, std::min(lb[j], t[i][j])); 
            rt[j] = std::min(clamp[j], std::max(rt[j], t[i][j])); 
        } 

    Vec2i p; 
    for (p.x=lb.x; p.x<=rt.x; p.x++) 
        for (p.y=lb.y; p.y<=rt.y; p.y++) 
        { 
            Vec3f bc  = barycentric(t, p); 
            if (bc.x<0 || bc.y<0 || bc.z<0) continue; 
            image.set(p.x, p.y, color); 
        } 
}

void naive_triangle(Vec2i p0, Vec2i p1, Vec2i p2, TGAImage &image, TGAColor color) 
{ 
    if(p0.y > p1.y) std::swap(p0,p1);
    if(p0.y > p2.y) std::swap(p0,p2);
    if(p1.y > p2.y) std::swap(p1,p2);
    
    int ah,bh,ch,aw,bw,cw;
    ah = p2.y - p0.y;
    bh = p1.y - p0.y;
    ch = p2.y - p1.y;
    aw = p2.x - p0.x;
    bw = p1.x - p0.x;
    cw = p2.x - p1.x;

    for (int y=p0.y; y<=p1.y; y++) 
    { 
        float ap = (float)(y-p0.y)/ah; 
        float bp  = (float)(y-p0.y)/bh;
        int ax = p0.x + aw*ap; 
        int bx = p0.x + bw*bp;
        if (ax > bx) std::swap(ax,bx);
        for (int x = ax; x <= bx; ++x)
            image.set(x, y, color); 
    }

    for (int y=p1.y; y<=p2.y; y++) 
    { 
        float ap = (float)(y-p0.y)/ah; 
        float cp  = (float)(y-p1.y)/ch;
        int ax = p0.x + aw*ap; 
        int cx = p1.x + cw*cp;
        if (ax > cx) std::swap(ax,cx);
        for (int x = ax; x <= cx; ++x)
            image.set(x, y, color); 
    }
}


int main(int argc, char** argv) 
{
    // const int width  = 200,
    //           height = 200;

    // TGAImage image(width, height, TGAImage::RGB);

    // Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)}; 
    // Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)}; 
    // Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)}; 
    
    // parallel_triangle(t0,image, red);
    // parallel_triangle(t1,image, white);
    // parallel_triangle(t2,image, green);
    // naive_triangle(t0[0], t0[1], t0[2], image, red); 
    // naive_triangle(t1[0], t1[1], t1[2], image, white); 
    // naive_triangle(t2[0], t2[1], t2[2], image, green);

    const int width  = 800,
            height = 800;
    Model *model = argc == 2 ?
        new Model(argv[1]) :
        new Model("obj/african_head.obj");

    TGAImage image(width, height, TGAImage::RGB);

    for (int i=0; i<model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        Vec2i t[3];
        Vec3f w[3];
        for (int j=0; j<3; j++) 
        {
            Vec3f v = model->vert(face[j]);
            w[j] = v;
            t[j].x = (v.x+1.)*width/2.;
            t[j].y = (v.y+1.)*height/2.;
        }
        Vec3f n = cross((w[2] - w[0]), (w[1] - w[0]));
        n.normalize();
        float intensity = n * Vec3f(.0f, .0f, -1.0f);
        if (intensity > .0f)
            parallel_triangle(t, image, TGAColor(255*intensity, 0, 0, 255));
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}

