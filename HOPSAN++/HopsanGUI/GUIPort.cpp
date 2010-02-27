//$Id$

#include "HopsanCore.h"
#include "GUIPort.h"
#include "plotwidget.h"
#include <QObject>
#include <QGraphicsObject>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QTabWidget>
#include <QStringList>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QWidget>
#include <QGraphicsItem>
#include "GUIComponent.h"
#include <iostream>
#include <QDebug>

#include <QtGui>

GUIPort::GUIPort(Port *kernelPort, qreal x, qreal y, qreal width, qreal height, QGraphicsView *parentView, GUIComponent *component, QGraphicsItem *parent)
        : QGraphicsRectItem(x, y, width, height,parent)
{
    //Core interaction
    mpKernelPort = kernelPort;
    //

    mParentView = parentView;
    mComponent = component;
    rectPos.setX(x);
    rectPos.setY(y);
    pRectParent = parent;
    this->setAcceptHoverEvents(true);

    QBrush brush(Qt::green);
    this->setBrush(brush);

    QObject::connect(this,SIGNAL(portClicked(GUIPort*)),this->getParentView(),SLOT(addConnector(GUIPort*)));
}

GUIPort::~GUIPort()
{
}

void GUIPort::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    this->setCursor(Qt::CrossCursor);
    QBrush brush(Qt::blue);
    this->setBrush(brush);
    std::cout << "GUIPort.cpp: " << "hovering over port" << std::endl;
}

void GUIPort::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QBrush brush(Qt::green);
    this->setBrush(brush);
    this->setCursor(Qt::ArrowCursor);
}


QGraphicsView *GUIPort::getParentView()
{
    return mParentView;
}

GUIComponent *GUIPort::getComponent()
{
    return mComponent;
}


void GUIPort::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    //QGraphicsItem::mousePressEvent(event); //Don't work if this is called
    if (event->button() == Qt::LeftButton)
    {
        std::cout << "GUIPort.cpp: " << "portClick emitted\n";
        emit portClicked(this);
    }
    else if (event->button() == Qt::RightButton)
    {
        std::cout << "GUIPort.cpp: " << "RightClick" << std::endl;
    }
}


#include "Port.h"
void GUIPort::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    std::cout << "GUIPort.cpp: " << "contextMenuEvent" << std::endl;

    if ((!(this->mpKernelPort->isConnected())) || (this->mpKernelPort->getTimeVectorPtr()->empty()))
    {
        event->ignore();
    }
    else
    {
        QMenu menu;
        QAction *plotAction = menu.addAction("Plot");
        QAction *selectedAction = menu.exec(event->screenPos());

        if (selectedAction == plotAction)
        {
            plot();
        }
    }
}


void GUIPort::plot() //VANSINNE!
{
    std::cout << "GUIPort.cpp: " << "Plot()" << std::endl;

    size_t dataLength = this->mpKernelPort->getTimeVectorPtr()->size();

    QVector<double> time = QVector<double>::fromStdVector(*(this->mpKernelPort->getTimeVectorPtr())); //Inte lampligt att skyffla data pa detta viset
    QVector<double> y(dataLength);// = QVector<double>::fromStdVector((this->mpKernelPort->getDataVectorPtr()->at(1)));
    
    for (int i = 0; i<dataLength; ++i) //Denna loop ar inte klokt
    {
        //timeq[i] = this->mpKernelPort->getTimeVectorPtr()->at(i);
        y[i] = (this->mpKernelPort->getDataVectorPtr()->at(i)).at(1);
    }

    PlotWidget *newPlot = new PlotWidget(time,y);
    newPlot->show();

}
