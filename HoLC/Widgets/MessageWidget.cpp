/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

#include <QGridLayout>
#include <QPushButton>

#include "MessageWidget.h"

MessageWidget::MessageWidget(QWidget *parent) :
    QWidget(parent)
{
    //Create widgets
    mpTextEdit = new QTextEdit(this);
    mpTextEdit->setReadOnly(true);
    QPushButton *pClearButton = new QPushButton("Clear", this);
    pClearButton->setMaximumWidth(100);

    //Create layout
    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(mpTextEdit,  0,0,1,2);
    pLayout->addWidget(pClearButton,1,0,1,1);
    this->setLayout(pLayout);

    //Setup connections
    connect(pClearButton, SIGNAL(clicked()), this, SLOT(clear()));

    clear();
}


void MessageWidget::addText(const QString &text, const QColor color)
{
    mpTextEdit->undo();
    mpTextEdit->setTextColor(color);
    mpTextEdit->append(text);
    mpTextEdit->setTextColor(Qt::black);
    mpTextEdit->append(">> ");
    mpTextEdit->moveCursor( QTextCursor::End, QTextCursor::MoveAnchor);
}


void MessageWidget::clear()
{
    mpTextEdit->clear();
    mpTextEdit->append(">> ");
}
