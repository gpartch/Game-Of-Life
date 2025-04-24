#include "Grid.hpp"

Grid::Grid(QWidget *parent) : QOpenGLWidget(parent)
{
    dim = 500;
    width = dim*devicePixelRatio();
    height = dim*devicePixelRatio();
    border = 25;
    out = 0;
    color_step = 10;
    color_idx = 0;

    zoom = 1;
    user_pos.setX(.5);
    user_pos.setY(.5);

    colorfile = "../colors.txt";
    fragfile = "../gol.frag";

    alive = {0,1,1,0,0,0,0,0};
    dead = {1,1,0,1,1,1,1,1};

    probability = 30;
    t_step = 100;
    t = 0;
    iterations = 0;
    wrapping = true;

    skip_iteration = false;

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
    glViewport(0,0,width,height);

    // if the time between iterations has passed, compute a new generation
    if (!skip_iteration)
    {
        //  Select and bind output buffer
        out = iterations%2;
        if (!framebuffer[out]->bind()) {
            qFatal("Failed to bind framebuffer");
        }

        glClear(GL_COLOR_BUFFER_BIT);
        rgb_f color = calcColor();

        if(iterations == 0)
        { 
            glClear(GL_COLOR_BUFFER_BIT);
            glColor3f(1.0,1.0,1.0);
            initPattern();
        }
        else
        {
            //  Enable shader
            shader->bind();
            //  Set offsets
            float dX = 1.0/width;
            float dY = 1.0/height;
            shader->setUniformValue("dX",dX);
            shader->setUniformValue("dY",dY);
            shader->setUniformValue("img",0);

            shader->setUniformValue("red",color.r);
            shader->setUniformValue("green",color.g);
            shader->setUniformValue("blue",color.b);

            // Source framebuffer
            glBindTexture(GL_TEXTURE_2D,framebuffer[1-out]->texture());

            //  Compute generation
            glClear(GL_COLOR_BUFFER_BIT);
            glEnable(GL_TEXTURE_2D);
            glBegin(GL_QUADS);
            glTexCoord2f(0,0); glVertex2f(0,0);
            glTexCoord2f(0,1); glVertex2f(0,height);
            glTexCoord2f(1,1); glVertex2f(width,height);
            glTexCoord2f(1,0); glVertex2f(width,0);
            glEnd();
            glDisable(GL_TEXTURE_2D);

            //  Done with shader
            shader->release();
        }
        framebuffer[out]->release();

        //  Increment generations and display
        iterations++;
        emit viewerIterations(QString::number(iterations));
        if(t == 0) {iterations = 0; emit viewerIterations(QString::number(iterations));}
    }

    double left = qBound(0.0, (0.5 - zoom / 2.0) + user_pos.x(), 1.0 - zoom);
    double right = qBound(zoom, (0.5 + zoom / 2.0) + user_pos.x(), 1.0);
    double bottom = qBound(0.0, (0.5 - zoom / 2.0) + user_pos.y(), 1.0 - zoom);
    double top = qBound(zoom, (0.5 + zoom / 2.0) + user_pos.y(), 1.0);

    // using brute force, make sure the texture box stays square
    // if(left == 0 && right != 1) right = left + zoom;
    // else if (right == 1 && left != 0) left = right - zoom;

    // if(bottom == 0 && top != 1) top = bottom + zoom;
    // else if (top == 1 && bottom != 0) bottom = top - zoom;

    // qInfo() << "-------------------";
    // qInfo() << "left:" << left << "right:" << right << "bottom:" << bottom << "top:" << top;
    // qInfo() << "right-left:" << right-left << "top-bottom:" << top-bottom;
    // qInfo() << "zoom:" << zoom;

    //  Print to screen
    int texture = framebuffer[out]->texture();
    glBindTexture(GL_TEXTURE_2D,texture);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2f(left,bottom); glVertex2f(0+border,0+border);
    glTexCoord2f(left,top); glVertex2f(0+border,height-border);
    glTexCoord2f(right,top); glVertex2f(width-border,height-border);
    glTexCoord2f(right,bottom); glVertex2f(width-border,0+border);
    glEnd();
    glDisable(GL_TEXTURE_2D);

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

    //qInfo() << "width:" << width << "height:" << height;

    skip_iteration = false;
    update();
}
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
            int r = line.mid(0,2).toInt(nullptr, 16);
            int g = line.mid(2,2).toInt(nullptr, 16);
            int b = line.mid(4,2).toInt(nullptr, 16);

            //qInfo() << "Adding color" << r << g << b;
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
    skip_iteration = false;
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
void Grid::gridRestart()
{
    t = 0;
    emit viewerElapsedTime("00:00");

    iterations = 0;
    emit viewerIterations("0");

    skip_iteration = false;
    update();
}
rgb_f Grid::calcColor()
{
    // if the transition color has been fully transitioned to, change current color to transition color
    if(iterations % color_step == 0 && iterations != 0) color_idx = (color_idx + 1) % gridColors.size();

    // get current color and color being transitioned to
    QColor c1 = gridColors.at(color_idx);
    QColor c2 = gridColors.at((color_idx + 1)% gridColors.size());

    // get the degree of transition
    int d = iterations % color_step;

    // get the differences between red, green, and blue values
    float r_diff = c2.redF() - c1.redF();
    float g_diff = c2.greenF() - c1.greenF();
    float b_diff = c2.blueF() - c1.blueF();

    // get the change in color value per step
    float dr = r_diff/color_step;
    float dg = g_diff/color_step;
    float db = b_diff/color_step;

    // calculate color components
    float red = c1.redF() + dr * d;
    float green = c1.greenF() + dg * d;
    float blue = c1.blueF() + db * d;

    // assemble color
    return rgb_f(qBound(0.0f,red,1.0f),qBound(0.0f,green,1.0f),qBound(0.0f,blue,1.0f));
}
void Grid::mousePressEvent(QMouseEvent* event)
{
    //qInfo() << "event:" << event->x() << event->y();
    if (event->button() == Qt::LeftButton)
    {
        mouse_click = true;
        mouse_pos = event->pos();
    }
    //  Remember mouse location
}
void Grid::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) mouse_click = false;
}
void Grid::mouseMoveEvent(QMouseEvent* event)
{
    // Only pan when the left mouse button is clicked
    if (mouse_click) {
        // Calculate the change in mouse position
        QPoint diff = event->pos() - mouse_pos;
        mouse_pos = event->pos();

        // Adjust user_pos based on the mouse movement and zoom level
        user_pos.rx() -= diff.x() / (double)width * zoom;
        user_pos.ry() += diff.y() / (double)height * zoom;

        // Adjust clamping range based on zoom and aspect ratio
        double aspect = (double)width / height;
        double zoomX = zoom;
        double zoomY = zoom;
        if (aspect > 1.0) {
            zoomY /= aspect;
        } else {
            zoomX *= aspect;
        }

        // Clamp user_pos to ensure the visible area stays within bounds
        user_pos.setX(qBound(zoomX / 2.0 - 0.5, user_pos.x(), 0.5 - zoomX / 2.0));
        user_pos.setY(qBound(zoomY / 2.0 - 0.5, user_pos.y(), 0.5 - zoomY / 2.0));

        // Trigger a redraw
        skip_iteration = true;
        
        //update();
        repaint();
    }
    
}
void Grid::wheelEvent(QWheelEvent* event)
{
    if (event->angleDelta().y() > 0) {
        zoom -= .1; // Zoom in
    } else {
        zoom += .1; // Zoom out
    }
    if(zoom > 1) zoom = 1;
    else if(zoom < .1) zoom = .1;
    skip_iteration = true;
    update();
}
