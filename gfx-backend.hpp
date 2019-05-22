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

namespace GW{
    void debug(std::string msg = "?"){
        static int i = 0;

        std::cout << "Error[" << i++ << "," << msg << "]: " << glewGetErrorString(glGetError()) << std::endl;
    }

    class Context;
    class Window{
        SDL_Window* window;
        SDL_GLContext context;
    public:
        struct Mode{
            struct GLVersion{
                unsigned int major;
                unsigned int minor;
            };

            GLVersion version;
            Color<unsigned int> colorResolution;
            bool doubleBuffered;

            Mode(GLVersion version = { 3, 3 }, Color<unsigned int> colorResolution = { 8, 8, 8, 8 }, bool doubleBuffered = true){
                this->version = version;
                this->colorResolution = colorResolution;
                this->doubleBuffered = doubleBuffered;
            }
        };

        Window(const std::string& title, unsigned int width, unsigned int height, Mode mode = Mode()){
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, mode.colorResolution.r);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, mode.colorResolution.g);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, mode.colorResolution.b);
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, mode.colorResolution.a);
            SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, mode.colorResolution.sum());

            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, mode.doubleBuffered);
            
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, mode.version.major);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, mode.version.minor);

            window = SDL_CreateWindow(title.c_str(), 
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        }
        ~Window(){
            SDL_DestroyWindow(window);
        }

        Context* createContext();
        SDL_Window* getRef(){
            return window;
        }
        Uint32 getID(){
            return SDL_GetWindowID(window);
        }
        void clear(float r = 0.0, float g = 0.0, float b = 0.0, float a = 1.0){
            glClear(GL_COLOR_BUFFER_BIT);
            glClearColor(r, g, b, a);
        }
        void update(){
            SDL_GL_SwapWindow(window);
        }

        int getWidth(){
            int width;
            SDL_GetWindowSize(window, &width, NULL);
            return width;
        }
        int getHeight(){
            int height;
            SDL_GetWindowSize(window, NULL, &height);
            return height;
        }

        void setTitle(std::string title){
            SDL_SetWindowTitle(window, title.c_str());  
        }
    };

    class Context{
        SDL_GLContext context;
    public:
        Context(Window* window){
            context = SDL_GL_CreateContext(window->getRef());

            glewExperimental = true;
            GLenum status = glewInit();
            if(status != GLEW_OK){
                std::cout << "Error initializing GLEW : ";
                std::cout << glewGetErrorString(status) << std::endl;
            }
        }
        ~Context(){
            SDL_GL_DeleteContext(context);
        }

        void bind(Window* window){
            SDL_GL_MakeCurrent(window->getRef(), context);
        }
    };

    Context* Window::createContext(){
        return new Context(this);
    }

    class Input{
        std::map<SDL_Keycode, bool> keyMap;
    public:
        void setKey(SDL_Keycode key, bool value){
            keyMap[key] = value;
        }
        bool getKey(SDL_Keycode key) const {
            auto it = keyMap.find(key);

            if(it != keyMap.end()){
                return it->second;
            }
            return false;
        }
        bool getKey(std::vector<SDL_Keycode> keys){
            for(auto i: keys){
                if(getKey(i)){
                    return true;
                }
            }
            return false;
        }
    };



    class GLObject{
    protected:
        GLuint ref;

        GLObject(){}
        virtual ~GLObject(){}
    public:
        GLuint getRef() const{ return ref; }
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
        ~SubShader(){ glDeleteShader(ref); }

        ShaderType getType() const{ return type; }

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
        
        void bind(){ glUseProgram(ref); }
        void unbind(){ glUseProgram(0); }

        template<ShaderType type> void attach(const SubShader<type>* shader){
            glAttachShader(ref, shader->getRef());
        }
        template<ShaderType type> void detach(const SubShader<type>* shader){
            glDetachShader(ref, shader->getRef());
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
        Buffer(){ glGenBuffers(1, &ref); }
        ~Buffer(){ glDeleteBuffers(1, &ref); }

        size_t getLen() const { return len; }
        GLenum getType() const { return format.elementType; }
        vbFormat getFormat() const { return format; }

        void bind() const{ glBindBuffer(target, ref); }
        void unbind() const { glBindBuffer(target, 0); }
        
        template<typename T> void setData(std::vector<T> data, vbFormat format){
            this->len = data.size();
            this->format = format;
            
            bind();
            glBufferData(target, data.size() * sizeof(data[0]), data.data(), format.storageHint);
            unbind();
        }
    };
    
    class VertexArray: public GLObject{
    public:
        VertexArray(){ glGenVertexArrays(1, &ref); }
        ~VertexArray(){ glDeleteVertexArrays(1, &ref); }

        void bind() const{ glBindVertexArray(ref); }
        void unbind() const{ glBindVertexArray(0); }

        void draw(Buffer<INDEX>* eBuffer, GLenum mode = GL_TRIANGLES) const{
            bind();
            eBuffer->bind();
            glDrawElements(mode, eBuffer->getLen(), GL_UNSIGNED_INT, 0);
            unbind();
        }

        void bindAttribute(size_t i, Buffer<ATTRIBUTE>* vbuffer){
            bind();
            vbuffer->bind();

            auto fmt = vbuffer->getFormat();

            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i, fmt.elementSize, fmt.elementType, 
                fmt.normalized, fmt.padding, fmt.offset);
            unbind();

            glDisableVertexAttribArray(i);
        }
    };
}