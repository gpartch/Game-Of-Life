HEADERS +=  Viewer.hpp \
            Grid.hpp

SOURCES +=  Viewer.cpp \
            Grid.cpp \
            GameOfLife.cpp

LIBS += -lopengl32

QT += core opengl gui widgets openglwidgets
