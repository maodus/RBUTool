#include <GLFW/glfw3.h>  // Will include glad/gl.h if needed
#include <core\Common.h>
#include <imgui_internal.h>
#include <stdio.h>

#include <gui/ChartMetaView.hpp>
#include <gui\App.hpp>
#include <gui\ChartWindow.hpp>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"

#ifdef _WIN32
#include <windows.h>

// Forward-declare your existing main
int main(int argc, char** argv);

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  // Use MSVC-provided globals to pass args along
  return main(__argc, __argv);
}
#endif

App application;

void draw(GLFWwindow* window, bool is_refresh);

static void glfw_error_callback(int error, const char* description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static void refresh(GLFWwindow* window) { draw(window, true); }

int main(int argc, char** argv) {
  // Setup GLFW
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) return 1;

  // Decide GL+GLSL versions
  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Use
  // for 3.2+ glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);          //
  // Needed on Mac

  // Create window with OpenGL context
  float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(
      glfwGetPrimaryMonitor());  // Valid on GLFW 3.3+ only
  GLFWwindow* window =
      glfwCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale),
                       "RBU Tool", nullptr, nullptr);
  if (window == NULL) return 1;

  glfwSetWindowRefreshCallback(window, refresh);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);  // Enable vsync

  // Initialize OpenGL loader (e.g. glad, glew) if needed here.
  // You must call gladLoadGL() or similar depending on your setup.

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  ImGuiStyle& style = ImGui::GetStyle();
  style.ScaleAllSizes(
      main_scale);  // Bake a fixed style scale. (until we have a solution for
                    // dynamic style scaling, changing this requires resetting
                    // Style + calling this again)
  style.FontScaleDpi =
      main_scale;  // Set initial font scale. (using io.ConfigDpiScaleFonts=true
                   // makes this unnecessary. We leave both here for
                   // documentation purpose)

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);
  int currentTab = 0;

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    draw(window, false);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}

void draw(GLFWwindow* window, bool is_refresh) {
  if (!window) return;

  if (!is_refresh) {
    glfwPollEvents();

    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      return;
    }
  }

  ImGuiIO& io = ImGui::GetIO();

  // Start ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();

  ImGui::NewFrame();
  application.render();
  ImGui::Render();

  int display_w, display_h;
  glfwGetFramebufferSize(window, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  glfwSwapBuffers(window);
}
