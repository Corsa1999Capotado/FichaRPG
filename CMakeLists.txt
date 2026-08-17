cmake_minimum_required(VERSION 3.16)

project(FichaRPG VERSION 0.1.0 LANGUAGES CXX)

if(WIN32)
    enable_language(RC)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

add_executable(FichaRPG
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/character.h
    src/character.cpp
    src/fichatemplate.h
    src/fichatemplate.cpp
    src/formulaengine.h
    src/formulaengine.cpp
    src/tema.h
    src/tema.cpp
    src/gerenciadortema.h
    src/gerenciadortema.cpp
    src/temadialog.h
    src/temadialog.cpp
    src/exportador.h
    src/exportador.cpp
    src/armazenamento.h
    src/armazenamento.cpp
    src/cardficha.h
    src/cardficha.cpp
    src/telamenu.h
    src/telamenu.cpp
    src/telaedicao.h
    src/telaedicao.cpp
    src/telavisualizacao.h
    src/telavisualizacao.cpp
    src/editornotas.h
    src/editornotas.cpp
    src/gerenciarpastasdialog.h
    src/gerenciarpastasdialog.cpp
    src/preferencias.h
    src/preferencias.cpp
    src/painelfichas.h
    src/painelfichas.cpp
    src/cardpainel.h
    src/cardpainel.cpp
    src/painellateralfichas.h
    src/painellateralfichas.cpp
    src/protegerdados.h
    src/protegerdados.cpp
    src/httpsincrono.h
    src/httpsincrono.cpp
    src/googleauth.h
    src/googleauth.cpp
    src/googledrive.h
    src/googledrive.cpp
    src/sincronizadordrive.h
    src/sincronizadordrive.cpp
    src/googledrivedialog.h
    src/googledrivedialog.cpp
    src/contadialog.h
    src/contadialog.cpp
    src/imagemutil.h
    src/imagemutil.cpp
    src/ajustarfotodialog.h
    src/ajustarfotodialog.cpp
    resources/icons.qrc
)

if(WIN32)
    target_sources(FichaRPG PRIVATE resources/app.rc)
endif()

target_include_directories(FichaRPG PRIVATE src)

target_link_libraries(FichaRPG PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)

if(WIN32)
    set_target_properties(FichaRPG PROPERTIES WIN32_EXECUTABLE TRUE)
    target_link_libraries(FichaRPG PRIVATE crypt32)
endif()
