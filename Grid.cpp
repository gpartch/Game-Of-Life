#include "Grid.hpp"

Grid::Grid(QWidget *parent) : QOpenGLWidget(parent)
{
    dim = 500;
    width = dim*devicePixelRatio();
    height = dim*devicePixelRatio();
    buffer = 25;
    out = 0;

    colorfile = "../colors.txt";
    fragfile = "../gol.frag";

    alive = {0,1,1,0,0,0,0,0};
    dead = {1,1,0,1,1,1,1,1};

    probability = 20;
    t_step = 500;
    t = 0;
    iterations = 0;
    wrapping = false;

    

    // set timer properties
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
    return QSize(500, 500);
}

void Grid::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 1);

    // Load shader
    shader = new QOpenGLShaderProgram;
    // Fragment shader
    if (!shader->addShaderFromSourceFile(QOpenGLShader::Fragment,fragfile))
        qFatal() << "Error compiling" << fragfile << "\n" << shader->log();
    //  Link
    if (!shader->link())
        qFatal() << "Error linking shader\n"+shader->log();
}

void Grid::paintGL()
{
    //  Select output buffer
    out = iterations%2;
    if (!framebuffer[out]->bind()) {
        qFatal("Failed to bind framebuffer");
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0f, 1.0f, 1.0f); // Set color to white
    glViewport(0,0,width,height);

    // if(iterations == 0)
    // {
        glClear(GL_COLOR_BUFFER_BIT);
        glColor3f(1,1,1);
        // Initialize pattern
        initPattern();
    // }
    // else
    // {
    //     //  Enable shader
    //     shader->bind();
    //     //  Set offsets
    //     float dX = 1.0/width;
    //     float dY = 1.0/height;
    //     shader->setUniformValue("dX",dX);
    //     shader->setUniformValue("dY",dY);
    //     shader->setUniformValue("img",0);

    //     // Source framebuffer
    //     glBindTexture(GL_TEXTURE_2D,framebuffer[out]->texture());
    //     // glBindTexture(GL_TEXTURE_2D,framebuffer[1-out]->texture());

    //     //  Compute generation
    //     glClear(GL_COLOR_BUFFER_BIT);
    //     glEnable(GL_TEXTURE_2D);
    //     glBegin(GL_QUADS);
    //     glTexCoord2f(0,0); glVertex2f(0,0);
    //     glTexCoord2f(0,1); glVertex2f(0,height);
    //     glTexCoord2f(1,1); glVertex2f(width,height);
    //     glTexCoord2f(1,0); glVertex2f(width,0);
    //     glEnd();
    //     glDisable(GL_TEXTURE_2D);

    //     //  Done with shader
    //     shader->release();
    // }

    //  Blit to screen
    framebuffer[out]->release();
    int texture = framebuffer[out]->texture();
    glBindTexture(GL_TEXTURE_2D,texture);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex2f(0,0);
    glTexCoord2f(0,1); glVertex2f(0,height);
    glTexCoord2f(1,1); glVertex2f(width,height);
    glTexCoord2f(1,0); glVertex2f(width,0);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    //  Increment generations and display
    iterations++;
    emit viewerIterations(QString::number(iterations));
    if(t == 0) {iterations = 0; emit viewerIterations(QString::number(iterations));}

    glFlush();
}

void Grid::resizeGL(int w, int h)
{
    // Prevent division by zero
    if (h == 0) h = 1;

    // convert from device-independent pixels to actual physical pixels
    width = w*devicePixelRatio();
    height = h*devicePixelRatio();

    // set coordinate system
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    // Handle textures
    for (int k=0;k<2;k++)
    {
        if (framebuffer[k]) delete framebuffer[k];
        framebuffer[k] = new QOpenGLFramebufferObject(width,height);
    }
    setTextureProperties();

    iterations = 0;
    emit viewerIterations("0");

    t = 0;
    emit viewerElapsedTime("00:00");

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
    framebuffer[out]->bind();
    
    for(int w=0; w<width; w++)
    {
        for(int h=0; h<height; h++)
        {
            if(r_gen->bounded(100) <= probability)
            {
                // GLubyte dot[] = {0xFF};
                // glRasterPos2i(w, height - h);
                // glBitmap(1,1,0,0,0,0,dot);
                glBegin(GL_QUADS);
                    glVertex2f(w,h);
                    glVertex2f(w+1,h);
                    glVertex2f(w+1,h+1);
                    glVertex2f(w,h+1);
                glEnd();
            }
        }
    }

    // test square
    // int size = 50;
    // glBegin(GL_QUADS);
    //     glVertex2f(0,0);
    //     glVertex2f(size,0);
    //     glVertex2f(size,size);
    //     glVertex2f(0,size);

    //     glVertex2f(width,height);
    //     glVertex2f(width-size,height);
    //     glVertex2f(width-size,height-size);
    //     glVertex2f(width,height-size);
    // glEnd();
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

    emit viewerFrequency(QString::number(t_step));
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

void Grid::setTextureProperties()
{
    makeCurrent();
    for (int k=0;k<2;k++)
        if (framebuffer[k])
        {
            //  Nearest returns exact cell values
            glBindTexture(GL_TEXTURE_2D,framebuffer[k]->texture());
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
            //  Wrap to create circular universe
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,wrapping?GL_REPEAT:GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,wrapping?GL_REPEAT:GL_CLAMP_TO_EDGE);
        }
}
