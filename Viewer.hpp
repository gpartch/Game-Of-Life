#ifndef VIEWER_HPP
#define VIEWER_HPP

#include <QMainWindow>
#include <QGridLayout>
#include <QWidget>
#include <QDockWidget>
#include <QSizePolicy>
#include <QGroupBox>
#include <QLabel>
#include <QButtonGroup>
#include <QPushButton>
#include <QComboBox>
#include <QDoubleSpinBox>

#include "Grid.hpp"

class Viewer: public QMainWindow
{
    Q_OBJECT
    QSize sizeHint() const {return QSize(800,800);}

    public:
        Viewer(QWidget *parent = nullptr);
        ~Viewer();
    private:
        QComboBox* patterns = nullptr;
        
    private slots:
        void viewerAddPattern(QString);
};

#endif
