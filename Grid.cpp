#include "Grid.hpp"

Grid::Grid(QWidget *parent) : QOpenGLWidget(parent)
{
    dim = 1;

    // Set size policy to allow scaling while maintaining aspect ratio
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    readColorFile("../colors.txt");
}
Grid::~Grid() {}

void Grid::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void Grid::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // circle(1,20);
    float size = .5;
    glColor3f(1,1,1);
    glBegin(GL_QUADS);
        glVertex2d(-size,-size);
        glVertex2d(size,-size);
        glVertex2d(size,size);
        glVertex2d(-size,size);
    glEnd();

    glFlush();
}

void Grid::resizeGL(int w, int h)
{
    // Prevent division by zero
    if (h == 0) h = 1;

    // Calculate aspect ratio
    float aspect = static_cast<float>(w) / h;

    // Set the viewport to cover the entire widget
    glViewport(0, 0, w, h);

    // Adjust the projection matrix to maintain aspect ratio
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (aspect > 1.0f) {
        // Wider than tall
        glOrtho(-dim * aspect, dim * aspect, -dim, dim, -dim, dim);
    } else {
        // Taller than wide
        glOrtho(-dim, dim, -dim / aspect, dim / aspect, -dim, dim);
    }

    // Reset the model-view matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    update();
}

//---------------------------------------------------------------------------------------

void Grid::readColorFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODeviceBase::ReadOnly, QFileDevice::ReadUser)) {
        qWarning("Failed to open color file");
        gridColors.push_back(QColor(0,0,0));
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        // is it in rgb hex format? eg #RRGGBB
        if(line.startsWith('#') && line.length() == 7)
        {
            // remove the # symbol
            line.removeFirst();
            // parse into RR GG BB
            int r = line.mid(0,2).toInt();
            int g = line.mid(2,2).toInt();
            int b = line.mid(4,2).toInt();

            gridColors.push_back(QColor(r, g, b));
        }
        else
        {
            qWarning("Invalid color string in color file");
        }
    }
}

void Grid::circle(float r, int subsec)
{
    float dth = 360 / subsec;
    glBegin(GL_TRIANGLES);
    for(int i = 0; i <= 360; i += dth)
    {
        glVertex2d(r*Cos(i),r*Sin(i));
    }
    glEnd();
}

bool Grid::hasHeightForWidth() const
{
    return true; // Indicate that height depends on width
}

int Grid::heightForWidth(int width) const
{
    return width; // Maintain a 1:1 aspect ratio
}
