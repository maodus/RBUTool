cmake_minimum_required(VERSION 3.15)
project(RBUTool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW settings
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_INSTALL OFF CACHE BOOL "" FORCE)


add_subdirectory(external/glfw)
add_subdirectory(external/midifile)

set(IMGUI_SRC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui PUBLIC
    external/imgui
    external/imgui/backends
    external/glfw/include   
)

add_library(imguifiledialog STATIC
    external/ImGuiFileDialog/ImGuiFileDialog.cpp
)

target_include_directories(imguifiledialog PUBLIC
    external/imgui
    external/ImGuiFileDialog
)
# Use std::filesystem in the dialog
target_compile_definitions(imguifiledialog PUBLIC IGFD_USE_STD_FILESYSTEM)

# Create main app
set(PROJECT_SRC
    # Entry point
    src/main.cpp

    # GUI
    src/gui/App.cpp
    src/gui/AudioConversion.cpp
    src/gui/ChartWindow.cpp
    src/gui/ChartMetaView.cpp
    src/gui/components/TrackView.cpp

    # Core
    src/core/RBUProcessor.cpp
    src/core/Tempo.cpp
    src/core/Measure.cpp
    src/core/Beat.cpp
    src/core/TrackEventManager.cpp

    # Core: MIDI
    src/core/midi/MidiProcessor.cpp

    # Core: Post-processing
    src/core/post/Passes.cpp
    src/core/post/PostPipeline.cpp

    # Core: ATM
    src/core/atm/ATMHeader.cpp
    src/core/atm/ATMContainer.cpp

    # Core: Selection
    src/core/selection/SelectionManager.cpp

    # Crypto
    src/crypto/HC128.cpp

    # Tools
    src/tool/FFmpegRunner.cpp
    src/tool/ExeRunner.cpp
    src/tool/AT3ToolRunner.cpp
)

add_executable(RBUTool ${PROJECT_SRC})

set_target_properties(RBUTool PROPERTIES
    WIN32_EXECUTABLE $<CONFIG:Release>
)

target_compile_definitions(RBUTool PRIVATE IMGUI_DEFINE_MATH_OPERATORS)


# Add include paths
target_include_directories(RBUTool PRIVATE
    include
)

# Force-export headers for our build
target_include_directories(midifile PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/midifile/include
)

find_package(OpenGL REQUIRED)

target_link_libraries(RBUTool PRIVATE
    imgui
    glfw
    imguifiledialog
    OpenGL::GL
    midifile
)