cmake_minimum_required(VERSION 3.16)

project(TexasSolverGui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Network)
find_package(Threads REQUIRED)

# The solver runs on OpenCL devices. Without an OpenCL SDK on the system, the Khronos headers and ICD loader
# are fetched; the loader dispatches to whatever OpenCL drivers (NVIDIA, AMD, Intel) are installed.
find_package(OpenCL QUIET)
if(NOT OpenCL_FOUND)
    include(FetchContent)
    set(OPENCL_ICD_LOADER_BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(OPENCL_ICD_LOADER_BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
    FetchContent_Declare(OpenCLHeaders
        GIT_REPOSITORY https://github.com/KhronosGroup/OpenCL-Headers.git
        GIT_TAG v2024.10.24
        GIT_SHALLOW TRUE)
    FetchContent_Declare(OpenCLICDLoader
        GIT_REPOSITORY https://github.com/KhronosGroup/OpenCL-ICD-Loader.git
        GIT_TAG v2024.10.24
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(OpenCLHeaders OpenCLICDLoader)
endif()

# Embed the OpenCL kernels in the binary as a byte array.
set(CFR_KERNEL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/solver/gpu/cfr_kernels.cl)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CFR_KERNEL_FILE})
file(READ ${CFR_KERNEL_FILE} CFR_KERNEL_HEX HEX)
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," CFR_KERNEL_HEX "${CFR_KERNEL_HEX}")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/generated/cfr_kernels_cl.h.tmp
    "// Generated from src/solver/gpu/cfr_kernels.cl\nstatic const char kCfrKernelSource[] = {${CFR_KERNEL_HEX}0x00};\n")
configure_file(${CMAKE_CURRENT_BINARY_DIR}/generated/cfr_kernels_cl.h.tmp
    ${CMAKE_CURRENT_BINARY_DIR}/generated/cfr_kernels_cl.h COPYONLY)

# Value types and the wire protocol, shared by the solver core, the server and the GUI.
set(COMMON_SOURCES
    src/Card.cpp
    src/library.cpp
    src/api/protocol.cpp
)
set(COMMON_HEADERS
    src/Card.h
    src/library.h
    include/api/solver_dtos.h
    include/api/isolver_backend.h
    include/api/protocol.h
    include/json.hpp
    include/tools/tinyformat.h
)

set(CORE_SOURCES
    src/Deck.cpp
    src/GameTree.cpp
    src/compairer/Dic5Compairer.cpp
    src/experimental/TCfrSolver.cpp
    src/nodes/ActionNode.cpp
    src/nodes/ChanceNode.cpp
    src/nodes/GameActions.cpp
    src/nodes/GameTreeNode.cpp
    src/nodes/ShowdownNode.cpp
    src/nodes/TerminalNode.cpp
    src/ranges/PrivateCards.cpp
    src/ranges/PrivateCardsManager.cpp
    src/ranges/RiverCombs.cpp
    src/ranges/RiverRangeManager.cpp
    src/runtime/PokerSolver.cpp
    src/solver/BestResponse.cpp
    src/solver/CfrSolver.cpp
    src/solver/PCfrSolver.cpp
    src/solver/Solver.cpp
    src/solver/gpu/GpuCfrEngine.cpp
    src/solver/gpu/OpenClContext.cpp
    src/tools/CommandLineTool.cpp
    src/tools/GameTreeBuildingSettings.cpp
    src/tools/PrivateRangeConverter.cpp
    src/tools/Rule.cpp
    src/tools/StreetSetting.cpp
    src/tools/lookup8.cpp
    src/tools/progressbar.cpp
    src/tools/utils.cpp
    src/trainable/CfrPlusTrainable.cpp
    src/trainable/DiscountedCfrTrainable.cpp
    src/trainable/DiscountedCfrTrainableHF.cpp
    src/trainable/DiscountedCfrTrainableSF.cpp
    src/trainable/Trainable.cpp
)
set(CORE_HEADERS
    src/Deck.h
    src/GameTree.h
    src/compairer/Compairer.h
    src/compairer/Dic5Compairer.h
    src/experimental/TCfrSolver.h
    src/nodes/ActionNode.h
    src/nodes/ChanceNode.h
    src/nodes/GameActions.h
    src/nodes/GameTreeNode.h
    src/nodes/ShowdownNode.h
    src/nodes/TerminalNode.h
    src/ranges/PrivateCards.h
    src/ranges/PrivateCardsManager.h
    src/ranges/RiverCombs.h
    src/ranges/RiverRangeManager.h
    src/runtime/PokerSolver.h
    src/solver/BestResponse.h
    src/solver/CfrSolver.h
    src/solver/PCfrSolver.h
    src/solver/Solver.h
    src/solver/gpu/GpuCfrEngine.h
    src/solver/gpu/OpenClContext.h
    src/solver/gpu/cfr_kernels.cl
    src/tools/CommandLineTool.h
    src/tools/GameTreeBuildingSettings.h
    src/tools/PrivateRangeConverter.h
    src/tools/Rule.h
    src/tools/StreetSetting.h
    src/tools/lookup8.h
    src/tools/progressbar.h
    src/tools/utils.h
    src/trainable/CfrPlusTrainable.h
    src/trainable/DiscountedCfrTrainable.h
    src/trainable/DiscountedCfrTrainableHF.h
    src/trainable/DiscountedCfrTrainableSF.h
    src/trainable/Trainable.h
    include/tools/half-1-12-0.h
)
# The GUI talks to the solver only through RemoteBackend; it must not link TexasSolverCore.
set(GUI_SOURCES
    main.cpp
    mainwindow.cpp
    boardselector.cpp
    htmltableview.cpp
    qstextedit.cpp
    qstreeview.cpp
    rangeselector.cpp
    settingeditor.cpp
    strategyexplorer.cpp
    src/api/remote_backend.cpp
    src/api/server_process.cpp
    src/ui/boardselectortabledelegate.cpp
    src/ui/boardselectortablemodel.cpp
    src/ui/detailitemdelegate.cpp
    src/ui/detailviewermodel.cpp
    src/ui/detailwindowsetting.cpp
    src/ui/droptextedit.cpp
    src/ui/htmltablerangeview.cpp
    src/ui/rangeselectortabledelegate.cpp
    src/ui/rangeselectortablemodel.cpp
    src/ui/roughstrategyitemdelegate.cpp
    src/ui/roughstrategyviewermodel.cpp
    src/ui/strategyitemdelegate.cpp
    src/ui/tablestrategymodel.cpp
    src/ui/treeitem.cpp
    src/ui/treemodel.cpp
    src/ui/worditemdelegate.cpp
)
set(GUI_HEADERS
    mainwindow.h
    boardselector.h
    htmltableview.h
    qstextedit.h
    qstreeview.h
    rangeselector.h
    settingeditor.h
    strategyexplorer.h
    src/ui/boardselectortabledelegate.h
    src/ui/boardselectortablemodel.h
    src/ui/detailitemdelegate.h
    src/ui/detailviewermodel.h
    src/ui/detailwindowsetting.h
    src/ui/droptextedit.h
    src/ui/htmltablerangeview.h
    src/ui/rangeselectortabledelegate.h
    src/ui/rangeselectortablemodel.h
    src/ui/roughstrategyitemdelegate.h
    src/ui/roughstrategyviewermodel.h
    src/ui/strategyitemdelegate.h
    src/ui/tablestrategymodel.h
    src/ui/treeitem.h
    src/ui/treemodel.h
    src/ui/worditemdelegate.h
    include/api/remote_backend.h
    include/api/server_process.h
)
set(FORMS
    mainwindow.ui
    boardselector.ui
    rangeselector.ui
    settingeditor.ui
    strategyexplorer.ui
)
set(RESOURCES
    translations.qrc
)
if(WIN32)
    list(APPEND RESOURCES resources/windows/TexasSolverGui.rc)
endif()

add_library(TexasSolverCommon STATIC ${COMMON_SOURCES} ${COMMON_HEADERS})
target_include_directories(TexasSolverCommon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(TexasSolverCommon PUBLIC QT_DEPRECATED_WARNINGS)
target_link_libraries(TexasSolverCommon PUBLIC Qt${QT_VERSION_MAJOR}::Core)

add_library(TexasSolverCore STATIC ${CORE_SOURCES} ${CORE_HEADERS})
target_include_directories(TexasSolverCore PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)
target_compile_definitions(TexasSolverCore PUBLIC CL_TARGET_OPENCL_VERSION=120)
target_link_libraries(TexasSolverCore PUBLIC
    TexasSolverCommon
    OpenCL::OpenCL
)
add_executable(TexasSolverGui WIN32 ${GUI_SOURCES} ${GUI_HEADERS} ${FORMS} ${RESOURCES})
if(APPLE)
    set_target_properties(TexasSolverGui PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE texassolver_logo.icns
    )
endif()
target_include_directories(TexasSolverGui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(TexasSolverGui PRIVATE
    TexasSolverCommon
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

set(SERVER_SOURCES
    src/server/main.cpp
    src/api/local_backend.cpp
)

set(SERVER_HEADERS
    include/api/local_backend.h
)

# No WIN32: this must be a console program. compairer.qrc registers :/resources.
add_executable(TexasSolverServer ${SERVER_SOURCES} ${SERVER_HEADERS} compairer.qrc)
target_include_directories(TexasSolverServer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(TexasSolverServer PRIVATE
    TexasSolverCore
    Qt${QT_VERSION_MAJOR}::Network
    Threads::Threads
)

# Ship the fetched OpenCL loader next to the server, which is what runs the solver.
if(WIN32 AND TARGET OpenCL)
    add_custom_command(TARGET TexasSolverServer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:OpenCL>" "$<TARGET_FILE_DIR:TexasSolverServer>"
        COMMENT "Deploying OpenCL loader"
    )
endif()

# Console benchmark comparing the OpenCL and CPU solvers on the same spot.
option(TEXASSOLVER_BUILD_BENCH "Build the TexasSolverBench console tool" OFF)
if(TEXASSOLVER_BUILD_BENCH)
    add_executable(TexasSolverBench src/tools/bench_main.cpp)
    target_link_libraries(TexasSolverBench PRIVATE TexasSolverCore)
endif()

# Local mode launches TexasSolverServer from the GUI's own directory.
add_dependencies(TexasSolverGui TexasSolverServer)

# Copy the Qt and compiler runtime next to the binary so it can be launched
# straight out of the build directory.
if(WIN32)
    get_target_property(QMAKE_EXECUTABLE Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QMAKE_EXECUTABLE}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET TexasSolverGui POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                    --no-translations
                    --compiler-runtime
                    "$<TARGET_FILE:TexasSolverGui>"
            COMMENT "Deploying Qt runtime"
        )
        if(MINGW)
            get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
            add_custom_command(TARGET TexasSolverGui POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
                        "${MINGW_BIN_DIR}/libstdc++-6.dll"
                        "${MINGW_BIN_DIR}/libwinpthread-1.dll"
                        "$<TARGET_FILE_DIR:TexasSolverGui>"
                COMMENT "Deploying MinGW runtime"
            )
        endif()
    endif()
endif()
