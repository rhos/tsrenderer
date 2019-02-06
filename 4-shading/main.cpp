#include <vector>
#include <iostream>
#include <cmath>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255),
               red   = TGAColor(255, 0,   0,   255),
               green = TGAColor(0  , 255, 0,   255),
               blue  = TGAColor(0  , 0,   255, 255);

const int width  = 800,
          height = 800,
          depth  = 800;

Vec3f light_dir = Vec3f(1, 1, 1).normalize();
Vec3f eye(1, 1, 3);
Vec3f center(0, 0, 0);

Model *model;

Matrix viewport(int x, int y, int w, int h)
{
    Matrix m = Matrix::identity(4);
    m[0][3] = x + w / 2.f;
    m[1][3] = y + h / 2.f;
    m[2][3] = depth / 2.f;

    m[0][0] = w / 2.f;
    m[1][1] = h / 2.f;
    m[2][2] = depth / 2.f;
    return m;
}

Matrix lookat(Vec3f eye, Vec3f center, Vec3f up)
{
    Vec3f z = (eye - center).normalize();
    Vec3f x = (up ^ z).normalize();
    Vec3f y = (z ^ x).normalize();
    Matrix res = Matrix::identity(4);
    for (int i = 0; i < 3; i++)
    {
        res[0][i] = x[i];
        res[1][i] = y[i];
        res[2][i] = z[i];
        res[i][3] = -center[i];
    }
    return res;
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

void triangle(Vec3i* t, Vec2i*uv, float* intensity, int* zbuffer, TGAImage &image)
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
            int zval = 0;
            float ival = 0;
            int uval = 0, vval = 0;
            for(int i = 0; i < 3; ++i) 
            {
                zval += t[i].z*bc[i];
                ival += intensity[i]*bc[i];
                uval += uv[i].x*bc[i];
                vval += uv[i].y*bc[i];
            }
            int zind = p.x + p.y * width;
            if(zval >= zbuffer[zind])
            {
                zbuffer[zind] = zval;
                auto color = model->diffuse(Vec2i(uval,vval));
                image.set(p.x, p.y, color*intensity[0]); 
            }
        } 
}

int main(int argc, char **argv)
{
    model = argc == 2 ? 
        new Model(argv[1]) : 
        new Model("obj/african_head.obj");

    TGAImage image(width, height, TGAImage::RGB);

    int *zbuffer = new int[width * height];
    for (int i = 0; i < width * height; ++i)
        zbuffer[i] = -std::numeric_limits<int>::max();

    Matrix ModelView = lookat(eye, center, Vec3f(0, 1, 0));

    Matrix Projection = Matrix::identity(4);
    Projection[3][2] = -1.f / (eye - center).norm();

    Matrix ViewPort = viewport(0,0,width,height);

    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        Vec3i t[3];
        Vec2i uv[3];
        float intensity[3];
        for (int j = 0; j < 3; j++)
        {
            Vec3f v = model->vert(face[j]);
            t[j] = Vec3f(ViewPort * Projection * ModelView * Matrix(v));
            intensity[j] = model->norm(i, j) * light_dir;
            uv[j] = model->uv(i, j);
        }
        triangle(t, uv, intensity, zbuffer, image);
    }
    image.flip_vertically();
    image.write_tga_file("output.tga");

    TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
    for (int i = 0; i < width; i++)
        for (int j = 0; j < height; j++)
            zbimage.set(i, j, TGAColor(zbuffer[i + j * width]));
    zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    zbimage.write_tga_file("zbuffer.tga");

    delete model;
    delete[] zbuffer;
    return 0;
}
