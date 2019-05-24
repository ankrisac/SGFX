#pragma once
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <SDL2/SDL.h>

#include <iostream>
#include <exception>
#include <memory>

#include <string>
#include <vector>
#include <array>
#include <map>


struct Vec3{
    float x;
    float y;
    float z;
};
template<typename T> struct Color{
    T r;
    T g;
    T b;
    T a;

    T sum(){
        return r + g + b + a;
    }
};

namespace GL{
    void debug(std::string msg = "?"){
        static int i = 0;

        std::cout << "Error[" << i++ << "," << msg << "]: " << glewGetErrorString(glGetError()) << std::endl;
    }

    class GLObject{
    protected:
        GLuint ref;

        GLObject(){}
        virtual ~GLObject(){}
    public:
        GLuint getRef() const{ 
            return ref; 
        }
    };

    enum ShaderType{
        VERTEX = GL_VERTEX_SHADER,
        FRAGMENT = GL_FRAGMENT_SHADER,
        GEOMETRY = GL_GEOMETRY_SHADER,
    };
    template<ShaderType type> class SubShader: public GLObject{
    public:
        SubShader(std::string source){
            ref = glCreateShader(type);

            GLchar* srcArray[] = { (GLchar*)source.c_str() };
            GLsizei lenArray[] = { (GLsizei)source.length() };

            glShaderSource(ref, 1, srcArray, lenArray);
            glCompileShader(ref);

            if(getParameter(GL_COMPILE_STATUS) == GL_FALSE){
                std::cout << "Error compiling " << getName() << std::endl;
                std::cout << getInfoLog() << std::endl;
            }
        }
        ~SubShader(){ 
            glDeleteShader(ref); 
        }

        ShaderType getType() const{ 
            return type; 
        }

        GLint getParameter(GLenum flag) const{
            GLint value;
            glGetShaderiv(ref, flag, &value);    
            
            return value;
        }
        std::string getInfoLog() const{
            GLchar infoLog[1024];
            glGetShaderInfoLog(ref, sizeof(infoLog), NULL, infoLog);
            
            return std::string(infoLog);
        }
        std::string getName() const{
            switch(type){
                case ShaderType::VERTEX:
                    return "vertex subshader";
                case ShaderType::FRAGMENT:
                    return "fragment subshader";
                case ShaderType::GEOMETRY:
                    return "geometry subshader";
            }
        }
    };

    class Shader: public GLObject{
    public:
        Shader(){ 
            ref = glCreateProgram(); 
        }
        ~Shader(){ 
            GLint numShaders = getParameter(GL_ATTACHED_SHADERS);
            GLuint* shaders = new GLuint[numShaders];

            glGetAttachedShaders(ref, numShaders, NULL, shaders); 

            for(GLsizei i = 0; i < numShaders; ++i){
                glDetachShader(ref, shaders[i]);
            }
            
            delete[] shaders;

            glDeleteProgram(ref); 
        }
        
        void bind(){ 
            glUseProgram(ref); 
        }
        void unbind(){ 
            glUseProgram(0); 
        }

        template<ShaderType type> void attach(const SubShader<type>& shader){
            glAttachShader(ref, shader.getRef());
        }
        template<ShaderType type> void detach(const SubShader<type>& shader){
            glDetachShader(ref, shader.getRef());
        }
        void bindAttribute(unsigned int i, std::string name){
            glBindAttribLocation(ref, i, name.c_str());
        }

        void compile(){
            glLinkProgram(ref);
            if(!getParameter(GL_LINK_STATUS)){
                std::cout << "Error linking program:" << std::endl;
                std::cout << getInfoLog() << std::endl;
            }

            glValidateProgram(ref);
            if(!getParameter(GL_LINK_STATUS)){
                std::cout << "Error validating program:" << std::endl;
                std::cout << getInfoLog() << std::endl;
            }
        }

        GLuint getUniform(std::string name) const{
            return glGetUniformLocation(ref, name.c_str());
        }
        GLint getParameter(GLenum flag) const{
            GLint value;
            glGetProgramiv(ref, flag, &value);
            return value;
        }
        std::string getInfoLog() const{
            GLchar infoLog[1024];
            glGetProgramInfoLog(ref, sizeof(infoLog), NULL, infoLog);
            return std::string(infoLog);
        }
    };


    enum BufferType{
        ATTRIBUTE = GL_ARRAY_BUFFER,
        INDEX = GL_ELEMENT_ARRAY_BUFFER,

        UNIFORM = GL_UNIFORM_BUFFER,
        TEXTURE = GL_TEXTURE_BUFFER,
    };

    struct vbFormat{
        GLenum elementType;
        size_t elementSize;

        GLenum storageHint;
        size_t padding; 
        const void* offset;

        bool normalized;

        vbFormat(GLenum type = GL_FLOAT, size_t size = 3, GLenum hint = GL_STATIC_DRAW, 
            size_t padding = 0, const void* offset = 0, bool norm = false){
            this->elementType = type;
            this->elementSize = size;
            
            this->storageHint = hint;
            this->padding = padding;
            this->offset = offset;

            this->normalized = norm;
        }
    };

    template<BufferType target> class Buffer: public GLObject{
        size_t len;
        vbFormat format;
    public:
        Buffer(){ 
            glGenBuffers(1, &ref); 
        }
        ~Buffer(){ 
            glDeleteBuffers(1, &ref); 
        }

        size_t getLen() const{ 
            return len; 
        }
        GLenum getType() const{
            return format.elementType; 
        }
        vbFormat getFormat() const{ 
            return format; 
        }

        void bind() const{ 
            glBindBuffer(target, ref); 
        }
        void unbind() const { 
            glBindBuffer(target, 0); 
        }
        
        template<typename T> void setData(const std::vector<T>& data, const vbFormat format){
            this->len = data.size();
            this->format = format;
            
            bind();
            glBufferData(target, data.size() * sizeof(data[0]), data.data(), format.storageHint);
            unbind();
        }
    };
    
    class VertexArray: public GLObject{
    public:
        VertexArray(){ 
            glGenVertexArrays(1, &ref); 
        }
        ~VertexArray(){ 
            glDeleteVertexArrays(1, &ref); 
        }

        void bind() const{ 
            glBindVertexArray(ref); 
        }
        void unbind() const{ 
            glBindVertexArray(0); 
        }

        void draw(const Buffer<INDEX>& eBuffer, GLenum mode = GL_TRIANGLES) const{
            bind();
            eBuffer.bind();

            glDrawElements(mode, eBuffer.getLen(), GL_UNSIGNED_INT, 0);
            
            unbind();
        }

        void bindAttribute(const size_t i, const Buffer<ATTRIBUTE>& vbuffer){
            bind();
            vbuffer.bind();

            auto fmt = vbuffer.getFormat();

            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i, fmt.elementSize, fmt.elementType, 
                fmt.normalized, fmt.padding, fmt.offset);
            unbind();

            glDisableVertexAttribArray(i);
        }
    };
}
