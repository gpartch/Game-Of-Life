#include "Pattern.hpp"

Pattern::Pattern(QString filename, bool& successful_init)
{
    dx = 0;
    dy = 0;
    successful_init = loadPatternFile(filename);
}
bool Pattern::loadPatternFile(QString filename)
{
    qInfo() << "loading pattern" << filename;
    QString pattern_string;

    if(!filename.contains(".rle"))
    {
        qInfo("Other file types not supported, must be a .rle file");
    }
    else
    {
        pattern_string = loadRLE(filename);

        if(pattern_string == "")
        {
            qInfo() << "Failed to load pattern" << filename;
        }
        else
        {
            return parseRLEString(pattern_string);
        }
    }
    return false;
}
QString Pattern::loadRLE(QString filename)
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
        if (line[1] == 'N') name = line.mid(3);
        line = in.readLine();
    }
    if (name.isEmpty()) name = "unnamed";
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
        dx = words[0].mid(2).toInt();
        dy = words[1].mid(2).toInt();
        if(dx <= 0 || dy <= 0) qFatal(qPrintable(QString("Invalid pattern x or y value(s),(x,y): %1, %2").arg(dx).arg(dy)));
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
bool Pattern::parseRLEString(QString rle)
{
    QQueue<bool> queue;
    QString str_count;
    int rle_len = rle.length();
    for(int i=0; i<rle_len; i++)
    {
        char c = rle[i].unicode();
        int count = str_count.toInt();
        int diff = ((dx*dy)- queue.size()) % dx;
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
                    for (int j=0; j<count; j++) {for(int k=0; k<dx; k++) {queue.enqueue(0);}}
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

    if(queue.size() % dx != 0 || queue.size()/dx != dy)
    {
        qInfo() << "invalid number of elements in queue!" << queue.size();
    }
    else
    {
        pattern_grid.resize(dy);
        for(int i=0; i<dy; i++)
        {
            for(int j=0; j<dx; j++)
            {
                bool e = queue.dequeue();
                pattern_grid.at(i).push_back(e);
            }
        }
        return true;
    }
    return false;
}
void Pattern::draw(QOpenGLContext* context,  QOpenGLFramebufferObject* fb, int width, int height)
{
    context->makeCurrent(context->surface());
    fb->bind();
    // determine where to start on the grid + how wide the pattern can be
    int start_col = qBound(0, width / 2 - dx / 2, width);
    int start_row = qBound(0, height / 2 - dy / 2, height);
    int n_dx = qMin(dx, width - start_col);
    int n_dy = qMin(dy, height - start_row);

    // draw pattern
    for (int r=0; r<n_dy; r++) {
        for (int c=0; c<n_dx; c++) {
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
    fb->release();
}
QString Pattern::getName()
{
    return name;
}
