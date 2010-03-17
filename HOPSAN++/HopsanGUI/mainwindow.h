//$Id$

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "plotwidget.h"
#include "MessageWidget.h"


class QGridLayout;
class QHBoxLayout;
class QMenuBar;
class QMenu;
class QStatusBar;
class QAction;
class QString;
class QPlainTextEdit;
class SimulationSetupWidget;
class ProjectTabWidget;
class GraphicsView;
class GraphicsScene;
class LibraryWidget;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QWidget *mpCentralwidget;
    QGridLayout *mpCentralgrid;
    ProjectTabWidget *mpProjectTabs;
    QGridLayout *mpTabgrid;
    LibraryWidget *mpLibrary;
    SimulationSetupWidget *mpSimulationSetupWidget;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuNew;
    QMenu *menuLibs;
    QMenu *menuSimulation;
    QMenu *menuView;
    QMenu *menuPlot;
    MessageWidget *mpMessageWidget;
    QStatusBar *statusBar;
    QAction *actionOpen;
    QAction *actionSave;
    QAction *actionSaveAs;
    QAction *actionClose;
    QAction *actionProject;
    QAction *actionLoadLibs;
    QAction *actionSimulate;
    QAction *actionPlot;


    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *saveAsAction;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *simulateAction;
    QAction *plotAction;

    QToolBar *fileToolBar;
    QToolBar *clipboardToolBar;
    QToolBar *simToolBar;


    //QString libDir;

    //GraphicsScene *scene;
    //GraphicsView *view;

    //VariableListDialog *variableList;

    void closeEvent(QCloseEvent *event);

private slots:
    //void addLibs(QString libDir, QString parentLib=QString());
    //void addLibs();
    void plot();

private:
    void createActions();
    void createToolbars();


};

#endif // MAINWINDOW_H
