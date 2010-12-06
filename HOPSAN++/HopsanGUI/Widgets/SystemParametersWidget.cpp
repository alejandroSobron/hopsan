/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Linköping University,
 * Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Linköping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

/*
 * HopsanGUI
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 */

//!
//! @file   SystemParametersWidget.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-10-04
//!
//! @brief Contains a System parameter widget class
//!
//$Id$

#include <QtGui>

#include "../MainWindow.h"
#include "SystemParametersWidget.h"

#include <QWidget>
#include <QDialog>

#include "ProjectTabWidget.h"
#include "../GUIObjects/GUISystem.h"

#include "../common.h"


SystemParameterTableWidget::SystemParameterTableWidget(int rows, int columns, QWidget *parent)
    : QTableWidget(rows, columns, parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setSelectionMode(QAbstractItemView::SingleSelection);

    setBaseSize(400, 500);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->hide();

    update();

    connect(this, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(changeParameter(QTableWidgetItem*)));
}


void SystemParameterTableWidget::keyPressEvent(QKeyEvent *event)
{
    QTableWidget::keyPressEvent(event);
    if(event->key() == Qt::Key_Delete)
    {
        if(currentColumn()<1)
        {
            std::cout << "Delete current Item" << std::endl;
            removeSelectedParameters();
        }
    }
}


//! Construtor for System Parameters widget, where the user can see and change the System parameters in the model.
//! @param parent Pointer to the main window
SystemParametersWidget::SystemParametersWidget(MainWindow *parent)
    : QWidget(parent)
{
    //mpParentMainWindow = parent;
    //Set the name and size of the main window
    this->setObjectName("SystemParameterWidget");
    this->resize(400,500);
    this->setWindowTitle("System Parameters");

    mpSystemParametersTable = new SystemParameterTableWidget(0,1,this);

    mpAddButton = new QPushButton(tr("&Set"), this);
    mpAddButton->setFixedHeight(30);
    mpAddButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mpAddButton->setAutoDefault(false);
    QFont tempFont = mpAddButton->font();
    tempFont.setBold(true);
    mpAddButton->setFont(tempFont);

    mpRemoveButton = new QPushButton(tr("&Unset"), this);
    mpRemoveButton->setFixedHeight(30);
    mpRemoveButton->setAutoDefault(false);
    mpRemoveButton->setFont(tempFont);

    mpGridLayout = new QGridLayout(this);
    mpGridLayout->addWidget(mpSystemParametersTable, 0, 0);
    mpGridLayout->addWidget(mpAddButton, 1, 0);
    mpGridLayout->addWidget(mpRemoveButton, 2, 0);

    connect(mpAddButton, SIGNAL(clicked()), mpSystemParametersTable, SLOT(openComponentPropertiesDialog()));
    connect(mpRemoveButton, SIGNAL(clicked()), mpSystemParametersTable, SLOT(removeSelectedParameters()));
}


//! @brief Used for parameter changes done directly in the label
void SystemParameterTableWidget::changeParameter(QTableWidgetItem *item)
{
    //Filter out value labels
    if(item->column() == 1)
    {
        QTableWidgetItem *neighborItem = itemAt(item->row(), item->column()-1);
        QString parName = neighborItem->text();
        QString parValue = item->text();

        //Do not do update, then crash due to the rebuild of the QTableWidgetItems
        setParameter(parName, parValue, false);
    }
}


double SystemParameterTableWidget::getParameter(QString name)
{
    return gpMainWindow->mpProjectTabs->getCurrentSystem()->getCoreSystemAccessPtr()->getSystemParameter(name);
}


bool SystemParameterTableWidget::hasParameter(QString name)
{
    return gpMainWindow->mpProjectTabs->getCurrentSystem()->getCoreSystemAccessPtr()->hasSystemParameter(name);
}


//! Slot that adds a System parameter value
//! @param name Lookup name for the System parameter
//! @param value Value of the System parameter
void SystemParameterTableWidget::setParameter(QString name, double value, bool doUpdate)
{
    //Error check
    if(!(gpMainWindow->mpProjectTabs->getCurrentSystem()->getCoreSystemAccessPtr()->setSystemParameter(name, value)))
    {
        QMessageBox::critical(0, "Hopsan GUI",
                              QString("'%1' is an invalid name for a system parameter.")
                              .arg(name));
        return;
    }
    if(doUpdate)
    {
        update();
    }

    emit modifiedSystemParameter();
}


//! Slot that adds a System parameter value
//! @param name Lookup name for the System parameter
//! @param value Value of the System parameter
void SystemParameterTableWidget::setParameter(QString name, QString valueTxt, bool doUpdate)
{
    //Error check
    bool isDbl;
    double value = valueTxt.toDouble((&isDbl));
    if(!(isDbl))
    {
        QMessageBox::critical(0, "Hopsan GUI",
                              QString("'%1' is not a valid number.")
                              .arg(valueTxt));
        QString oldValue = QString::number(gpMainWindow->mpProjectTabs->getCurrentSystem()->getCoreSystemAccessPtr()->getSystemParameter(name));
        QList<QTableWidgetItem *> items = selectedItems();
        //Error if size() > 1, but it should not be! :)
        for(int i = 0; i<items.size(); ++i)
        {
            items[i]->setText(oldValue);
        }
    }
    else
    {
        setParameter(name, value, doUpdate);
    }
}


void SystemParameterTableWidget::setParameters()
{
//    if(gpMainWindow->mpProjectTabs->getCurrentSystem()->getCoreSystemAccessPtr()->getNumberOfSystemParameters() > 0)
//    {
        for(int i=0; i<rowCount(); ++i)
        {
            QString name = item(i, 0)->text();
            double value = item(i, 1)->text().toDouble();
            setParameter(name, value);
        }
//    }
}


//! Slot that removes all selected System parameters in parameter table
//! @todo This shall remove the actual System parameters when they have been implemented, wherever they are stored.
void SystemParameterTableWidget::removeSelectedParameters()
{
    QList<QTableWidgetItem *> pSelectedItems = selectedItems();
    QStringList parametersToRemove;
    QString tempName;

    for(int i=0; i<pSelectedItems.size(); ++i)
    {
        tempName = item(pSelectedItems[i]->row(),0)->text();
        if(!parametersToRemove.contains(tempName))
        {
            parametersToRemove.append(tempName);
        }
    }

    for(int j=0; j<parametersToRemove.size(); ++j)
    {
        qDebug() << "Removing: " << parametersToRemove[j];
        gpMainWindow->mpProjectTabs->getCurrentSystem()->getCoreSystemAccessPtr()->removeSystemParameter(parametersToRemove.at(j));
    }

    update();
}


//! Slot that opens "Add Parameter" dialog, where the user can add new System parameters
void SystemParameterTableWidget::openComponentPropertiesDialog()
{
    QDialog *pAddComponentPropertiesDialog = new QDialog(this);
    pAddComponentPropertiesDialog->setWindowTitle("Set System Parameter");

    mpNameLabel = new QLabel("Name: ", this);
    mpNameBox = new QLineEdit(this);
    mpValueLabel = new QLabel("Value: ", this);
    mpValueBox = new QLineEdit(this);
    mpValueBox->setValidator(new QDoubleValidator(this));
    mpAddInDialogButton = new QPushButton("Set", this);
    mpDoneInDialogButton = new QPushButton("Done", this);
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pButtonBox->addButton(mpAddInDialogButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(mpDoneInDialogButton, QDialogButtonBox::ActionRole);

    QGridLayout *pDialogLayout = new QGridLayout(this);
    pDialogLayout->addWidget(mpNameLabel,0,0);
    pDialogLayout->addWidget(mpNameBox,0,1);
    pDialogLayout->addWidget(mpValueLabel,1,0);
    pDialogLayout->addWidget(mpValueBox,1,1);
    pDialogLayout->addWidget(pButtonBox,2,0,1,2);
    pAddComponentPropertiesDialog->setLayout(pDialogLayout);
    pAddComponentPropertiesDialog->show();

    connect(mpDoneInDialogButton,SIGNAL(clicked()),pAddComponentPropertiesDialog,SLOT(close()));
    connect(mpAddInDialogButton,SIGNAL(clicked()),this,SLOT(addParameter()));
}


//! @Private help slot that adds a parameter from the selected name and value in "Add Parameter" dialog
void SystemParameterTableWidget::addParameter()
{
    bool ok;    
    setParameter(mpNameBox->text(), mpValueBox->text().toDouble(&ok));
}


//! Updates the parameter table from the contents list
void SystemParameterTableWidget::update()
{
    QMap<std::string, double>::iterator it;
    QMap<std::string, double> tempMap = gpMainWindow->mpProjectTabs->getCurrentSystem()->getCoreSystemAccessPtr()->getSystemParametersMap();

    clear();

    if(tempMap.isEmpty())
    {
        setColumnCount(1);
        setRowCount(1);
        verticalHeader()->hide();

        QTableWidgetItem *item = new QTableWidgetItem();
        item->setText("No System parameters set.");
        item->setBackgroundColor(QColor("white"));
        item->setTextAlignment(Qt::AlignCenter);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        setItem(0,0,item);
    }
    else
    {
        setRowCount(0);
        setColumnCount(2);
        setColumnWidth(0, 120);
        verticalHeader()->show();
    }
    for(it=tempMap.begin(); it!=tempMap.end(); ++it)
    {
        QString valueString;
        valueString.setNum(it.value());
        insertRow(rowCount());
        QTableWidgetItem *nameItem = new QTableWidgetItem(QString(it.key().c_str()));
        QTableWidgetItem *valueItem = new QTableWidgetItem(valueString);
        nameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        valueItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
        setItem(rowCount()-1, 0, nameItem);
        setItem(rowCount()-1, 1, valueItem);
    }
}
