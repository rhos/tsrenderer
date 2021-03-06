#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255),
               red   = TGAColor(255, 0,   0,   255);

void naive_line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0-x1)<std::abs(y0-y1)) 
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    
    if (x0>x1) 
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    for (int x=x0; x<=x1; x++) 
    {
        float t = (x-x0)/(float)(x1-x0);
        int y = y0*(1.-t) + y1*t;
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}

void bresenham_line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0-x1) < std::abs(y0-y1)) 
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    
    if (x0 > x1) 
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = y1 - y0;
    int yi = 1;
    if (dy < 0)
    {
        yi = -1;
        dy = -dy;
    }
    int D = 2 * dy - dx;
    int y = y0;

    for(int x = x0; x <= x1; ++x)
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

int main(int argc, char** argv) 
{
    const int width  = 800,
              height = 800;
    Model *model = argc == 2 ?
        new Model(argv[1]) :
        new Model("obj/african_head.obj");

    TGAImage image(width, height, TGAImage::RGB);

    for (int i=0; i<model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        for (int j=0; j<3; j++) 
        {
            Vec3f v0 = model->vert(face[j]);
            Vec3f v1 = model->vert(face[(j+1)%3]);
            int x0 = (v0.x+1.)*width/2.;
            int y0 = (v0.y+1.)*height/2.;
            int x1 = (v1.x+1.)*width/2.;
            int y1 = (v1.y+1.)*height/2.;
            bresenham_line(x0, y0, x1, y1, image, red);
            naive_line(x0, y0, x1, y1, image, white);
        }
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}

