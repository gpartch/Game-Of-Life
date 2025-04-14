#ifndef GRID_HPP
#define GRID_HPP

#include <vector>
#include <iostream>
#include <string>
#include <cmath>

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QColor>
#include <QFile>

using std::stoi;

#define Cos(x) (cos((x)*3.14159265/180))
#define Sin(x) (sin((x)*3.14159265/180))

class Grid : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
    QSize sizeHint() const {return QSize(800,800);}

    public:
        Grid(QWidget *parent = nullptr);
        ~Grid();

        // GL functions
        void initializeGL() override;
        void paintGL() override;
        void resizeGL(int w, int h) override;

        void readColorFile(const QString &filename);
        void circle(float r, int subsec);
        
        bool hasHeightForWidth() const override;
        int heightForWidth(int width) const override;
        
    private:
        int dim;
        
        std::vector<QColor> gridColors;
        static const int GRID_SIZE = 100;
        bool grid[GRID_SIZE][GRID_SIZE] = {false};
        //QOpenGLShaderProgram *program;
};

#endif
