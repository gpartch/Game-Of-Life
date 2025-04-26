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
//struct rule {int persist[8];};
struct rgb_f 
{
    float r,g,b;
    rgb_f(float nr, float ng, float nb)
    {
        r = nr; g = ng; b = nb;
    }
};
struct pattern
{
    QString name;
    QString rule;
    int dx,dy;
    vector<vector<bool>> pattern_grid;
    pattern() {dx = 0; dy = 0;}
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
        void initPattern();
        QString formatTime(int t);
        void setTextureProperties();
        rgb_f calcColor();
        void loadPattern(QString filename);
        QString loadRLE(QString filename, pattern& p);
        void parseRLEString(QString rle, pattern &p);
        void initPatterns();


        void mousePressEvent(QMouseEvent* e) override;
        void mouseReleaseEvent(QMouseEvent*) override;           //  Mouse released
        void mouseMoveEvent(QMouseEvent*) override;              //  Mouse moved
        void wheelEvent(QWheelEvent*) override;                  //  Mouse wheel
        
        bool hasHeightForWidth() const override;
        int heightForWidth(int width) const override;
        
    private:
    int dim;
        // window
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
        vector<pattern> patterns;
        QString patterns_dir;
        
        // color
        QString colorfile;
        std::vector<QColor> gridColors; // color gradient values
        int color_step; // the number of iterations it takes to switch colors
        int color_idx; // index of the current color being transitioned from
        
        // time
        int t_step; // the amount of time between iterations in milliseconds
        int t; // time in milliseconds
        QTimer timer;
        

    public slots:
        void gridTimeout();
        void gridPlay();
        void gridPause();
        void gridRestart();

    signals:
        void viewerElapsedTime(QString time);
        void viewerIterations(QString iterations);
        void viewerFrequency(QString frequency);

    protected:
        
};

#endif
