#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/animator.h>
#include <learnopengl/animation.h>
#include <learnopengl/model_animation.h>

#include <iostream>
#include <vector>
#include <ltc_matrix.hpp>



void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadCubemap(vector<std::string> faces);
void do_movement(GLfloat deltaTime);
unsigned int loadTexture(const char* path, bool gammaCorrection);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const glm::vec3 LIGHT_COLOR = glm::vec3(1.0f, 1.0f, 1.0f); // 조명 색 변경
bool keys[1024]; // activated keys
glm::vec3 areaLightTranslate;
Shader* ltcShaderPtr;

// camera
Camera camera(glm::vec3(0.0f, 1.0f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), 180.0f, 0.0f);
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
struct VertexAL {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

const GLfloat psize = 10.0f;
// 평면 바닥 좌표
VertexAL planeVertices[6] = {
    { {-psize, 0.0f, -psize}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },
    { {-psize, 0.0f,  psize}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} },
    { { psize, 0.0f,  psize}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f} },
    { {-psize, 0.0f, -psize}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },
    { { psize, 0.0f,  psize}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f} },
    { { psize, 0.0f, -psize}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} }
};
// area_light 좌표
VertexAL areaLightVertices[6] = {
    { {0.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} }, // 0 1 5 4
    { {0.0f, 1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },
    { {0.0f, -1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
    { {0.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
    { {0.0f, -1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
    { {0.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} }
};

GLuint planeVBO, planeVAO;
GLuint areaLightVBO, areaLightVAO;

// 평면 바닥, area_light 바인딩
void configureMockupData()
{
    // PLANE
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);

    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
        (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
        (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // texcoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
        (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    // AREA LIGHT
    glGenVertexArrays(1, &areaLightVAO);
    glBindVertexArray(areaLightVAO);

    glGenBuffers(1, &areaLightVBO);
    glBindBuffer(GL_ARRAY_BUFFER, areaLightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(areaLightVertices), areaLightVertices, GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
        (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
        (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // texcoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
        (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    glBindVertexArray(0);
}

void renderPlane()
{
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void renderAreaLight()
{
    glBindVertexArray(areaLightVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}



struct LTC_matrices {
    GLuint mat1;
    GLuint mat2;
};

GLuint loadMTexture()
{
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64,
        0, GL_RGBA, GL_FLOAT, LTC1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

GLuint loadLUTTexture()
{
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64,
        0, GL_RGBA, GL_FLOAT, LTC2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}



// 재질의 거칠기 업데이트
void incrementRoughness(float step)
{
    static glm::vec3 color = glm::vec3(0.439216f, 0.501961f, 0.564706f);
    static float roughness = 0.5f;
    roughness += step;
    roughness = glm::clamp(roughness, 0.0f, 1.0f);
    //std::cout << "roughness: " << roughness << '\n';
    ltcShaderPtr->use();
    ltcShaderPtr->setVec4("material.albedoRoughness", glm::vec4(color, roughness));
    glUseProgram(0);


}

// 조명의 강도 업데이트
void incrementLightIntensity(float step)
{
    static float intensity = 4.0f;
    intensity += step;
    intensity = glm::clamp(intensity, 0.0f, 10.0f);
    //std::cout << "intensity: " << intensity << '\n';
    ltcShaderPtr->use();
    ltcShaderPtr->setFloat("areaLight.intensity", intensity);
    glUseProgram(0);

}

// 조명의 양면 작동 여부
void switchTwoSided(bool doSwitch)
{
    static bool twoSided = true;
    if (doSwitch) twoSided = !twoSided;
    //std::cout << "twoSided: " << std::boolalpha << twoSided << '\n';
    ltcShaderPtr->use();
    ltcShaderPtr->setFloat("areaLight.twoSided", twoSided);
    glUseProgram(0);
}

int main()
{
    // glfw: initialize and configure
   // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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
    glfwSetKeyCallback(window, key_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    stbi_set_flip_vertically_on_load(true);


    glEnable(GL_DEPTH_TEST);

    LTC_matrices mLTC;
    mLTC.mat1 = loadMTexture();
    mLTC.mat2 = loadLUTTexture();

    // SHADERS _ area lights
    Shader shaderLTC("shader/7.area_light.vs", "shader/7.area_light.fs");
    ltcShaderPtr = &shaderLTC;
    Shader shaderLightPlane("shader/7.light_plane.vs", "shader/7.light_plane.fs");

    // TEXTURES
    unsigned int concreteTexture = loadTexture("resources/textures/concreteTexture.png", true);

    // SHADER CONFIGURATION
    shaderLTC.use();
    shaderLTC.setVec3("areaLight.points[0]", areaLightVertices[0].position);
    shaderLTC.setVec3("areaLight.points[1]", areaLightVertices[1].position);
    shaderLTC.setVec3("areaLight.points[2]", areaLightVertices[4].position);
    shaderLTC.setVec3("areaLight.points[3]", areaLightVertices[5].position);
    shaderLTC.setVec3("areaLight.color", LIGHT_COLOR);
    shaderLTC.setInt("LTC1", 0);
    shaderLTC.setInt("LTC2", 1);
    shaderLTC.setInt("material.diffuse", 2);
    incrementRoughness(0.0f);
    incrementLightIntensity(0.0f);
    switchTwoSided(false);
    glUseProgram(0);

    shaderLightPlane.use();
    {
        glm::mat4 model(1.0f);
        shaderLightPlane.setMat4("model", model);
    }
    shaderLightPlane.setVec3("lightColor", LIGHT_COLOR);
    glUseProgram(0);

    // 3D OBJECTS
    configureMockupData();
    areaLightTranslate = glm::vec3(-8.0f, 2.0f, 0.0f);

    // 애니메이션 shader 설정
    Shader ourShader("shader/anim_model.vs", "shader/anim_model.fs");
    Model ourModel("resources/objects/vampire/dancing_vampire.dae");
    Animation danceAnimation("resources/objects/vampire/dancing_vampire.dae", &ourModel);
    Animator animator(&danceAnimation);

    // skybox shader 설정
    Shader skyboxShader("shader/6.1.skybox.vs", "shader/6.1.skybox.fs");
    Shader car1Shader("shader/car1.vs", "shader/car1.fs");


    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


    //load models
    Model car1("resources/objects/car1/car1.obj");
    Model car2("resources/objects/car2/car2.obj");
    Model car3("resources/objects/car3/car3.obj");
    Model car4("resources/objects/car4/car4.obj");


    //skybox faces
    vector<std::string> faces
    {
        "resources/textures/dark/right.png",
        "resources/textures/dark/left.png",
        "resources/textures/dark/top.png",
        "resources/textures/dark/bottom.png",
        "resources/textures/dark/front.png",
        "resources/textures/dark/back.png"
    };

    unsigned int cubemapTexture = loadCubemap(faces);
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    glm::vec3 lightPos(-6.0f,1.0f,0.0f); // Initial light position


    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        do_movement(deltaTime);

        processInput(window);
        animator.UpdateAnimation(deltaTime);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 애니메이션 
        ourShader.use();

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.5f, 100.0f);

        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        auto transforms = animator.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i)
            ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-7.0f, 0.0f, 0.0f)); // 애니메이션 위치
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// 애니메이션 크기
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // 애니메이션 회전

        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);

        

        // area_light 
        shaderLTC.use();
        model = glm::mat4(1.0f);
        glm::mat3 normalMatrix = glm::mat3(model);
        shaderLTC.setMat4("model", model);
        shaderLTC.setMat3("normalMatrix", normalMatrix);
        view = camera.GetViewMatrix();
        shaderLTC.setMat4("view", view);
        projection = glm::perspective(
            glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        shaderLTC.setMat4("projection", projection);
        shaderLTC.setVec3("viewPosition", camera.Position);
        shaderLTC.setVec3("areaLightTranslate", areaLightTranslate);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mLTC.mat1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mLTC.mat2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, concreteTexture);
        renderPlane();
        glUseProgram(0);

        shaderLightPlane.use();
        model = glm::translate(model, areaLightTranslate);
        shaderLightPlane.setMat4("model", model);
        shaderLightPlane.setMat4("view", view);
        shaderLightPlane.setMat4("projection", projection);
        renderAreaLight();
        glUseProgram(0);
        
        
        // car1 shader 설정
        car1Shader.use();
        car1Shader.setVec3("lightPos", areaLightTranslate);
        car1Shader.setVec3("viewPos", camera.Position);
        car1Shader.setVec3("lightColor", LIGHT_COLOR);
        car1Shader.setMat4("projection", projection);
        car1Shader.setMat4("view", view);

        glm::mat4 model1 = glm::mat4(1.0f);
        model1 = glm::translate(model1, glm::vec3(-5.0f, 0.0f, 0.0f));
        model1 = glm::scale(model1, glm::vec3(0.1f, 0.1f, 0.1f));
        
        car1Shader.setMat4("model", model1);
        car1.Draw(car1Shader);

        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, glm::vec3(1.0f, 0.0f, 0.0f));
        model2 = glm::scale(model2, glm::vec3(0.1f, 0.1f, 0.1f));
        model2 = glm::rotate(model2, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); //rotate를 통해 obj회전
        car1Shader.setMat4("model", model2);
        car2.Draw(car1Shader);

        glm::mat4 model3 = glm::mat4(1.0f);
        model3 = glm::translate(model3, glm::vec3(-4.0f, 0.0f, 10.0f));
        model3 = glm::scale(model3, glm::vec3(0.1f, 0.1f, 0.1f));
        car1Shader.setMat4("model", model3);
        car3.Draw(car1Shader);

        glm::mat4 model4 = glm::mat4(1.0f);
        model4 = glm::translate(model4, glm::vec3(0.0f, 0.0f, 0.0f));
        model4 = glm::scale(model4, glm::vec3(0.1f, 0.1f, 0.1f));
        car1Shader.setMat4("model", model4);
        car4.Draw(car1Shader);

        glm::mat4 model5 = glm::mat4(1.0f);
        model5 = glm::translate(model5, glm::vec3(0.0f, 0.0f, 0.0f));
        model5 = glm::scale(model5, glm::vec3(0.1f, 0.1f, 0.1f));
        model5 = glm::rotate(model5, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // rotate를 통해 obj 회전
        car1Shader.setMat4("model", model5);
        car2.Draw(car1Shader);

        glm::mat4 model6 = glm::mat4(1.0f);
        model6 = glm::translate(model6, glm::vec3(-6.0f, 0.0f, 10.0f));
        model6 = glm::scale(model6, glm::vec3(0.1f, 0.1f, 0.1f));
        car1Shader.setMat4("model", model6);
        car3.Draw(car1Shader);

        glm::mat4 model7 = glm::mat4(1.0f);
        model7 = glm::translate(model7, glm::vec3(-2.0f, 0.0f, 0.0f));
        model7 = glm::scale(model7, glm::vec3(0.1f, 0.1f, 0.1f));
        car1Shader.setMat4("model", model7);
        car4.Draw(car1Shader);

        glm::mat4 model8 = glm::mat4(1.0f);
        model8 = glm::translate(model8, glm::vec3(-4.0f, 0.0f, 0.0f));
        model8 = glm::scale(model8, glm::vec3(0.1f, 0.1f, 0.1f));
        car1Shader.setMat4("model", model8);
        car1.Draw(car1Shader);

     
        

 

        // draw skybox as last
        glDepthFunc(GL_LEQUAL); 
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); 
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // depth 디폴트값



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteVertexArrays(1, &areaLightVAO);
    glDeleteBuffers(1, &areaLightVBO);


    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------

    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);

    glfwTerminate();
    return 0;

}

void do_movement(GLfloat deltaTime)
{
    float cameraSpeed = deltaTime * 3.0f;

    if (keys[GLFW_KEY_W]) {
        camera.ProcessKeyboard(FORWARD, cameraSpeed);
    }
    else if (keys[GLFW_KEY_S]) {
        camera.ProcessKeyboard(BACKWARD, cameraSpeed);
    }
    if (keys[GLFW_KEY_A]) {
        camera.ProcessKeyboard(LEFT, cameraSpeed);
    }
    else if (keys[GLFW_KEY_D]) {
        camera.ProcessKeyboard(RIGHT, cameraSpeed);
    }

    if (keys[GLFW_KEY_R]) {
        if (keys[GLFW_KEY_LEFT_SHIFT]) incrementRoughness(0.01f);
        else incrementRoughness(-0.01f);
    }

    if (keys[GLFW_KEY_I]) {
        if (keys[GLFW_KEY_LEFT_SHIFT]) incrementLightIntensity(0.025f);
        else incrementLightIntensity(-0.025f);
    }

    if (keys[GLFW_KEY_LEFT]) {
        areaLightTranslate.z += 0.01f;
    }
    if (keys[GLFW_KEY_RIGHT]) {
        areaLightTranslate.z -= 0.01f;
    }
    if (keys[GLFW_KEY_UP]) {
        areaLightTranslate.y += 0.01f;
    }
    if (keys[GLFW_KEY_DOWN]) {
        areaLightTranslate.y -= 0.01f;
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    static unsigned short wireframe = 0;

    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            return;
        case GLFW_KEY_B:
            switchTwoSided(true);
            break;
        default:
            keys[key] = true;
            break;
        }
    }

    if (action == GLFW_RELEASE)
    {
        if (key == GLFW_KEY_SPACE) {
            switch (wireframe)
            {
            case 0:
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                wireframe = 1;
                break;
            default:
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                wireframe = 0;
                break;
            }
        }
        else {
            keys[key] = false;
        }
    }
}

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
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
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
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

/*
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
*/

unsigned int loadTexture(char const* path, bool gammaCorrection)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum internalFormat;
        GLenum dataFormat;
        if (nrComponents == 1)
        {
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrComponents == 3)
        {
            internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// skybox 사용
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}