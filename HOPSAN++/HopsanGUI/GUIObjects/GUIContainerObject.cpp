//$Id$

#include "GUIContainerObject.h"

//! @todo clean these up
#include "../Widgets/ProjectTabWidget.h"
#include "../MainWindow.h"
#include "../Dialogs/ComponentPropertiesDialog.h"
#include "../GUIPort.h"
#include "../GUIConnector.h"
#include "../Utilities/GUIUtilities.h"
#include "../UndoStack.h"
#include "../Widgets/MessageWidget.h"
#include "../GraphicsView.h"
#include "../Widgets/LibraryWidget.h"
#include "../loadObjects.h"
#include "../CoreAccess.h"
#include "GUIObject.h"
#include "GUIComponent.h"
#include "GUIGroup.h"
#include "GUISystemPort.h"
#include "GUIWidgets.h"
#include "GUISystem.h"
#include "../CopyStack.h"
#include "../Widgets/QuickNavigationWidget.h"
#include "Widgets/PlotWidget.h"
#include <float.h>

#include <QDomElement>


GUIContainerObject::GUIContainerObject(QPoint position, qreal rotation, const GUIModelObjectAppearance* pAppearanceData, selectionStatus startSelected, graphicsType gfxType, GUIContainerObject *pParentContainer, QGraphicsItem *pParent)
        : GUIModelObject(position, rotation, pAppearanceData, startSelected, gfxType, pParentContainer, pParent)
{
        //Initialize
    setIsCreatingConnector(false);
    mIsRenamingObject = false;
    mPortsHidden = false;
    mUndoDisabled = false;
    mGfxType = USERGRAPHICS;

    mHighestWidgetIndex = 0;

    mPasteOffset = -30;

    //Create the scene
    mpScene = new QGraphicsScene();

    //Create the undastack
    mUndoStack = new UndoStack(this);
    mUndoStack->clear();

    //Establish connections
    //connect(this->systemPortAction, SIGNAL(triggered()), SLOT(addSystemPort()));
    connect(this, SIGNAL(checkMessages()), gpMainWindow->mpMessageWidget, SLOT(checkMessages()));
    connect(gpMainWindow->undoAction, SIGNAL(triggered()), this, SLOT(undo()));
    connect(gpMainWindow->redoAction, SIGNAL(triggered()), this, SLOT(redo()));
    connect(gpMainWindow->mpUndoWidget->getUndoButton(), SIGNAL(pressed()), this, SLOT(undo()));
    connect(gpMainWindow->mpUndoWidget->getRedoButton(), SIGNAL(pressed()), this, SLOT(redo()));
    connect(gpMainWindow->mpUndoWidget->getClearButton(), SIGNAL(pressed()), this, SLOT(clearUndo()));
    connect(gpMainWindow->hidePortsAction, SIGNAL(triggered(bool)), this, SLOT(hidePorts(bool)));


}

GUIContainerObject::~GUIContainerObject()
{
    qDebug() << ",,,,,,,,,,,,GUIContainer destructor";
}

void GUIContainerObject::makeRootSystem()
{
    mContainerStatus = ROOT;
}


//! @todo use enums instead
int GUIContainerObject::findPortEdge(QPointF center, QPointF pt)
{
    //By swapping place of pt1 and pt2 we get the angle in the same coordinate system as the view
    QPointF diff = pt-center;
    //qDebug() << "=============The Diff: " << diff;

    //If only one sysport default to left side
    //! @todo Do this smarter later and take ito account port orientation, or position relative all other components, need to extend this function a bit for that though
    if (diff.manhattanLength() < 1.0)
    {
        return 2;
    }

    qreal angle = normRad(qAtan2(diff.x(), diff.y()));
    qDebug() << "angle: " << rad2deg(angle);
    if (fabs(angle) <= M_PI_4)
    {
        return 0;
    }
    else if (fabs(angle) >= 3.0*M_PI_4)
    {
        return 2;
    }
    else if (angle > M_PI_4)
    {
        return 1;
    }
    else
    {
        return 3;
    }
}


void GUIContainerObject::refreshExternalPortsAppearanceAndPosition()
{
    //refresh the external port poses
    GUIModelObjectMapT::iterator moit;
    double val;

    //Set the initial values to be overwriten by the if bellow
    //! @todo maybe declare these or something like really big number in HopsanGUI to avoid needing to include float.h
    double xMin=FLT_MAX, xMax=FLT_MIN, yMin=FLT_MAX, yMax=FLT_MIN;
    for(moit = mGUIModelObjectMap.begin(); moit != mGUIModelObjectMap.end(); ++moit)
    {
        if(moit.value()->type() == GUISYSTEMPORT)
        {
            //check x max and min
            val = moit.value()->getCenterPos().x();
            xMin = std::min(xMin,val);
            xMax = std::max(xMax,val);
            //check y max and min
            val = moit.value()->getCenterPos().y();
            yMin = std::min(yMin,val);
            yMax = std::max(yMax,val);
        }
    }
    //! @todo Find out if it is possible to ask the scene or view for this information instead of calulating it ourselves
    QPointF center = QPointF((xMax+xMin)/2.0, (yMax+yMin)/2.0);
    //qDebug() << "center max min: " << center << " " << xMin << " " << xMax << " " << yMin << " " << yMax;

    QVector<GUIPort*> leftEdge;
    QVector<GUIPort*> rightEdge;
    QVector<GUIPort*> topEdge;
    QVector<GUIPort*> bottomEdge;

    for(moit = mGUIModelObjectMap.begin(); moit != mGUIModelObjectMap.end(); ++moit)
    {
        if(moit.value()->type() == GUISYSTEMPORT)
        {
            //            QLineF line = QLineF(center, moit.value()->getCenterPos());
            //            this->getContainedScenePtr()->addLine(line); //debug-grej

            int edge = findPortEdge(center, moit.value()->getCenterPos());
            //qDebug() << " sysp: " << moit.value()->getName() << " edge: " << edge;

            switch (edge) {
            case 0:
                rightEdge.append(this->getPort(moit.value()->getName()));
                break;
            case 1:
                bottomEdge.append(this->getPort(moit.value()->getName()));
                break;
            case 2:
                leftEdge.append(this->getPort(moit.value()->getName()));
                break;
            case 3:
                topEdge.append(this->getPort(moit.value()->getName()));
                break;
            }
        }
    }

    //Now disperse the port icons evenly along each edge
    QVector<GUIPort*>::iterator it;
    qreal disp; //Dispersion factor
    qreal sdisp; //sumofdispersionfactors

    //! @todo maybe we should be able to update rotation in all of these also
    //! @todo need to be sure we sort them in the correct order first
    //! @todo wierd to use createfunction to refresh graphics, but ok for now
    disp = 1.0/((qreal)(rightEdge.size()+1));
    sdisp=disp;
    for (it=rightEdge.begin(); it!=rightEdge.end(); ++it)
    {
        (*it)->updatePositionByFraction(1.0, sdisp);
        this->createExternalPort((*it)->getName());    //refresh the external port graphics
        sdisp += disp;
    }

    disp = 1.0/((qreal)(bottomEdge.size()+1));
    sdisp=disp;
    for (it=bottomEdge.begin(); it!=bottomEdge.end(); ++it)
    {
        (*it)->updatePositionByFraction(sdisp, 1.0);
        this->createExternalPort((*it)->getName());    //refresh the external port graphics
        sdisp += disp;
    }

    disp = 1.0/((qreal)(leftEdge.size()+1));
    sdisp=disp;
    for (it=leftEdge.begin(); it!=leftEdge.end(); ++it)
    {
        (*it)->updatePositionByFraction(0.0, sdisp);
        this->createExternalPort((*it)->getName());    //refresh the external port graphics
        sdisp += disp;
    }

    disp = 1.0/((qreal)(topEdge.size()+1));
    sdisp=disp;
    for (it=topEdge.begin(); it!=topEdge.end(); ++it)
    {
        (*it)->updatePositionByFraction(sdisp, 0.0);
        this->createExternalPort((*it)->getName());    //refresh the external port graphics
        sdisp += disp;
    }
}

GUIContainerObject::CONTAINERSTATUS GUIContainerObject::getContainerStatus()
{
    return mContainerStatus;
}

//! @brief Use this function to calculate the placement of the ports on a subsystem icon.
//! @param[in] w width of the subsystem icon
//! @param[in] h heigth of the subsystem icon
//! @param[in] angle the angle in radians of the line between center and the actual port
//! @param[out] x the new calculated horizontal placement for the port
//! @param[out] y the new calculated vertical placement for the port
//! @todo rename this one and maybe change it a bit as it is now included in this class, it should be common for subsystems and groups
void GUIContainerObject::calcSubsystemPortPosition(const double w, const double h, const double angle, double &x, double &y)
{
    //! @todo make common PI declaration, maybe also PIhalf or include math.h and use M_PI
    double tanAngle = tan(angle);//Otherwise division by zero
    if(fabs(tanAngle) < 0.0001)
    {
        tanAngle=.0001;
    }
    if(angle>3.1415*3.0/2.0)
    {
        x=-std::max(std::min(h/tanAngle, w), -w);
        y=std::max(std::min(w*tanAngle, h), -h);
    }
    else if(angle>3.1415)
    {
        x=-std::max(std::min(h/tanAngle, w), -w);
        y=-std::max(std::min(w*tanAngle, h), -h);
    }
    else if(angle>3.1415/2.0)
    {
        x=std::max(std::min(h/tanAngle, w), -w);
        y=-std::max(std::min(w*tanAngle, h), -h);
    }
    else
    {
        x=std::max(std::min(h/tanAngle, w), -w);
        y=std::max(std::min(w*tanAngle, h), -h);
    }
}


CoreSystemAccess *GUIContainerObject::getCoreSystemAccessPtr()
{
    //Should be overloaded
    return 0;
}

//! @brief Retunrs a pointer to the contained scene
QGraphicsScene *GUIContainerObject::getContainedScenePtr()
{
    return this->mpScene;
}


void GUIContainerObject::createPorts()
{
//    //! @todo make sure that all old ports and connections are cleared, (in case we reload, but maybe we can discard old system and create new in that case)
//    //Create the graphics for the ports but do NOT create new ports, use the system ports within the subsystem
//    PortAppearanceMapT::iterator it;
//    for (it = mGUIModelObjectAppearance.getPortAppearanceMap().begin(); it != mGUIModelObjectAppearance.getPortAppearanceMap().end(); ++it)
//    {
//        //Create new external port if if does not already exist (this is the ussual case for individual components)
//        GUIPort *pPort = this->getPort(it.key());

//        if ( pPort == 0 )
//        {
//            qDebug() << "##This is OK though as this means that we should create the stupid port for the first time";
//            //! @todo fix this
//            //qDebug() << "getNode and portType for " << it.key();
//            //SystemPort "Component Name" (GuiModelObjectName) and portname is same
//            //One other way would be to ask our parent to find the types of our ports but that would be even more strange and would not work on the absolute root system
//            //! @todo to minimaze search time make a get porttype  and nodetype function, we need to search twice now
//            QString nodeType = this->getCoreSystemAccessPtr()->getNodeType(it.key(), it.key());
//            QString portType = this->getCoreSystemAccessPtr()->getPortType(it.key(), it.key());
//            it.value().selectPortIcon(getTypeCQS(), portType, nodeType);

//            qreal x = it.value().x;
//            qreal y = it.value().y;

//            pPort = new GUIPort(it.key(), x*mpIcon->sceneBoundingRect().width(), y*mpIcon->sceneBoundingRect().height(), &(it.value()), this);
//            mPortListPtrs.append(pPort);
//        }
//        else
//        {
//            //The external port already seems to exist, lets update it incase something has changed
//            //! @todo Maybe need to have a refresh portappearance function, dont really know if thiss will ever be used though, will fix when it becomes necessary
//            //pPort->mpP
//            qDebug() << "----------------------------------------ExternalPort already exist does not create it again: " << it.key() << " in: " << this->getName();
//        }
//    }
    assert(false);
}

//! @brief This method creates ONE external port. It assumes that port appearance information for this port exists
//! @todo maybe defualt create that info if it is missing
void GUIContainerObject::createExternalPort(QString portName)
{
    //Fetch appearance data
    PortAppearanceMapT::iterator it = mGUIModelObjectAppearance.getPortAppearanceMap().find(portName);
    if (it != mGUIModelObjectAppearance.getPortAppearanceMap().end())
    {
        //! @todo to minimaze search time make a get porttype  and nodetype function, we need to search twice now
        QString nodeType = this->getCoreSystemAccessPtr()->getNodeType(it.key(), it.key());
        QString portType = this->getCoreSystemAccessPtr()->getPortType(it.key(), it.key());
        it.value().selectPortIcon(getTypeCQS(), portType, nodeType);

        //Create new external port if if does not already exist (this is the usual case for individual components)
        GUIPort *pPort = this->getPort(it.key());
        if ( pPort == 0 )
        {
            qDebug() << "##This is OK though as this means that we should create the stupid port for the first time";

            qreal x = it.value().x;
            qreal y = it.value().y;

            if (this->type() == GUIGROUP)
            {
                pPort = new GroupPort(it.key(), x*mpIcon->sceneBoundingRect().width(), y*mpIcon->sceneBoundingRect().height(), &(it.value()), this);
            }
            else
            {
                pPort = new GUIPort(it.key(), x*mpIcon->sceneBoundingRect().width(), y*mpIcon->sceneBoundingRect().height(), &(it.value()), this);
            }


            mPortListPtrs.append(pPort);
        }
        else
        {

            //The external port already seems to exist, lets update it incase something has changed
            //! @todo Maybe need to have a refresh portappearance function, dont really know if thiss will ever be used though, will fix when it becomes necessary
            pPort->refreshPortGraphics();
            qDebug() << "--------------------------ExternalPort already exist refreshing its graphics: " << it.key() << " in: " << this->getName();
        }
    }
    else
    {
        qDebug() << "Could not find portappearance info for port: " << portName << " in: " << this->getName();
        assert(false);
    }
}

//! @todo maybe we should use a map instead to make delete more efficient, (may not amtter usually not htat many external ports)
void GUIContainerObject::removeExternalPort(QString portName)
{
    //qDebug() << "mPortListPtrs.size(): " << mPortListPtrs.size();
    QList<GUIPort*>::iterator plit;
    for (plit=mPortListPtrs.begin(); plit!=mPortListPtrs.end(); ++plit)
    {
        if ((*plit)->getName() == portName )
        {
            delete *plit;
            mPortListPtrs.erase(plit);
            break;
        }
    }
    //qDebug() << "mPortListPtrs.size(): " << mPortListPtrs.size();
}

//! @brief Temporary addSubSystem functin should be same later on
//! Adds a new component to the GraphicsView.
//! @param componentType is a string defining the type of component.
//! @param position is the position where the component will be created.
//! @param name will be the name of the component.
//! @returns a pointer to the created and added object
//! @todo only modelobjects for now
GUIModelObject* GUIContainerObject::addGUIModelObject(GUIModelObjectAppearance* pAppearanceData, QPoint position, qreal rotation, selectionStatus startSelected, nameVisibility nameStatus, undoStatus undoSettings)
{
        //Deselect all other components and connectors
    emit deselectAllGUIObjects();
    emit deselectAllGUIConnectors();

    //qDebug()  << "Adding GUIModelObject, typename: " << componentTypeName << " displayname: " << pAppearanceData->getName() << " systemname: " << this->getName();
    QString componentTypeName = pAppearanceData->getTypeName();
    if (componentTypeName == "Subsystem")
    {
        mpTempGUIModelObject= new GUISystem(position, rotation, pAppearanceData, this, startSelected, mGfxType);

            //Disconnect new subsystem with ctrl-z and ctrl-y (they will be reconnected when entering system)
        disconnect(gpMainWindow->undoAction, SIGNAL(triggered()), mpTempGUIModelObject, SLOT(undo()));
        disconnect(gpMainWindow->redoAction, SIGNAL(triggered()), mpTempGUIModelObject, SLOT(redo()));
    }
    else if (componentTypeName == "SystemPort") //!< @todo dont hardcode
    {
        mpTempGUIModelObject = new GUISystemPort(pAppearanceData, position, rotation, this, startSelected, mGfxType);
        //Add appearance data for the external version of this systemport to the continer object so that the external port can be created with the creatPorts method
        mGUIModelObjectAppearance.getPortAppearanceMap().insert(mpTempGUIModelObject->getName(), GUIPortAppearance());
        this->createExternalPort(mpTempGUIModelObject->getName());
        this->refreshExternalPortsAppearanceAndPosition();
    }
    else if (componentTypeName == "HopsanGUIGroup") //!< @todo dont hardcode
    {
        mpTempGUIModelObject = new GUIGroup(position, rotation, pAppearanceData, this);
        //Disconnect new subsystem with ctrl-z and ctrl-y (they will be reconnected when entering system)
        disconnect(gpMainWindow->undoAction, SIGNAL(triggered()), mpTempGUIModelObject, SLOT(undo()));
        disconnect(gpMainWindow->redoAction, SIGNAL(triggered()), mpTempGUIModelObject, SLOT(redo()));
    }
    else //Assume some standard component type
    {
        mpTempGUIModelObject = new GUIComponent(pAppearanceData, position, rotation, this, startSelected, mGfxType);
    }

    mpScene->addItem(mpTempGUIModelObject);

    emit checkMessages();

    if ( mGUIModelObjectMap.contains(mpTempGUIModelObject->getName()) )
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Trying to add component with name: " + mpTempGUIModelObject->getName() + " that already exist in GUIObjectMap, (Not adding)");
        //! @todo Is this check really necessary? Two objects cannot have the same name anyway...
    }
    else
    {
        mGUIModelObjectMap.insert(mpTempGUIModelObject->getName(), mpTempGUIModelObject);
    }

        //Set name visibility status (no undo because it will be registered as an added object anyway)
    if(nameStatus == NAMEVISIBLE)
    {
        mpTempGUIModelObject->showName(NOUNDO);
    }
    else
    {
        mpTempGUIModelObject->hideName(NOUNDO);
    }


    if(undoSettings == UNDO)
    {
        mUndoStack->registerAddedObject(mpTempGUIModelObject);
    }

    //this->setFocus();

    return mpTempGUIModelObject;
}


QList<QStringList> GUIContainerObject::getFavoriteParameters()
{
    return mFavoriteParameters;
}


void GUIContainerObject::setFavoriteParameter(QString componentName, QString portName, QString dataName, QString dataUnit)
{
    QStringList tempParameter;
    tempParameter.append(componentName);
    tempParameter.append(portName);
    tempParameter.append(dataName);
    tempParameter.append(dataUnit);
    if(!mFavoriteParameters.contains(tempParameter))
    {
        mFavoriteParameters.append(tempParameter);
    }
    gpMainWindow->mpPlotWidget->mpPlotParameterTree->updateList();

    mpParentProjectTab->hasChanged();
}


void GUIContainerObject::removeFavoriteParameterByComponentName(QString componentName)
{
    QList<QStringList>::iterator it;
    for(it=
    mFavoriteParameters.begin(); it!=
    mFavoriteParameters.end(); ++it)
    {
        if((*it).at(0) == componentName)
        {
            mFavoriteParameters.removeAll((*it));
            return;
        }
    }
    gpMainWindow->makeSurePlotWidgetIsCreated();
    gpMainWindow->mpPlotWidget->mpPlotParameterTree->updateList();

    mpParentProjectTab->hasChanged();
}


void GUIContainerObject::addTextWidget(QPoint position, undoStatus undoSettings)
{
    GUITextWidget *tempTextWidget;
    tempTextWidget = new GUITextWidget("Text", position, 0, DESELECTED, this, mHighestWidgetIndex);
    mTextWidgetList.append(tempTextWidget);
    mWidgetMap.insert(mHighestWidgetIndex, tempTextWidget);
    ++mHighestWidgetIndex;
    if(undoSettings == UNDO)
    {
        mUndoStack->registerAddedTextWidget(tempTextWidget);
    }
    mpParentProjectTab->hasChanged();
}


void GUIContainerObject::addBoxWidget(QPoint position, undoStatus undoSettings)
{
    GUIBoxWidget *tempBoxWidget;
    tempBoxWidget = new GUIBoxWidget(position, 0, DESELECTED, this, mHighestWidgetIndex);
    mBoxWidgetList.append(tempBoxWidget);
    mWidgetMap.insert(mHighestWidgetIndex, tempBoxWidget);
    ++mHighestWidgetIndex;
    if(undoSettings == UNDO)
    {
        mUndoStack->registerAddedBoxWidget(tempBoxWidget);
    }
    mpParentProjectTab->hasChanged();
}


//! Delete GUIObject with specified name
//! @param objectName is the name of the componenet to delete
void GUIContainerObject::deleteGUIModelObject(QString objectName, undoStatus undoSettings)
{
    //qDebug() << "deleteGUIModelObject(): " << objectName << " in: " << this->getName() << " coresysname: " << this->getCoreSystemAccessPtr()->getRootSystemName() ;
    this->removeFavoriteParameterByComponentName(objectName);   //Does nothing unless this is a system

    GUIModelObjectMapT::iterator it = mGUIModelObjectMap.find(objectName);
    GUIModelObject* obj_ptr = it.value();

        //Remove connectors that are connected to the model object
    QList<GUIConnector *> pConnectorList = obj_ptr->getGUIConnectorPtrs();
    for(int i=0; i<pConnectorList.size(); ++i)
    {
        this->removeConnector(pConnectorList[i], undoSettings);
    }

    if (undoSettings == UNDO)
    {
        //Register removal of model object in undo stack
        this->mUndoStack->registerDeletedObject(it.value());
        emit componentChanged();
    }


    if (it != mGUIModelObjectMap.end())
    {
        //! @todo maybe this should be handled somwhere else (not sure maybe this is the best place)
        if ((*it)->type() == GUISYSTEMPORT )
        {
            this->removeExternalPort((*it)->getName());
        }

        mGUIModelObjectMap.erase(it);
        mSelectedGUIObjectsList.removeOne(obj_ptr);
        mpScene->removeItem(obj_ptr);
        delete(obj_ptr);
        emit checkMessages();
    }
    else
    {
        //qDebug() << "In delete GUIObject: could not find object with name " << objectName;
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Error: Could not delete object with name " + objectName + ", object not found");
    }
    mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! This function is used to rename a SubGUIObject
void GUIContainerObject::renameGUIModelObject(QString oldName, QString newName, undoStatus undoSettings)
{
    //Avoid work if no change is requested
    if (oldName != newName)
    {
        QString modNewName;
            //First find record with old name
        GUIModelObjectMapT::iterator it = mGUIModelObjectMap.find(oldName);
        if (it != mGUIModelObjectMap.end())
        {
                //Make a backup copy
            GUIModelObject* obj_ptr = it.value();
                //Erase old record
            mGUIModelObjectMap.erase(it);
                //Set new name, first in core then in gui object
            //qDebug() << "Renaming: " << oldName << " " << newName << " type: " << obj_ptr->type();
            switch (obj_ptr->type())
            {
            case GUICOMPONENT:
                //qDebug() << "GUICOMPONENT";
            case GUISYSTEM :
                //qDebug() << "GUISYSTEM";
                modNewName = this->getCoreSystemAccessPtr()->renameSubComponent(oldName, newName);
                break;
            case GUISYSTEMPORT :
                //qDebug() << "GUISYSTEMPORT";
                modNewName = this->getCoreSystemAccessPtr()->renameSystemPort(oldName, newName);
                break;
            //default :
                //qDebug() << "default";
                    //No Core rename action
            }
            //qDebug() << "modNewName: " << modNewName;
            obj_ptr->setDisplayName(modNewName);
                //Re insert
            mGUIModelObjectMap.insert(obj_ptr->getName(), obj_ptr);
        }
        else
        {
            //qDebug() << "Old name: " << oldName << " not found";
            //! @todo Maybe we should give the user a message?
        }

        if (undoSettings == UNDO)
        {
            mUndoStack->newPost();
            mUndoStack->registerRenameObject(oldName, modNewName);
            emit componentChanged();
        }
    }
    emit checkMessages();
}


//! Tells whether or not a component with specified name exist in the GraphicsView
bool GUIContainerObject::hasGUIModelObject(QString name)
{
    return (mGUIModelObjectMap.count(name) > 0);
}


//! Returns a pointer to the component with specified name.
GUIModelObject *GUIContainerObject::getGUIModelObject(QString name)
{
    if(!mGUIModelObjectMap.contains(name))
    {
        qDebug() << "Request for pointer to non-existing component" << endl;
        assert(false);
    }
    return mGUIModelObjectMap.find(name).value();
}


//! @brief Find a connector in the connector vector
GUIConnector* GUIContainerObject::findConnector(QString startComp, QString startPort, QString endComp, QString endPort)
{
    GUIConnector *item;
    for(int i = 0; i < mSubConnectorList.size(); ++i)
    {
        if((mSubConnectorList[i]->getStartComponentName() == startComp) &&
           (mSubConnectorList[i]->getStartPortName() == startPort) &&
           (mSubConnectorList[i]->getEndComponentName() == endComp) &&
           (mSubConnectorList[i]->getEndPortName() == endPort))
        {
            item = mSubConnectorList[i];
            break;
        }
        //Find even if the caller mixed up start and stop
        else if((mSubConnectorList[i]->getStartComponentName() == endComp) &&
                (mSubConnectorList[i]->getStartPortName() == endPort) &&
                (mSubConnectorList[i]->getEndComponentName() == startComp) &&
                (mSubConnectorList[i]->getEndPortName() == startPort))
        {
            item = mSubConnectorList[i];
            break;
        }
    }
    assert(!item == 0);
    return item;
}


//! @brief Tells whether or not there is a connector between two specified ports
bool GUIContainerObject::hasConnector(QString startComp, QString startPort, QString endComp, QString endPort)
{
    for(int i = 0; i < mSubConnectorList.size(); ++i)
    {
        if((mSubConnectorList[i]->getStartComponentName() == startComp) &&
           (mSubConnectorList[i]->getStartPortName() == startPort) &&
           (mSubConnectorList[i]->getEndComponentName() == endComp) &&
           (mSubConnectorList[i]->getEndPortName() == endPort))
        {
            return true;
        }
        //Find even if the caller mixed up start and stop
        else if((mSubConnectorList[i]->getStartComponentName() == endComp) &&
                (mSubConnectorList[i]->getStartPortName() == endPort) &&
                (mSubConnectorList[i]->getEndComponentName() == startComp) &&
                (mSubConnectorList[i]->getEndPortName() == startPort))
        {
            return true;
        }
    }
    return false;
}


//! Removes the connector from the model.
//! @param pConnector is a pointer to the connector to remove.
//! @param undoSettings is true if the removal of the connector shall not be registered in the undo stack, for example if this function is called by a redo-function.
void GUIContainerObject::removeConnector(GUIConnector* pConnector, undoStatus undoSettings)
{
    bool doDelete = false;
    bool startPortHasMoreConnections = false;
    bool endPortWasConnected = false;
    bool endPortHasMoreConnections = false;

    if(undoSettings == UNDO)
    {
        mUndoStack->registerDeletedConnector(pConnector);
    }

    qDebug() << "removeConnector in: " << this->getName();
    for(int i = 0; i < mSubConnectorList.size(); ++i)
    {
        if(mSubConnectorList[i] == pConnector)
        {
             //! @todo some error handling both ports must exist and be connected to each other
             if(pConnector->isConnected())
             {
                 GUIPort *pStartP = pConnector->getStartPort();
                 GUIPort *pEndP = pConnector->getEndPort();
                 this->getCoreSystemAccessPtr()->disconnect(pStartP->getGuiModelObjectName(), pStartP->getName(), pEndP->getGuiModelObjectName(), pEndP->getName());
                 emit checkMessages();
                 endPortWasConnected = true;
             }
             doDelete = true;
        }
        else if( (pConnector->getStartPort() == mSubConnectorList[i]->getStartPort()) ||
                 (pConnector->getStartPort() == mSubConnectorList[i]->getEndPort()) )
        {
            startPortHasMoreConnections = true;
        }
        else if( (pConnector->getEndPort() == mSubConnectorList[i]->getStartPort()) ||
                 (pConnector->getEndPort() == mSubConnectorList[i]->getEndPort()) )
        {
            endPortHasMoreConnections = true;
        }
        if(mSubConnectorList.empty())
        {
            break;
        }
    }

    if(endPortWasConnected && !endPortHasMoreConnections)
    {
        pConnector->getEndPort()->setVisible(!mPortsHidden);
        pConnector->getEndPort()->setIsConnected(false);
    }

    if(!startPortHasMoreConnections)
    {
        pConnector->getStartPort()->setVisible(!mPortsHidden);
        pConnector->getStartPort()->setIsConnected(false);
    }
    else if(startPortHasMoreConnections && !endPortWasConnected)
    {
        pConnector->getStartPort()->setVisible(false);
        pConnector->getStartPort()->setIsConnected(true);
    }

    if(doDelete)
    {
        mSubConnectorList.removeAll(pConnector);
        mSelectedSubConnectorsList.removeAll(pConnector);
        mpScene->removeItem(pConnector);
        delete pConnector;
    }
    mpParentProjectTab->mpGraphicsView->updateViewPort();
}



//! Begins creation of connector or complete creation of connector depending on the mIsCreatingConnector flag.
//! @param pPort is a pointer to the clicked port, either start or end depending on the mIsCreatingConnector flag.
//! @param undoSettings is true if the added connector shall not be registered in the undo stack, for example if this function is called by a redo function.
void GUIContainerObject::createConnector(GUIPort *pPort, undoStatus undoSettings)
{
    qDebug() << "mIsCreatingConnector: " << getIsCreatingConnector();
        //When clicking start port
    if (!getIsCreatingConnector())
    {
        qDebug() << "CreatingConnector in: " << this->getName() << " startPortName: " << pPort->getName();
        //GUIConnectorAppearance *pConnApp = new GUIConnectorAppearance(pPort->getPortType(), mpParentProjectTab->setGfxType);
        mpTempConnector = new GUIConnector(pPort, this);
        this->deselectAll(); //! @todo maybe this should be a signal
        setIsCreatingConnector(true);
        mpTempConnector->drawConnector();
    }
        //When clicking end port
    else
    {
        qDebug() << "clicking end port: " << pPort->getName();
        GUIPort *pStartPort = mpTempConnector->getStartPort();

        bool success = this->getCoreSystemAccessPtr()->connect(pStartPort->getGuiModelObjectName(), pStartPort->getName(), pPort->getGuiModelObjectName(), pPort->getName() );
        qDebug() << "GUI Connect: " << success;
        if (success)
        {
            setIsCreatingConnector(false);
            QPointF newPos = pPort->mapToScene(pPort->boundingRect().center());
            mpTempConnector->updateEndPoint(newPos);
            pPort->getGuiModelObject()->rememberConnector(mpTempConnector);
            mpTempConnector->setEndPort(pPort);

            //If systemport refresh graphics
            qDebug() << "Port Types: " << mpTempConnector->getStartPort()->getPortType() << " " << mpTempConnector->getEndPort()->getPortType();
            QString cqsType, portType, nodeType;
            if (mpTempConnector->getStartPort()->getPortType() == "SYSTEMPORT") //! @todo not hardcoded should be defined somewhere
            {
                cqsType = mpTempConnector->getStartPort()->getGuiModelObject()->getTypeCQS();
                portType = mpTempConnector->getStartPort()->getPortType();
                nodeType = mpTempConnector->getStartPort()->getNodeType();
                mpTempConnector->getStartPort()->refreshPortGraphics(cqsType, portType, nodeType);
            }
            if (mpTempConnector->getEndPort()->getPortType() == "SYSTEMPORT") //! @todo not hardcoded should be defined somewhere
            {
                cqsType = mpTempConnector->getEndPort()->getGuiModelObject()->getTypeCQS();
                portType = mpTempConnector->getEndPort()->getPortType();
                nodeType = mpTempConnector->getEndPort()->getNodeType();
                mpTempConnector->getEndPort()->refreshPortGraphics(cqsType, portType, nodeType);
            }

                //Hide ports; connected ports shall not be visible
            mpTempConnector->getStartPort()->hide();
            mpTempConnector->getEndPort()->hide();

            mSubConnectorList.append(mpTempConnector);

            mUndoStack->newPost();
            mpParentProjectTab->hasChanged();
            if(undoSettings == UNDO)
            {
                mUndoStack->registerAddedConnector(mpTempConnector);
            }
        }
        emit checkMessages();
     }
}

//! Copies the selected components, and then deletes them.
//! @see copySelected()
//! @see paste()
void GUIContainerObject::cutSelected(CopyStack *xmlStack)
{
    this->copySelected(xmlStack);
    this->mUndoStack->newPost("cut");
    emit deleteSelected();
    mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! Puts the selected components in the copy stack, and their positions in the copy position stack.
//! @see cutSelected()
//! @see paste()
void GUIContainerObject::copySelected(CopyStack *xmlStack)
{
    gCopyStack.clear();

    QDomElement *copyRoot;
    if(xmlStack == 0)
    {
        copyRoot = gCopyStack.getCopyRoot();
    }
    else
    {
        copyRoot = xmlStack->getCopyRoot();
    }

        //Copy components
    QList<GUIModelObject *>::iterator it;
    for(it = mSelectedGUIObjectsList.begin(); it!=mSelectedGUIObjectsList.end(); ++it)
    {
        (*it)->saveToDomElement(*copyRoot);
    }

        //Copy connectors
    for(int i = 0; i != mSubConnectorList.size(); ++i)
    {
        if(mSubConnectorList[i]->getStartPort()->getGuiModelObject()->isSelected() && mSubConnectorList[i]->getEndPort()->getGuiModelObject()->isSelected() && mSubConnectorList[i]->isActive())
        {
            mSubConnectorList[i]->saveToDomElement(*copyRoot);
        }
    }

        //Copy widgets
    //! @todo All widgets should probably use the same load/save functions. Then we could use the mSelectedWidgetList, and this would be much nicer.
    QList<GUIBoxWidget *>::iterator itbw;
    for(itbw = mBoxWidgetList.begin(); itbw!=mBoxWidgetList.end(); ++itbw)
    {
        if((*itbw)->isSelected())
        {
            (*itbw)->saveToDomElement(*copyRoot);
        }
    }
    QList<GUITextWidget *>::iterator ittw;
    for(ittw = mTextWidgetList.begin(); ittw!=mTextWidgetList.end(); ++ittw)
    {
        if((*ittw)->isSelected())
        {
            (*ittw)->saveToDomElement(*copyRoot);
        }
    }
}


//! Creates each item in the copy stack, and places it on its respective position in the position copy stack.
//! @see cutSelected()
//! @see copySelected()
void GUIContainerObject::paste(CopyStack *xmlStack)
{

    gpMainWindow->mpMessageWidget->printGUIDebugMessage(gCopyStack.getXML());

    mUndoStack->newPost("paste");
    mpParentProjectTab->hasChanged();

    QDomElement *copyRoot;
    if(xmlStack == 0)
    {
        copyRoot = gCopyStack.getCopyRoot();
    }
    else
    {
        copyRoot = xmlStack->getCopyRoot();
    }

        //Deselect all components & connectors
    emit deselectAllGUIObjects();
    emit deselectAllGUIConnectors();

    QHash<QString, QString> renameMap;       //Used to track name changes, so that connectors will know what components are called

        //Paste components
    QDomElement objectElement = copyRoot->firstChildElement("component");
    while(!objectElement.isNull())
    {
        GUIModelObject *pObj = loadGUIModelObject(objectElement, gpMainWindow->mpLibrary, this);

            //Apply offset to pasted object
        QPointF oldPos = pObj->pos();
        pObj->moveBy(mPasteOffset, mPasteOffset);
        mUndoStack->registerMovedObject(oldPos, pObj->pos(), pObj->getName());

        renameMap.insert(objectElement.attribute("name"), pObj->getName());
        objectElement.setAttribute("name", renameMap.find(objectElement.attribute("name")).value());
        objectElement = objectElement.nextSiblingElement("component");
    }

        //Paste connectors
    QDomElement connectorElement = copyRoot->firstChildElement("connect");
    while(!connectorElement.isNull())
    {
            //Replace names of start and end component, since they likely have been changed
        connectorElement.setAttribute("startcomponent", renameMap.find(connectorElement.attribute("startcomponent")).value());
        connectorElement.setAttribute("endcomponent", renameMap.find(connectorElement.attribute("endcomponent")).value());

        loadConnector(connectorElement, this, UNDO);

        GUIConnector *tempConnector = this->findConnector(connectorElement.attribute("startcomponent"), connectorElement.attribute("startport"),
                                                          connectorElement.attribute("endcomponent"), connectorElement.attribute("endport"));

            //Apply offset to connector and register it in undo stack
//        QList<QPointF> oldPosList;
//        for(size_t i=0; i<tempConnector->getNumberOfLines()-2; ++i)
//        {
//            oldPosList.append(tempConnector->getLine(i+1)->pos());
//        }
        tempConnector->moveAllPoints(mPasteOffset, mPasteOffset);
        tempConnector->drawConnector();
        for(int i=0; i<(tempConnector->getNumberOfLines()-2); ++i)
        {
            mUndoStack->registerModifiedConnector(QPointF(tempConnector->getLine(i)->pos().x()-mPasteOffset, tempConnector->getLine(i)->pos().y()-mPasteOffset),
                                                  tempConnector->getLine(i+1)->pos(), tempConnector, i+1);
        }

        connectorElement = connectorElement.nextSiblingElement("connect");
    }

        //Paste text widgets
    QDomElement textElement = copyRoot->firstChildElement("textwidget");
    while(!textElement.isNull())
    {
        loadTextWidget(textElement, this, NOUNDO);
        mTextWidgetList.last()->setSelected(true);
        mTextWidgetList.last()->moveBy(mPasteOffset, mPasteOffset);
        mUndoStack->registerAddedTextWidget(mTextWidgetList.last());
        textElement = textElement.nextSiblingElement("textwidget");
    }

        //Paste box widgets
    QDomElement boxElement = copyRoot->firstChildElement("boxwidget");
    while(!boxElement.isNull())
    {
        loadBoxWidget(boxElement, this, NOUNDO);
        mBoxWidgetList.last()->setSelected(true);
        mBoxWidgetList.last()->moveBy(mPasteOffset, mPasteOffset);
        mUndoStack->registerAddedBoxWidget(mBoxWidgetList.last());
        boxElement = boxElement.nextSiblingElement("boxwidget");
    }

        //Select all pasted comonents
    QHash<QString, QString>::iterator itn;
    for(itn = renameMap.begin(); itn != renameMap.end(); ++itn)
    {
        mGUIModelObjectMap.find(itn.value()).value()->setSelected(true);
    }

    mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! Groups the selected objects together
void GUIContainerObject::groupSelected(QPointF pt)
{
    //! @todo dont hardcode group appearance like this
    GUIModelObjectAppearance appdata;
    appdata.setIconPathUser("subsystemtmp.svg");
    appdata.setBaseIconPath("../../HopsanGUI/"); //!< @todo This is EXTREAMLY BAD

    //! @todo add like all other guimodelobjects add as proper object
    GUIGroup *pGroup = new GUIGroup(pt.toPoint(), 0.0, &appdata, this);
    this->getContainedScenePtr()->addItem(pGroup);

    CopyStack copyStack;
    this->cutSelected(&copyStack);
    //this->copySelected(&copyStack);
    //pGroup->setContents(&copyStack);
}


//! Selects all objects and connectors.
void GUIContainerObject::selectAll()
{
    emit selectAllGUIObjects();
    emit selectAllGUIConnectors();
}


//! Deselects all objects and connectors.
void GUIContainerObject::deselectAll()
{
    emit deselectAllGUIObjects();
    emit deselectAllGUIConnectors();
}


//! Hides all component names.
//! @see showNames()
void GUIContainerObject::hideNames()
{
    mUndoStack->newPost("hideallnames");
    mIsRenamingObject = false;
    emit deselectAllNameText();
    emit hideAllNameText();
}


//! Shows all component names.
//! @see hideNames()
void GUIContainerObject::showNames()
{
    mUndoStack->newPost("showallnames");
    emit showAllNameText();
}


//! Slot that sets hide ports flag to true or false
void GUIContainerObject::hidePorts(bool doIt)
{
    mPortsHidden = doIt;
}


//! Slot that tells the mUndoStack to execute one undo step. Necessary because the undo stack is not a QT object and cannot use its own slots.
//! @see redo()
//! @see clearUndo()
void GUIContainerObject::undo()
{
    mUndoStack->undoOneStep();
}


//! Slot that tells the mUndoStack to execute one redo step. Necessary because the redo stack is not a QT object and cannot use its own slots.
//! @see undo()
//! @see clearUndo()
void GUIContainerObject::redo()
{
    mUndoStack->redoOneStep();
}

//! Slot that tells the mUndoStack to clear itself. Necessary because the redo stack is not a QT object and cannot use its own slots.
//! @see undo()
//! @see redo()
void GUIContainerObject::clearUndo()
{
    mUndoStack->clear();
}


//! Returns true if at least one GUIObject is selected
bool GUIContainerObject::isObjectSelected()
{
    return (mSelectedGUIObjectsList.size() > 0);
}


//! Returns true if at least one GUIConnector is selected
bool GUIContainerObject::isConnectorSelected()
{
    return (mSelectedSubConnectorsList.size() > 0);
}

QString GUIContainerObject::getUserIconPath()
{
    return this->mGUIModelObjectAppearance.getIconPathUser();
}

//! @todo do we return full path or relative
QString GUIContainerObject::getIsoIconPath()
{
    return this->mGUIModelObjectAppearance.getIconPathISO();
}

//! @todo do we safe full path or relative
void GUIContainerObject::setUserIconPath(QString path)
{
    QFileInfo fi;
    fi.setFile(path);
    this->mGUIModelObjectAppearance.setIconPathUser(fi.fileName());
    this->mGUIModelObjectAppearance.setBaseIconPath(fi.absolutePath()+"/");
}

void GUIContainerObject::setIsoIconPath(QString path)
{
    QFileInfo fi;
    fi.setFile(path);
    this->mGUIModelObjectAppearance.setIconPathISO(fi.fileName());
    this->mGUIModelObjectAppearance.setBaseIconPath(fi.absolutePath()+"/");
}

//! Access function for mIsCreatingConnector
//! @param isConnected is the new value
void GUIContainerObject::setIsCreatingConnector(bool isCreatingConnector)
{
    mIsCreatingConnector = isCreatingConnector;
}


//! Access function for mIsCreatingConnector
bool GUIContainerObject::getIsCreatingConnector()
{
    return mIsCreatingConnector;
}


//! Disables the undo function for the current model
void GUIContainerObject::disableUndo()
{
    if(!mUndoDisabled)
    {
        QMessageBox disableUndoWarningBox(QMessageBox::Warning, tr("Warning"),tr("Disabling undo history will clear all undo history for this model. Do you want to continue?"), 0, 0);
        disableUndoWarningBox.addButton(tr("&Yes"), QMessageBox::AcceptRole);
        disableUndoWarningBox.addButton(tr("&No"), QMessageBox::RejectRole);
        disableUndoWarningBox.setWindowIcon(gpMainWindow->windowIcon());

        if (disableUndoWarningBox.exec() == QMessageBox::AcceptRole)
        {
            this->clearUndo();
            mUndoDisabled = true;
            gpMainWindow->undoAction->setDisabled(true);
            gpMainWindow->redoAction->setDisabled(true);
        }
        else
        {
            return;
        }
    }
    else
    {
        mUndoDisabled = false;
        gpMainWindow->undoAction->setDisabled(false);
        gpMainWindow->redoAction->setDisabled(false);
    }
}


//! Enables or disables the undo buttons depending on whether or not undo is disabled in current tab
void GUIContainerObject::updateUndoStatus()
{
    if(mUndoDisabled)
    {
        gpMainWindow->undoAction->setDisabled(true);
        gpMainWindow->redoAction->setDisabled(true);
    }
    else
    {
        gpMainWindow->undoAction->setDisabled(false);
        gpMainWindow->redoAction->setDisabled(false);
    }
}

//! Sets the iso graphics option for the model
void GUIContainerObject::setGfxType(graphicsType gfxType)
{
    this->mGfxType = gfxType;
    this->mpParentProjectTab->mpGraphicsView->updateViewPort();
    emit setAllGfxType(mGfxType);
}

//! @brief A slot that opens the properties dialog
void GUIContainerObject::openPropertiesDialogSlot()
{
    this->openPropertiesDialog();
}


//! Slot that tells all selected name texts to deselect themselves
void GUIContainerObject::deselectSelectedNameText()
{
    emit deselectAllNameText();
}

//! @todo Maybe should try to reduce multiple copys of same functions with other GUIObjects
void GUIContainerObject::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    QAction *loadAction = menu.addAction(tr("Load Subsystem File"));
    if(!mModelFileInfo.filePath().isEmpty())
    {
        loadAction->setDisabled(true);
    }
    QAction *pAction = this->buildBaseContextMenu(menu, event->screenPos());
    if (pAction == loadAction)
    {
        //! @todo use loadHMF once we have scraped teh text based stuff and only uses xml
        //loadFromHMF();
        QDir fileDialog;
        QFile file;
        QString modelFilePath = QFileDialog::getOpenFileName(mpParentProjectTab->mpParentProjectTabWidget, tr("Choose Subsystem File"),
                                                             fileDialog.currentPath() + QString(MODELPATH),
                                                             tr("Hopsan Model Files (*.hmf)"));

        file.setFileName(modelFilePath);
        QDomDocument domDocument;
        QDomElement hmfRoot = loadXMLDomDocument(file, domDocument, HMF_ROOTTAG);
        if (!hmfRoot.isNull())
        {
            //! @todo Check version numbers
            //! @todo check if we could load else give error message and dont attempt to load
            QDomElement systemElement = hmfRoot.firstChildElement(HMF_SYSTEMTAG);
            this->setModelFileInfo(file); //Remember info about the file from which the data was loaded
            this->loadFromDomElement(systemElement);
        }
    }

    //Dont call GUIModelObject::contextMenuEvent as that will open an other menu after this one is closed
    //GUIModelObject::contextMenuEvent(event);
    ////QGraphicsItem::contextMenuEvent(event);
}

void GUIContainerObject::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    GUIModelObject::mouseDoubleClickEvent(event);
    this->enterContainer();
}

void GUIContainerObject::openPropertiesDialog()
{
    //! @todo maybe move code from system here
    //Do Nothing
}


void GUIContainerObject::enterContainer()
{
    //First deselect everything so that buttons pressed in the view are not sent to obejcts in the previous container
    //this->deselectAll(); //! @todo maybe this should be a signal
    //! @todo there is apperantly a deselect all guiwidgets also that is not in deselect all, has nothing to do with this code though
    //this->deselect();
    mpParentContainerObject->deselectAll(); //deselect myself and anyone else

    //Show this scene
    mpParentProjectTab->mpGraphicsView->setScene(getContainedScenePtr());
    mpParentProjectTab->mpGraphicsView->setContainerPtr(this);
    mpParentProjectTab->mpQuickNavigationWidget->addOpenContainer(this);

        //Disconnect parent system and connect new system with actions
    disconnect(gpMainWindow->hideNamesAction,      SIGNAL(triggered()),        mpParentContainerObject,     SLOT(hideNames()));
    disconnect(gpMainWindow->showNamesAction,      SIGNAL(triggered()),        mpParentContainerObject,     SLOT(showNames()));
    disconnect(gpMainWindow->disableUndoAction,    SIGNAL(triggered()),        mpParentContainerObject,     SLOT(disableUndo()));
    disconnect(gpMainWindow->cutAction,            SIGNAL(triggered()),        mpParentContainerObject,     SLOT(cutSelected()));
    disconnect(gpMainWindow->copyAction,           SIGNAL(triggered()),        mpParentContainerObject,     SLOT(copySelected()));
    disconnect(gpMainWindow->pasteAction,          SIGNAL(triggered()),        mpParentContainerObject,     SLOT(paste()));
    disconnect(gpMainWindow->propertiesAction,     SIGNAL(triggered()),        mpParentContainerObject,     SLOT(openPropertiesDialogSlot()));
    disconnect(gpMainWindow->undoAction,           SIGNAL(triggered()),        mpParentContainerObject,     SLOT(undo()));
    disconnect(gpMainWindow->redoAction,           SIGNAL(triggered()),        mpParentContainerObject,     SLOT(redo()));

    connect(gpMainWindow->hideNamesAction,      SIGNAL(triggered()),        this,     SLOT(hideNames()));
    connect(gpMainWindow->showNamesAction,      SIGNAL(triggered()),        this,     SLOT(showNames()));
    connect(gpMainWindow->disableUndoAction,    SIGNAL(triggered()),        this,     SLOT(disableUndo()));
    connect(gpMainWindow->cutAction,            SIGNAL(triggered()),        this,     SLOT(cutSelected()));
    connect(gpMainWindow->copyAction,           SIGNAL(triggered()),        this,     SLOT(copySelected()));
    connect(gpMainWindow->pasteAction,          SIGNAL(triggered()),        this,     SLOT(paste()));
    connect(gpMainWindow->propertiesAction,     SIGNAL(triggered()),        this,     SLOT(openPropertiesDialogSlot()));
    connect(gpMainWindow->undoAction,           SIGNAL(triggered()),        this,     SLOT(undo()));
    connect(gpMainWindow->redoAction,           SIGNAL(triggered()),        this,     SLOT(redo()));

        //Upddate plot widget and undo widget to new container
    gpMainWindow->makeSurePlotWidgetIsCreated();
    gpMainWindow->mpPlotWidget->mpPlotParameterTree->updateList();
    gpMainWindow->mpUndoWidget->refreshList();
    gpMainWindow->undoAction->setDisabled(this->mUndoDisabled);
    gpMainWindow->redoAction->setDisabled(this->mUndoDisabled);
}


void GUIContainerObject::exitContainer()
{
    this->deselectAll();
    //Go back to parent system
    mpParentProjectTab->mpGraphicsView->setScene(this->mpParentContainerObject->getContainedScenePtr());
    mpParentProjectTab->mpGraphicsView->setContainerPtr(this->mpParentContainerObject);

        //Disconnect this system and connect parent system with undo and redo actions
    disconnect(gpMainWindow->hideNamesAction,      SIGNAL(triggered()),        this,     SLOT(hideNames()));
    disconnect(gpMainWindow->showNamesAction,      SIGNAL(triggered()),        this,     SLOT(showNames()));
    disconnect(gpMainWindow->disableUndoAction,    SIGNAL(triggered()),        this,     SLOT(disableUndo()));
    disconnect(gpMainWindow->cutAction,            SIGNAL(triggered()),        this,     SLOT(cutSelected()));
    disconnect(gpMainWindow->copyAction,           SIGNAL(triggered()),        this,     SLOT(copySelected()));
    disconnect(gpMainWindow->pasteAction,          SIGNAL(triggered()),        this,     SLOT(paste()));
    disconnect(gpMainWindow->propertiesAction,     SIGNAL(triggered()),        this,     SLOT(openPropertiesDialogSlot()));
    disconnect(gpMainWindow->undoAction,           SIGNAL(triggered()),        this,     SLOT(undo()));
    disconnect(gpMainWindow->redoAction,           SIGNAL(triggered()),        this,     SLOT(redo()));

    connect(gpMainWindow->hideNamesAction,      SIGNAL(triggered()),        mpParentContainerObject,     SLOT(hideNames()));
    connect(gpMainWindow->showNamesAction,      SIGNAL(triggered()),        mpParentContainerObject,     SLOT(showNames()));
    connect(gpMainWindow->disableUndoAction,    SIGNAL(triggered()),        mpParentContainerObject,     SLOT(disableUndo()));
    connect(gpMainWindow->cutAction,            SIGNAL(triggered()),        mpParentContainerObject,     SLOT(cutSelected()));
    connect(gpMainWindow->copyAction,           SIGNAL(triggered()),        mpParentContainerObject,     SLOT(copySelected()));
    connect(gpMainWindow->pasteAction,          SIGNAL(triggered()),        mpParentContainerObject,     SLOT(paste()));
    connect(gpMainWindow->propertiesAction,     SIGNAL(triggered()),        mpParentContainerObject,     SLOT(openPropertiesDialogSlot()));
    connect(gpMainWindow->undoAction,           SIGNAL(triggered()),        mpParentContainerObject,     SLOT(undo()));
    connect(gpMainWindow->redoAction,           SIGNAL(triggered()),        mpParentContainerObject,     SLOT(redo()));

        //Update plot widget and undo widget to new container
    gpMainWindow->makeSurePlotWidgetIsCreated();
    gpMainWindow->mpPlotWidget->mpPlotParameterTree->updateList();
    gpMainWindow->mpUndoWidget->refreshList();
    gpMainWindow->undoAction->setDisabled(mpParentContainerObject->mUndoDisabled);
    gpMainWindow->redoAction->setDisabled(mpParentContainerObject->mUndoDisabled);

        //Refresh external port appearance
    //! @todo We only need to do this if ports have change, right now we always refresh, dont know if this is a big deal
    this->refreshExternalPortsAppearanceAndPosition();
}
