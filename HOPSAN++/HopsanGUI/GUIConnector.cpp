//$Id$

#include "GUIConnector.h"
#include <QDebug>
#include "GUIPort.h"
#include <vector>
#include <QStyleOptionGraphicsItem>
#include <math.h>

//! Constructor.
//! @param startpos defines the start position of the connector, normally the center of the starting port.
//! @param primaryPen defines the default width and color of the line.
//! @param activePen defines the width and color of the line when it is selected.
//! @param hoverPen defines the width and color of the line when hovered by the mouse cursor.
//! @param *parentView is a pointer to the GraphicsView the connector belongs to.
//! @param parent is the parent of the port.
GUIConnector::GUIConnector(QPointF startpos, QPen primaryPen, QPen activePen, QPen hoverPen, GraphicsView *parentView, QGraphicsItem *parent)
        : QGraphicsWidget(parent)
{
    this->mpParentView = parentView;

    setFlags(QGraphicsItem::ItemIsFocusable);

    this->setPos(startpos);

    this->updateStartPoint(startpos);

    this->mPrimaryPen = primaryPen;
    this->mActivePen = activePen;
    this->mHoverPen = hoverPen;

    this->mEndPortConnected = false;

    this->drawConnector();

    this->mMakingDiagonal = false;
}


//! Constructor used to create a whole connector at once. Used when loading models.
//! @param *startPort is a pointer to the start port.
//! @param *endPort is a pointer to the end port.
//! @param points is the point vector for the connector.
//! @param primaryPen defines the default width and color of the line.
//! @param activePen defines the width and color of the line when it is selected.
//! @param hoverPen defines the width and color of the line when hovered by the mouse cursor.
//! @param *parentView is a pointer to the GraphicsView the connector belongs to.
//! @param parent is the parent of the port.
GUIConnector::GUIConnector(GUIPort *startPort, GUIPort *endPort, std::vector<QPointF> points, QPen primaryPen, QPen activePen, QPen hoverPen, GraphicsView *parentView, QGraphicsItem *parent)
{
    this->mpParentView = parentView;
    setFlags(QGraphicsItem::ItemIsFocusable);
    mpStartPort = startPort;
    mpEndPort = endPort;
    QPointF startPos = getStartPort()->mapToScene(getStartPort()->boundingRect().center());
    this->setPos(startPos);

        //Set pen styles
    this->mPrimaryPen = primaryPen;
    this->mActivePen = activePen;
    this->mHoverPen = hoverPen;
    mPoints = points;

        //Setup the geometries vector based on the point geometry
    for(std::size_t i=0; i != mPoints.size()-1; ++i)
    {
        if(mPoints[i].x() == mPoints[i+1].x())
            mGeometries.push_back(GUIConnector::HORIZONTAL);
        else if(mPoints[i].y() == mPoints[i+1].y())
            mGeometries.push_back(GUIConnector::VERTICAL);
        else
            mGeometries.push_back(GUIConnector::DIAGONAL);
    }

    mEndPortConnected = true;
    emit endPortConnected();
    this->setPassive();
    connect(this->mpEndPort->getComponent(),SIGNAL(componentDeleted()),this,SLOT(deleteMe()));

        //Create the lines, so that drawConnector has something to work with
    for(std::size_t i = 0; i != mPoints.size()-1; ++i)
    {
        mpTempLine = new GUIConnectorLine(mapFromScene(mPoints[i]).x(), mapFromScene(mPoints[i]).y(),
                                          mapFromScene(mPoints[i+1]).x(), mapFromScene(mPoints[i+1]).y(),
                                          primaryPen, activePen, hoverPen, i, this);
        qDebug() << "Creating line from " << mPoints[i].x() << ", " << mPoints[i].y() << " to " << mPoints[i+1].x() << " " << mPoints[i+1].y();
        mpLines.push_back(mpTempLine);
        mpTempLine->setConnected();
        mpTempLine->setPassive();
        connect(mpTempLine,SIGNAL(lineSelected(bool, int)),this,SLOT(doSelect(bool, int)));
        connect(mpTempLine,SIGNAL(lineMoved(int)),this, SLOT(updateLine(int)));
        connect(mpTempLine,SIGNAL(lineHoverEnter()),this,SLOT(setHovered()));
        connect(mpTempLine,SIGNAL(lineHoverLeave()),this,SLOT(setUnHovered()));
        connect(this,SIGNAL(endPortConnected()),mpTempLine,SLOT(setConnected()));
    }

    this->drawConnector();

        //Make all lines selectable and all lines except first and last movable
    for(std::size_t i=1; i!=mpLines.size()-1; ++i)
        mpLines[i]->setFlag(QGraphicsItem::ItemIsMovable, true);
    for(std::size_t i=0; i!=mpLines.size(); ++i)
        mpLines[i]->setFlag(QGraphicsItem::ItemIsSelectable, true);

      //Add arrow to the connector if it is of signal type
    if(mpEndPort->getPortType() == Port::READPORT && mpEndPort->mpCorePort->getNodeType() == "NodeSignal")
        this->getLastLine()->addEndArrow();
    else if(mpEndPort->getPortType() == Port::WRITEPORT && mpEndPort->mpCorePort->getNodeType() == "NodeSignal")
        this->mpLines[0]->addStartArrow();

    mpStartPort->getComponent()->addConnector(this);
    mpEndPort->getComponent()->addConnector(this);
}


//! Destructor.
GUIConnector::~GUIConnector()
{
    mpLines.clear();
    //! @todo more cleanup
}


//! Inserts a new point to the connector and adjusts the previous point accordingly, depending on the geometry vector.
//! @param point is the position where the point shall be inserted, normally the cursor position.
//! @see removePoint(bool deleteIfEmpty)
void GUIConnector::addPoint(QPointF point)
{
    //point = this->mapFromScene(point);
    mPoints.push_back(point);

    if(getNumberOfLines() == 0 && getStartPort()->getPortDirection() == GUIPort::VERTICAL)
    {
        mGeometries.push_back(GUIConnector::HORIZONTAL);
        qDebug() << "Setting first connector to HORIZONTAL";
    }
    else if(getNumberOfLines() == 0 && getStartPort()->getPortDirection() == GUIPort::HORIZONTAL)
    {
        mGeometries.push_back(GUIConnector::VERTICAL);
        qDebug() << "Setting first connector to VERTICAL";
    }
    else if(getNumberOfLines() != 0 && mGeometries.back() == GUIConnector::HORIZONTAL)
    {
        mGeometries.push_back(GUIConnector::VERTICAL);
        qDebug() << "Last was HORIZONTAL, setting to VERTICAL";
    }
    else if(getNumberOfLines() != 0 && mGeometries.back() == GUIConnector::VERTICAL)
    {
        mGeometries.push_back(GUIConnector::HORIZONTAL);
        qDebug() << "Last was VERTICAL, setting to HORIZONTAL";
    }
    else if(getNumberOfLines() != 0 && mGeometries.back() == GUIConnector::DIAGONAL)
    {
        mGeometries.push_back(GUIConnector::DIAGONAL);
        //Give new line correct angle!
    }
    if(mPoints.size() > 1)
        drawConnector();
}


//! Removes the last point in the connecetor. Asks to delete the connector if deleteIfEmpty is true and if no lines or only one non-diagonal lines remains.
//! @param deleteIfEmpty tells whether or not the connector shall be deleted if too few points remains.
//! @see addPoint(QPointF point)
void GUIConnector::removePoint(bool deleteIfEmpty)
{
    qDebug() << "mPoints.size() = " << mPoints.size();
    mPoints.pop_back();
    mGeometries.pop_back();

    if(mPoints.size() == 2 and !mMakingDiagonal)
    {
        mPoints.pop_back();
        mGeometries.pop_back();
    }
    drawConnector();
    if(mPoints.size() == 1 && deleteIfEmpty)
    {
        deleteMe();
    }
}


//! Sets the pointer to the start port of a connector.
//! @param *port is the pointer to the new start port.
//! @see setEndPort(GUIPort *port)
//! @see getStartPort()
//! @see getEndPort()
void GUIConnector::setStartPort(GUIPort *port)
{
    this->mpStartPort = port;
    connect(this->mpStartPort->getComponent(),SIGNAL(componentDeleted()),this,SLOT(deleteMe()));
}


//! Sets the pointer to the end port of a connector, and executes the final tasks before creation of the connetor is complete. Then flags that the end port is connected.
//! @param *port is the pointer to the new end port.
//! @see setStartPort(GUIPort *port)
//! @see getStartPort()
//! @see getEndPort()
void GUIConnector::setEndPort(GUIPort *port)
{
    this->mEndPortConnected = true;
    this->mpEndPort = port;
    this->removePoint();        //Remove the extra point that is created by the mouse click event when clicking on the port
    this->updateEndPoint(port->mapToScene(port->boundingRect().center()));
    connect(this->mpEndPort->getComponent(),SIGNAL(componentDeleted()),this,SLOT(deleteMe()));

        //Make all lines selectable and all lines except first and last movable
    if(mpLines.size() > 1)
    {
        for(std::size_t i=1; i!=mpLines.size()-1; ++i)
            mpLines[i]->setFlag(QGraphicsItem::ItemIsMovable, true);
    }
    for(std::size_t i=0; i!=mpLines.size(); ++i)
        mpLines[i]->setFlag(QGraphicsItem::ItemIsSelectable, true);

        //Add arrow to the connector if it is of signal type
    if(port->getPortType() == Port::READPORT && port->mpCorePort->getNodeType() == "NodeSignal")
        this->getLastLine()->addEndArrow();
    else if(port->getPortType() == Port::WRITEPORT && port->mpCorePort->getNodeType() == "NodeSignal")
        this->mpLines[0]->addStartArrow();

    emit endPortConnected();
    this->setPassive();
}


//! Cycles all lines and gives them the specified pen styles.
//! @param primaryPen defines the default width and color of the line.
//! @param activePen defines the width and color of the line when it is selected.
//! @param hoverPen defines the width and color of the line when hovered by the mouse cursor.
void GUIConnector::setPens(QPen primaryPen, QPen activePen, QPen hoverPen)
{
    for (std::size_t i=0; i!=mpLines.size(); ++i )
    {
        mpLines[i]->setPens(primaryPen, activePen, hoverPen);
    }
}


//! Returns the number of lines in a connector.
int GUIConnector::getNumberOfLines()
{
    return mpLines.size();
}


//! Returns the geometry type of the specified line.
//! @param lineNumber is the number of the specified line in the mpLines vector.
GUIConnector::geometryType GUIConnector::getGeometry(int lineNumber)
{
    return mGeometries[lineNumber];
}


//! Returns the point vector used by the connector.
std::vector<QPointF> GUIConnector::getPointsVector()
{
    return mPoints;
}


//! Returns a pointer to the start port of a connector.
//! @see setStartPort(GUIPort *port)
//! @see setEndPort(GUIPort *port)
//! @see getEndPort()
GUIPort *GUIConnector::getStartPort()
{
    return this->mpStartPort;
}


//! Returns a pointer to the end port of a connector.
//! @see setStartPort(GUIPort *port)
//! @see setEndPort(GUIPort *port)
//! @see getStartPort()
GUIPort *GUIConnector::getEndPort()
{
    return this->mpEndPort;
}


//! Returns the line with specified number.
//! @param line is the number of the wanted line.
//! @see getThirdLastLine()
//! @see getSecondLastLine()
//! @see getLastLine()
GUIConnectorLine *GUIConnector::getLine(int line)
{
    return mpLines[line];
}


//! Returns the last line of the connector.
//! @see getThirdLastLine()
//! @see getSecondLastLine()
//! @see getLine(int line)
GUIConnectorLine *GUIConnector::getLastLine()
{
    return mpLines[mpLines.size()-1];
}


//! Returns the second last line of the connector.
//! @see getThirdLastLine()
//! @see getLastLine()
//! @see getLine(int line)
GUIConnectorLine *GUIConnector::getSecondLastLine()
{
    return mpLines[mpLines.size()-2];
}


//! Returns the third last line of the connector.
//! @see getSecondLastLine()
//! @see getLastLine()
//! @see getLine(int line)
GUIConnectorLine *GUIConnector::getThirdLastLine()
{
    return mpLines[mpLines.size()-3];
}


//! Returns true if the connector is connected at both ends, otherwise false.
bool GUIConnector::isConnected()
{
    return mEndPortConnected;
}


//! Returns true if the line currently being drawn is a diagonal one, otherwise false.
//! @see makeDiagonal(bool enable)
bool GUIConnector::isMakingDiagonal()
{
    return mMakingDiagonal;
}


//! Draws lines between the points in the mPoints vector, and stores them in the mpLines vector.
void GUIConnector::drawConnector()
{
    if(!mEndPortConnected)
    {
        //End port is not connected, which means we are creating a new line
        //! @todo Make this smarter, so that lines that are not changed are not removed and then re-added

            //Remove all lines
        while(!mpLines.empty())
        {
            this->scene()->removeItem(mpLines.back());
            mpLines.pop_back();
        }
        mpLines.clear();

            //Create new lines from the mPoints vector
        if(mPoints.size() > 1)
        {
            for(std::size_t i = 0; i != mPoints.size()-1; ++i)
            {
                mpTempLine = new GUIConnectorLine(mapFromScene(mPoints[i]).x(), mapFromScene(mPoints[i]).y(),
                                                  mapFromScene(mPoints[i+1]).x(), mapFromScene(mPoints[i+1]).y(),
                                                  mPrimaryPen, mActivePen, mHoverPen, mpLines.size(), this);
                mpTempLine->setPassive();
                connect(mpTempLine,SIGNAL(lineSelected(bool, int)),this,SLOT(doSelect(bool, int)));
                connect(mpTempLine,SIGNAL(lineMoved(int)),this, SLOT(updateLine(int)));
                connect(mpTempLine,SIGNAL(lineHoverEnter()),this,SLOT(setHovered()));
                connect(mpTempLine,SIGNAL(lineHoverLeave()),this,SLOT(setUnHovered()));
                connect(this,SIGNAL(endPortConnected()),mpTempLine,SLOT(setConnected()));
                mpLines.push_back(mpTempLine);
            }
        }
        if(!mEndPortConnected && mpLines.size() > 1)
        {
            mpLines.back()->setActive();
            mpLines[mpLines.size()-2]->setPassive();
        }
    }
    else
    {
            //Retrieve start and end points from ports in case components have moved
        updateStartPoint(getStartPort()->mapToScene(getStartPort()->boundingRect().center()));
        updateEndPoint(getEndPort()->mapToScene(getEndPort()->boundingRect().center()));

            //Redraw the lines based on the mPoints vector
        for(std::size_t i = 0; i != mPoints.size()-1; ++i)
        {
            mpLines[i]->setLine(mapFromScene(mPoints[i]), mapFromScene(mPoints[i+1]));
        }
    }

        //Remove the extra lines if there are too many
    while(mPoints.size() < mpLines.size()+1)
    {
        delete(mpLines.back());
        mpLines.pop_back();
        this->scene()->update();
    }
}


//! Updates the first point of the connector, and adjusts the second point accordingly depending on the geometry vector.
//! @param point is the new start point.
//! @see updateEndPoint(QPointF point)
void GUIConnector::updateStartPoint(QPointF point)
{
    if(mPoints.size() == 0)
        mPoints.push_back(point);
    else
        mPoints[0] = point;

    if(mPoints.size() != 1)
    {
        if(mGeometries[0] == GUIConnector::HORIZONTAL)
            mPoints[1] = QPointF(mPoints[0].x(),mPoints[1].y());
        else if(mGeometries[0] == GUIConnector::VERTICAL)
            mPoints[1] = QPointF(mPoints[1].x(),mPoints[0].y());
    }
}


//! Updates the last point of the connector, and adjusts the second last point accordingly depending on the geometry vector.
//! @param point is the new start point.
//! @see updateEndPoint(QPointF point)
void GUIConnector::updateEndPoint(QPointF point)
{
    mPoints.back() = point;
    if(mGeometries.back() == GUIConnector::HORIZONTAL)
    {
        mPoints[mPoints.size()-2] = QPointF(point.x(),mPoints[mPoints.size()-2].y());
    }
    else if(mGeometries.back() == GUIConnector::VERTICAL)
    {
        mPoints[mPoints.size()-2] = QPointF(mPoints[mPoints.size()-2].x(),point.y());
    }
}


//! Updates the mPoints vector when a line has been moved. Used to make lines follow each other when they are moved, and to make sure horizontal lines can only move vertically and vice versa.
//! @param lineNumber is the number of the line that has moved.
void GUIConnector::updateLine(int lineNumber)
{
    qDebug() << "updateLine(), lineNumber = " << lineNumber;
   if ((mEndPortConnected) && (lineNumber != 0) && (lineNumber != mpLines.size()))
    {
        if(mGeometries[lineNumber] == GUIConnector::HORIZONTAL)
        {
            mPoints[lineNumber] = QPointF(getLine(lineNumber)->mapToScene(getLine(lineNumber)->line().p1()).x(), mPoints[lineNumber].y());
            mPoints[lineNumber+1] = QPointF(getLine(lineNumber)->mapToScene(getLine(lineNumber)->line().p2()).x(), mPoints[lineNumber+1].y());
        }
        else if (mGeometries[lineNumber] == GUIConnector::VERTICAL)
        {
            mPoints[lineNumber] = QPointF(mPoints[lineNumber].x(), getLine(lineNumber)->mapToScene(getLine(lineNumber)->line().p1()).y());
            mPoints[lineNumber+1] = QPointF(mPoints[lineNumber+1].x(), getLine(lineNumber)->mapToScene(getLine(lineNumber)->line().p2()).y());
        }
    }

    drawConnector();
}


//! Tells the connector to create one diagonal lines instead of the last two horizontal/vertical, or to return to horizontal/diagonal mode.
//! @param enable indicates whether diagonal mode shall be enabled or disabled.
//! @see isMakingDiagonal()
void GUIConnector::makeDiagonal(bool enable)
{
    QCursor cursor;
    if(enable)
    {
        mMakingDiagonal = true;
        removePoint();
        mGeometries.back() = GUIConnector::DIAGONAL;
        mPoints.back() = mpParentView->mapToScene(mpParentView->mapFromGlobal(cursor.pos()));
        drawConnector();
    }
    else
    {
        if(mGeometries[mGeometries.size()-2] == GUIConnector::HORIZONTAL)
        {
            mGeometries.back() = GUIConnector::VERTICAL;
            mPoints.back() = QPointF(mPoints.back().x(), mPoints[mPoints.size()-2].y());
        }
        else if(mGeometries[mGeometries.size()-2] == GUIConnector::VERTICAL)
        {
            mGeometries.back() = GUIConnector::HORIZONTAL;
            mPoints.back() = QPointF(mPoints[mPoints.size()-2].x(), mPoints.back().y());
        }
        else if(mGeometries[mGeometries.size()-2] == GUIConnector::DIAGONAL)
        {
            if(abs(mPoints[mPoints.size()-2].x() - mPoints[mPoints.size()-3].x()) > abs(mPoints[mPoints.size()-2].y() - mPoints[mPoints.size()-3].y()))
                mGeometries.back() = GUIConnector::HORIZONTAL;
            else
                mGeometries.back() = GUIConnector::VERTICAL;
            mPoints.back() = QPointF(mPoints[mPoints.size()-2].x(), mPoints.back().y());
        }
        addPoint(mpParentView->mapToScene(mpParentView->mapFromGlobal(cursor.pos())));
        drawConnector();
        mMakingDiagonal = false;
    }
}


//! Slot that activates or deactivates the connector if one of its lines is selected or deselected.
//! @param lineSelected tells whether the signal was induced by selection or deselection of a line.
//! @see setActive()
//! @see setPassive()
void GUIConnector::doSelect(bool lineSelected, int lineNumber)
{
    if(this->mEndPortConnected)     //Non-finished lines shall not be selectable
    {
        if(lineSelected)
        {
            this->setActive();
            for (std::size_t i=0; i != mpLines.size(); ++i)
            {
               if(i != (std::size_t)lineNumber)
                   mpLines[i]->setSelected(false);
            }
        }
        else
        {
        bool noneSelected = true;
            for (std::size_t i=0; i != mpLines.size(); ++i)
            {
               if(mpLines[i]->isSelected())
                   noneSelected = false;
            }
            if(noneSelected)
                this->setPassive();
       }
    }
}


//! Activates a connector, activates each line and connects delete function with delete key.
//! @see setPassive()
void GUIConnector::setActive()
{
    connect(this->mpParentView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
    if(this->mEndPortConnected)
    {
        mIsActive = true;
        for (std::size_t i=0; i!=mpLines.size(); ++i )
        {
            mpLines[i]->setActive();
            //mpLines[i]->setSelected(true);         //???
        }
    }
}


//! Deactivates a connector, deactivates each line and disconnects delete function with delete key.
//! @see setActive()
void GUIConnector::setPassive()
{
    disconnect(this->mpParentView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
    if(this->mEndPortConnected)
    {
        mIsActive = false;
        for (std::size_t i=0; i!=mpLines.size(); ++i )
        {
            mpLines[i]->setPassive();
            mpLines[i]->setSelected(false);       //OBS! Kanske inte blir bra...
        }
    }
}


//! Changes connector style to hovered if it is not active. Used when mouse starts hovering a line.
//! @see setUnHovered()
void GUIConnector::setHovered()
{
    if(this->mEndPortConnected && !this->mIsActive)
    {
        for (std::size_t i=0; i!=mpLines.size(); ++i )
        {
            mpLines[i]->setHovered();
        }
    }
}


//! Changes connector style back to normal if it is not active. Used when mouse stops hovering a line.
//! @see setHovered()
//! @see setPassive()
void GUIConnector::setUnHovered()
{
    if(this->mEndPortConnected && !this->mIsActive)
    {
        for (std::size_t i=0; i!=mpLines.size(); ++i )
        {
            mpLines[i]->setPassive();
        }
    }
}


//! Asks my parent to delete myself
void GUIConnector::deleteMe()
{
    mpParentView->removeConnector(this);
}


//------------------------------------------------------------------------------------------------------------------------//


//! Constructor.
//! @param x1 is the x-coordinate of the start position of the line.
//! @param y1 is the y-coordinate of the start position of the line.
//! @param x2 is the x-coordinate of the end position of the line.
//! @param y2 is the y-coordinate of the end position of the line.
//! @param primaryPen defines the default color and width of the line.
//! @param activePen defines the color and width of the line when it is selected.
//! @param hoverPen defines the color and width of the line when it is hovered by the mouse cursor.
//! @param lineNumber is the number of the line in the connector.
//! @param *parent is a pointer to the parent object.
GUIConnectorLine::GUIConnectorLine(qreal x1, qreal y1, qreal x2, qreal y2, QPen primaryPen, QPen activePen, QPen hoverPen, int lineNumber, GUIConnector *parent)
        : QGraphicsLineItem(x1,y1,x2,y2,parent)
{
    mpParentGUIConnector = parent;
    setFlags(QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption);
    this->mPrimaryPen = primaryPen;
    this->mActivePen = activePen;
    this->mHoverPen = hoverPen;
    this->mLineNumber = lineNumber;
    this->setAcceptHoverEvents(true);
    this->mParentConnectorEndPortConnected = false;
    this->startPos = QPointF(x1,y1);
    this->endPos = QPointF(x2,y2);
    //this->mpParentGUIConnector->mGeometries.push_back(GUIConnector::HORIZONTAL);
    this->mHasStartArrow = false;
    this->mHasEndArrow = false;
    this->mArrowSize = 10.0;
    this->mArrowAngle = 0.6;
}


//! Destructor
GUIConnectorLine::~GUIConnectorLine()
{
}


//! Reimplementation of paint function. Removes the ugly dotted selection box.
void GUIConnectorLine::paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w)
{
    QStyleOptionGraphicsItem *_o = const_cast<QStyleOptionGraphicsItem*>(o);
    _o->state &= ~QStyle::State_Selected;
    QGraphicsLineItem::paint(p,_o,w);
}


//! Changes the style of the line to active.
//! @see setPassive()
//! @see setHovered()
void GUIConnectorLine::setActive()
{
        this->setPen(mActivePen);
}


//! Changes the style of the line to default.
//! @see setActive()
//! @see setHovered()
void GUIConnectorLine::setPassive()
{
        this->setPen(mPrimaryPen);
}


//! Changes the style of the line to hovered.
//! @see setActive()
//! @see setPassive()
void GUIConnectorLine::setHovered()
{
        this->setPen(mHoverPen);
}


//! Defines what shall happen if the line is clicked.
//! @todo Check if this is really needed. It is not used anywhere.
void GUIConnectorLine::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit lineClicked();
    QGraphicsLineItem::mousePressEvent(event);
}


//! Devines what shall happen if the mouse cursor enters the line. Change cursor if the line is movable.
//! @see hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
void GUIConnectorLine::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if(this->flags().testFlag((QGraphicsItem::ItemIsMovable)))
    {
        if(this->mParentConnectorEndPortConnected && this->mpParentGUIConnector->getGeometry(getLineNumber()) == GUIConnector::VERTICAL)
        {
            this->setCursor(Qt::SizeVerCursor);
        }
        else if(this->mParentConnectorEndPortConnected && this->mpParentGUIConnector->getGeometry(getLineNumber()) == GUIConnector::HORIZONTAL)
        {
            this->setCursor(Qt::SizeHorCursor);
        }
    }
    emit lineHoverEnter();
}


//! Defines what shall happen when mouse cursor leaves the line.
//! @see hoverEnterEvent(QGraphicsSceneHoverEvent *event)
void GUIConnectorLine::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    emit lineHoverLeave();
}



//! Returns the number of the line in the connector.
int GUIConnectorLine::getLineNumber()
{
    return mLineNumber;
}


//! Defines what shall happen if the line is selected or moved.
QVariant GUIConnectorLine::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
         emit lineSelected(this->isSelected(), this->mLineNumber);
    }
    if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        emit lineMoved(this->mLineNumber);
    }
    return value;
}


//! Tells the line that its parent connector has been connected at both ends
void GUIConnectorLine::setConnected()
{
    mParentConnectorEndPortConnected = true;
}


//! Reimplementation of setLine; stores the start and end positions before changing them
//! @param x1 is the x-coordinate of the start position.
//! @param y1 is the y-coordinate of the start position.
//! @param x2 is the x-coordinate of the end position.
//! @param y2 is the y-coordinate of the end position.
void GUIConnectorLine::setLine(QPointF pos1, QPointF pos2)
{
    this->startPos = this->mapFromParent(pos1);
    this->endPos = this->mapFromParent(pos2);
    if(this->mHasEndArrow)
    {
        delete(this->mArrowLine1);
        delete(this->mArrowLine2);
        this->addEndArrow();
    }
    else if(this->mHasStartArrow)
    {
        delete(this->mArrowLine1);
        delete(this->mArrowLine2);
        this->addStartArrow();
    }
    QGraphicsLineItem::setLine(this->mapFromParent(pos1).x(),this->mapFromParent(pos1).y(),
                               this->mapFromParent(pos2).x(),this->mapFromParent(pos2).y());
}


//! Adds an arrow at the end of the line.
//! @see addStartArrow()
void GUIConnectorLine::addEndArrow()
{
    qreal angle = atan2((this->endPos.y()-this->startPos.y()), (this->endPos.x()-this->startPos.x()));
    mArrowLine1 = new QGraphicsLineItem(this->endPos.x(),
                                        this->endPos.y(),
                                        this->endPos.x()-mArrowSize*cos(angle+mArrowAngle),
                                        this->endPos.y()-mArrowSize*sin(angle+mArrowAngle), this);
    mArrowLine2 = new QGraphicsLineItem(this->endPos.x(),
                                        this->endPos.y(),
                                        this->endPos.x()-mArrowSize*cos(angle-mArrowAngle),
                                        this->endPos.y()-mArrowSize*sin(angle-mArrowAngle), this);
    this->setPen(this->pen());
    this->mHasEndArrow = true;
}


//! Adds an arrow at the start of the line.
//! @see addEndArrow()
void GUIConnectorLine::addStartArrow()
{
    qreal angle = atan2((this->endPos.y()-this->startPos.y()), (this->endPos.x()-this->startPos.x()));
    mArrowLine1 = new QGraphicsLineItem(this->startPos.x(),
                                        this->startPos.y(),
                                        this->startPos.x()+mArrowSize*cos(angle+mArrowAngle),
                                        this->startPos.y()+mArrowSize*sin(angle+mArrowAngle), this);
    mArrowLine2 = new QGraphicsLineItem(this->startPos.x(),
                                        this->startPos.y(),
                                        this->startPos.x()+mArrowSize*cos(angle-mArrowAngle),
                                        this->startPos.y()+mArrowSize*sin(angle-mArrowAngle), this);
    this->setPen(this->pen());
    this->mHasStartArrow = true;
}


//! Reimplementation of inherited setPen function to include arrow pen too.
void GUIConnectorLine::setPen (const QPen &pen)
{
    QGraphicsLineItem::setPen(pen);
    if(this->mHasStartArrow | this->mHasEndArrow)       //Update arrow lines two, but ignore dashes
    {
        QPen tempPen = this->pen();
        tempPen = QPen(tempPen.color(), tempPen.width(), Qt::SolidLine);
        mArrowLine1->setPen(tempPen);
        mArrowLine2->setPen(tempPen);
        mArrowLine1->line();
    }
}


//! Set function for all three pen styles (primary, active and hover).
void GUIConnectorLine::setPens(QPen primaryPen, QPen activePen, QPen hoverPen)
{
    mPrimaryPen = primaryPen;
    mActivePen = activePen;
    mHoverPen = hoverPen;
    this->setPassive();
}
