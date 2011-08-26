/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   ProjectTabWidget.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-02-05
//!
//! @brief Contain classes for Project Tabs
//!
//$Id$

#include <QtGui>
#include <QSizePolicy>
#include <QHash>

#include "ProjectTabWidget.h"
#include "MessageWidget.h"

#include "../MainWindow.h"
#include "../GraphicsView.h"
#include "../InitializationThread.h"
#include "../SimulationThread.h"
#include "../ProgressBarThread.h"
#include "../Configuration.h"
#include "../Utilities/XMLUtilities.h"
#include "../GUIObjects/GUISystem.h"
#include "../Widgets/LibraryWidget.h"
#include "../version.h"

//! @class ProjectTab
//! @brief The ProjectTab class is a Widget to contain a simulation model
//!
//! ProjectTab contains a drawing space to create models.
//!


//! Constructor.
//! @param parent defines a parent to the new instanced object.
ProjectTab::ProjectTab(ProjectTabWidget *parent)
    : QWidget(parent)
{
    this->setPalette(gConfig.getPalette());

    this->setMouseTracking(true);

    mpParentProjectTabWidget = parent;
    mpQuickNavigationWidget = new QuickNavigationWidget(this);
    mpSystem = new GUISystem(this, 0);

    connect(this, SIGNAL(checkMessages()), gpMainWindow->mpMessageWidget, SLOT(checkMessages()));
    connect(this, SIGNAL(simulationFinished()), this, SLOT(collectPlotData()));

    emit checkMessages();

    double timeStep = mpSystem->getCoreSystemAccessPtr()->getDesiredTimeStep();

    gpMainWindow->setTimeStepInToolBar(timeStep);

    mIsSaved = true;

    mpGraphicsView  = new GraphicsView(this);
    mpGraphicsView->setScene(mpSystem->getContainedScenePtr());

    QVBoxLayout *tabLayout = new QVBoxLayout(this);
    tabLayout->setSpacing(0);
    tabLayout->addWidget(mpQuickNavigationWidget);
    tabLayout->addWidget(mpGraphicsView);
    //this->setLayout(tabLayout);

    mpGraphicsView->centerView();

    mLastSimulationTime = 0;
}

ProjectTab::~ProjectTab()
{
    //qDebug() << "projectTab destructor";
    //First make sure that we go to the top level system, we dont want to be inside a subsystem while it is beeing deleted
    this->mpQuickNavigationWidget->gotoContainerAndCloseSubcontainers(0);
    //Now delete the root system, all subcontents will be automatically removed by the mpSystem destructor
    delete mpSystem;
}


//! Should be called when a model has changed in some sense,
//! e.g. a component added or a connection has changed.
void ProjectTab::hasChanged()
{
    //qDebug() << "hasChanged()";
    if (mIsSaved)
    {
        QString tabName = mpParentProjectTabWidget->tabText(mpParentProjectTabWidget->currentIndex());

        if(!tabName.endsWith("*"))
        {
            tabName.append("*");
        }
        mpParentProjectTabWidget->setTabText(mpParentProjectTabWidget->currentIndex(), tabName);

        mIsSaved = false;
    }
}


//! @brief Returns a pointer to the system in the tab
GUISystem *ProjectTab::getSystem()
{
    return mpSystem;
}


//! @brief Returns a pointer to the graphics view displayed in the tab
GraphicsView *ProjectTab::getGraphicsView()
{
    return mpGraphicsView;
}


//! @brief Returns a pointer to the quick navigation widget
QuickNavigationWidget *ProjectTab::getQuickNavigationWidget()
{
    return mpQuickNavigationWidget;
}


//! @brief Returns last simulation time for tab
int ProjectTab::getLastSimulationTime()
{
    return mLastSimulationTime;
}


//! @brief Returns whether or not the current project is saved
bool ProjectTab::isSaved()
{
    return mIsSaved;
}


//! @brief Set function to tell the tab whether or not it is saved
void ProjectTab::setSaved(bool value)
{
    if(value)
    {
        QString tabName = mpParentProjectTabWidget->tabText(mpParentProjectTabWidget->currentIndex());
        if(tabName.endsWith("*"))
        {
            tabName.chop(1);
        }
        mpParentProjectTabWidget->setTabText(mpParentProjectTabWidget->currentIndex(), tabName);
    }
    mIsSaved = value;
}


//! Simulates the model in the tab in a separate thread, the GUI runs a progressbar parallel to the simulation.
bool ProjectTab::simulate()
{

    MessageWidget *pMessageWidget = gpMainWindow->mpMessageWidget;

    mpSystem->updateStartTime();
    mpSystem->updateStopTime();
    mpSystem->updateTimeStep();

        //Setup simulation parameters
    double startTime = mpSystem->getStartTime();
    double finishTime = mpSystem->getStopTime();
    double dt = finishTime - startTime;
    size_t nSteps = dt/mpSystem->getCoreSystemAccessPtr()->getDesiredTimeStep();
    size_t nSamples = mpSystem->getNumberOfLogSamples();

    if(!mpSystem->getCoreSystemAccessPtr()->isSimulationOk())
    {
        emit checkMessages();
        return false;
    }

    qDebug() << "Initializing simulation: " << startTime << nSteps << finishTime;

        //Ask core to initialize simulation
    InitializationThread actualInitialization(mpSystem->getCoreSystemAccessPtr(), startTime, finishTime, nSamples, this);
    actualInitialization.start();
    actualInitialization.setPriority(QThread::HighestPriority);

    ProgressBarThread progressThread(this);
    QProgressDialog progressBar(tr("Initializing simulation..."), tr("&Abort initialization"), 0, 0, this);
    if(gConfig.getEnableProgressBar())
    {
        progressBar.setWindowModality(Qt::WindowModal);
        progressBar.setWindowTitle(tr("Simulate!"));
        size_t i=0;
        while (actualInitialization.isRunning())
        {
            progressThread.start();
            progressThread.setPriority(QThread::TimeCriticalPriority);//(QThread::LowestPriority);
            progressThread.wait();
            progressBar.setValue(i++);
            if (progressBar.wasCanceled())
            {
                mpSystem->getCoreSystemAccessPtr()->stop();
            }
        }
        progressBar.setValue(i);
    }

    actualInitialization.wait(); //Make sure actualSimulation do not goes out of scope during simulation
    actualInitialization.quit();


        //Ask core to execute (and finalize) simulation
    QTime simTimer;
    if (!progressBar.wasCanceled())
    {
        if(gConfig.getUseMulticore())
            gpMainWindow->mpMessageWidget->printGUIInfoMessage("Starting Multi Threaded Simulation");
        else
            gpMainWindow->mpMessageWidget->printGUIInfoMessage("Starting Single Threaded Simulation");


        simTimer.start();
        SimulationThread actualSimulation(mpSystem->getCoreSystemAccessPtr(), startTime, finishTime, this);
        actualSimulation.start();
        actualSimulation.setPriority(QThread::HighestPriority);
            //! @todo TimeCriticalPriority seem to work on dual core, is it a problem on single core machines only?
        //actualSimulation.setPriority(QThread::TimeCriticalPriority); //No bar appears in Windows with this prio

        if(gConfig.getEnableProgressBar())
        {
            progressBar.setLabelText(tr("Running simulation..."));
            progressBar.setCancelButtonText(tr("&Abort simulation"));
            progressBar.setMinimum(0);
            progressBar.setMaximum(nSteps);
            while (actualSimulation.isRunning())
            {
               progressThread.start();
               progressThread.setPriority(QThread::LowestPriority);
               progressThread.wait();
               progressBar.setValue((size_t)(mpSystem->getCoreSystemAccessPtr()->getCurrentTime()/dt * nSteps));
               if (progressBar.wasCanceled())
               {
                  mpSystem->getCoreSystemAccessPtr()->stop();
               }
            }
            progressThread.quit();
            progressBar.setValue((size_t)(mpSystem->getCoreSystemAccessPtr()->getCurrentTime()/dt * nSteps));
        }

        actualSimulation.wait(); //Make sure actualSimulation do not goes out of scope during simulation
        actualSimulation.quit();
        //emit checkMessages();
    }

    QString timeString;
    timeString.setNum(simTimer.elapsed());
    mLastSimulationTime = simTimer.elapsed();
    if (progressBar.wasCanceled())
    {
        pMessageWidget->printGUIInfoMessage(QString(tr("Simulation of '").append(mpSystem->getCoreSystemAccessPtr()->getRootSystemName()).append(tr("' was terminated!"))));
    }
    else
    {
        pMessageWidget->printGUIInfoMessage(QString(tr("Simulated '").append(mpSystem->getCoreSystemAccessPtr()->getRootSystemName()).append(tr("' successfully!  Simulation time: ").append(timeString).append(" ms"))));
        emit simulationFinished();
        //this->mpParentProjectTabWidget->mpParentMainWindow->mpPlotWidget->mpVariableList->updateList();
    }
    emit checkMessages();

    return !(progressBar.wasCanceled());
}


//! Slot that saves current project to old file name if it exists.
//! @see saveProjectTab(int index)
void ProjectTab::save()
{
    saveModel(EXISTINGFILE);
}


//! Slot that saves current project to a new model file.
//! @see saveProjectTab(int index)
void ProjectTab::saveAs()
{
    saveModel(NEWFILE);
}


//! @brief Slot that tells the current system to collect plot data from core
void ProjectTab::collectPlotData()
{
    //If we collect plot data, we can plot and calculate losses, so enable these buttons
    gpMainWindow->mpPlotAction->setEnabled(true);
    gpMainWindow->mpShowLossesAction->setEnabled(true);

    //Tell container to do the job
    this->mpGraphicsView->getContainerPtr()->collectPlotData();
}


//! Saves the model and the viewport settings in the tab to a model file.
//! @param saveAsFlag tells whether or not an already existing file name shall be used
//! @see saveProjectTab()
//! @see loadModel()
void ProjectTab::saveModel(saveTarget saveAsFlag)
{
    if((mpSystem->getModelFileInfo().filePath().isEmpty()) || (saveAsFlag == NEWFILE))
    {
        QDir fileDialogSaveDir;
        QString modelFilePath;
        modelFilePath = QFileDialog::getSaveFileName(this, tr("Save Model File"),
                                                             fileDialogSaveDir.currentPath() + QString(MODELPATH),
                                                             tr("Hopsan Model Files (*.hmf)"));

        if(modelFilePath.isEmpty())     //Don't save anything if user presses cancel
        {
            return;
        }
        mpSystem->setModelFile(modelFilePath);
    }

    QFile file(mpSystem->getModelFileInfo().filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        return;
    }

        //Sets the model name (must set this name before saving or else systemports wont know the real name of their rootsystem parent)
    mpSystem->setName(mpSystem->getModelFileInfo().baseName());

        //Update the basepath for relative appearance data info
    mpSystem->setAppearanceDataBasePath(mpSystem->getModelFileInfo().absolutePath());

        //Save xml document
    QDomDocument domDocument;
    QDomElement hmfRoot = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, "0"); //!< @todo need to get coreversion in here somehow, maybe have a global that is set when the hopsan core is instansiated

        //Save the model component hierarcy
    //! @todo maybe use a saveload object instead of calling save imediately (only load object exist for now), or maybe this is fine
    mpSystem->saveToDomElement(hmfRoot);

        //Save to file
    const int IndentSize = 4;
    QFile xmlhmf(mpSystem->getModelFileInfo().filePath());
    if (!xmlhmf.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        return;
    }
    QTextStream out(&xmlhmf);
    appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
    domDocument.save(out, IndentSize);

        //Set the tab name to the model name, efectively removing *, also mark the tab as saved
    QString tabName = mpSystem->getModelFileInfo().baseName();
    mpParentProjectTabWidget->setTabText(mpParentProjectTabWidget->currentIndex(), tabName);
    gConfig.addRecentModel(mpSystem->getModelFileInfo().filePath());
    gpMainWindow->updateRecentList();
    this->setSaved(true);
}



//! @class ProjectTabWidget
//! @brief The ProjectTabWidget class is a container class for ProjectTab class
//!
//! ProjectTabWidget contains ProjectTabWidget widgets.
//!


//! Constructor.
//! @param parent defines a parent to the new instanced object.
ProjectTabWidget::ProjectTabWidget(MainWindow *parent)
        :   QTabWidget(parent)
{
    this->setPalette(gConfig.getPalette());

    connect(this, SIGNAL(checkMessages()), gpMainWindow->mpMessageWidget, SLOT(checkMessages()));
    connect(this, SIGNAL(currentChanged(int)), gpMainWindow, SLOT(updateToolBarsToNewTab()));
    connect(this, SIGNAL(currentChanged(int)), gpMainWindow, SLOT(refreshUndoWidgetList()));

    setTabsClosable(true);
    mNumberOfUntitledTabs = 0;

    connect(this,SIGNAL(currentChanged(int)),SLOT(tabChanged()));
    connect(this,SIGNAL(tabCloseRequested(int)),SLOT(closeProjectTab(int)));
    connect(this,SIGNAL(tabCloseRequested(int)),SLOT(tabChanged()));

    this->hide();
}


//!  Tells current tab to export itself to PDF. This is needed because a direct connection to current tab would be too complicated.


//! Returns a pointer to the currently active project tab - be sure to check that the number of tabs is not zero before calling this
ProjectTab *ProjectTabWidget::getCurrentTab()
{
    return qobject_cast<ProjectTab *>(currentWidget());
}


//! Returns a pointer to the currently active project tab - be sure to check that the number of tabs is not zero before calling this
ProjectTab *ProjectTabWidget::getTab(int index)
{
    return qobject_cast<ProjectTab *>(widget(index));
}


//! @brief Returns a pointer to the current top level system model
//! Be sure to check that the number of tabs is not zero before calling this.
GUISystem *ProjectTabWidget::getCurrentTopLevelSystem()
{
    return getCurrentTab()->getSystem();
}


//! @brief Returns a pointer to the currently open container object in current tab
GUIContainerObject *ProjectTabWidget::getCurrentContainer()
{
    return getCurrentTab()->getGraphicsView()->getContainerPtr();
}


//! @brief Returns a pointer to the currently open container object in specified tab
GUIContainerObject *ProjectTabWidget::getContainer(int index)
{
    return getTab(index)->getGraphicsView()->getContainerPtr();
}


//! @brief Returns a pointer to the top level system model at specified tab
//! Be sure to check that the tab exist before calling this.
GUISystem *ProjectTabWidget::getSystem(int index)
{
    return getTab(index)->getSystem();
}


//! @brief Adds an existing ProjectTab object to itself.
//! @see closeProjectTab(int index)
void ProjectTabWidget::addProjectTab(ProjectTab *projectTab, QString tabName)
{
    projectTab->setParent(this);

    addTab(projectTab, tabName);
    setCurrentWidget(projectTab);

    emit newTabAdded();
}


//! @brief Adds a ProjectTab object (a new tab) to itself.
//! @see closeProjectTab(int index)
void ProjectTabWidget::addNewProjectTab(QString tabName)
{
    tabName.append(QString::number(mNumberOfUntitledTabs));

    ProjectTab *newTab = new ProjectTab(this);
    newTab->getSystem()->setName(tabName);

    this->addTab(newTab, tabName);
    this->setCurrentWidget(newTab);

    newTab->setSaved(true);

    mNumberOfUntitledTabs += 1;
}


//! @brief Closes current project.
//! @param index defines which project to close.
//! @return true if closing went ok. false if the user canceled the operation.
//! @see closeAllProjectTabs()
bool ProjectTabWidget::closeProjectTab(int index)
{
    if (!(getTab(index)->isSaved()))
    {
        QString modelName;
        modelName = tabText(index);
        modelName.chop(1);
        QMessageBox msgBox;
        msgBox.setWindowIcon(gpMainWindow->windowIcon());
        msgBox.setText(QString("The model '").append(modelName).append("'").append(QString(" is not saved.")));
        msgBox.setInformativeText("Do you want to save your changes before closing?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);

        int answer = msgBox.exec();

        switch (answer)
        {
        case QMessageBox::Save:
            // Save was clicked
            getTab(index)->save();
            break;
        case QMessageBox::Discard:
            // Don't Save was clicked
            break;
        case QMessageBox::Cancel:
            // Cancel was clicked
            return false;
        default:
            // should never be reached
            return false;
        }
    }


    if (getTab(index)->getSystem()->hasOpenPlotCurves())
    {
        QMessageBox msgBox;
        msgBox.setWindowIcon(gpMainWindow->windowIcon());
        msgBox.setText(QString("All open plot curves from this model will be lost."));
        msgBox.setInformativeText("Are you sure you want to close?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);

        int answer = msgBox.exec();

        switch (answer)
        {
        case QMessageBox::Ok:
            // Ok was clicked
            break;
        case QMessageBox::Cancel:
            // Cancel was clicked
            return false;
        default:
            // should never be reached
            return false;
        }
    }

    //Disconnect signals
    //std::cout << "ProjectTabWidget: " << "Closing project: " << qPrintable(tabText(index)) << std::endl;
    //statusBar->showMessage(QString("Closing project: ").append(tabText(index)));
    disconnect(gpMainWindow->mpResetZoomAction,       SIGNAL(triggered()),    getTab(index)->getGraphicsView(),   SLOT(resetZoom()));
    disconnect(gpMainWindow->mpZoomInAction,          SIGNAL(triggered()),    getTab(index)->getGraphicsView(),   SLOT(zoomIn()));
    disconnect(gpMainWindow->mpZoomOutAction,         SIGNAL(triggered()),    getTab(index)->getGraphicsView(),   SLOT(zoomOut()));
    disconnect(gpMainWindow->mpExportPDFAction,       SIGNAL(triggered()),    getTab(index)->getGraphicsView(),   SLOT(exportToPDF()));
    disconnect(gpMainWindow->mpCenterViewAction,      SIGNAL(triggered()),    getTab(index)->getGraphicsView(),   SLOT(centerView()));

    disconnect(gpMainWindow->mpSimulateAction,        SIGNAL(triggered()),    getTab(index),                      SLOT(simulate()));
    disconnect(gpMainWindow->mpSaveAction,            SIGNAL(triggered()),    getTab(index),                      SLOT(save()));
    disconnect(gpMainWindow->mpSaveAsAction,          SIGNAL(triggered()),    getTab(index),                      SLOT(saveAs()));

    getContainer(index)->disconnectMainWindowActions();

    getCurrentContainer()->setUndoEnabled(false, true);  //This is necessary to prevent each component from registering it being deleted in the undo stack

    //Delete project tab
    delete widget(index);
    //We dont need to call removeTab here, this seems to be handled automatically
    return true;
}


//! @brief Closes all opened projects.
//! @return true if closing went ok. false if the user canceled the operation.
//! @see closeProjectTab(int index)
//! @see saveProjectTab()
bool ProjectTabWidget::closeAllProjectTabs()
{
    gConfig.clearLastSessionModels();

    while(count() > 0)
    {
        setCurrentIndex(count()-1);
        gConfig.addLastSessionModel(getCurrentTopLevelSystem()->getModelFileInfo().filePath());
        if (!closeProjectTab(count()-1))
        {
            return false;
        }
    }
    return true;
}


//! @brief Loads a model from a file and opens it in a new project tab.
//! @see loadModel(QString modelFileName)
//! @see Model(saveTarget saveAsFlag)
void ProjectTabWidget::loadModel()
{
    QDir fileDialogOpenDir;
    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose Model File"),
                                                         fileDialogOpenDir.currentPath() + QString(MODELPATH),
                                                         tr("Hopsan Model Files (*.hmf)"));
    if(!modelFileName.isEmpty())
    {
        loadModel(modelFileName);
    }
}


//! @brief Help function that loads a model from the text in a QAction object.
//! Used to facilitate recent models function.
void ProjectTabWidget::loadModel(QAction *action)
{
    loadModel(action->text());
}


//! @brief Loads a model from a file and opens it in a new project tab.
//! @param modelFileName is the path to the loaded file
//! @see loadModel()
//! @see saveModel(saveTarget saveAsFlag)
void ProjectTabWidget::loadModel(QString modelFileName)
{
    //! @todo maybe  write utility function that opens filel checks existance and sets fileinfo
    QFile file(modelFileName);   //Create a QFile object
    if(!file.exists())
    {
        qDebug() << "File not found: " + file.fileName();
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("File not found: " + file.fileName());
        return;
    }
    QFileInfo fileInfo(file);

    //Make sure file not already open
    for(int t=0; t!=this->count(); ++t)
    {
        if(this->getContainer(t)->getModelFileInfo().filePath() == fileInfo.filePath())
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::information(this, tr("Error"), tr("Unable to load model. File is already open."));
            return;
        }
    }

    gpMainWindow->registerRecentModel(fileInfo);

    this->addProjectTab(new ProjectTab(this), fileInfo.baseName());
    ProjectTab *pCurrentTab = this->getCurrentTab();
    pCurrentTab->getSystem()->setUndoEnabled(false, true);

    //Check if this is an expected hmf xml file
    //! @todo maybe write helpfunction that does this directly in system (or container)
    QDomDocument domDocument;
    QDomElement hmfRoot = loadXMLDomDocument(file, domDocument, HMF_ROOTTAG);
    if (!hmfRoot.isNull())
    {
        //! @todo check if we could load else give error message and dont attempt to load
        QDomElement systemElement = hmfRoot.firstChildElement(HMF_SYSTEMTAG);
        pCurrentTab->getSystem()->setModelFileInfo(file); //Remember info about the file from which the data was loaded
        pCurrentTab->getSystem()->setAppearanceDataBasePath(pCurrentTab->getSystem()->getModelFileInfo().absolutePath());
        pCurrentTab->getSystem()->loadFromDomElement(systemElement);
    }
    else
    {
        //! @todo give some cool error message
    }
    pCurrentTab->setSaved(true);

    pCurrentTab->getSystem()->setUndoEnabled(true, true);

    emit newTabAdded();
}


void ProjectTabWidget::tabChanged()
{
    if(count() > 0) { this->show(); }
    else { this->hide(); }

    for(int i=0; i<count(); ++i)
    {
            //If you add a disconnect here, remember to also add it to the close tab function!
        //! @todo  Are these connections such connection that are supposed to be permanent conections? otherwise they should be in the disconnectMainWindowActions function
        disconnect(gpMainWindow->mpResetZoomAction,       SIGNAL(triggered()),        getTab(i)->getGraphicsView(),  SLOT(resetZoom()));
        disconnect(gpMainWindow->mpZoomInAction,          SIGNAL(triggered()),        getTab(i)->getGraphicsView(),  SLOT(zoomIn()));
        disconnect(gpMainWindow->mpZoomOutAction,         SIGNAL(triggered()),        getTab(i)->getGraphicsView(),  SLOT(zoomOut()));
        disconnect(gpMainWindow->mpExportPDFAction,       SIGNAL(triggered()),        getTab(i)->getGraphicsView(),  SLOT(exportToPDF()));
        disconnect(gpMainWindow->mpCenterViewAction,      SIGNAL(triggered()),        getTab(i)->getGraphicsView(),  SLOT(centerView()));

        getContainer(i)->disconnectMainWindowActions();

        disconnect(gpMainWindow->mpSimulateAction,        SIGNAL(triggered()),        getTab(i),          SLOT(simulate()));
        disconnect(gpMainWindow->mpSaveAction,            SIGNAL(triggered()),        getTab(i),          SLOT(save()));
        disconnect(gpMainWindow->mpSaveAsAction,          SIGNAL(triggered()),        getTab(i),          SLOT(saveAs()));
    }
    if(this->count() != 0)
    {
        connect(gpMainWindow->mpSimulateAction,       SIGNAL(triggered()),        getCurrentTab(),        SLOT(simulate()));
        connect(gpMainWindow->mpSaveAction,           SIGNAL(triggered()),        getCurrentTab(),        SLOT(save()));
        connect(gpMainWindow->mpSaveAsAction,         SIGNAL(triggered()),        getCurrentTab(),        SLOT(saveAs()));

        connect(gpMainWindow->mpResetZoomAction,      SIGNAL(triggered()),        getCurrentTab()->getGraphicsView(),    SLOT(resetZoom()));
        connect(gpMainWindow->mpZoomInAction,         SIGNAL(triggered()),        getCurrentTab()->getGraphicsView(),    SLOT(zoomIn()));
        connect(gpMainWindow->mpZoomOutAction,        SIGNAL(triggered()),        getCurrentTab()->getGraphicsView(),    SLOT(zoomOut()));
        connect(gpMainWindow->mpExportPDFAction,      SIGNAL(triggered()),        getCurrentTab()->getGraphicsView(),    SLOT(exportToPDF()));
        connect(gpMainWindow->mpCenterViewAction,     SIGNAL(triggered()),        getCurrentTab()->getGraphicsView(),    SLOT(centerView()));

        getCurrentContainer()->connectMainWindowActions();

        getCurrentContainer()->updateMainWindowButtons();
        getCurrentTopLevelSystem()->updateSimulationParametersInToolBar();

        if(gpMainWindow->mpLibrary->mGfxType != getCurrentTab()->getSystem()->getGfxType())
        {
            gpMainWindow->mpLibrary->setGfxType(getCurrentTab()->getSystem()->getGfxType());
        }

        gpMainWindow->mpToggleNamesAction->setChecked(!getCurrentContainer()->areNamesHidden());
        gpMainWindow->mpTogglePortsAction->setChecked(!getCurrentContainer()->arePortsHidden());
        gpMainWindow->mpShowLossesAction->setChecked(getCurrentContainer()->areLossesVisible());
    }
}



void ProjectTabWidget::saveCurrentModelToWrappedCode()
{
    qobject_cast<GUISystem*>(getCurrentContainer())->saveToWrappedCode();
}



void ProjectTabWidget::createSimulinkWrapperFromCurrentModel()
{
    qobject_cast<GUISystem*>(getCurrentContainer())->createSimulinkSourceFiles();
}


void ProjectTabWidget::showLosses(bool show)
{
    qobject_cast<GUISystem*>(getCurrentContainer())->showLosses(show);
}
