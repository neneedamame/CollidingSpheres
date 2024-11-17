#include <CGAL/Installation/internal/disable_deprecation_warnings_and_errors.h>

#include <CGAL/Surface_mesh_default_triangulation_3.h>
#include <CGAL/Complex_2_in_triangulation_3.h>
#include <CGAL/make_surface_mesh.h>
#include <CGAL/Implicit_surface_3.h>
#include <CGAL/IO/facets_in_complex_2_to_triangle_mesh.h>
#include <CGAL/Surface_mesh.h>

#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

// default triangulation for Surface_mesher
typedef CGAL::Surface_mesh_default_triangulation_3 Tr;

// c2t3
typedef CGAL::Complex_2_in_triangulation_3<Tr> C2t3;

typedef Tr::Geom_traits GT;
typedef GT::Sphere_3 Sphere_3;
typedef GT::Point_3 Point_3;
typedef GT::FT FT;

typedef FT(*Function)(Point_3);

typedef CGAL::Implicit_surface_3<GT, Function> Surface_3;

typedef CGAL::Surface_mesh<Point_3> Surface_mesh;

FT sphere_function(Point_3 p) {
    const FT x2 = p.x() * p.x(), y2 = p.y() * p.y(), z2 = p.z() * p.z();
    return x2 + y2 + z2 - 1;
}

std::vector<glm::vec3> points;
std::vector<unsigned int> indices;

void InitSphere() {
    Tr tr;            // 3D-Delaunay triangulation
    C2t3 c2t3(tr);   // 2D-complex in 3D-Delaunay triangulation

    // defining the surface
    Surface_3 surface(sphere_function,             // pointer to function
        Sphere_3(CGAL::ORIGIN, 2.)); // bounding sphere
    // Note that "2." above is the *squared* radius of the bounding sphere!

    // defining meshing criteria
    CGAL::Surface_mesh_default_criteria_3<Tr> criteria(30.,  // angular bound
        0.1,  // radius bound
        0.1); // distance bound
    // meshing surface
    CGAL::make_surface_mesh(c2t3, surface, criteria, CGAL::Non_manifold_tag());

    Surface_mesh sm;
    CGAL::facets_in_complex_2_to_triangle_mesh(c2t3, sm);
    for (const auto& vertice : sm.vertices()) {
        //std::cout << vertice.idx() << std::endl;
        const auto& point = sm.point(vertice);
        points.emplace_back(point.x(), point.y(), point.z());
    }
    for (const auto& face : sm.faces()) {
        for (const auto& v : CGAL::vertices_around_face(sm.halfedge(face), sm)) {
            indices.push_back(v.idx());
        }
    }
}

// 窗口大小调整回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// 键盘输入回调
void process_input(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// 着色器源代码
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

void main() {
    FragColor = vec4(0.2, 0.6, 0.8, 1.0); // 蓝绿色
}
)";


#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// 鼠标控制的全局变量
bool leftButtonPressed = false;  // 是否按下左键
bool rightButtonPressed = false; // 是否按下右键
float lastX = 400, lastY = 300;  // 上一次鼠标位置
float yaw = -90.0f, pitch = 0.0f; // 摄像机的旋转角度（yaw 左右旋转，pitch 上下旋转）
glm::vec3 cameraPos(0.0f, 0.0f, 3.0f);  // 摄像机位置
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f); // 摄像机方向
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);   // 摄像机的上方向
float cameraSpeed = 0.05f; // 摄像机平移速度

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS)
            leftButtonPressed = true;
        else if (action == GLFW_RELEASE)
            leftButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS)
            rightButtonPressed = true;
        else if (action == GLFW_RELEASE)
            rightButtonPressed = false;
    }
}


void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    static bool firstMouse = true; // 第一次移动鼠标时初始化位置

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xOffset = xpos - lastX;
    float yOffset = lastY - ypos; // 鼠标的 Y 方向是反的
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // 鼠标灵敏度
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    if (rightButtonPressed) {
        // 更新摄像机方向
        yaw += xOffset;
        pitch += yOffset;

        // 限制俯仰角
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        // 更新摄像机的前向向量
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);
    }

    if (leftButtonPressed) {
        // 平移摄像机（沿右方向和上方向）
        glm::vec3 right = glm::normalize(glm::cross(cameraFront, cameraUp));
        cameraPos += -xOffset * cameraSpeed * right; // 左右平移
        cameraPos += yOffset * cameraSpeed * cameraUp; // 上下平移
    }
}


int main() {
    InitSphere();

    // 初始化 GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Rendering", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback); // 鼠标按键回调
    glfwSetCursorPosCallback(window, cursor_position_callback); // 鼠标移动回调


    // 初始化 GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 创建 VAO, VBO, EBO
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // 编译顶点着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // 检查编译错误
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    // 编译片段着色器
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // 检查编译错误
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    // 链接着色器程序
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // 检查链接错误
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader Program Linking Failed:\n" << infoLog << std::endl;
    }

    // 删除着色器
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 设置投影和视图矩阵
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);


    // 渲染循环
    while (!glfwWindowShouldClose(window)) {
        process_input(window);

        // 计算视图矩阵
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // 上传到着色器
        glUseProgram(shaderProgram);
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // 上传矩阵
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        //unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);

        // 绘制填充面
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }



    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
       

}




//#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
//#include <CGAL/Surface_mesh.h>
//#include <CGAL/Sphere_3.h>
//#include <CGAL/Implicit_surface_3.h>
//
//#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
//#include <CGAL/make_surface_mesh.h>
//#include <CGAL/Implicit_surface_3.h>
//#include <CGAL/Surface_mesh_traits_generator_3.h>
//#include <CGAL/Surface_mesh_default_criteria_3.h>
//
//typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
//typedef CGAL::Surface_mesh<K::Point_3> Mesh;
//typedef K::Point_3 Point;
//typedef K::FT FT;
//
//K::FT sphere_function(const K::Point_3& p) {
//    return CGAL::squared_distance(p, K::Point_3(0, 0, 0)) - 1; // 球体方程: x^2 + y^2 + z^2 = 1
//}
//
//int main() {
//    // 使用隐式函数定义一个球体
//    typedef CGAL::Implicit_surface_3<K, decltype(&sphere_function)> Surface;
//    Surface surface(sphere_function, K::Sphere_3(Point(0, 0, 0), 2.0));
//
//    // 使用 Surface_mesh_traits_generator_3 提取特性类型
//    typedef CGAL::Surface_mesh_traits_generator_3<Surface>::Type Traits;
//
//    // 验证生成的特性类型
//    static_assert(std::is_class<Traits>::value, "Traits 类型未生成成功！");
//
//    // 定义网格生成标准
//    typedef CGAL::Surface_mesh_default_criteria_3<Traits> Criteria;
//    Criteria criteria{ 30, 0.1, 0.1 }; // 最大边长、表面误差、曲率误差
//
//    //// 创建 Surface_mesh 对象并生成网格
//    //typedef CGAL::Surface_mesh<Point> Mesh;
//    //Mesh mesh;
//    //CGAL::make_surface_mesh(mesh, surface, criteria, CGAL::Manifold_with_boundary_tag());
//
//    //if (mesh.is_empty()) {
//    //    std::cerr << "网格生成失败！" << std::endl;
//    //    return EXIT_FAILURE;
//    //}
//
//    //std::cout << "网格生成成功！" << std::endl;
//    return EXIT_SUCCESS;
//}
//
