//!
//! @file   ProjectTabWidget.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-02-05
//!
//! @brief Contain classes for Project Tabs
//!
//$Id$


#include "HopsanCore.h"
#include "ProjectTabWidget.h"
#include "GUIObject.h"
#include "GUIPort.h"
#include "GUIConnector.h"
#include "LibraryWidget.h"
#include "mainwindow.h"
#include "SimulationSetupWidget.h"
#include "MessageWidget.h"
#include "SimulationThread.h"
#include "InitializationThread.h"
#include <QtGui>
#include <QSizePolicy>
#include "version.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cassert>


//! @class GraphicsView
//! @brief The GraphicsView class is a class which display the content of a scene of components.
//!


//! Constructor.
//! @param parent defines a parent to the new instanced object.
GraphicsView::GraphicsView(HopsanEssentials *hopsan, ComponentSystem *model, ProjectTab *parent)
        : QGraphicsView(parent)
{
    mpParentProjectTab = parent;

    this->mpHopsan = hopsan;
    this->mpModel = model;
    this->setDragMode(RubberBandDrag);
    this->setInteractive(true);
    this->setEnabled(true);
    this->setAcceptDrops(true);
    this->mIsCreatingConnector = false;
    this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    this->setSceneRect(0,0,5000,5000);
    this->centerOn(this->sceneRect().center());
    this->mBackgroundColor = QColor(Qt::white);
    this->setBackgroundBrush(mBackgroundColor);

    this->createActions();
    this->createMenus();

    MainWindow *pMainWindow = mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow;
//    MainWindow *pMainWindow = (qobject_cast<MainWindow *>(parent->parent()->parent()->parent())); //Ugly!!!
    connect(this, SIGNAL(checkMessages()), pMainWindow->mpMessageWidget, SLOT(checkMessages()));
    connect(this->systemPortAction, SIGNAL(triggered()), SLOT(systemPortSlot()));
}

void GraphicsView::createMenus()
{
    menuInsert = new QMenu(this);
    menuInsert->setObjectName("menuInsert");
    menuInsert->setTitle("Insert");
    menuInsert->addAction(systemPortAction);
}

void GraphicsView::createActions()
{
    systemPortAction = new QAction(this);
    systemPortAction->setText("System Port");
}


void GraphicsView::contextMenuEvent ( QContextMenuEvent * event )
{
    if(!this->mIsCreatingConnector)
    {
        if (QGraphicsItem *item = itemAt(event->pos()))
        {
            qDebug() << "You righet clicked on a comonent";
            QGraphicsView::contextMenuEvent(event);
        }
        else
        {
            qDebug() << "You didn't right click on an component.";
            QMenu menu(this);
            menu.addMenu(menuInsert);
            menu.exec(event->globalPos());
        }
    }
}


//! Destructor.
GraphicsView::~GraphicsView()
{
    //delete guiComponent; //Skumt att ta bort en guiComponent?
}


//! Defines what happens when moving an object in a GraphicsView.
//! @param event contains information of the drag operation.
void GraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-text"))
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}


//! Defines what happens when drop an object in a GraphicsView.
//! @param event contains information of the drop operation.
void GraphicsView::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-text"))
    {
        QByteArray *data = new QByteArray;
        *data = event->mimeData()->data("application/x-text");

        QDataStream stream(data,QIODevice::ReadOnly);

        QStringList appearanceData;
        stream >> appearanceData;

        QString componentTypeName = appearanceData.at(0);
        QString iconDir = appearanceData.at(1);

        event->accept();

  //      QCursor cursor;
//        QPoint position = this->mapFromScene(cursor.pos());

        QPoint position = event->pos();

        qDebug() << "GraphicsView: " << "x=" << position.x() << "  " << "y=" << position.y();

//        GUIComponent *guiComponent = new GUIComponent(mpHopsan,iconDir,componentTypeName,mapToScene(position).toPoint(),this);
      //  GUIComponent *guiComponent = new GUIComponent(mpHopsan,appearanceData,mapToScene(position).toPoint(),this);

        //this->addComponent(appearanceData, this->mapToScene(position).toPoint());

        //this->addComponent(appearanceData.at(0), this->mapToScene(position).toPoint());
        this->addGUIObject(appearanceData.at(0), appearanceData, this->mapToScene(position).toPoint());


        delete data;
    }
}


GUIConnector *GraphicsView::getTempConnector()
{
    return this->mpTempConnector;
}

//! @brief Temporary addSubSystem functin should be same later on
//! Adds a new component to the GraphicsView.
//! @param componentType is a string defining the type of component.
//! @param position is the position where the component will be created.
//! @param name will be the name of the component.
void GraphicsView::addGUIObject(QString componentType, QStringList appearanceData, QPoint position, QString name, bool startSelected)
{
    qDebug() << "Request to add gui object at (" << position.x() << " " << position.y() << ")";

    //MainWindow *pMainWindow = qobject_cast<MainWindow *>(this->parent()->parent()->parent()->parent()->parent());

    GUIObject *pGuiObject;
    if (componentType == "Subsystem")
    {
        pGuiObject = new GUISubsystem(mpHopsan, appearanceData, position, this->mpParentProjectTab->mpGraphicsScene);
    }
    if (componentType == "SystemPort")
    {
        pGuiObject = new GUISystemPort(mpHopsan, appearanceData, position, this->mpParentProjectTab->mpGraphicsScene);
    }
    else //Assume some component type
    {
        pGuiObject = new GUIComponent(mpHopsan, appearanceData, position, this->mpParentProjectTab->mpGraphicsScene);
    }

    qDebug() << "=====================Get initial name: " << pGuiObject->getName() << "requested: " << name;
    if (!name.isEmpty())
    {
        qDebug() << "name not empty, setting to: " << name;
        //Set name, do NOT try to do smart rename. (If component already exist with new component default name that other component would be renamed)
        pGuiObject->setName(name, true);
    }

    //Core interaction
    qDebug() << "=====================Get name before add: " << pGuiObject->getName();
    if (componentType == "SystemPort")
    {
        mpParentProjectTab->mpComponentSystem->addSystemPort(pGuiObject->getName().toStdString());
    }
    else
    {

        if (componentType == "Subsystem")
        {
            GUISubsystem *pSys = qobject_cast<GUISubsystem *>(pGuiObject);
            this->mpParentProjectTab->mpComponentSystem->addComponent(pSys->getHopsanCoreSystemComponentPtr());
        }
        else
        {
            GUIComponent *pComp = qobject_cast<GUIComponent *>(pGuiObject);
            this->mpParentProjectTab->mpComponentSystem->addComponent(pComp->getHopsanCoreComponentPtr());
        }

        pGuiObject->refreshName();
    }
    emit checkMessages();
    qDebug() << "=====================Get name after add: " << pGuiObject->getName();
    //

    pGuiObject->setSelected(startSelected);

    //guiComponent->setPos(this->mapToScene(position));
    //qDebug() << "GraphicsView: " << pGuiObject->parent();

    this->mGUIObjectMap.insert(pGuiObject->getName(), pGuiObject);

    qDebug() << "GUI Object created at (" << pGuiObject->x() << " " << pGuiObject->y() << ")";

}

//! Adds a new component to the GraphicsView.
//! @param componentType is a string defining the type of component.
//! @param position is the position where the component will be created.
//! @param name will be the name of the component.
void GraphicsView::addComponent(QString componentType, QPoint position, QString name, bool startSelected)
{
    MainWindow *pMainWindow = qobject_cast<MainWindow *>(this->parent()->parent()->parent()->parent()->parent());
    LibraryWidget *pLibrary = pMainWindow->mpLibrary;
    QStringList appearanceData = pLibrary->getAppearanceData(componentType);

    GUIComponent *pGuiComponent = new GUIComponent(mpHopsan, appearanceData, position, this->mpParentProjectTab->mpGraphicsScene);

    qDebug() << "=====================Get initial name: " << pGuiComponent->getName() << "requested: " << name;
    if (!name.isEmpty())
    {
        qDebug() << "name not empty, setting to: " << name;
        //Set name, do NOT try to do smart rename. (If component already exist with new component default name that other component would be renamed)
        pGuiComponent->setName(name, true);
    }

    pGuiComponent->setSelected(startSelected);

    //Core interaction
    qDebug() << "=====================Get name before add: " << pGuiComponent->getName();
    this->mpParentProjectTab->mpComponentSystem->addComponent(pGuiComponent->mpCoreComponent);
    //    qobject_cast<ProjectTab *>(this->parent())->mpModel->addComponent(guiComponent->mpCoreComponent);
    pGuiComponent->refreshName();
    emit checkMessages();
    qDebug() << "=====================Get name after add: " << pGuiComponent->getName();
    //

    //guiComponent->setPos(this->mapToScene(position));
    qDebug() << "GraphicsView: " << pGuiComponent->parent();

     //mLibraryMapPtrs.insert(libraryName, newLibContent);
    //this->mComponentMap.insert()
    this->mGUIObjectMap.insert(pGuiComponent->getName(), pGuiComponent);
    //APAthis->scene()->addItem(guiComponent);

}


void GraphicsView::addSystemPort(QPoint position, QString name, bool startSelected)
{
    qDebug() <<"Adding a system port";

}

void GraphicsView::systemPortSlot()
{
      QCursor cursor;
      QPoint position = this->mapFromScene(cursor.pos());
      addSystemPort(position);
}

//! Delete componenet with specified name
//! @param componentName is the name of the componenet to delete
void GraphicsView::deleteGUIObject(QString componentName)
{
    //qDebug() << "In delete component";
    QMap<QString, GUIObject *>::iterator it;
    it = mGUIObjectMap.find(componentName);

    QMap<QString, GUIConnector *>::iterator it2;
    for(it2 = this->mConnectionMap.begin(); it2!=this->mConnectionMap.end(); ++it2)
    {
        if(it2.key().contains(componentName))
            mConnectionMap.erase(it2);
        if(mConnectionMap.empty())
            break;
    }

    if (it != mGUIObjectMap.end())
    {
        GUIObject* obj_ptr = it.value();
        mGUIObjectMap.erase(it);
        obj_ptr->deleteInHopsanCore();
        scene()->removeItem(obj_ptr);
        delete(obj_ptr);
        emit checkMessages();
    }
    else
    {
        qDebug() << "In delete component: could not find component with name " << componentName;
    }
    this->setBackgroundBrush(mBackgroundColor);
}

//! This function is used to rename a GUI Component (including key rename in component map)
void GraphicsView::renameGUIObject(QString oldName, QString newName)
{
    //First find record with old name
    QMap<QString, GUIObject *>::iterator it = mGUIObjectMap.find(oldName);
    if (it != mGUIObjectMap.end())
    {
        //Make a backup copy
        GUIObject* obj_ptr = it.value();
        //Erase old record
        mGUIObjectMap.erase(it);
        //Rename (core rename will be handled by core), here we force a core only rename (true) so that we dont get stuck in a loop (as rename might be called again)
        obj_ptr->setName(newName, true);
        //Re insert
        mGUIObjectMap.insert(obj_ptr->getName(), obj_ptr);
        qDebug() << "GUI rename: " << oldName << " " << obj_ptr->getName();
    }
    else
    {
        qDebug() << "Old name: " << oldName << " not found";
    }

    emit checkMessages();
}


//! Tells whether or not a component with specified name exist in the GraphicsView
bool GraphicsView::haveGUIObject(QString name)
{
    if (mGUIObjectMap.count(name) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//! Defines what happens when scrolling the mouse in a GraphicsView.
//! @param event contains information of the scrolling operation.
void GraphicsView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() and Qt::ControlModifier)
    {
        qreal factor = pow(1.41,(-event->delta()/240.0));
        this->scale(factor,factor);
    }
}

void GraphicsView::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() and Qt::ControlModifier)
    {
        if (this->mIsCreatingConnector)
        {
            QCursor cursor;
            mpTempConnector->getLastLine()->setGeometry(GUIConnectorLine::DIAGONAL);
        }
        else
        {
            this->setDragMode(QGraphicsView::ScrollHandDrag);
        }
    }
    if (event->key() == Qt::Key_Delete)
        emit keyPressDelete();
    if (event->modifiers() and Qt::ControlModifier and event->key() == Qt::Key_R)
        emit keyPressR();
    if (event->key() == Qt::Key_Escape)
    {
        if(this->mIsCreatingConnector)
        {
            delete(mpTempConnector);
            mIsCreatingConnector = false;
        }
    }
    if(event->modifiers() and Qt::ControlModifier and event->key() == Qt::Key_Up)
    {
        emit keyPressUp();
    }
    if(event->modifiers() and Qt::ControlModifier and event->key() == Qt::Key_Down)
        emit keyPressDown();
    if(event->modifiers() and Qt::ControlModifier and event->key() == Qt::Key_Left)
        emit keyPressLeft();
    if(event->modifiers() and Qt::ControlModifier and event->key() == Qt::Key_Right)
        emit keyPressRight();
    if (event->modifiers() and Qt::ControlModifier and event->key() == Qt::Key_X)
        this->cutSelected();
    if (event->modifiers() and Qt::ControlModifier and event->key() == Qt::Key_C)
        this->copySelected();
    if (event->modifiers() and Qt::ControlModifier and event->key() == Qt::Key_V)
        this->paste();
    if (event->modifiers() and Qt::ControlModifier and event->key() == Qt::Key_A)
        this->selectAll();

    QGraphicsView::keyPressEvent ( event );
}


void GraphicsView::keyReleaseEvent(QKeyEvent *event)
{
    this->setDragMode(QGraphicsView::RubberBandDrag);
    if (this->mIsCreatingConnector)
    {
        if (mpTempConnector->getLastLine()->getGeometry()==GUIConnectorLine::HORIZONTAL)
        {
           mpTempConnector->getLastLine()->setGeometry(GUIConnectorLine::VERTICAL);
        }
        else
        {
           mpTempConnector->getLastLine()->setGeometry(GUIConnectorLine::HORIZONTAL);
        }
    }

    QGraphicsView::keyReleaseEvent ( event );
}


//! Defines what happens when the mouse is moving in a GraphicsView.
//! @param event contains information of the mouse moving operation.
void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    QCursor cursor;
    //qDebug() << "X=" << this->mapFromGlobal(cursor.pos()).x() << "  " << "Y=" << this->mapFromGlobal(cursor.pos()).y();
    this->setBackgroundBrush(mBackgroundColor);

    if (this->mIsCreatingConnector)
        mpTempConnector->updateConnector(mpTempConnector->startPos, this->mapToScene(event->pos()));
}


//! Defines what happens when clicking in a GraphicsView.
//! @param event contains information of the mouse click operation.
void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        qDebug() << "You right clicked!";
        if (this->mIsCreatingConnector)
        {
            if (mpTempConnector->getNumberOfLines() < 3)
                this->mIsCreatingConnector = false;
            mpTempConnector->removeLine(this->mapToScene(event->pos()));
            if(mIsCreatingConnector)
            {
                this->setBackgroundBrush(mBackgroundColor);
                mpTempConnector->updateConnector(mpTempConnector->startPos, this->mapToScene(event->pos()));
            }
        }
    }
    else if  ((event->button() == Qt::LeftButton) && (this->mIsCreatingConnector))
    {
        //if (mpTempConnector->getLastLine()->getGeometry()==GUIConnectorLine::DIAGONAL)
        //{
        //    mpTempConnector->addFreeLine();
        //    mpTempConnector->getLastLine()->setGeometry(GUIConnectorLine::HORIZONTAL);
        //}
        mpTempConnector->addFreeLine();
    }
    emit viewClicked();
    QGraphicsView::mousePressEvent(event);
}


//! Returns a pointer to the component with specified name.
GUIObject *GraphicsView::getGUIObject(QString name)
{
    //qDebug() << mComponentMap.size();
    return mGUIObjectMap.find(name).value();
    //! @todo Cast exception or something if component is not found
}


//! Begin creation of connector or complete creation of connector depending on the mIsCreatingConnector boolean.
void GraphicsView::addConnector(GUIPort *pPort)
{
    if (!mIsCreatingConnector)
    {
        std::cout << "GraphicsView: " << "Adding connector";
        QPointF oldPos = pPort->mapToScene(pPort->boundingRect().center());

        QPen passivePen,activePen,hoverPen;
        if((pPort->mpCorePort->getNodeType() == "NodeHydraulic") | (pPort->mpCorePort->getNodeType() == "NodeMechanic"))
        {
            passivePen = QPen(QColor("black"),1, Qt::SolidLine, Qt::RoundCap);
            activePen = QPen(QColor("red"), 2, Qt::SolidLine, Qt::RoundCap);
            hoverPen = QPen(QColor("darkRed"),2, Qt::SolidLine, Qt::RoundCap);
        }
        else if(pPort->mpCorePort->getNodeType() == "NodeSignal")
        {
            passivePen = QPen(QColor("blue"),1, Qt::DashLine, Qt::RoundCap);
            activePen = QPen(QColor("red"), 2, Qt::DashLine, Qt::RoundCap);
            hoverPen = QPen(QColor("darkRed"),2, Qt::DashLine, Qt::RoundCap);
        }

        mpTempConnector = new GUIConnector(oldPos.x(), oldPos.y(), oldPos.x(), oldPos.y(), passivePen, activePen, hoverPen, this);
        this->scene()->addItem(mpTempConnector);
        this->mIsCreatingConnector = true;
        pPort->getComponent()->addConnector(mpTempConnector);
        mpTempConnector->addFreeLine();
        mpTempConnector->setStartPort(pPort);
    }
    else
    {
        //! @todo This will lead to crash if you click to fast to moany times on the same port
        mpTempConnector->removeLine(pPort->mapToScene(pPort->boundingRect().center()));
        //Core interaction
        Port *start_port = mpTempConnector->getStartPort()->mpCorePort;
        Port *end_port = pPort->mpCorePort;
        bool sucess = mpModel->connect(start_port, end_port);
        if (sucess)
        {
            mIsCreatingConnector = false;
            QPointF newPos = pPort->mapToScene(pPort->boundingRect().center());
            mpTempConnector->updateConnector(mpTempConnector->startPos, newPos);
            pPort->getComponent()->addConnector(mpTempConnector);
            mpTempConnector->setEndPort(pPort);

            mpTempConnector->getStartPort()->hide();
            mpTempConnector->getEndPort()->hide();

            std::stringstream tempStream;
            tempStream << mpTempConnector->getStartPort()->getComponent()->getName().toStdString() << " " << mpTempConnector->getStartPort()->getPortNumber() << " " <<
                          mpTempConnector->getEndPort()->getComponent()->getName().toStdString() << " " << mpTempConnector->getEndPort()->getPortNumber();
            this->mConnectionMap.insert(QString(tempStream.str().c_str()), mpTempConnector);

            //qDebug() << mConnectionVector.last();
        }
        emit checkMessages();
        //
    }
}


void GraphicsView::removeConnector(GUIConnector* pConnector)
{
    //! @todo some error handling both ports must exist and be connected to each other
    //Core interaction
    mpModel->disconnect(pConnector->getStartPort()->mpCorePort, pConnector->getEndPort()->mpCorePort);
    emit checkMessages();
    //
    scene()->removeItem(pConnector);
    delete pConnector;
}


void GraphicsView::selectAll()
{
        //Select all components
    QMap<QString, GUIObject *>::iterator it;
    for(it = this->mGUIObjectMap.begin(); it!=this->mGUIObjectMap.end(); ++it)
    {
        it.value()->setSelected(true);
    }
        //Deselect all connectors
    QMap<QString, GUIConnector*>::iterator it2;
    for(it2 = this->mConnectionMap.begin(); it2!=this->mConnectionMap.end(); ++it2)
    {
        it2.value()->doSelect(true);
    }
}


ComponentSystem *GraphicsView::getModelPointer()
{
    return this->mpModel;
}


void GraphicsView::cutSelected()
{
    this->copySelected();

    emit keyPressDelete();
//    QMap<QString, GUIComponent *>::iterator it;
//    for(it = this->mComponentMap.begin(); it!=this->mComponentMap.end(); ++it)
//    {
//        if(it.value()->isSelected())
//            this->deleteComponent(it.value()->getName());
//        if(mComponentMap.empty())
//            break;
//    }
    this->setBackgroundBrush(mBackgroundColor);
}


void GraphicsView::copySelected()
{
    this->mCopyData.clear();

    QMap<QString, GUIObject *>::iterator it;
    for(it = this->mGUIObjectMap.begin(); it!=this->mGUIObjectMap.end(); ++it)
    {
        if (it.value()->isSelected())
        {
            mCopyData << "COMPONENT";
            mCopyData << it.value()->getTypeName();
            mCopyData << it.value()->getName();
            mCopyDataPos << it.value()->pos();
        }
    }

    QMap<QString, GUIConnector *>::iterator it2;
    for(it2 = this->mConnectionMap.begin(); it2!=this->mConnectionMap.end(); ++it2)
    {
        stringstream connectionStream(it2.key().toStdString());
        std::string name1, port1, name2, port2;
        connectionStream >> name1;
        connectionStream >> port1;
        connectionStream >> name2;
        connectionStream >> port2;
        if(mGUIObjectMap.find(QString(name1.c_str())).value()->isSelected() and mGUIObjectMap.find(QString(name2.c_str())).value()->isSelected())
        {
            qDebug() << "Copying connection between" << QString(name1.c_str()) << " and " <<QString(name2.c_str()) << ".";

//            mCopyData << "CONNECT " << it2.key().toStdString().c_str();
//            for(int i = 0; i!=it2.value()->mLines.size(); ++i)
//            {
//                int geometry = it2.value()->mLines[i]->getGeometry();
//                switch (geometry)
//                {
//                    case 0:
//                        mCopyData << " VERTICAL " << (it2.value()->mLines[i]->endPos.y()-it2.value()->mLines[i]->startPos.y());
//                        break;
//                    case 1:
//                        mCopyData << " HORIZONTAL " << (it2.value()->mLines[i]->endPos.x()-it2.value()->mLines[i]->startPos.x());
//                        break;
//                    case 2:
//                        mCopyData << " DIAGONAL" << (it2.value()->mLines[i]->endPos.x()-it2.value()->mLines[i]->startPos.x()) << (it2.value()->mLines[i]->endPos.y()-it2.value()->mLines[i]->startPos.y());
//                        break;
//                }
//            }
//            mCopyData << "\n";

        }
    }
}

void GraphicsView::paste()
{
    QMap<QString, GUIObject*>::iterator it;
    QMap<QString, GUIConnector*>::iterator it2;

        //Deselect all components
    for(it = this->mGUIObjectMap.begin(); it!=this->mGUIObjectMap.end(); ++it)
    {
        it.value()->setSelected(false);
    }

        //Deselect all connectors
    for(it2 = this->mConnectionMap.begin(); it2!=this->mConnectionMap.end(); ++it2)
    {
        it2.value()->doSelect(false);
    }

    QString tempString;
    QString componentName;
    QString componentType;
    int j = 0;
    for(int i = 0; i!=mCopyData.size(); ++i)
    {
        tempString = mCopyData[i];
        if(tempString == "COMPONENT")
        {
            ++i;
            componentType = mCopyData[i];
            ++i;
            componentName = mCopyData[i];
            this->addComponent(componentType, mCopyDataPos[j].toPoint(), componentName, true);
            ++j;
        }
    }
    this->setBackgroundBrush(mBackgroundColor);
}


//! @class GraphicsScene
//! @brief The GraphicsScene class is a container for graphicsl components in a simulationmodel.
//!

//! Constructor.
//! @param parent defines a parent to the new instanced object.
GraphicsScene::GraphicsScene(ProjectTab *parent)
        :   QGraphicsScene(parent)
{
    mpParentProjectTab = parent;
    setSceneRect(0.0, 0.0, 800.0, 600.0);
    connect(this, SIGNAL(changed( const QList<QRectF> & )),this->parent(), SLOT(hasChanged()));
}


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
    mpParentProjectTabWidget = parent;

    MainWindow *pMainWindow = (qobject_cast<MainWindow *>(parent->parent()->parent())); //Ugly!!!
    connect(this, SIGNAL(checkMessages()), pMainWindow->mpMessageWidget, SLOT(checkMessages()));

    //Core interaction
    mpComponentSystem = mpParentProjectTabWidget->mpHopsan->CreateComponentSystem();
    mpComponentSystem->setName("APA");
    mpComponentSystem->setDesiredTimestep(.001);
    mpComponentSystem->setTypeCQS("S");
    emit checkMessages();
    //

    double timeStep = mpComponentSystem->getDesiredTimeStep();
    mpParentProjectTabWidget->mpParentMainWindow->mpSimulationSetupWidget->setTimeStepLabel(timeStep);

    mIsSaved = true;
    mModelFileName.clear();

    mpGraphicsScene = new GraphicsScene(this);
    mpGraphicsView  = new GraphicsView(mpParentProjectTabWidget->mpHopsan, mpComponentSystem, this);

    //mpView = view;

    mpGraphicsView->setScene(mpGraphicsScene);

    QVBoxLayout *tabLayout = new QVBoxLayout;
    tabLayout->addWidget(mpGraphicsView);
//    addStretch(1);

//    setWindowModified(true);

    setLayout(tabLayout);

    this->useIsoGraphics = true;

}


//! Should be called when a model has changed in some sense,
//! e.g. a component added or a connection has changed.
void ProjectTab::hasChanged()
{
    if (mIsSaved)
    {
        QString tabName = mpParentProjectTabWidget->tabText(mpParentProjectTabWidget->currentIndex());

        tabName.append("*");
        mpParentProjectTabWidget->setTabText(mpParentProjectTabWidget->currentIndex(), tabName);

        mIsSaved = false;
    }
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
    mpParentMainWindow = parent;

    mpHopsan = HopsanEssentials::getInstance();

    MainWindow *pMainWindow = (qobject_cast<MainWindow *>(parent)); //Ugly!!!
    pMainWindow->mpMessageWidget->setHopsanCorePtr(mpHopsan);
    connect(this, SIGNAL(checkMessages()), pMainWindow->mpMessageWidget, SLOT(checkMessages()));


    setTabsClosable(true);
    mNumberOfUntitledTabs = 0;

    connect(this,SIGNAL(tabCloseRequested(int)),SLOT(closeProjectTab(int)));

}


//! Access current tabwidget.
//! @return the current tabwidget
ProjectTab *ProjectTabWidget::getCurrentTab()
{
    return qobject_cast<ProjectTab *>(currentWidget());
}


//! Adds an existing ProjectTab object to itself.
//! @see closeProjectTab(int index)
void ProjectTabWidget::addProjectTab(ProjectTab *projectTab, QString tabName)
{
    projectTab->setParent(this);

    addTab(projectTab, tabName);
    setCurrentWidget(projectTab);
}


//! Adds a ProjectTab object (a new tab) to itself.
//! @see closeProjectTab(int index)
void ProjectTabWidget::addNewProjectTab(QString tabName)
{
    tabName.append(QString::number(mNumberOfUntitledTabs));

    ProjectTab *newTab = new ProjectTab(this);
    newTab->mIsSaved = false;

    addTab(newTab, tabName.append(QString("*")));
    setCurrentWidget(newTab);

    mNumberOfUntitledTabs += 1;
}


//! Saves current project.
//! @see saveProjectTab(int index)
void ProjectTabWidget::saveProjectTab()
{
    saveProjectTab(currentIndex(), false);
}

//! Saves current project to a new model file.
//! @see saveProjectTab(int index)
void ProjectTabWidget::saveProjectTabAs()
{
    saveProjectTab(currentIndex(), true);
}


//! Saves project at index.
//! @param index defines which project to save.
//! @see saveProjectTab()
void ProjectTabWidget::saveProjectTab(int index, bool saveAs)
{
    ProjectTab *currentTab = qobject_cast<ProjectTab *>(widget(index));
    QString tabName = tabText(index);

    if (currentTab->mIsSaved)
    {
        //Nothing to do
        //statusBar->showMessage(QString("Project: ").append(tabName).append(QString(" is already saved")));
    }
    else
    {
        /*Add some "saving code" in the future:
         *
         *
         *
         */
        tabName.chop(1);
        setTabText(index, tabName);
        //statusBar->showMessage(QString("Project: ").append(tabName).append(QString(" saved")));
        std::cout << "ProjectTabWidget: " << qPrintable(QString("Project: ").append(tabName).append(QString(" saved"))) << std::endl;
        currentTab->mIsSaved = true;
    }
    this->saveModel(saveAs);
}


//! Closes current project.
//! @param index defines which project to close.
//! @return true if closing went ok. false if the user canceled the operation.
//! @see closeAllProjectTabs()
bool ProjectTabWidget::closeProjectTab(int index)
{
    if (!(qobject_cast<ProjectTab *>(widget(index))->mIsSaved))
    {
        QString modelName;
        modelName = tabText(index);
        modelName.chop(1);
        QMessageBox msgBox;
        msgBox.setText(QString("The model '").append(modelName).append("'").append(QString(" is not saved.")));
        msgBox.setInformativeText("Do you want to save your changes before closing?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);

        int answer = msgBox.exec();

        switch (answer)
        {
        case QMessageBox::Save:
            // Save was clicked
            std::cout << "ProjectTabWidget: " << "Save and close" << std::endl;
            saveProjectTab(index, false);
            removeTab(index);
            return true;
        case QMessageBox::Discard:
            // Don't Save was clicked
            removeTab(index);
            return true;
        case QMessageBox::Cancel:
            // Cancel was clicked
            std::cout << "ProjectTabWidget: " << "Cancel closing" << std::endl;
            return false;
        default:
            // should never be reached
            return false;
        }
    }
    else
    {
        std::cout << "ProjectTabWidget: " << "Closing project: " << qPrintable(tabText(index)) << std::endl;
        //statusBar->showMessage(QString("Closing project: ").append(tabText(index)));
        removeTab(index);
        return true;
    }
}


//! Closes all opened projects.
//! @return true if closing went ok. false if the user canceled the operation.
//! @see closeProjectTab(int index)
//! @see saveProjectTab()
bool ProjectTabWidget::closeAllProjectTabs()
{
    while(count() > 0)
    {
        setCurrentIndex(count()-1);
        if (!closeProjectTab(count()-1))
        {
            return false;
        }
    }
    return true;
}


//! Simulates the model in current open tab in a separate thread, the GUI runs a progressbar parallel to the simulation.
void ProjectTabWidget::simulateCurrent()
{
    if (!currentWidget())
    {
        mpParentMainWindow->mpMessageWidget->printGUIMessage(QString("There is no open system to simulate"));
        return;
    }

    ProjectTab *pCurrentTab = getCurrentTab();

    double startTime = pCurrentTab->mpParentProjectTabWidget->mpParentMainWindow->mpSimulationSetupWidget->getStartTimeLabel();
    double finishTime = pCurrentTab->mpParentProjectTabWidget->mpParentMainWindow->mpSimulationSetupWidget->getFinishTimeLabel();

    double *pCoreComponentTime = pCurrentTab->mpComponentSystem->getTimePtr();
    QString timeTxt;
    double dt = finishTime - startTime;
    size_t nSteps = dt/pCurrentTab->mpComponentSystem->getDesiredTimeStep();

    QProgressDialog progressBar(tr("Initialize simulation..."), tr("&Abort initialization"), 0, 0, this);
    std::cout << progressBar.geometry().width() << " " << progressBar.geometry().height() << std::endl;
    progressBar.setWindowModality(Qt::WindowModal);
    progressBar.setWindowTitle(tr("Simulate!"));

    InitializationThread actualInitialization(pCurrentTab->mpComponentSystem, startTime, finishTime, this);
    size_t i=0;
    actualInitialization.start();
    actualInitialization.setPriority(QThread::TimeCriticalPriority);
    while (actualInitialization.isRunning())
    {
        progressBar.setValue(i++);
        if (progressBar.wasCanceled())
        {
            pCurrentTab->mpComponentSystem->stop();
        }
    }
    progressBar.setValue(i);
    actualInitialization.wait(); //Make sure actualSimulation do not goes out of scope during simulation

    if (!progressBar.wasCanceled())
    {
        SimulationThread actualSimulation(pCurrentTab->mpComponentSystem, startTime, finishTime, this);
        actualSimulation.start();
        actualSimulation.setPriority(QThread::TimeCriticalPriority);
        progressBar.setLabelText(tr("Running simulation..."));
        progressBar.setCancelButtonText(tr("&Abort simulation"));
        progressBar.setMinimum(0);
        progressBar.setMaximum(nSteps);
        while (actualSimulation.isRunning())
        {
            progressBar.setValue((size_t)(*pCoreComponentTime/dt * nSteps));
            if (progressBar.wasCanceled())
            {
                pCurrentTab->mpComponentSystem->stop();
            }
        }
        progressBar.setValue((size_t)(*pCoreComponentTime/dt * nSteps));
        actualSimulation.wait(); //Make sure actualSimulation do not goes out of scope during simulation
    }

    if (progressBar.wasCanceled())
        mpParentMainWindow->mpMessageWidget->printGUIMessage(QString(tr("Simulation of '").append(QString::fromStdString(pCurrentTab->mpComponentSystem->getName())).append(tr("' was terminated!"))));
    else
        mpParentMainWindow->mpMessageWidget->printGUIMessage(QString(tr("Simulated '").append(QString::fromStdString(pCurrentTab->mpComponentSystem->getName())).append(tr("' successfully!"))));

    emit checkMessages();

}

//! Loads a model from a file and opens it in a new project tab.
//! @see saveModel(bool saveAs)
void ProjectTabWidget::loadModel()
{
    QDir fileDialogOpenDir;

    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose Model File"),
                                                         fileDialogOpenDir.currentPath(),
                                                         tr("Hopsan Model Files (*.hmf)"));
    if (modelFileName.isEmpty())
        return;


    std::ifstream modelFile (modelFileName.toStdString().c_str());

    QFileInfo fileInfo(modelFileName);

    this->addProjectTab(new ProjectTab(this), fileInfo.fileName());
    ProjectTab *pCurrentTab = qobject_cast<ProjectTab *>(currentWidget());
    pCurrentTab->mModelFileName = modelFileName;

        //Necessary declarations
    string inputLine;
    string inputWord;
    string componentType;
    string componentName;
    string startComponentName, endComponentName;
    int startPortNumber, endPortNumber;
    int length, heigth;
    int posX, posY;
    int nameTextPos;
    qreal rotation;
    string tempString;

    while (! modelFile.eof() )
    {
            //Read the line
        getline(modelFile,inputLine);                                   //Read a line
        stringstream inputStream(inputLine);

            //Extract first word unless stream is empty
        if ( inputStream >> inputWord )
        {
            //----------- Create New SubSystem -----------//

            if ( inputWord == "HOPSANGUIVERSION")
            {
                inputStream >> tempString;
                if(QString(tempString.c_str()) > QString(HOPSANGUIVERSION))
                {
                    mpParentMainWindow->mpMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in newer version of Hopsan"));
                }
                else if(QString(tempString.c_str()) < QString(HOPSANGUIVERSION))
                {
                    mpParentMainWindow->mpMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in older version of Hopsan"));
                }
            }
            else if ( inputWord == "HOPSANGUIMODELFILEVERSION")
            {
                inputStream >> tempString;
                if(QString(tempString.c_str()) > QString(HOPSANGUIMODELFILEVERSION))
                {
                    mpParentMainWindow->mpMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in newer version of Hopsan"));
                }
                else if(QString(tempString.c_str()) < QString(HOPSANGUIMODELFILEVERSION))
                {
                    mpParentMainWindow->mpMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in older version of Hopsan"));
                }
            }
            else if ( inputWord == "HOPSANGUICOMPONENTDESCRIPTIONFILEVERSION")
            {
                inputStream >> tempString;
                qDebug() << inputWord.c_str() << " " << tempString.c_str();
                if(QString(tempString.c_str()) > QString(HOPSANGUICOMPONENTDESCRIPTIONFILEVERSION))
                {
                    mpParentMainWindow->mpMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in newer version of Hopsan"));
                }
                else if(QString(tempString.c_str()) < QString(HOPSANGUICOMPONENTDESCRIPTIONFILEVERSION))
                {
                    mpParentMainWindow->mpMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in older version of Hopsan"));
                }
            }


            if ( inputWord == "COMPONENT" )
            {
                inputStream >> componentType;
                inputStream >> componentName;
                inputStream >> posX;
                inputStream >> posY;
                inputStream >> rotation;
                inputStream >> nameTextPos;

                //! @todo This component need to be loaded in the library, or maybe we should auto load it if possible if missing (probably dfficult)
                QStringList appearanceData = mpParentMainWindow->mpLibrary->getAppearanceData(QString(componentType.c_str()));
                pCurrentTab->mpGraphicsView->addGUIObject(QString(componentType.c_str()), appearanceData, QPoint(posX, posY), QString(componentName.c_str()));
                pCurrentTab->mpGraphicsView->getGUIObject(QString(componentName.c_str()))->setNameTextPos(nameTextPos);
                while(pCurrentTab->mpGraphicsView->getGUIObject(QString(componentName.c_str()))->rotation() != rotation)
                {
                    pCurrentTab->mpGraphicsView->getGUIObject(QString(componentName.c_str()))->rotate();
                }

            }
            if ( inputWord == "CONNECT" )
            {
                GraphicsView *pCurrentView = pCurrentTab->mpGraphicsView;
                inputStream >> startComponentName;
                inputStream >> startPortNumber;
                inputStream >> endComponentName;
                inputStream >> endPortNumber;
                GUIPort *startPort = pCurrentView->getGUIObject(QString(startComponentName.c_str()))->getPort(startPortNumber);
                pCurrentView->addConnector(startPort);
                GUIConnector *pTempConnector = pCurrentView->getTempConnector();
                pCurrentView->scene()->addItem(pTempConnector);
                while(inputStream >> inputWord)
                {
                    if(inputWord == "VERTICAL")
                    {
                        inputStream >> heigth;
                        pTempConnector->addFixedLine(0, heigth, GUIConnectorLine::VERTICAL);
                    }
                    else if (inputWord == "HORIZONTAL")
                    {
                        inputStream >> length;
                        pTempConnector->addFixedLine(length, 0, GUIConnectorLine::HORIZONTAL);
                    }
                    else if (inputWord == "DIAGONAL")
                    {
                        inputStream >> length;
                        inputStream >> heigth;
                        pTempConnector->addFixedLine(length, heigth, GUIConnectorLine::DIAGONAL);
                    }
                    else
                    {
                    }
                }
                GUIPort *endPort = pCurrentView->getGUIObject(QString(endComponentName.c_str()))->getPort(endPortNumber);
                QPointF newPos = endPort->mapToScene(endPort->boundingRect().center());
                pTempConnector->updateConnector(pTempConnector->startPos, newPos);
                endPort->getComponent()->addConnector(pTempConnector);
                pTempConnector->setEndPort(endPort);
                pTempConnector->getStartPort()->hide();
                pTempConnector->getEndPort()->hide();
                pCurrentView->mIsCreatingConnector = false;

                std::stringstream tempStream;
                tempStream << startPort->getComponent()->getName().toStdString() << " " << startPort->getPortNumber() << " " <<
                              endPort->getComponent()->getName().toStdString() << " " << endPort->getPortNumber();
                pCurrentView->mConnectionMap.insert(QString(tempStream.str().c_str()), pTempConnector);
                bool success = pCurrentView->getModelPointer()->connect(startPort->mpCorePort, endPort->mpCorePort);
                if (!success)
                {
                    qDebug() << "Unsuccessful connection try" << endl;
                    assert(false);
                }
            }

        }
    }
    emit checkMessages();
}


//! Saves the model in the active project tab to a model file.
//! @param saveAs tells whether or not an already existing file name shall be used
//! @see saveProjectTab()
//! @see loadModel()
void ProjectTabWidget::saveModel(bool saveAs)
{
    ProjectTab *pCurrentTab = qobject_cast<ProjectTab *>(currentWidget());
    GraphicsView *pCurrentView = pCurrentTab->mpGraphicsView;

    QString modelFileName;
    if(pCurrentTab->mModelFileName.isEmpty() | saveAs)
    {
        QDir fileDialogSaveDir;
        modelFileName = QFileDialog::getSaveFileName(this, tr("Save Model File"),
                                                             fileDialogSaveDir.currentPath(),
                                                             tr("Hopsan Model Files (*.hmf)"));
        pCurrentTab->mModelFileName = modelFileName;
    }
    else
    {
        modelFileName = pCurrentTab->mModelFileName;
    }
    std::ofstream modelFile (modelFileName.toStdString().c_str());
    QFileInfo fileInfo(modelFileName);

    modelFile << "--------------------------------------------------------------" << std::endl;
    modelFile << "-------------------  HOPSAN NG MODEL FILE  -------------------" << std::endl;
    modelFile << "--------------------------------------------------------------" << std::endl;
    modelFile << "HOPSANGUIVERSION " << HOPSANGUIVERSION << std::endl;
    modelFile << "HOPSANGUIMODELFILEVERSION " << HOPSANGUIMODELFILEVERSION << std::endl;
    modelFile << "HOPSANGUICOMPONENTDESCRIPTIONFILEVERSION " << HOPSANGUICOMPONENTDESCRIPTIONFILEVERSION << std::endl;
    modelFile << "--------------------------------------------------------------" << std::endl;

    QMap<QString, GUIObject*>::iterator it;
    for(it = pCurrentView->mGUIObjectMap.begin(); it!=pCurrentView->mGUIObjectMap.end(); ++it)
    {
        QPointF pos = it.value()->mapToScene(it.value()->boundingRect().center());
        modelFile << "COMPONENT " << it.value()->getTypeName().toStdString() << " " << it.key().toStdString()
                  << " " << pos.x() << " " << pos.y() << " " << it.value()->rotation() << " " << it.value()->getNameTextPos() << std::endl;
    }

    modelFile << "--------------------------------------------------------------" << std::endl;

    QMap<QString, GUIConnector *>::iterator it2;
    for(it2 = pCurrentView->mConnectionMap.begin(); it2!=pCurrentView->mConnectionMap.end(); ++it2)
    {
        modelFile << "CONNECT " << it2.key().toStdString();
        for(size_t i = 0; i!=it2.value()->mLines.size(); ++i)
        {
            int geometry = it2.value()->mLines[i]->getGeometry();
            switch (geometry)
            {
                case 0:
                    modelFile << " VERTICAL " << (it2.value()->mLines[i]->endPos.y()-it2.value()->mLines[i]->startPos.y());
                    break;
                case 1:
                    modelFile << " HORIZONTAL " << (it2.value()->mLines[i]->endPos.x()-it2.value()->mLines[i]->startPos.x());
                    break;
                case 2:
                    modelFile << " DIAGONAL" << (it2.value()->mLines[i]->endPos.x()-it2.value()->mLines[i]->startPos.x()) << (it2.value()->mLines[i]->endPos.y()-it2.value()->mLines[i]->startPos.y());
                    break;
            }
        }
        modelFile << "\n";
    }
    modelFile << "--------------------------------------------------------------" << std::endl;
}



void ProjectTabWidget::setIsoGraphics(bool value)
{
    qDebug() << "Debug X1";
    this->getCurrentTab()->useIsoGraphics = value;
    qDebug() << "Use ISO graphics = " << value;

    QPen passivePen,activePen,hoverPen;
    ProjectTab *pCurrentTab = getCurrentTab();
    GraphicsView *pCurrentView = pCurrentTab->mpGraphicsView;
    QMap<QString, GUIConnector *>::iterator it;
    for(it = pCurrentView->mConnectionMap.begin(); it!=pCurrentView->mConnectionMap.end(); ++it)
    {
        if(value)
        {
            if((it.value()->getEndPort()->mpCorePort->getNodeType() == "NodeHydraulic") | (it.value()->getEndPort()->mpCorePort->getNodeType() == "NodeMechanic"))
            {
                passivePen = QPen(QColor("black"),1, Qt::SolidLine, Qt::RoundCap);
                activePen = QPen(QColor("red"), 2, Qt::SolidLine, Qt::RoundCap);                    //1.6180339887499
                hoverPen = QPen(QColor("darkRed"),2, Qt::SolidLine, Qt::RoundCap);
            }
            else if(it.value()->getEndPort()->mpCorePort->getNodeType() == "NodeSignal")
            {
                passivePen = QPen(QColor("blue"),1, Qt::DashLine);
                activePen = QPen(QColor("red"), 2, Qt::DashLine);
                hoverPen = QPen(QColor("darkRed"),2, Qt::DashLine);
            }
        }
        else
        {
            if((it.value()->getEndPort()->mpCorePort->getNodeType() == "NodeHydraulic") | (it.value()->getEndPort()->mpCorePort->getNodeType() == "NodeMechanic"))
            {
                passivePen = QPen(QColor("black"),2, Qt::SolidLine, Qt::RoundCap);
                activePen = QPen(QColor("red"), 3, Qt::SolidLine, Qt::RoundCap);                    //1.6180339887499
                hoverPen = QPen(QColor("darkRed"),3, Qt::SolidLine, Qt::RoundCap);
            }
            else if(it.value()->getEndPort()->mpCorePort->getNodeType() == "NodeSignal")
            {
                passivePen = QPen(QColor("blue"),1, Qt::DashLine);
                activePen = QPen(QColor("red"), 2, Qt::DashLine);
                hoverPen = QPen(QColor("darkRed"),2, Qt::DashLine);
            }
        }

        it.value()->setPens(activePen, passivePen, hoverPen);

    }
}
