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

    QGridLayout* layout = new QGridLayout(menu_widget);
    layout->setRowStretch(10,100);

    QGridLayout* time_lay = new QGridLayout();
    QGroupBox* time_box = new QGroupBox("Time");
    QButtonGroup* time_btn_group = new QButtonGroup();
    time_btn_group->setExclusive(true);
    QPushButton* play = new QPushButton("Play");
    play->setChecked(true);
    QPushButton* pause = new QPushButton("Pause");
    QLabel* elapsed_label = new QLabel("Elapsed Time (s):");
    QLabel* elapsed_time = new QLabel("00:00");
    time_btn_group->addButton(play);
    time_btn_group->addButton(pause);
    time_lay->addWidget(play,0,0);
    time_lay->addWidget(pause,0,1);
    time_lay->addWidget(elapsed_label,1,0);
    time_lay->addWidget(elapsed_time,1,1);
    time_box->setLayout(time_lay);
    layout->addWidget(time_box,0,0);


    //setLayout(layout);

    // Calculate the required size for the main window
    int gridWidth = grid->sizeHint().width();
    int gridHeight = grid->sizeHint().height();
    int menuWidth = menu->minimumWidth();
    int totalWidth = gridWidth + menuWidth;
    int totalHeight = gridHeight;

    // Resize the main window to fit the grid and menu dock
    resize(totalWidth, totalHeight);


    // viewer signals
        connect(play, SIGNAL(clicked()), grid, SLOT(gridPlay()));
        connect(pause, SIGNAL(clicked()), grid, SLOT(gridPause()));
    // grid signals
        connect(grid, SIGNAL(viewerElapsedTime(QString)), elapsed_time, SLOT(setText(QString)));
}

Viewer::~Viewer()
{
    //delete *grid;
}
