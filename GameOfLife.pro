HEADERS +=  Viewer.hpp \
            Grid.hpp

SOURCES +=  Viewer.cpp \
            Grid.cpp \
            GameOfLife.cpp

LIBS += -lopengl32

CONFIG += console

QT += core opengl gui widgets openglwidgets
