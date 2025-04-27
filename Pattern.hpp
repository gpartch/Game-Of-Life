#ifndef PATTERN_H
#define PATTERN_H

#include <QWindow>
#include <QString>
#include <QOpenGLWidget>
#include <QQueue>
#include <QtGlobal>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QFile>
#include <QOpenGLFunctions>

#include <vector>

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

class Pattern  : protected QOpenGLFunctions
{
    public:
        Pattern(QString filename, bool& successful_init);
        virtual ~Pattern() {};
        bool loadPatternFile(QString filename);
        QString loadRLE(QString filename);
        bool parseRLEString(QString rle);

        void draw(QOpenGLContext* context, QOpenGLFramebufferObject* fb, int width, int height);
        QString getName();

    private:
        int num_states;
        bool neighborhood[8];

        QString name;
        QString rule;
        int dx,dy;
        vector<vector<bool>> pattern_grid;
};

#endif
