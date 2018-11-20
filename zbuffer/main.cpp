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
    auto uv = Vec3i(v0.x, v1.x, v2.x) ^ Vec3i(v0.y, v1.y, v2.y);
    if (uv.z == 0) return Vec3f(-1,1,1);
    Vec2f uvs(uv.x/(float)uv.z, uv.y/(float)uv.z);
    return Vec3f(1.f - uvs.x - uvs.y, uvs.y, uvs.x);
}

void triangle(Vec2i* t, TGAImage &image, TGAColor color)
{
    Vec2i lb(image.get_width()-1,  image.get_height()-1); 
    Vec2i rt(0, 0); 
    Vec2i clamp(image.get_width()-1, image.get_height()-1); 
    for (int i=0; i<3; i++) 
        for (int j=0; j<2; j++) 
        { 
            lb.raw[j] = std::max(0, std::min(lb.raw[j], t[i].raw[j])); 
            rt.raw[j] = std::min(clamp.raw[j], std::max(rt.raw[j], t[i].raw[j])); 
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

int main(int argc, char** argv) 
{
    const int   width  = 800,
                height = 800;

    Model *model = argc == 2 ?
        new Model(argv[1]) :
        new Model("obj/predator.obj");

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
        Vec3f n = (w[2] - w[0]) ^ (w[1] - w[0]);
        n.normalize();
        float intensity = n * Vec3f(.0f, .0f, -1.0f);
        if (intensity > .0f)
            triangle(t, image, TGAColor(255*intensity, 0, 0, 255));
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}

