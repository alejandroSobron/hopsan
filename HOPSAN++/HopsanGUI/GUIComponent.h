//$Id$

#ifndef GUICOMPONENT_H
#define GUICOMPONENT_H

#include <QGraphicsWidget>
#include <QGraphicsView>
#include <vector>
#include "GUIComponent.h"
#include "ProjectTabWidget.h"

class GraphicsScene;
class GraphicsView;
class GUIConnector;
class QGraphicsSvgItem;
class GUIComponentNameTextItem;
class HopsanEssentials;
class Component;
class GUIComponentSelectionBox;
class GUIPort;

class GUIObject : public QGraphicsWidget
{
    Q_OBJECT
public:
    GUIObject(QPoint position, QString iconPath, GraphicsScene *scene, QGraphicsItem *parent = 0);
    ~GUIObject();
    //QGraphicsView *getParentView();
    void addConnector(GUIConnector *item);

    virtual QString getName();
    void refreshName();
    virtual void setName(QString name, bool doOnlyLocalRename=false);

    int getPortNumber(GUIPort *port);
    int getNameTextPos();
    void setNameTextPos(int textPos);

    void showPorts(bool visible);
    GUIPort *getPort(int number);

    GraphicsScene *mpParentGraphicsScene;
    GraphicsView *mpParentGraphicsView;

    enum { Type = UserType + 2 };
    int type() const;

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void groupComponents(QList<QGraphicsItem*> compList);

signals:
    void componentMoved();
    void componentDeleted();

public slots:
//     void deleteMe();
     void rotate();
     void moveUp();
     void moveDown();
     void moveLeft();
     void moveRight();
     void hideName();
     void showName();

protected:
    QGraphicsSvgItem *mpIcon;
    GUIComponentNameTextItem *mpNameText;
    GUIComponentSelectionBox *mpSelectionBox;
    double mTextOffset;
    QGraphicsLineItem *mpTempLine;
    //std::vector<GUIConnector*> mConnectors;        //Inteded to store connectors for each component

    QList<GUIPort*> mPortListPtrs;
    int mNameTextPos;
    bool mIconRotation;

protected slots:
    void fixTextPosition(QPointF pos);

};

class GUIComponent : public GUIObject
{
    Q_OBJECT
public:
    GUIComponent(HopsanEssentials *hopsan, QStringList parameterData, QPoint position, GraphicsScene *scene, QGraphicsItem *parent = 0);

    //Core interaction
    Component *mpCoreComponent;
    //

    QString getName();
    void setName(QString name, bool doOnlyLocalRename=false);
    QString getTypeName();

    enum { Type = UserType + 3 };
    int type() const;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void openParameterDialog();

    QString mComponentTypeName;

public slots:
     void deleteMe();
};


class GUIGroup : public GUIObject
{
    Q_OBJECT
public:
    GUIGroup(QList<QGraphicsItem*> compList, QPoint position, GraphicsScene *scene, QGraphicsItem *parent = 0);

//    QString getName();
//    void setName(QString name, bool doOnlyLocalRename=false);

    enum { Type = UserType + 4 };
    int type() const;

protected:
    GraphicsScene *mpGroupScene;

//    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
//    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
//    void openParameterDialog();
//
//    QString mComponentTypeName;
//
//    GraphicsScene *mpGroupScene;
//
//public slots:
//     void deleteMe();
};


class GUIComponentNameTextItem : public QGraphicsTextItem
{
    Q_OBJECT
private:
    GUIObject* mpParentGUIComponent;

public:
    GUIComponentNameTextItem(GUIObject *pParent);

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void focusOutEvent(QFocusEvent *event);

signals:
    void textMoved(QPointF pos);

};



class GUIComponentSelectionBox : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT
public:
    GUIComponentSelectionBox(qreal x1, qreal y1, qreal x2, qreal y2, QPen activePen, QPen hoverPen, GUIObject *parent = 0);
    ~GUIComponentSelectionBox();
    void setActive();
    void setPassive();
    void setHovered();

    GUIObject *mpParentGUIComponent;

private:
    std::vector<QGraphicsLineItem*> mLines;
    QPen mActivePen;
    QPen mHoverPen;
};



class GUISubsystem : public GUIObject
{
    Q_OBJECT
public:
    GUISubsystem(HopsanEssentials *hopsan, QStringList parameterData, QPoint position, GraphicsScene *scene, QGraphicsItem *parent = 0);

private:
    QString mModelFilePath;
    QString mGraphicsFilePath;
    bool   mIsEmbedded;
};

#endif // GUICOMPONENT_H
