#pragma once
#include "gfx-backend.hpp"
#include "matrix.hpp"

namespace GX{
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

            Mode(GLVersion version = { 3, 3 }, 
                Color<unsigned int> colorResolution = { 8, 8, 8, 8 }, 
                bool doubleBuffered = true){
                
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

    class Mesh{
        GL::Buffer<GL::ATTRIBUTE> VBO;
        GL::Buffer<GL::INDEX> EBO;
        GL::VertexArray VAO;
    public:
        Mesh(std::vector<rowVec<float, 3>> vertices, std::vector<unsigned int> indices)
        :VBO(), EBO(), VAO()
        {
            VBO.setData(vertices, GL::vbFormat(GL_FLOAT, 3, GL_STATIC_DRAW, 0, 0, false));
            EBO.setData(indices, GL::vbFormat(GL_UNSIGNED_INT, 1, GL_STATIC_DRAW, 0, 0, false));

            VAO.bindAttribute(0, VBO);
        }
        ~Mesh(){}

        void draw(){
            VAO.draw(EBO, GL_TRIANGLES);
        }
    };

    class BaseShader{
    protected:
        GL::Shader shader;
    
        BaseShader(){}
        virtual ~BaseShader(){}
    public:
        void bind(){
            shader.bind();
        }
        GLuint getUniform(std::string uniform){
            return shader.getUniform(uniform);
        }
    };

    class ShaderVF: public BaseShader{
        GL::SubShader<GL::VERTEX> vert;
        GL::SubShader<GL::FRAGMENT> frag;
    public:
        ShaderVF(std::string vertSrc, std::string fragSrc, std::vector<std::string> attributes)
        :vert(vertSrc), frag(fragSrc){
            shader.attach(vert);
            shader.attach(frag);

            for(size_t i = 0; i < attributes.size(); ++i){
                shader.bindAttribute(i, attributes[i]);
            }
            
            shader.compile();
        }
    };

    class ShaderGVF: public BaseShader{
        GL::SubShader<GL::VERTEX> vert;
        GL::SubShader<GL::FRAGMENT> frag;
        GL::SubShader<GL::GEOMETRY> geom;
    public:
        ShaderGVF(std::string geomSrc, std::string vertSrc, std::string fragSrc, 
            std::vector<std::string> attributes)
        :geom(fragSrc), vert(vertSrc), frag(fragSrc){
            shader.attach(geom);
            shader.attach(vert);
            shader.attach(frag);

            for(size_t i = 0; i < attributes.size(); ++i){
                shader.bindAttribute(i, attributes[i]);
            }

            shader.compile();
        }
    };
}
