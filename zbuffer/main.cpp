#include <vector>
#include <algorithm>
#include "tgaimage.h"
#include "geometry.h"
#include "model.h"

const TGAColor white = TGAColor(255, 255, 255, 255),
               red   = TGAColor(255, 0,   0,   255),
               green = TGAColor(0  , 255, 0,   255),
               blue  = TGAColor(0  , 0,   255, 255);

const int width  = 800,
          height = 800,
          depth  = 800;

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

Vec3f barycentric(Vec3i* t, Vec2i p)
{
    Vec3i vs[2];
    for (int i = 0; i < 2; ++i)
    {
        vs[i].x = t[2][i] - t[0][i];
        vs[i].y = t[2][i] - t[1][i];
        vs[i].z = p[i] - t[2][i];
    }
    auto uv = vs[0] ^ vs[1];
    if (uv.z == 0) 
        return Vec3f(-1,1,1);
    return Vec3f(1.f-(uv.x+uv.y)/(float)uv.z, uv.y/(float)uv.z, uv.x/(float)uv.z);
}

void triangle(Vec3i* t, int* zbuffer, TGAImage &image, Vec2i* uv, TGAImage &texture, float intensity)
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
            int zval = 0, uval = 0, vval = 0;
            for(int i = 0; i < 3; ++i) 
            {
                zval += t[i].z*bc[i];
                uval += uv[i].x*bc[i];
                vval += uv[i].y*bc[i];
            }
            int zind = p.x + p.y * width;
            if(zval >= zbuffer[zind])
            {
                zbuffer[zind] = zval;
                auto color = texture.get(uval, vval);
                image.set(p.x, p.y, TGAColor(color.r * intensity, color.g * intensity, color.b * intensity, 255)); 
            }
        } 
}

int main(int argc, char** argv) 
{
    Model *model = argc == 2 ?
        new Model(argv[1]) :
        new Model("obj/african_head.obj");

    TGAImage image(width, height, TGAImage::RGB);
    TGAImage texture;
    texture.read_tga_file("obj/african_head_diffuse.tga");
    texture.flip_vertically();

    int* zbuffer = new int[width * height];
    for (int i = 0; i < width * height; ++i)
        zbuffer[i] = -std::numeric_limits<int>::max();

    for (int i=0; i<model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        std::vector<int> tface = model->tface(i);
        Vec3i t[3];
        Vec3f w[3];
        Vec2i uv[3];
        for (int j=0; j<3; j++) 
        {
            Vec3f v = model->vert(face[j]);
            w[j] = v;
            t[j].x = (v.x+1.)*width/2.;
            t[j].y = (v.y+1.)*height/2.;
            t[j].z = (v.z+1.)*depth/2.;

            Vec3f tex = model->tex(tface[j]);
            uv[j].x = tex.x * texture.get_width();
            uv[j].y = tex.y * texture.get_height();
        }
        Vec3f n = (w[2] - w[0]) ^ (w[1] - w[0]);
        n.normalize();
        float intensity = n * Vec3f(.0f, .0f, -1.0f);
        if (intensity > .0f)
            triangle(t, zbuffer, image, uv, texture, intensity);
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}

