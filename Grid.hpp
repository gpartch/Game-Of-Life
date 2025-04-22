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

using std::stoi;
using std::vector;

// Conway's Game Of Life:
/* alive:
[1] - 0
[2] - 1
[3] - 1
[4] - 0
[5] - 0
[6] - 0
[7] - 0
[8] - 0
*/
/* dead:
[1] - 1
[2] - 1
[3] - 0
[4] - 1
[5] - 1
[6] - 1
[7] - 1
[8] - 1
*/

// does the state persist or flip based on the number of neighbors, 0 if it flips, 1 if the state stays the same
struct rule {int persist[8];};
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

        void readColorFile(const QString &filename);
        void initPattern();
        QString formatTime(int t);
        void setTextureProperties();
        rgb_f calcColor();
        
        bool hasHeightForWidth() const override;
        int heightForWidth(int width) const override;
        
    private:
    int dim;
        int width;
        int height;
        int buffer;
        QString colorfile;
        QString fragfile;
        int probability; // when generating texture, the probability that a given pixel is black
        bool wrapping;
        int out; // output buffer
        int color_step; // the number of iterations it takes to switch colors
        int color_idx; // index of the current color being transitioned from
        
        std::vector<QColor> gridColors; // color gradient values
        rule alive;
        rule dead;
        int iterations; // counter for the number of iterations
        float t_step; // the amount of time between iterations in milliseconds
        int t; // time in milliseconds

        QTimer timer;
        QRandomGenerator* r_gen;
        QOpenGLFramebufferObject* framebuffer[2];
        QOpenGLShaderProgram* shader;

    public slots:
        void gridTimeout();
        void gridPlay();
        void gridPause();
        void gridRestart();

    signals:
        void viewerElapsedTime(QString time);
        void viewerIterations(QString iterations);
        void viewerFrequency(QString frequency);
};

#endif
