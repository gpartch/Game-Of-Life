#include "Grid.hpp"

Grid::Grid(QWidget *parent) : QOpenGLWidget(parent)
{
    width = 500;
    height = 500;
    buffer = 25;

    colorfile = "../colors.txt";
    fragfile = "../gol.frag";

    alive = {0,1,1,0,0,0,0,0};
    dead = {1,1,0,1,1,1,1,1};

    probability = 20;
    t_step = 100;
    t = 0;
    iterations = -1;

    // set timer properties
    //timer.setInterval(0);
    timer.setTimerType(Qt::PreciseTimer);
    connect(&timer,SIGNAL(timeout()),this,SLOT(gridTimeout()));
    timer.start(t_step);

    r_gen = new QRandomGenerator();
    for(int i=0; i<2; i++) framebuffer[i] = nullptr;

    // Set size policy to allow scaling while maintaining asp ratio
    setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

    readColorFile(colorfile);
}
Grid::~Grid() {}

QSize Grid::sizeHint() const
{
    return QSize(width, height);
}

void Grid::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    for(int i=0; i<2; i++) framebuffer[i] = new QOpenGLFramebufferObject(width,height);

    //  Load shader
    //shader = new QOpenGLShaderProgram;
    //  Fragment shader
    // if (!shader->addShaderFromSourceFile(QOpenGLShader::Fragment,fragfile))
    //     qFatal() << "Error compiling" << fragfile << "\n" << shader->log();
    // //  Link
    // if (!shader->link())
    //     qFatal() << "Error linking shader\n"+shader->log();
}

void Grid::paintGL()
{
    //glClear(GL_COLOR_BUFFER_BIT);

    // if(iterations == -1)
    // {
        glClear(GL_COLOR_BUFFER_BIT);
        glColor3f(1,1,1);
        //  Initialize pattern
        initPattern();
        iterations++;
    //}

    glFlush();
}

void Grid::resizeGL(int w, int h)
{
    // Prevent division by zero
    if (h == 0) h = 1;

    width = w;
    height = h;

    // Set the viewport to cover the entire widget
    glViewport(0, 0, w, h);

    // Adjust the projection matrix to maintain aspect ratio
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-buffer, w+buffer, -buffer, h+buffer, -1, 1);

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

void Grid::initPattern()
{
    makeCurrent();
    glClear(GL_COLOR_BUFFER_BIT);
    for(int w=0; w<width; w++)
    {
        for(int h=0; h<height; h++)
        {
            if(r_gen->bounded(100) <= probability)
            {
                GLubyte dot[] = {0xFF};
                glRasterPos2i(w, height - h);
                glBitmap(1,1,0,0,0,0,dot);
            }
        }
    }
}

bool Grid::hasHeightForWidth() const
{
    return true; // Indicate that height depends on width
}

int Grid::heightForWidth(int width) const
{
    return width; // Maintain a 1:1 asp ratio
}

void Grid::gridTimeout()
{
    t += t_step;
    QString time = formatTime(t);
    emit viewerElapsedTime(time);
    update();
}
void Grid::gridPlay()
{
    timer.start();
}
void Grid::gridPause()
{
    timer.stop();
}
QString Grid::formatTime(int time)
{
    int iseconds = (time/1000)%60;
    int iminutes = floor(time/60000);
    QString sseconds = QString::number(iseconds);
    QString sminutes = QString::number(iminutes);
    if (sseconds.length() == 1) sseconds = "0" + sseconds;
    if (sminutes.length() == 1) sminutes = "0" + sminutes;
    if (sminutes.length() > 2) qFatal() << "Exceeded maximum play time";
    return sminutes + ":" + sseconds;
}
