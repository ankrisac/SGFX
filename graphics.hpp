#pragma once
#include "gfx-backend.hpp"
#include "matrix.hpp"

class Mesh{
    GW::Buffer<GW::ATTRIBUTE> VBO;
    GW::Buffer<GW::INDEX> EBO;
    GW::VertexArray VAO;
public:
    Mesh(std::vector<rowVec<float, 3>> vertices, std::vector<unsigned int> indices)
    :VBO(), EBO(), VAO()
    {
        VBO.setData(vertices, GW::vbFormat(GL_FLOAT, 3, GL_STATIC_DRAW, 0, 0, false));
        EBO.setData(indices, GW::vbFormat(GL_UNSIGNED_INT, 1, GL_STATIC_DRAW, 0, 0, false));

        VAO.bindAttribute(0, &VBO);
    }
    ~Mesh(){}

    void draw(){
        VAO.draw(&EBO, GL_TRIANGLES);
    }
};
class VFShader{
    GW::SubShader<GW::VERTEX> vert;
    GW::SubShader<GW::FRAGMENT> frag;
    GW::Shader shader;
public:
    VFShader(std::string vertSrc, std::string fragSrc, std::vector<std::string> attributes)
    :vert(vertSrc), frag(fragSrc), shader(){
        shader.attach(&vert);
        shader.attach(&frag);

        for(size_t i = 0; i < attributes.size(); ++i){
            shader.bindAttribute(i, attributes[i]);
        }
        shader.compile();
    }
    ~VFShader(){}

    void bind(){
        shader.bind();    
    }
    GLuint getUniform(std::string uniform){
        return shader.getUniform(uniform);
    }
};