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
    selected_pattern = -1;
    patterns_dir = "../patterns/";
    unnamed_pattern_ctr = 0;

    zoom = 1;
    user_pos.setX(0);
    user_pos.setY(0);

    colorfile = "../colors.txt";
    fragfile = "../gol.frag";

    probability = 30;
    t_step = 100;
    t = 0;
    iterations = 0;
    wrapping = true;

    skip_iteration = false;

    // set timer properties
    timer.setTimerType(Qt::PreciseTimer);
    connect(&timer,SIGNAL(timeout()),this,SLOT(clockTimeout()));
    timer.start(100);
    iter_timer.setTimerType(Qt::PreciseTimer);
    connect(&iter_timer,SIGNAL(timeout()),this,SLOT(iterationTimeout()));
    iter_timer.start(t_step);

    r_gen = new QRandomGenerator();
    for(int i=0; i<2; i++) framebuffer[i] = nullptr;

    // Set size policy to allow scaling while maintaining asp ratio
    setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

    readColorFile(colorfile);

    //loadPattern("../patterns/spacefillersynthactivation.rle");
    // initPatterns();
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
            initGrid();
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
void Grid::readColorFile(const QString filename)
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
void Grid::initGrid()
{
    makeCurrent();
    framebuffer[out]->bind();

    // no selected pattern, randomize grid
    if (selected_pattern == -1)
    {
        for (int w = 0; w < width; w++)
        {
            for (int h = 0; h < height; h++)
            {
                if (r_gen->bounded(100) <= probability)
                {
                    glBegin(GL_QUADS);
                        glVertex2f(w, h);
                        glVertex2f(w + 1, h);
                        glVertex2f(w + 1, h + 1);
                        glVertex2f(w, h + 1);
                    glEnd();
                }
            }
        }
    }
    // draw RLE pattern
    else
    {
        // retrieve pattern information
        vector<vector<bool>> pattern_grid = patterns.at(selected_pattern).pattern_grid;
        int p_dx = patterns.at(selected_pattern).dx;
        int p_dy = patterns.at(selected_pattern).dy;

        // determine where to start on the grid + how wide the pattern can be
        int start_col = qBound(0, width / 2 - p_dx / 2, width);
        int start_row = qBound(0, height / 2 - p_dy / 2, height);
        int dx = qMin(p_dx, width - start_col);
        int dy = qMin(p_dy, height - start_row);

        //qInfo() << "drawing pattern";
        // draw pattern
        for (int r=0; r<dy; r++) {
            for (int c=0; c<dx; c++) {
                if (pattern_grid.at(r).at(c)) {
                    // Invert the y-coordinate
                    int inverted_row = dy - 1 - r;

                    glBegin(GL_QUADS);
                        glVertex2f(start_col + c,       start_row + inverted_row);
                        glVertex2f(start_col + c + 1,   start_row + inverted_row);
                        glVertex2f(start_col + c + 1,   start_row + inverted_row + 1);
                        glVertex2f(start_col + c,       start_row + inverted_row + 1);
                    glEnd();
                }
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
void Grid::clockTimeout()
{
    t += t_step;
    QString time = formatTime(t);
    emit viewerElapsedTime(time);
}
void Grid::iterationTimeout()
{
    skip_iteration = false;
    update();
}
void Grid::gridPlay()
{
    timer.start();
    iter_timer.start();
}
void Grid::gridPause()
{
    timer.stop();
    iter_timer.stop();
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
        L_click = true;
        mouse_pos = event->pos();
    }
    //  Remember mouse location
}
void Grid::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) L_click = false;
}
void Grid::mouseMoveEvent(QMouseEvent* event)
{
    // Only pan when the left mouse button is clicked
    if (L_click) {
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
QString Grid::loadPattern(QString filename)
{
    qInfo() << "loading pattern" << filename;
    pattern new_pattern;
    QString pattern_string;

    if(!filename.contains(".rle"))
    {
        qInfo("Other file types not supported, must be a .rle file");
        return "";
    }
    else loadRLE(filename, new_pattern);

    pattern_string = loadRLE(filename, new_pattern);

    if(pattern_string == "")
    {
        qInfo() << "Failed to load pattern" << filename;
        return "";
    }
    else
    {
        //new_pattern.pattern_grid.resize(new_pattern.dy);
        bool success = parseRLEString(pattern_string, new_pattern);
        if(success) {patterns.push_back(new_pattern); return new_pattern.name;}
        else return "";
    }
}
QString Grid::loadRLE(QString filename, pattern& p)
{
    //  Open file
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        qInfo() << "Cannot open file "+filename;
        return "";
    }
    QTextStream in(&file);
    QString line = in.readLine();
    //  Skip header
    while (line[0] == '#')
    {
        if (line[1] == 'N') p.name = line.mid(3);
        line = in.readLine();
    }
    if (p.name.isEmpty()) p.name = "unnamed" + QString::number(unnamed_pattern_ctr);
    //  Process header line
    line = line.simplified();
    line.replace(" ","");
    QStringList words = line.split(",");
    if (words.length()<3) qInfo() << "Invalid header line "+line;
    else if (!words[0].startsWith("x=")) qInfo() << "Missing x= "+line;
    else if (!words[1].startsWith("y=")) qInfo() << "Missing y= "+line;
    else if (!words[2].startsWith("rule=")) qInfo() << "Missing rule= "+line;
    else if (words[2].mid(5) != "B3/S23" && words[2].mid(5) != "b3/s23") qInfo() << "Only rule B3/S23 implemented, "+words[2].mid(5)+" not supported.";
    else
    {
        p.dx = words[0].mid(2).toInt();
        p.dy = words[1].mid(2).toInt();
        if(p.dx <= 0 || p.dy <= 0) qFatal(qPrintable(QString("Invalid pattern x or y value(s),(x,y): %1, %2").arg(p.dx).arg(p.dy)));
        //  Read pattern
        QString pattern_string = "";
        while (!in.atEnd())
        {
            pattern_string += in.readLine();
        }
        //  Remove whitespace
        pattern_string = pattern_string.simplified();
        pattern_string.replace(" ", "");
        //  Reset simulation
        return pattern_string;
    }
    return "";
}
bool Grid::parseRLEString(QString rle, pattern &p)
{
    QQueue<bool> queue;
    QString str_count;
    int rle_len = rle.length();
    for(int i=0; i<rle_len; i++)
    {
        char c = rle[i].unicode();
        int count = str_count.toInt();
        int diff = ((p.dx*p.dy)- queue.size()) % p.dx;
        switch(c)
        {
            // dead
            case 'b': 
            {

                if(count == 0) count = 1;
                for(int j=0; j<count; j++) {queue.enqueue(0);}
                str_count.clear();
                break;
            }
            // alive
            case 'o': 
            {
                if(count == 0) count = 1;
                for(int j=0; j<count; j++) {queue.enqueue(1);} 
                str_count.clear();
                break;
            }
            // new line
            case '$': 
            {
                // pad with 0s if necessary to get a full row
                if(diff != 0) for(int j=0; j<diff; j++) {queue.enqueue(0);}
                // account for $$ notation (extra line of 0s)
                if(rle[i+1].unicode() == '$') {count+=2; i++;}
                // need to add extra lines of 0s
                if(count != 0)
                {
                    // adjust for rle notation, eg 2$ is just one extra line of 0s, not 2
                    count--;
                    for (int j=0; j<count; j++) {for(int k=0; k<p.dx; k++) {queue.enqueue(0);}}
                }
                str_count.clear();
                break;
            }
            // end of rle
            case '!':
            {
                // pad with 0s to get a full row
                if(diff != 0) for(int j=0; j<diff; j++) {queue.enqueue(0);}
                break;
            }
            default: str_count.append(c);
        }
    }

    if(queue.size() % p.dx != 0 || queue.size()/p.dx != p.dy)
    {
        qInfo() << "invalid number of elements in queue!" << queue.size();
        return false;
        //qFatal("invalid number of elements in queue! %d",(int)queue.size());
    }
    // else if (queue.size()/p.dx != p.dy)
    // {
    //     // determine if there are too many lines or too few
    //     int diff = p.dy - queue.size()/p.dx;
    //     qInfo() << "invalid number of rows, diff:" << diff;
    //     // too few, fill with 0s
    //     if(diff > 0) for(int i=0; i<diff; i++) queue.enqueue(0);
    //     else for(int i=0; i<diff; i++) queue.dequeue();
    // }
    else
    {
        p.pattern_grid.resize(p.dy);
        for(int i=0; i<p.dy; i++)
        {
            for(int j=0; j<p.dx; j++)
            {
                bool e = queue.dequeue();
                //e == 0 ? std::cout<<"0" : std::cout<<"1";
                p.pattern_grid.at(i).push_back(e);
            }
        }
        return true;
    }
}
void Grid::initPatterns()
{
    // add default random pattern
    emit viewerAddPattern("random");
    QDir pd(patterns_dir);
    QStringList ptns = pd.entryList(QStringList() << "*.rle" << "*.RLE",QDir::Files);
    int len = ptns.length();
    QString new_name;
    for(int i=0; i<len; i++)
    {
        QString new_pattern_file = ptns[i];
        new_name = loadPattern(patterns_dir + new_pattern_file);
        if (new_name != "") {emit viewerAddPattern(new_name);}
    }
    
}
void Grid::gridLoadPattern(int idx)
{
    //qInfo() << "gridLoadPattern called with idx" << idx;
    idx--;
    if(idx < -1 || idx >= static_cast<int>(patterns.size())) qInfo() << "Invalid pattern index selected:" << idx;
    else
    {
        selected_pattern = idx;

        iterations = 0;
        emit viewerIterations("0");
        t = 0;
        emit viewerElapsedTime("00:00");
        skip_iteration = false;
        update();
    }
}
void Grid::gridSetFrequency(double new_t_step)
{
    // convert from seconds to milliseconds
    //int new_t_step = t_step * 1000;
    iter_timer.setInterval(new_t_step);
}
