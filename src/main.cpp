#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

using namespace std;
using namespace glm;

class Application : public EventCallbacks
{

    public:

        WindowManager * windowManager = nullptr;

        // Our shader program
        std::shared_ptr<Program> prog;

        // Shape to be used (from obj file)
        shared_ptr<Shape> shape;

        // Contains vertex information for OpenGL
        GLuint VertexArrayID;

        // Data necessary to give our triangle to OpenGL
        GLuint VertexBufferID;

        float upperarm_rot;
        float forearm_rot;
        float hand_rot;
        float tmp;

        float x_disp = 0;
        bool rot_arm = false;

        void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
        {
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            {
                glfwSetWindowShouldClose(window, GL_TRUE);
            }
            else if ( (key == GLFW_KEY_J || key == GLFW_KEY_N || key == GLFW_KEY_A) && action == GLFW_PRESS)
            {
                x_disp -= 0.1; 
            }
            // Right 
            else if ( (key == GLFW_KEY_SEMICOLON || key == GLFW_KEY_O || key == GLFW_KEY_D) && action == GLFW_PRESS)
            {
                x_disp += 0.1; 
            }
            else if (key == GLFW_KEY_C && action == GLFW_PRESS)
            {
                rot_arm = !rot_arm;
            }
        }

        void mouseCallback(GLFWwindow *window, int button, int action, int mods)
        {
            double posX, posY;

            if (action == GLFW_PRESS)
            {
                glfwGetCursorPos(window, &posX, &posY);
                cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
            }
        }

        void resizeCallback(GLFWwindow *window, int width, int height)
        {
            glViewport(0, 0, width, height);
        }

        void init(const std::string& resourceDirectory)
        {
            GLSL::checkVersion();

            // Set background color.
            glClearColor(.12f, .34f, .56f, 1.0f);
            // Enable z-buffer test.
            glEnable(GL_DEPTH_TEST);

            upperarm_rot = 0;
            forearm_rot = 0;
            tmp = 0;

            // Initialize the GLSL program.
            prog = make_shared<Program>();
            prog->setVerbose(true);
            prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
            prog->init();
            prog->addUniform("P");
            prog->addUniform("V");
            prog->addUniform("M");
            prog->addAttribute("vertPos");
            prog->addAttribute("vertNor");
        }

        void initGeom(const std::string& resourceDirectory)
        {
            // Initialize mesh.
            shape = make_shared<Shape>();
            shape->loadMesh(resourceDirectory + "/cube.obj");
            shape->resize();
            shape->init();
        }

        void render()
        {
            // Get current frame buffer size.
            int width, height;
            glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
            glViewport(0, 0, width, height);

            // Clear framebuffer.
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //Use the matrix stack for Lab 6
            float aspect = width/(float)height;

            // Create the matrix stacks - please leave these alone for now
            auto Projection = make_shared<MatrixStack>();
            auto View = make_shared<MatrixStack>();
            auto Model = make_shared<MatrixStack>();

            const static float shoulder_x = 1.0;
            const static float shoulder_disp = 1;

            // Apply perspective projection.
            Projection->pushMatrix();
            Projection->perspective(45.0f, aspect, 0.01f, 100.0f);

            // View is identity - for now
            View->pushMatrix();

            // Draw a stack of cubes with indiviudal transforms
            prog->bind();
            glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
            glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));

            // draw bottom cube
            Model->pushMatrix();
            {
                Model->loadIdentity();
                // draw the bottom cube with these 'global transforms'
                Model->translate(vec3(x_disp, 0, -5));
                Model->scale(vec3(0.6, 0.6, 0.6));
                glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
                shape->draw(prog);
                
                // draw the bottom cubes 'arm' - relative to the position of the bottom cube
                // note you must change this to TWO jointed arm with hand
                Model->pushMatrix();
                {
                    // place at shoulder
                    Model->translate(vec3(shoulder_disp - 0.1, shoulder_disp, 0));
                    // rotate shoulder joint
                    Model->rotate(upperarm_rot, vec3(0, 0, 1));
                    // move to shoulder joint
                    Model->translate(vec3(shoulder_x, 0, 0));
                    
                    // Forearm
                    Model->pushMatrix();
                    {
                        Model->translate(vec3(0.9, 0, 0));
                        Model->rotate(forearm_rot, vec3(0, 0, 1));
                        Model->translate(vec3(0.4, 0, 0));

                        // Hand
                        Model->pushMatrix();
                        {
                            Model->translate(vec3(0.8, 0.0, 0));
                            Model->rotate(hand_rot, vec3(0, 0, 1));
                            Model->scale(0.25);
                            glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
                            shape->draw(prog);
                        }
                        Model->popMatrix();

                        Model->scale(vec3(0.5, 0.25, 0.25));
                        glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
                        shape->draw(prog);
                    }
                    Model->popMatrix();

                    // non-uniform scale
                    Model->scale(vec3(0.95, 0.25, 0.25));
                    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
                    shape->draw(prog);
                }
                Model->popMatrix();

                // Left arm
                Model->pushMatrix();
                {
                    // place at shoulder
                    Model->translate(vec3(-shoulder_disp - 0.2, shoulder_disp - 0.3, 0));
                    // rotate shoulder joint
                    Model->rotate(4.2, vec3(0, 0, 1));
                    // move to shoulder joint
                    Model->translate(vec3(shoulder_disp, 0, 0));
                    // non-uniform scale
                    Model->scale(vec3(1.5, 0.25, 0.25));
                    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
                    shape->draw(prog);
                }
                Model->popMatrix();

                // draw top cube - aka head
                Model->pushMatrix();
                {
                    Model->loadIdentity();
                    // play with these options
                    Model->translate(vec3(x_disp, 1.1, -5));
                    Model->rotate(0.5, vec3(0, 1, 0));
                    Model->scale(vec3(0.4, 0.4, 0.4));
                    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
                    shape->draw(prog);
                }
                Model->popMatrix();
            }
            Model->popMatrix();

            prog->unbind();

            // Pop matrix stacks.
            Projection->popMatrix();
            View->popMatrix();

            auto extract_sign = [](int a) { return a > 0 ? 1: -1; };

            upperarm_rot = 1.4*sin(glfwGetTime());
            if (std::abs(upperarm_rot) > 1)
            {
                upperarm_rot = extract_sign(upperarm_rot);
            }

            if (rot_arm)
            {
                forearm_rot = 0.5*sin(glfwGetTime());
            }
            hand_rot = 0.1*sin(glfwGetTime());
            

        }
};

int main(int argc, char *argv[])
{
    // Where the resources are loaded from
    std::string resourceDir = "../resources";

    if (argc >= 2)
    {
        resourceDir = argv[1];
    }

    Application *application = new Application();

    // Your main will always include a similar set up to establish your window
    // and GL context, etc.

    WindowManager *windowManager = new WindowManager();
    windowManager->init(640, 480);
    windowManager->setEventCallbacks(application);
    application->windowManager = windowManager;

    // This is the code that will likely change program to program as you
    // may need to initialize or set up different data and state

    application->init(resourceDir);
    application->initGeom(resourceDir);

    // Loop until the user closes the window.
    while (! glfwWindowShouldClose(windowManager->getHandle()))
    {
        // Render scene.
        application->render();

        // Swap front and back buffers.
        glfwSwapBuffers(windowManager->getHandle());
        // Poll for and process events.
        glfwPollEvents();
    }

    // Quit program.
    windowManager->shutdown();
    return 0;
}
