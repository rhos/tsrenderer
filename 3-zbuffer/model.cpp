#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        std::string trash;
        if (!line.compare(0, 2, "v ")) 
        {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v.raw[i];
            verts_.push_back(v);
        } 
        else if (!line.compare(0, 3, "vt ")) 
        {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v.raw[i];
            texs_.push_back(v);
        } 
        else if (!line.compare(0, 2, "f ")) 
        {
            std::vector<int> f;
            std::vector<int> t;
            int vidx, tidx;
            char slash;
            iss >> trash;
            while (iss >> vidx >> slash >> tidx >> trash) 
            {
                f.push_back(--vidx);
                t.push_back(--tidx);
            }
            faces_.push_back(f);
            tfaces_.push_back(t);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

std::vector<int> Model::tface(int idx) {
    return tfaces_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec3f Model::tex(int i) {
    return texs_[i];
}

