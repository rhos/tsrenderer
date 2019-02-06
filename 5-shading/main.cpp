#include <vector>
#include <iostream>
#include <algorithm>
#include <limits>
#include "model.h"
#include "gl.h"

const int width  = 800,
          height = 800,
          depth  = 800;

Vec3f light_dir = Vec3f(1, 1, 1).normalize();
Vec3f eye(1, 1, 3);
Vec3f center(0, 0, 0);

Model *model;

struct GouraudShader : public IShader 
{
    Vec3f intensity_vec;

    virtual Vec3f vertex(int iface, int ivert) 
    {
        auto face = model->face(iface);
        auto vert = model->vert(ivert);
        auto gl_Vertex = Vec3f(Viewport*Projection*ModelView*Matrix(vert));
        intensity_vec[ivert] = std::max(0.f, model->norm(iface, ivert)*light_dir); // get diffuse lighting intensity
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bc, TGAColor &color) 
    {
        float intensity = .0f;
        for(int i = 0; i< 3; ++i)
            intensity = intensity_vec[i] * bc[i];
        color = TGAColor(255, 255, 255);
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
