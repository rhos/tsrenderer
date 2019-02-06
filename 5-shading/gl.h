#pragma once
#include "tgaimage.h"
#include "geometry.h"
const TGAColor white = TGAColor(255, 255, 255, 255),
               red   = TGAColor(255, 0,   0,   255),
               green = TGAColor(0  , 255, 0,   255),
               blue  = TGAColor(0  , 0,   255, 255);


Matrix ModelView;
Matrix Viewport;
Matrix Projection;

struct IShader 
{
    virtual ~IShader() = default;
    virtual Vec3f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

void viewport(int x, int y, int w, int h, int depth)
{
    Viewport = Matrix::identity(4);
    Viewport[0][3] = x + w / 2.f;
    Viewport[1][3] = y + h / 2.f;
    Viewport[2][3] = depth / 2.f;

    Viewport[0][0] = w / 2.f;
    Viewport[1][1] = h / 2.f;
    Viewport[2][2] = depth / 2.f;
}

void lookat(Vec3f eye, Vec3f center, Vec3f up)
{
    Vec3f z = (eye - center).normalize();
    Vec3f x = (up ^ z).normalize();
    Vec3f y = (z ^ x).normalize();
    ModelView = Matrix::identity(4);
    for (int i = 0; i < 3; i++)
    {
        ModelView[0][i] = x[i];
        ModelView[1][i] = y[i];
        ModelView[2][i] = z[i];
        ModelView[i][3] = -center[i];
    }
}

void projection(float coeff)
{
    Projection = Matrix::identity(4);
    Projection[3][2] = coeff;
}

Vec3f barycentric(Vec3i* t, Vec2i p)
{
    Vec3i vs[2];
    for (int i = 0; i < 2; ++i)
    {
        vs[i].x = t[1][i] - t[0][i];
        vs[i].y = t[2][i] - t[0][i];
        vs[i].z = t[0][i] - p[i];
    }
    auto uv = vs[0] ^ vs[1];
    if (uv.z == 0) 
        return Vec3f(-1,1,1);

    return Vec3f(
        1.f-(uv.x+uv.y)/(float)uv.z,
        uv.x/(float)uv.z,
        uv.y/(float)uv.z);
}

void triangle(Vec3i* t, IShader& shader, TGAImage &zbuffer, TGAImage &image)
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
            Vec3f bc = barycentric(t, p); 
            if (bc.x<0 || bc.y<0 || bc.z<0) continue; 
            int zval = 0;
            for(int i = 0; i < 3; ++i) 
                zval += t[i].z*bc[i];
            if(zval <= zbuffer.get(p.x, p.y).bgra[0]) continue;

            TGAColor color;
            bool valid = shader.fragment(bc, color);
            if(valid)
            {
                zbuffer.set(p.x, p.y, TGAColor(zval));
                image.set(p.x, p.y, color);
            }
        } 
}