#include "Viewer.hpp"

Viewer::Viewer(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(tr("Game Of Life"));
    //setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);

    Grid *grid = new Grid(this);
    setCentralWidget(grid);
    
    // Create dock widget
    QDockWidget* menu = new QDockWidget(this);
    menu->setMinimumWidth(150);
    menu->setFeatures(menu->features() & ~QDockWidget::DockWidgetClosable);
    addDockWidget(Qt::RightDockWidgetArea,menu);
        QWidget* menu_widget = new QWidget(menu);
        menu->setWidget(menu_widget);

    // Set dock widget menu layout
    QGridLayout* layout = new QGridLayout(menu_widget);
    layout->setRowStretch(10,100);

    // Time box
    QGridLayout* time_lay = new QGridLayout();
    QGroupBox* time_box = new QGroupBox("Time");
    QButtonGroup* time_btn_group = new QButtonGroup();
    time_btn_group->setExclusive(true);
        QPushButton* play = new QPushButton("Play");
        play->setChecked(true);
        QPushButton* pause = new QPushButton("Pause");

        QLabel* elapsed_label = new QLabel("Elapsed Time:");
        QLabel* elapsed_time = new QLabel("00:00");

        QLabel* iterations_label = new QLabel("Iterations:");
        QLabel* iterations_num = new QLabel("0");

        QLabel* frequency_label = new QLabel("Frequency(ms):");
        QLabel* frequency_num = new QLabel("--");

        time_btn_group->addButton(play);
        time_btn_group->addButton(pause);

        time_lay->addWidget(play,0,0);
        time_lay->addWidget(pause,0,1);
        time_lay->addWidget(elapsed_label,1,0);
        time_lay->addWidget(elapsed_time,1,1);
        time_lay->addWidget(iterations_label,2,0);
        time_lay->addWidget(iterations_num,2,1);
        time_lay->addWidget(frequency_label,3,0);
        time_lay->addWidget(frequency_num,3,1);
    time_box->setLayout(time_lay);
    layout->addWidget(time_box,0,0);

    // Iterations box

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
        connect(grid, SIGNAL(viewerIterations(QString)), iterations_num, SLOT(setText(QString)));
        connect(grid, SIGNAL(viewerFrequency(QString)), frequency_num, SLOT(setText(QString)));
}

Viewer::~Viewer()
{
    //delete *grid;
}
