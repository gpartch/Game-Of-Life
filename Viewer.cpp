#include "Viewer.hpp"

Viewer::Viewer(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(tr("Game Of Life"));
    //setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);

    Grid *grid = new Grid(this);
    setCentralWidget(grid);
    
    QDockWidget* menu = new QDockWidget(this);
    addDockWidget(Qt::RightDockWidgetArea,menu);
        QWidget* menu_widget = new QWidget(menu);
        menu->setWidget(menu_widget);

    // QGridLayout* layout = new QGridLayout;

    // layout->setColumnStretch(0,100);
    // layout->setColumnMinimumWidth(0,400);
    // layout->setRowStretch(1,100);

    // layout->addWidget(grid,0,0);

    // setLayout(layout);
}

Viewer::~Viewer()
{
    //delete *grid;
}
