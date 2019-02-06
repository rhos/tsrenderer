#include <vector>
#include <iostream>
#include <algorithm>
#include <limits>
#include "model.h"
#include "gl.h"

const int width  = 800,
          height = 800,
          depth  = 255;

Vec3f light_dir = Vec3f(1, 1, 1).normalize();
Vec3f eye(1, -1, 5);
Vec3f center(0, 0, 0);

Model *model;

struct GouraudShader : public IShader 
{
    Vec3f intensity_vec;
    Vec2f uv_vec[3];
    Matrix m;
    Matrix mit;

    virtual Vec3f vertex(int iface, int ivert) 
    {
        auto face = model->face(iface);
        auto vert = model->vert(face[ivert]);
        auto gl_Vertex = Vec3f(Viewport*m*Matrix(vert));
        intensity_vec[ivert] = model->normal(iface, ivert)*light_dir;
        uv_vec[ivert] = model->uv(iface, ivert);
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bc, TGAColor &color) 
    {
        //float intensity = .0f;
        float uval = 0, vval = 0;
        for(int i = 0; i< 3; ++i)
        {
            //intensity += intensity_vec[i] * bc[i];
            uval += uv_vec[i].x*bc[i];
            vval += uv_vec[i].y*bc[i];
        }

        // if (intensity>.85) intensity = 1;
        // else if (intensity>.60) intensity = .80;
        // else if (intensity>.45) intensity = .60;
        // else if (intensity>.30) intensity = .45;
        // else if (intensity>.15) intensity = .30;
        // else intensity = 0;
        
        Vec2f uv(uval,vval);
        auto n = model->normal(uv);
        n = Vec3f(mit * Matrix(n)).normalize();
        auto l = Vec3f(m*Matrix(light_dir)).normalize();
        auto r = (n * (n*l*2.f) - l).normalize();
        auto diff = std::max(.0f, n*l);
        auto spec = pow(std::max(r.z, 0.0f), model->specular(uv));
        color = model->diffuse(uv);
        for (int i=0; i<3; i++) 
            color.bgra[i] = std::min<float>(5 + color.bgra[i]*(diff + .6*spec), 255);
        return true; 
    }
    
};

int main(int argc, char **argv)
{
    model = argc == 2 ? 
        new Model(argv[1]) : 
        new Model("obj/african_head.obj");

    TGAImage image(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    lookat(eye, center, Vec3f(0, 1, 0));
    projection(-1.f / (eye - center).norm());
    viewport(0,0,width,height, depth);


    GouraudShader shader;
    shader.m = Projection*ModelView;
    shader.mit = shader.m.inverse().transpose();
    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        Vec3i coords[3];
        for (int j = 0; j < 3; j++)
            coords[j] = shader.vertex(i,j);
        triangle(coords, shader, zbuffer, image);
    }
    image.flip_vertically();
    image.write_tga_file("output.tga");
    zbuffer.flip_vertically();
    zbuffer.write_tga_file("zbuffer.tga");

    delete model;
    return 0;
}
