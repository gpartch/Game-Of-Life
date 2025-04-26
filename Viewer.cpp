#include "Viewer.hpp"

Viewer::Viewer(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(tr("Game Of Life"));
    //setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);

    // Create dock widget
    QDockWidget* menu = new QDockWidget(this);
    menu->setFixedWidth(200);
    menu->setFeatures(menu->features() & ~QDockWidget::DockWidgetClosable);
    addDockWidget(Qt::RightDockWidgetArea,menu);
        QWidget* menu_widget = new QWidget(menu);
        //menu_widget->setFixedWidth()
        menu->setWidget(menu_widget);
        

    // Set dock widget menu layout
    QGridLayout* layout = new QGridLayout(menu_widget);
    layout->setRowStretch(10,100);

    // initialize grid
    Grid *grid = new Grid(this);
    setCentralWidget(grid);
    

    // Counter box
    QGridLayout* time_lay = new QGridLayout();
    QGroupBox* time_box = new QGroupBox("Counter",menu_widget);
    QButtonGroup* time_btn_group = new QButtonGroup(menu_widget);
    time_btn_group->setExclusive(true);
        QPushButton* play = new QPushButton("Play");
        play->setChecked(true);
        QPushButton* pause = new QPushButton("Pause");

        QLabel* elapsed_label = new QLabel("Elapsed Time:");
        QLabel* elapsed_time = new QLabel("00:00");

        QLabel* iterations_label = new QLabel("Iterations:");
        QLabel* iterations_num = new QLabel("0");

        QLabel* frequency_label = new QLabel("Frequency(ms):");
        //QLabel* frequency_num = new QLabel("--");
        QDoubleSpinBox* frequency_num = new QDoubleSpinBox(time_box);
        frequency_num->setMinimum(10);
        frequency_num->setMaximum(10000);
        frequency_num->setDecimals(0);
        frequency_num->setSingleStep(10);
        frequency_num->setValue(100);

        QPushButton* reset = new QPushButton("Reset");

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
        time_lay->addWidget(reset,4,0);
    time_box->setLayout(time_lay);
    layout->addWidget(time_box,0,0);

    // viewer signals
    connect(play,       SIGNAL(clicked()),                      grid,               SLOT(gridPlay()));
    connect(pause,      SIGNAL(clicked()),                      grid,               SLOT(gridPause()));
    connect(reset,      SIGNAL(clicked()),                      grid,               SLOT(gridRestart()));
    
    // grid signals
    connect(grid,       SIGNAL(viewerElapsedTime(QString)),     elapsed_time,       SLOT(setText(QString)));
    connect(grid,       SIGNAL(viewerIterations(QString)),      iterations_num,     SLOT(setText(QString)));
    //connect(grid,       SIGNAL(viewerFrequency(QString)),       frequency_num,      SLOT(setText(QString)));
    connect(grid,       SIGNAL(viewerAddPattern(QString)),      this,               SLOT(viewerAddPattern(QString)));
    connect(frequency_num, SIGNAL(valueChanged(double)), grid, SLOT(gridSetFrequency(double)));

    // Patterns Box
    patterns = new QComboBox(menu_widget);
    grid->initPatterns();
    QGridLayout* pat_grid = new QGridLayout();
    QGroupBox* pat_box = new QGroupBox("Patterns", menu_widget);
        pat_grid->addWidget(patterns,0,0);
    pat_box->setLayout(pat_grid);
    layout->addWidget(pat_box,1,0);
    connect(patterns,   SIGNAL(currentIndexChanged(int)),       grid,               SLOT(gridLoadPattern(int)));

    // Calculate the required size for the main window
    int gridWidth = grid->sizeHint().width();
    int gridHeight = grid->sizeHint().height();
    int menuWidth = menu->width();
    int totalWidth = gridWidth + menuWidth;
    int totalHeight = gridHeight;

    // Resize the main window to fit the grid and menu dock
    resize(totalWidth, totalHeight);


    
}

Viewer::~Viewer()
{
    //delete *grid;
}

void Viewer::viewerAddPattern(QString pattern)
{
    int idx = patterns->count();
    if(pattern != "") {patterns->insertItem(idx+1,pattern); qInfo() << "adding new pattern" << pattern;}
    //update();
}
