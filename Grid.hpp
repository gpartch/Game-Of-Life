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
#include <QRandomGenerator>
#include <QOpenGLFramebufferObject>
#include <QTimer>
#include <QtGlobal>
#include <QMouseEvent>
#include <QTextStream>
#include <QQueue>
#include <QDir>
#include <QWindow>

#include "Pattern.hpp"

using std::stoi;
using std::vector;

struct rgb_f 
{
    float r,g,b;
    rgb_f(float nr, float ng, float nb)
    {
        r = nr; g = ng; b = nb;
    }
};

#define Cos(x) (cos((x)*3.14159265/180))
#define Sin(x) (sin((x)*3.14159265/180))

class Grid : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
    
    public:
        Grid(QWidget *parent = nullptr);
        ~Grid();

        QSize sizeHint() const override;

        // GL functions
        void initializeGL() override;
        void paintGL() override;
        void resizeGL(int w, int h) override;

        void readColorFile(const QString filename);
        void initGrid();
        QString formatTime(int t);
        void setTextureProperties();
        rgb_f calcColor();
        
        void initPatterns();

        void mousePressEvent(QMouseEvent* e) override;
        void mouseReleaseEvent(QMouseEvent*) override;           //  Mouse released
        void mouseMoveEvent(QMouseEvent*) override;              //  Mouse moved
        void wheelEvent(QWheelEvent*) override;                  //  Mouse wheel
        
        bool hasHeightForWidth() const override;
        int heightForWidth(int width) const override;
        
    private:
        // window
        int dim;
        int width;
        int height;
        int border; // border around gol window

        // user interactivity 
        QPointF user_pos; // user position in terms of texture coordinates, eg 0-1 in x and y directions
        QPoint mouse_pos; // mouse position on window in screen coordinates, eg pixels
        float zoom; // window zoom, 1-.1,, also represents the length and width of the bounding box in texture coordinates
        bool L_click; // on when left mouse button is clicked
        
        // state management / other
        QString fragfile;
        bool wrapping;
        int probability; // when generating texture, the probability that a given pixel is black
        int out; // output buffer
        int iterations; // counter for the number of iterations
        bool skip_iteration;
        int selected_pattern;
        QRandomGenerator* r_gen;
        QOpenGLFramebufferObject* framebuffer[2];
        QOpenGLShaderProgram* shader;
        vector<Pattern*> patterns;
        QString patterns_dir;
        int unnamed_pattern_ctr;
        
        // color
        QString colorfile;
        std::vector<QColor> gridColors; // color gradient values
        int color_step; // the number of iterations it takes to switch colors
        int color_idx; // index of the current color being transitioned from
        
        // time
        int t_step; // the amount of time between iterations in milliseconds
        int t; // time in milliseconds
        QTimer timer; // track time
        QTimer iter_timer; // track iterations interval

    public slots:
        void clockTimeout();
        void iterationTimeout();
        void gridPlay();
        void gridPause();
        void gridRestart();
        void gridLoadPattern(int);
        void gridSetFrequency(double);

    signals:
        void viewerElapsedTime(QString time);
        void viewerIterations(QString iterations);
        void viewerFrequency(QString frequency);
        void viewerAddPattern(QString);
};

#endif
