/*
Copyright (c) 2022 Shera Adams

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

Except as contained in this notice, the name(s) of the above copyright holders
shall not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// Credit to learnopengl.com for Shader and Camera and Sphere
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <math.h>


#include "filesystem.h"
#include "shader.h"
#include "camera.h"
#include "model.h"
#include "pen_accent.h"
#include "pen_body.h"
#include "pen_clip.h"
#include "pen_point.h"
#include "sphere.h"
#include "ufo.h"

#include "objects.h"
#include "geometry.h"
#include "texture.h"

/* TEXT RENDERING */
struct Character {
    GLuint TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    GLuint Advance;
};

/* FUNCTIONS */
void RenderText(Shader& s, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void GetDesktopResolution(float& horizontal, float& vertical);
void SetShader(Shader lightingShader);


/* VARIABLES */
int texturePicker;
int t = 0;
int r = 0;
vector<unsigned int> textures;
float trajectory;
float x = -2.45613f;
float y = -.894599f;
float z = .499f;
std::map<GLchar, Character> Characters;
GLuint textVAO, textVBO;
bool Keys[1024];
bool firstMouse = true;
bool onPerspective = true;
float SCR_WIDTH = 1000;
float SCR_HEIGHT = 900;
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
float lastFrame = 0.0f; 
float deltaTime = 0.0f;
GLfloat xoffset = 0.0f, yoffset = 0.0f;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

Geometry geometry;
const glm::vec3* lightPositions = geometry.GetLightPositions();
const glm::vec3* pointLightPositions = geometry.GetPointLightPositions();
const glm::vec3 lightPos = geometry.GetLightPos();

void GetDesktopResolution(float& horizontal, float& vertical)
{
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    horizontal = desktop.right;
    vertical = desktop.bottom;
}
int main()
{
    GetDesktopResolution(SCR_WIDTH, SCR_HEIGHT);
    /* GLFW INITIALIZE */
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);


#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Game", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);


    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    /* GLFW INITIALIZE */

    /* TEXT RENDERING */

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    /* TEXT RENDERING */

    /* SHADERS */
    Shader lightCubeShader("lightbox.vs", "lightbox.fs");
    Shader lightingShader("specular.vs", "specular.fs");
    Shader skyboxShader("skybox.vs", "skybox.fs");
    Shader greenShader("glsl.vs", "light_green.fs");
    Shader pinkShader("glsl.vs", "light_pink.fs");
    Shader purpleShader("glsl.vs", "light_purple.fs");
    Shader textShader("tex.vs", "tex.fs");

    glm::mat4 Text_projection = glm::ortho(0.0f, SCR_WIDTH, 0.0f, SCR_HEIGHT);
    textShader.use();
    glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(Text_projection));

    /* VERTICES */
    std::vector<GLfloat>vertices = geometry.GetCubeVertices();
    std::vector<GLfloat>compassVertices = geometry.GetCompassVertices();
    std::vector<GLfloat>boxVertices = geometry.GetBoxVertices();
    std::vector<GLfloat>skyboxVertices = geometry.GetSkyboxVertices();
    std::vector<GLfloat>compass2Vertices = geometry.GetCompass2Vertices();

    /* TEXT RENDERING VAO-VBO*/
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /* LIGHT CUBE UNIFORM MATRICES */
    unsigned int lightCubeVAO, lightCubeVBO;
    glGenVertexArrays(1, &lightCubeVAO);
    glGenBuffers(1, &lightCubeVBO);
    glBindVertexArray(lightCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, skyboxVertices.size() * sizeof(GLfloat), &skyboxVertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    unsigned int uniformBlockIndexRed = glGetUniformBlockIndex(pinkShader.ID, "Matrices");
    unsigned int uniformBlockIndexGreen = glGetUniformBlockIndex(greenShader.ID, "Matrices");
    glUniformBlockBinding(pinkShader.ID, uniformBlockIndexRed, 0);
    glUniformBlockBinding(greenShader.ID, uniformBlockIndexGreen, 0);
    unsigned int uboMatrices;
    glGenBuffers(1, &uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));
    glm::mat4 projection = glm::perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    /* TEXTURES */
    Textures texture;
    unsigned int ufoTexture = texture.loadTexture("resources/textures/AdobeStock_257170070.jpg");
    unsigned int earthTexture = texture.loadTexture("resources/textures/earth0.png");
    unsigned int goldTexture = texture.loadTexture("resources/textures/AdobeStock_235275603.jpg");
    unsigned int flowerTexture = texture.loadTexture("resources/textures/AdobeStock_408749181.jpeg");
    unsigned int satelliteTexture = texture.loadTexture("resources/textures/satellite2.png");
    unsigned int cubeTexture = texture.loadTexture("resources/textures/box.png");
    unsigned int panelTexture = texture.loadTexture("resources/textures/satellite.png");
    unsigned int planeTexture1 = texture.loadTexture("resources/textures/AdobeStock_252775020.jpeg");
    unsigned int skyboxTexture = texture.loadTexture("resources/textures/viktorsaznov deepspace.jpeg");
    unsigned int planeTexture3 = texture.loadTexture("resources/textures/AdobeStock_481965458.jpeg");
    unsigned int planeTexture4 = texture.loadTexture("resources/textures/background5.jpg");
    unsigned int planeTexture5 = texture.loadTexture("resources/textures/AdobeStock_293211764.jpeg");


    textures.push_back(ufoTexture);
    textures.push_back(flowerTexture);
    textures.push_back(skyboxTexture);
    textures.push_back(cubeTexture);
    textures.push_back(planeTexture1);
    textures.push_back(planeTexture3);
    textures.push_back(planeTexture4);
    textures.push_back(planeTexture5);

    vector<std::string> faces
    {
        "resources/textures/right.jpg", // right 
        "resources/textures/left.jpg", // left 
        "resources/textures/top.jpg", // top  
        "resources/textures/bottom.jpg", // bottom  
        "resources/textures/front.jpg", // front
        "resources/textures/back.jpg", // back
    };
    unsigned int cubemap3Texture = texture.loadCubemap(faces);
    /* TEXTURES */
    



    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);

    /* SET THE PROJECTION AS PERSPECTIVE BY DEFAULT*/
    onPerspective = true;
    glfwSwapBuffers(window);
    /* RENDER LOOP */
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
       
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        /* LIGHTING SETTINGS FOR THE SCENE */
       
        SetShader(lightingShader);
        
        /* INITIALIZE VARAIBLES */
        glm::mat4 projection, view, model, light_models, model_dish;

        /* SET PROJECTION
        /****************************************************************/
        if (onPerspective)
            projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        if (!onPerspective)
            projection = glm::ortho(-2.0f, 2.0f, -1.5f, 1.5f, 1.0f, 100.0f);
       


        /* SET SHADER */
        view = camera.GetViewMatrix();
        lightingShader.use();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        lightingShader.setMat4("model", model);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, earthTexture);
        lightingShader.setMat4("model", model);
        model = glm::mat4(.5f);
        // code to make the earth spin in place
        model = glm::translate(model, glm::vec3(-.411121f, -1.2946 -45.8946f, -4.90606));
      //  model = glm::rotate(model, (GLfloat)glfwGetTime() * glm::radians(10.0f), glm::vec3(0.0, 0.100f, 0.0));
        model = glm::scale(model, glm::vec3(17));
        lightingShader.setMat4("model", model);
        Sphere sphere;
        sphere.Draw();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, panelTexture);
        lightingShader.setMat4("model", model);
        Objects box;
        box.link(vertices.size() * sizeof(GLfloat), &vertices[0]);
        for (int i = 0; i <= 5; i++)
        {
            if (i == 0)
            {   /* satellite wing left */
                model = glm::mat4(1.0f);
                model = glm::rotate(model, glm::radians(trajectory * 50), glm::vec3(0.0f, 1.0f, 1.0f));
                model = glm::translate(model, glm::vec3(-2.8004f, -0.900075f, 0.599999f));
                model = glm::scale(model, glm::vec3(.5f, .2f, .02f));

                lightingShader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            if (i == 1)
            { /* satellite wing right*/
                model = glm::mat4(1.0f);
                model = glm::rotate(model, glm::radians(trajectory * 50), glm::vec3(0.0f, 1.0f, 1.00f));
                model = glm::translate(model, glm::vec3(-1.96539f, -0.900075f, 0.599999f));
                model = glm::scale(model, glm::vec3(.5f, .2f, .02f));

                lightingShader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            if (i == 3)
            { /* satellite body*/
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, satelliteTexture);
                model = glm::mat4(1.0f);
                model = glm::rotate(model, glm::radians(trajectory * 50), glm::vec3(0.0f, 1.0f, 1.00f));
                model = glm::translate(model, glm::vec3(-2.3804f, -0.855599f, 0.629999f));
                model = glm::scale(model, glm::vec3(.25f));
                lightingShader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ufoTexture);
        model_dish = glm::mat4(1.0f);
        model_dish = glm::rotate(model_dish, glm::radians(trajectory * 50), glm::vec3(0.0f, 1.0f, 1.00f));
        model_dish = glm::rotate(model_dish, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.00f));
        model_dish = glm::translate(model_dish, glm::vec3(-2.3754f, 0.6594f, 0.854999f));
        model_dish = glm::scale(model_dish, glm::vec3(.3f));
        lightingShader.setMat4("model", model_dish);
        Ufo ufo;
        ufo.Draw();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, panelTexture);
        lightingShader.setMat4("model", model);

        // satellite right attachment
        Objects triangle;
        triangle.link(compassVertices.size() * sizeof(GLfloat), &compassVertices[0]);
        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(trajectory * 50), glm::vec3(0.0f, 1.0f, 1.00f));
        model = glm::translate(model, glm::vec3(-2.31113f, -0.899599f, 0.489f));
        model = glm::scale(model, glm::vec3(.2));
        lightingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // satellite left attachment

        Objects triangle2;
        triangle2.link(compass2Vertices.size() * sizeof(GLfloat), &compass2Vertices[0]);
        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(trajectory * 50), glm::vec3(0.0f, 1.0f, 1.00f));
        model = glm::translate(model, glm::vec3(-2.46113f, -0.899599f, 0.504f));
        model = glm::scale(model, glm::vec3(.2));
        lightingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        /* RENDER LIGHTS */
        pinkShader.setMat4("model", light_models);
        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindVertexArray(lightCubeVAO);
        float speed = 45.0f;
        float direction = -1.0;
        for (unsigned int i = 0; i < 2; i++)
        {
            if (i == 1)
            { /* PINK LIGHTS */
                pinkShader.use();
                pinkShader.setMat4("projection", projection);
                pinkShader.setMat4("view", view);
                light_models = glm::mat4(1.0f);
                // code to orbit the lights around the scene
                light_models = glm::rotate(light_models, (GLfloat)glfwGetTime() * glm::radians(speed) * direction * 2.0f, glm::vec3(0.0f, 1.0f, 0.f));
                light_models = glm::translate(light_models, lightPositions[i]);
                light_models = glm::scale(light_models, glm::vec3(.25f));
                pinkShader.setMat4("model", light_models);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            else
            { /* PURPLE LIGHTS */
                purpleShader.use();
                purpleShader.setMat4("projection", projection);
                purpleShader.setMat4("view", view);
                light_models = glm::mat4(1.0f);
                light_models = glm::rotate(light_models, (GLfloat)glfwGetTime() * glm::radians(speed) * 2.0f, glm::vec3(0.0f, 1.5f, 0.f));
                light_models = glm::translate(light_models, lightPositions[i]);
                light_models = glm::scale(light_models, glm::vec3(.25f));
                purpleShader.setMat4("model", light_models);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }

        /* RENDER SKYBOX */
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        Objects skybox;
        skybox.skybox(skyboxVertices.size() * sizeof(GLfloat), &skyboxVertices[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap3Texture);
        glDrawArrays(GL_TRIANGLES, 0, 72);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    /* DELETE VAOS AND CLEAR MEMORY */


    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &lightCubeVBO);


    glDeleteTextures(1, &flowerTexture);
    glDeleteTextures(1, &goldTexture);
    glDeleteTextures(1, &panelTexture);
    glDeleteTextures(1, &planeTexture1);
    glDeleteTextures(1, &satelliteTexture);
    glDeleteTextures(1, &planeTexture3);
    glDeleteTextures(1, &planeTexture4);
    glDeleteTextures(1, &planeTexture5);
    glDeleteTextures(1, &cubemap3Texture);
    glDeleteTextures(1, &ufoTexture);
    glDeleteTextures(1, &cubeTexture);

    glDeleteShader(lightingShader.ID);
    glDeleteShader(greenShader.ID);
    glDeleteShader(purpleShader.ID);
    glDeleteShader(skyboxShader.ID);
    glDeleteShader(pinkShader.ID);
    glDeleteShader(lightCubeShader.ID);

    glfwTerminate();
    return 0;
}

/* PROCESS INPUT */
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if ((glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS))
        onPerspective = false;
    if ((glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS))
        onPerspective = true;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
    if ((glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS))
        x -= .005;
    if ((glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS))
        x += .005;
    if ((glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS))
        y += .005;
    if ((glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS))
        y -= .005;
    if ((glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS))
        z -= .005;
    if ((glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS))
        z += .005;
    if ((glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS))
        trajectory += .05;
    if ((glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS))
        std::cout << "( " << x << "f, " << y << "f, " << z << "f)" << std::endl;

    if ((glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS))
    {
        if (t < textures.size() - 1)
        {
            t += 1;
            texturePicker = textures[t];
        }
        else
            t = 0;
    }
    if ((glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS))
    {
        if (r < textures.size() - 1)
        {
            r += 1;
            texturePicker = textures[r];
        }
        else
            r = 0;
    }
}

/* CALLBACKS */
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void SetShader(Shader lightingShader)
{
    lightingShader.use();
    lightingShader.setVec3("viewPos", camera.Position);
    lightingShader.setFloat("material.shininess", 32.0f);

    // directional light
    lightingShader.setVec3("dirLight.direction", 0.2f, 0.0f, 0.3f);
    lightingShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    lightingShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    lightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    // point light 1
    lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
    lightingShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
    lightingShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    lightingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    lightingShader.setFloat("pointLights[0].constant", 1.0f);
    lightingShader.setFloat("pointLights[0].linear", 0.09f);
    lightingShader.setFloat("pointLights[0].quadratic", 0.032f);
    // point light 2
    lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
    lightingShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
    lightingShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
    lightingShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
    lightingShader.setFloat("pointLights[1].constant", 1.0f);
    lightingShader.setFloat("pointLights[1].linear", 0.09f);
    lightingShader.setFloat("pointLights[1].quadratic", 0.032f);
    // point light 3
    lightingShader.setVec3("pointLights[2].position", pointLightPositions[2]);
    lightingShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
    lightingShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
    lightingShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
    lightingShader.setFloat("pointLights[2].constant", 1.0f);
    lightingShader.setFloat("pointLights[2].linear", 0.09f);
    lightingShader.setFloat("pointLights[2].quadratic", 0.032f);
    // point light 4
    lightingShader.setVec3("pointLights[3].position", pointLightPositions[3]);
    lightingShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
    lightingShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
    lightingShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
    lightingShader.setFloat("pointLights[3].constant", 1.0f);
    lightingShader.setFloat("pointLights[3].linear", 0.09f);
    lightingShader.setFloat("pointLights[3].quadratic", 0.032f);
    // spotLight
    lightingShader.setVec3("spotLight.position", camera.Position);
    lightingShader.setVec3("spotLight.direction", camera.Front);
    lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    lightingShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    lightingShader.setFloat("spotLight.constant", 1.0f);
    lightingShader.setFloat("spotLight.linear", 0.09f);
    lightingShader.setFloat("spotLight.quadratic", 0.032f);
    lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

    float ambient[] = { 0.5f, 0.5f, 0.5f, 1 };
    float diffuse[] = { 0.8f, 0.8f, 0.8f, 1 };
    float specular[] = { 1.0f, 1.0f, 1.0f, 1 };
    float shininess = 128;
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);


}