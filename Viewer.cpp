#include "Viewer.hpp"

Viewer::Viewer(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(tr("Game Of Life"));
    //setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);

    Grid *grid = new Grid(this);
    setCentralWidget(grid);
    
    QDockWidget* menu = new QDockWidget(this);
    menu->setMinimumWidth(150);
    menu->setFeatures(menu->features() & ~QDockWidget::DockWidgetClosable);
    addDockWidget(Qt::RightDockWidgetArea,menu);
        QWidget* menu_widget = new QWidget(menu);
        menu->setWidget(menu_widget);

    // QGridLayout* layout = new QGridLayout;

    // layout->setColumnStretch(0,100);
    // layout->setColumnMinimumWidth(0,400);
    // layout->setRowStretch(1,100);

    // layout->addWidget(grid,0,0);

    // setLayout(layout);

    // Calculate the required size for the main window
    int gridWidth = grid->sizeHint().width();
    int gridHeight = grid->sizeHint().height();
    int menuWidth = menu->minimumWidth();
    int totalWidth = gridWidth + menuWidth;
    int totalHeight = gridHeight;

    // Resize the main window to fit the grid and menu dock
    resize(totalWidth, totalHeight);
}

Viewer::~Viewer()
{
    //delete *grid;
}
