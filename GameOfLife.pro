HEADERS +=  Pattern.hpp \
            Grid.hpp \
            Viewer.hpp 
            
            

SOURCES +=  Pattern.cpp \
            Grid.cpp \
            Viewer.cpp \
            GameOfLife.cpp 
            

LIBS += -lopengl32

CONFIG += console

QT += core opengl gui widgets openglwidgets
