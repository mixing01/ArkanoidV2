#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "shader.h"
#include "stb_image.h"
#include "Model.h"
#include "constants.h"
#include "Block.h"
#include "Ball.h"
#include "Shield.h"
//parametry poczatkowe ekranu
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
float aspectRatio = (float)800/600;

unsigned int lives = 3;

bool endgame = false;

//zmienne odpowiedzialne za kontrole kamery
float zoom = 0;
float view_type = 1;

//kontrola paletki (tarczy)
float pad_speed = 0.09;
bool V_once = true;
bool ball_released = false;
bool ball_not_moving = true;
float shieldOffset = 0.0f;

//deklaracja modeli
Model tower = Model();
Model plane = Model();
Model wall = Model();
Shield shield = Shield();
glm::mat4 shieldM;
glm::mat4 ballM;
glm::mat4 ballMnew;
Block box = Block();
Ball ball = Ball();
Block bloki[4][7];
Block move1;
Block move2;
Block move3;

//pozycje wzgledne i predkosci ruchomych blokow
float t1 = 0, t2 = 0, t3 = 0;
float speed1 = 0.04, speed2 = -0.06, speed3 = 0.05;

//wspolrzedne ostatnio dotknietego klocka (dla klockow ruchomych odpowiednio (10,10), (20,20) i (30,30)
int lastTouchI = -1;
int lastTouchJ = -1;

//vertex array object (bufor) dla pod³o¿a - przechowuje tablice wierzcho³ków
unsigned int planeVAO;

void loadTexture(unsigned int* texture, const char* filename) {
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Blad tekstury" << std::endl;
    }
    stbi_image_free(data);
}

//deklaracja funkcji
void inputHandle(GLFWwindow* window);
void windowResizeCallback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void renderAll(Shader& shader, unsigned int* textures);
void drawBoxes(Shader& shader, unsigned int* textures);
void drawBall(Shader& shader);
bool blockCollisions(Block block, int i, int j);
bool kierunekOdbicia(glm::vec2 blockCenter, int i, int j);

void initProgram(GLFWwindow* window)
{
    glViewport(0, 0, 800, 600);
    glfwSetWindowSizeCallback(window, windowResizeCallback);
    glfwSetScrollCallback(window, scroll_callback);
}

int main()
{
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(800, 600, "Arkanoid", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
     
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    initProgram(window);

    //wczytanie modeli
    tower = Model("tower.obj");
    plane = Model("plane.obj");
    wall = Model("wall.obj");
    shield = Shield("shield.obj");
    box = Block("box.obj");
    ball = Ball("ball.obj");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 7; j++) {
            bloki[i][j] = box;
            if (i > 2)
                bloki[i][j].hp = 2;
            else
                bloki[i][j].hp = 1;
            bloki[i][j].X = -6 + 2 * j;
            bloki[i][j].Z = 9 + 2 * i;


        }
    }
    move1 = box;
    move2 = box;
    move3 = box;
    move1.hp = 2;
    move2.hp = 1;
    move3.hp = 1;
    move1.Z = 7;
    move1.X = 0;
    move2.Z = 5;
    move2.X = 0;
    move3.Z = 3;
    move3.X = 0;

    Shader phong = Shader("vertex.glsl", "fragment.glsl");

    //wczytanie tekstur
    unsigned int texture;
    loadTexture(&texture, "bricks.png");

    unsigned int texture2;
    loadTexture(&texture2, "tiles.jpg");

    unsigned int texture3;
    loadTexture(&texture3, "fire.jpg");

    unsigned int texture4;
    loadTexture(&texture4, "box.jpg");

    unsigned int texture5;
    loadTexture(&texture5, "zbysiu.jpg");

    unsigned int textures[] = { texture, texture2, texture3, texture4, texture5};

    //bufor wierzcho³ków
    unsigned int planeVBO;
    float planeVertices[] = {
         9.0f, -0.5f,  9.0f,  0.0f, 1.0f, 0.0f,   9.0f,  0.0f,
        -9.0f, -0.5f,  9.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -9.0f, -0.5f, -9.0f,  0.0f, 1.0f, 0.0f,   0.0f,  9.0f,

         9.0f, -0.5f,  9.0f,  0.0f, 1.0f, 0.0f,   9.0f,  0.0f,
        -9.0f, -0.5f, -9.0f,  0.0f, 1.0f, 0.0f,   0.0f,  9.0f,
         9.0f, -0.5f, -9.0f,  0.0f, 1.0f, 0.0f,   9.0f,  9.0f
    };
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    float angle = 0;

    while (!glfwWindowShouldClose(window) && endgame==false)
    {
        inputHandle(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::vec4 lightPos = glm::vec4(0.0f, 10.0f, 5.0f, 1.0f);
        float near_plane = 1.0f, far_plane = 50.0f;
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

        glm::mat4 lightView = glm::lookAt(glm::vec3(lightPos[0], lightPos[1], lightPos[2]),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        phong.use();
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);
        
        if (view_type == -1) {
            //rzut z góry
            view = glm::lookAt(glm::vec3(0.0f, 30.0f, 0.0f), glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else {
            //rzut g³ówny
            view = glm::lookAt(glm::vec3(0.0f, 20.0f+zoom, -20.0f-zoom), glm::vec3(0.0f, 5.0f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        }
        projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
        phong.setMat4("view", view);
        phong.setMat4("projection", projection);
        phong.setVec3("lightPos", lightPos[0],lightPos[1],lightPos[2]);
        phong.setVec3("viewPos", 0.0f, 20.0f + zoom, -20.0f - zoom);
        phong.setVec3("lightColor", 1.0, 1.0, 1.0);
        renderAll(phong, textures);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        angle = PI/50 * glfwGetTime();
        if (angle >= 2 * PI)
            glfwSetTime(0);
    }
    glfwTerminate();
    return 0;
}

void renderAll(Shader& shader, unsigned int* textures) {    

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[1]);    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    drawBall(shader);

    drawBoxes(shader, textures);
    glBindTexture(GL_TEXTURE_2D, textures[4]);
    shieldM = glm::translate(model, glm::vec3(shieldOffset, 0.0f, -10.5f));
    ballM = glm::translate(shieldM,glm::vec3(0.0f,-0.1f,2.0f));
    shieldM = glm::rotate(shieldM, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    shader.setMat4("model", glm::scale(shieldM,glm::vec3(0.03f,0.03f,0.03f)));
    shield.Draw(shader);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    shader.setMat4("model", glm::translate(model, glm::vec3(-8.0f, -1.0f, -8.0f)));
    tower.Draw(shader);
    shader.setMat4("model", glm::translate(model, glm::vec3(8.0f, -1.0f, -8.0f)));
    tower.Draw(shader);
    shader.setMat4("model", glm::translate(model, glm::vec3(-8.0f, -1.0f, 8.0f)));
    tower.Draw(shader);
    shader.setMat4("model", glm::translate(model, glm::vec3(8.0f, -1.0f, 8.0f)));
    tower.Draw(shader);
    shader.setMat4("model", glm::translate(model, glm::vec3(-9.0f, -1.0f, 0.0f)));
    wall.Draw(shader);
    shader.setMat4("model", glm::translate(model, glm::vec3(7.0f, -1.0f, 0.0f)));
    wall.Draw(shader);
    shader.setMat4("model", glm::rotate(glm::translate(model, glm::vec3(0.0f, -1.0f, 9.0f)), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    wall.Draw(shader);
}

void inputHandle(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        if (shieldOffset + pad_speed < 6) {
            shield.X_left += pad_speed;
            shield.X_right += pad_speed;
            shieldOffset += pad_speed;
            if(!ball_released)
                ball.center.x += pad_speed;
        }
            
    }   
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        if (shieldOffset - pad_speed > -6) {
            shield.X_left -= pad_speed;
            shield.X_right -= pad_speed;
            shieldOffset -= pad_speed;
            if(!ball_released)
                ball.center.x -= pad_speed;
        }
            
    }

    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
        if (V_once) {
            view_type *= -1;
            V_once = false;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (!ball_released) {
            ball_released = true;
        }
    }
        

    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE)
        V_once = true;
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    if (height == 0) return;
    aspectRatio = (float)width / (float)height;
    glViewport(0, 0, width, height);
    SCR_HEIGHT = height;
    SCR_WIDTH = width;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (zoom + yoffset<5 && zoom + yoffset > -5) {
        zoom += yoffset;
    }
}

void drawBoxes(Shader& shader, unsigned int* textures) {
    glm::mat4 firstM = glm::mat4(1.0f);
    firstM = glm::translate(firstM, glm::vec3(-6.0f, 0.25f, -0.10f));
    glm::mat4 rowM = firstM;
    for (int i = 0; i < 4; i++) {
        glm::mat4 colM = rowM;
        for (int j = 0; j < 7; j++) {
            if (bloki[i][j].hp > 0) {
                shader.setMat4("model", colM);
                if(bloki[i][j].hp==2)
                    glBindTexture(GL_TEXTURE_2D, textures[4]);
                else
                    glBindTexture(GL_TEXTURE_2D, textures[3]);
                bloki[i][j].Draw(shader);
            }
            colM = glm::translate(colM, glm::vec3(2.0f, 0.0f, 0.0f));
        }
        rowM = glm::translate(rowM, glm::vec3(0.0f, 0.0f, 2.0f));
    }
    if(move1.hp==2)
        glBindTexture(GL_TEXTURE_2D, textures[4]);
    else if(move1.hp==1)
        glBindTexture(GL_TEXTURE_2D, textures[3]);
    if (move1.hp > 0) {
        glm::mat4 M1 = glm::translate(firstM, glm::vec3(6.0f + t1, 0.0f, -1.9f));
        shader.setMat4("model", M1);
        move1.Draw(shader);
    }
    if (move2.hp == 2)
        glBindTexture(GL_TEXTURE_2D, textures[4]);
    else if (move2.hp == 1)
        glBindTexture(GL_TEXTURE_2D, textures[3]);
    if (move2.hp > 0) {
        glm::mat4 M1 = glm::translate(firstM, glm::vec3(6.0f + t2, 0.0f, -3.9f));
        shader.setMat4("model", M1);
        move2.Draw(shader);
    }
    if (move3.hp == 2)
        glBindTexture(GL_TEXTURE_2D, textures[4]);
    else if (move3.hp == 1)
        glBindTexture(GL_TEXTURE_2D, textures[3]);
    if (move3.hp > 0) {
        glm::mat4 M1 = glm::translate(firstM, glm::vec3(6.0f + t3, 0.0f, -5.9f));
        shader.setMat4("model", M1);
        move3.Draw(shader);
    }
    if (t1 + speed1 > 6.0f || t1 + speed1 < -6)
        speed1 *= -1;
    if (t2 + speed2 > 6.0f || t2 + speed2 < -6)
        speed2 *= -1;
    if (t3 + speed3 > 6.0f || t3 + speed3 < -6)
        speed3 *= -1;
    t1 += speed1;
    t2 += speed2;
    t3 += speed3;
    move1.X = t1;
    move2.X = t2;
    move3.X = t3;

}

void drawBall(Shader& shader) {
    bool isBlockLeft = false;
    //cout << "Kulka X: " << ball.center.x << " Kulka Z: " << ball.center.y << " Lewy: " << shield.X_left << " Prawy: " << shield.X_right << endl;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 7; j++) {
            if (bloki[i][j].hp > 0)
                isBlockLeft = true;
            if (blockCollisions(bloki[i][j],i,j)) {
                bloki[i][j].hp--;
                lastTouchI = i;
                lastTouchJ = j;
            }
        }
    }
    if (move1.hp > 0)
        isBlockLeft = true;
    if (blockCollisions(move1, 10, 10)) {
        move1.hp--;
        lastTouchI = 10;
        lastTouchJ = 10;
    }
    if (move2.hp > 0)
        isBlockLeft = true;
    if (blockCollisions(move2, 20, 20)) {
        move2.hp--;
        lastTouchI = 20;
        lastTouchJ = 20;
    }
    if (move3.hp > 0)
        isBlockLeft = true;
    if (blockCollisions(move3, 30, 30)) {
        move3.hp--;
        lastTouchI = 30;
        lastTouchJ = 30;
    }
    ball.wallCollisions(shield, &lastTouchI, &lastTouchJ);
    shader.setVec3("ballPos", ball.center.x, 0.0f, ball.center.y-8.5);
    if (ball_released) {
        if (ball_not_moving) {
            ball.initV();
            ball_not_moving = false;
            ballMnew = glm::translate(ballM, ball.v);
        }

        ballMnew = glm::translate(ballMnew, ball.v);
        ball.center.x += ball.v[0];
        ball.center.y += ball.v[2];
        
        shader.setMat4("model", ballMnew);
        ball.Draw(shader);
        
    }
    else {
        shader.setMat4("model", ballM);
        ball.Draw(shader);
    }
    if (ball.center.y < -20 || ball.center.x > 20 || ball.center.x < -20) {
        ball_released = false;
        ball_not_moving = true;
        ball.v = glm::vec3(0.0f, 0.0f, 0.0f);
        ball.center.y = 0;
        ball.center.x = shield.X_left + 1.5;
        lives--;
        if (lives == 0)
            endgame = true;
    }
    if (!isBlockLeft) {
        endgame = true;
    }
}

bool blockCollisions(Block block, int i, int j) {
    bool result = false;
    glm::vec2 polowaBloku = glm::vec2(0.75f, 0.75f);
    glm::vec2 blockCenter = glm::vec2(block.X, block.Z);
    glm::vec2 d = ball.center - blockCenter;
    glm::vec2 uciety = glm::clamp(d, -polowaBloku, polowaBloku);
    glm::vec2 najblizszy = blockCenter + uciety;
    glm::vec2 dNaj = najblizszy - ball.center;
    if (i != lastTouchI || j != lastTouchJ) {
        if (glm::length(dNaj) < ball.r) {
            if (block.hp > 0) {
                result = kierunekOdbicia(d, i, j);
            }
           
        }
    }

    return result;
}

bool moveCollisions(Block block, int i, int j) {
    bool result = false;
    glm::vec2 polowaBloku = glm::vec2(0.75f, 0.75f);
    glm::vec2 blockCenter = glm::vec2(block.X, block.Z);
    glm::vec2 d = ball.center - blockCenter;
    glm::vec2 uciety = glm::clamp(d, -polowaBloku, polowaBloku);
    glm::vec2 najblizszy = blockCenter + uciety;
    glm::vec2 dNaj = najblizszy - ball.center;
    if (i != lastTouchI || j != lastTouchJ) {
        if (glm::length(dNaj) < ball.r) {
            if (block.hp > 0) {
                result = kierunekOdbicia(d, i, j);
            }

        }
    }

    return result;
}

bool kierunekOdbicia(glm::vec2 doPilki, int i, int j) {
    glm::vec2 kierunki[] = {
        glm::vec2(0.0f, 1.0f),	// gora
        glm::vec2(1.0f, 0.0f),	// prawo
        glm::vec2(0.0f, -1.0f),	// dol
        glm::vec2(-1.0f, 0.0f)	// lewo
    };
    float max = 0.0f;
    int najblizej = -1;
    for (int i = 0; i < 4; i++)
    {
        //najwiêkszy iloczyn skalarny dla cos x = 1 - czyli k¹ta 0 stopni.
        //Im wiêkszy iloczyn skalarny tym blizej tego kierunku (sprawdzane kierunki w tablicy kierunki)
        float iloczynSkalarny = glm::dot(glm::normalize(doPilki), kierunki[i]);
        if (iloczynSkalarny > max)
        {
            max = iloczynSkalarny;
            najblizej = i;
        }
    }
    cout << najblizej << endl;
    if (najblizej == 0 || najblizej == 2) {
        lastTouchI = i;
        lastTouchJ = j;
        ball.v.z *= -1;
        return true;
    }
    else {
        lastTouchI = i;
        lastTouchJ = j;
        ball.v.x *= -1;
        return true;
    }
}