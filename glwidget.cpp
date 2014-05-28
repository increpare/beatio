/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>
#include <iostream>
#include "glwidget.h"
#include "helper.h"

//! [0]
GLWidget::GLWidget(Helper *helper, Window *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent), helper(helper)
{
    _parent=parent;
    helper->parent=this;
    elapsed = 0;
    //setFixedSize(200, 200);
    this->setMinimumSize(200,200);
    setAutoFillBackground(false);
}
//! [0]

//! [1]
void GLWidget::animate()
{
    elapsed = (elapsed + qobject_cast<QTimer*>(sender())->interval()) % 1000;
    repaint();
}
//! [1]

//! [2]
void GLWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    helper->paint(&painter, event, elapsed);
    painter.end();
    moving=false;
}
//! [2]

//! [3]
void GLWidget::mouseMoveEvent(QMouseEvent  * event)
{
   //this->window()->setWindowTitle("MOVING");

    if (helper->pressed)
    {
        switch(helper->toolmode)
        {
        case Helper::TM_Drag:
            {
                //QPointF newPos = event->pos()/helper->scale-helper->focus;
                helper->focus = event->pos()/helper->scale-helper->cursorPos;
            }
            break;
        case Helper::TM_Create:
            {

            }
            break;
        case Helper::TM_Select:
            {

            }
            break;
        case Helper::TM_Move:
            {
                bool haveOperatedOnAnchors=false;

                for (int i=0;i<helper->movingTargets.size();i++)
                {
                    int movingTarget = helper->movingTargets[i];
                    QRectF& rect = this->helper->beatGroups[movingTarget].rect;

                    qreal prevLeft = rect.left();
                    rect.moveTopLeft(helper->cursorPos+helper->selectionDiffs[i]);

                    // if rect is moved more left or right than the thingy allows, then inc/dec all dependencies
                    qreal diff =  prevLeft - rect.left();

                    qreal cellWidth = this->helper->beatGroups[movingTarget].cellWidth();

                    if (diff>cellWidth || diff<-cellWidth)
                    {
                        bool toRight = diff>cellWidth;
                        if (toRight)
                        {
                            helper->dragFrom.setX(helper->dragFrom.x()-cellWidth);
                        }
                        else
                        {
                            helper->dragFrom.setX(helper->dragFrom.x()+cellWidth);
                        }

                        for (int i=this->helper->anchors.size()-1;i>=0;i--)
                        {
                            Anchor& a = this->helper->anchors[i];
                            if (a.first.x()==movingTarget)
                            {
                                int cell = a.first.y();
                                if (!toRight && cell>0)
                                {
                                    a.first.setY(cell-1);
                                }
                                else if (toRight && (cell<this->helper->beatGroups[movingTarget].beats.size()))
                                {
                                    a.first.setY(cell+1);
                                }
                                else
                                {
                                    this->helper->anchors.remove(i);
                                    haveOperatedOnAnchors=true;
                                }
                            }
                        }
                    }
                }

                this->helper->onAnchorsChanged();
                this->helper->onBeatGroupsChanged();
            }
            break;
        default:
            break;
        }
    }

    if (!helper->pressed)
    {
        this->helper->toolmode= (event->modifiers()&(Qt::SHIFT|Qt::CTRL)) ? Helper::TM_Select :Helper::TM_None;
    }

    helper->cursorPos = event->pos()/helper->scale-helper->focus;

    if (helper->pressed)
    {
        switch (helper->toolmode)
        {
        case Helper::TM_Create:
            {
                helper->dragFrom.setY(helper->cursorPos.y());
                helper->cursorPos.setY(helper->cursorPos.y()+40);
            }
            break;
        case Helper::TM_Move:
            {

            }
            break;
        default:
            break;
        }
    }
    calcHitbox();
}

void GLWidget::calcHitbox()
{

    // figure out highlighting
    this->helper->hovering.type = Highlight::HT_None;
    //edges first
    for (int i=0;i<this->helper->beatGroups.size();i++)
    {
        BeatGroup& bg = this->helper->beatGroups[i];
        for (int j=0;j<=bg.beats.size();j++)
        {
            QRectF hitbox = helper->getEdgeHitBox(i,j);
            if (hitbox.contains(this->helper->cursorPos))
            {
                this->helper->hovering.type=Highlight::HT_Edge;
                this->helper->hovering.edge = QPoint(i,j);
                goto outedge;
            }
        }
    }
    outedge:;

        if (this->helper->hovering.type == Highlight::HT_None)
        {
            for (int i=0;i<this->helper->beatGroups.size();i++)
            {
                BeatGroup& bg = this->helper->beatGroups[i];
                for (int j=0;j<bg.beats.size();j++)
                {
                    QRectF hitbox = helper->getCellHitBox(i,j);
                    if (hitbox.contains(this->helper->cursorPos))
                    {
                        this->helper->hovering.type=Highlight::HT_Cell;
                        this->helper->hovering.cell = QPoint(i,j);
                        goto outcell;
                    }
                }
            }
            outcell:;

        }
        switch(this->helper->toolmode)
        {
        case Helper::TM_None:
            {
                switch (this->helper->hovering.type)
                {
                case Highlight::HT_Anchor:
                    this->setCursor(Qt::UpArrowCursor);
                    break;
                case Highlight::HT_Cell:
                    this->setCursor(Qt::SizeAllCursor);
                    break;
                case Highlight::HT_Edge:
                    this->setCursor(Qt::UpArrowCursor);
                    break;
                case Highlight::HT_None:
                    this->setCursor(Qt::ArrowCursor);
                    break;
                }
            }
            break;
        case Helper::TM_Anchor:
            {
                switch (this->helper->hovering.type)
                {
                case Highlight::HT_Anchor:
                    this->setCursor(Qt::ArrowCursor);
                    break;
                case Highlight::HT_Cell:
                    this->setCursor(Qt::ArrowCursor);
                    break;
                case Highlight::HT_Edge:
                    this->setCursor(Qt::UpArrowCursor);
                    break;
                case Highlight::HT_None:
                    this->setCursor(Qt::ArrowCursor);
                    break;
                }
            }
            break;
        case Helper::TM_Create:
            {
                this->setCursor(Qt::CrossCursor);
            }
            break;
        case Helper::TM_Drag:
            {
                if (this->helper->pressed)
                {
                    this->setCursor(Qt::OpenHandCursor);
                }
                else
                {
                    this->setCursor(Qt::ClosedHandCursor);
                }
            }
            break;
        case Helper::TM_Move:
            {
                this->setCursor(Qt::SizeAllCursor);
            }
            break;
        case Helper::TM_Select:
            {
                this->setCursor(Qt::ArrowCursor);
            }
            break;
        }

    }


void GLWidget::mousePressEvent(QMouseEvent  * event)
{

    calcHitbox();
    helper->cursorPos = event->pos()/helper->scale-helper->focus;
    helper->dragFrom = helper->cursorPos;

    helper->pressed=true;

    switch (event->button())
    {
    case Qt::LeftButton:
        {
            switch (helper->hovering.type)
            {
            case Highlight::HT_None:
                {
                    //if shift held, go into selection mode
                    if (event->modifiers()&(Qt::SHIFT|Qt::CTRL))
                    {
                        this->helper->toolmode=Helper::TM_Select;
                    }
                    else
                    {
                        this->helper->toolmode=Helper::TM_Drag;
                        this->setCursor(Qt::ClosedHandCursor);
                    }
                }
                break;
            case Highlight::HT_Cell:
                {
                    this->helper->toolmode=Helper::TM_Move;

                    this->setCursor(Qt::ArrowCursor);

                    if (!(event->modifiers()&(Qt::SHIFT|Qt::CTRL)))
                    {
                        //if shift not pressed, clear selection
                        helper->selection.clear();
                    }

                    QVector<QPoint> selection = helper->selection.getSelection();

                    for (int i=0;i<this->helper->beatGroups.size();i++)
                    {

                        if (helper->beatGroups[i].rect.contains(helper->cursorPos))
                        {
                            int selected = ((helper->cursorPos.x()-helper->beatGroups[i].rect.left())/helper->beatGroups[i].rect.width())*helper->beatGroups[i].beats.size();
                            QPoint selectedPoint = QPoint(i,selected);
                            int index = selection.indexOf(selectedPoint);
                            if (index>=0)
                            {
                                selection.remove(index);
                            }
                            else
                            {
                                selection.push_back(selectedPoint);
                            }
                            break;
                        }
                    }

                    this->setCursor(Qt::SizeAllCursor);
                    helper->movingTargets.clear();
                    helper->selectionDiffs.clear();
                    foreach(QPoint sel,selection)
                    {
                        if (helper->movingTargets.indexOf(sel.x())<0)
                        {
                            helper->movingTargets.push_back(sel.x());
                            this->helper->selectionDiffs.push_back(helper->beatGroups[sel.x()].rect.topLeft()-helper->cursorPos);
                        }
                    }

                    helper->selection.set(selection);
                }
                break;
            case Highlight::HT_Edge:

                this->helper->toolmode=Helper::TM_Anchor;
                this->setCursor(Qt::SizeVerCursor);

               this->helper->anchorFrom = helper->hovering.edge;

               helper->pressed=true;

                break;
            case Highlight::HT_Anchor:
                break;
            }
        }
        break;
    case Qt::RightButton:
        {
            switch (helper->hovering.type)
            {
            case Highlight::HT_None:
                this->helper->toolmode=Helper::TM_Create;
                this->setCursor(Qt::CrossCursor);
                break;
            case Highlight::HT_Cell:
                break;
            case Highlight::HT_Edge:
                break;
            case Highlight::HT_Anchor:
                break;
            }
        }
        break;
    default:
        break;
    }

    /*
    if (event->button()==Qt::LeftButton)
    {
        //this->window()->setWindowTitle("Pressed");
        helper->cursorPos = event->pos()/helper->scale-helper->focus;
        helper->dragFrom = helper->cursorPos;

        helper->pressed=true;

        switch (this->helper->toolmode)
        {
        case Helper::TM_Create:
            this->setCursor(Qt::CrossCursor);
            break;
        case Helper::TM_Drag:
            this->setCursor(Qt::ClosedHandCursor);
            break;
        case Helper::TM_Select:
            this->setCursor(Qt::ArrowCursor);

            if (!(event->modifiers()&(Qt::SHIFT|Qt::CTRL)))
            {
                //if shift not pressed, clear selection
                helper->selection.clear();
            }

            for (int i=0;i<this->helper->beatGroups.size();i++)
            {
                
                if (helper->beatGroups[i].rect.contains(helper->cursorPos))
                {

                    helper->pressed=false;
                    int selected = ((helper->cursorPos.x()-helper->beatGroups[i].rect.left())/helper->beatGroups[i].rect.width())*helper->beatGroups[i].beats.size();
                    QPoint selectedPoint = QPoint(i,selected);
                    int index = helper->selection.indexOf(selectedPoint);
                    if (index>=0)
                    {
                        helper->selection.remove(index);
                    }
                    else
                    {
                        helper->selection.push_back(selectedPoint);
                    }
                    break;
                }
            }
            break;
        case Helper::TM_Anchor:
            this->setCursor(Qt::SizeVerCursor);
            helper->pressed=false;

            for (int i=0;i<this->helper->beatGroups.size();i++)
            {
                QRectF biggerRect = helper->beatGroups[i].rect;
                biggerRect.setWidth((biggerRect.width() * (helper->beatGroups[i].beats.size()+1))/(helper->beatGroups[i].beats.size()));

                if (biggerRect.contains(helper->cursorPos))
                {

                    int selected = ((helper->cursorPos.x()-helper->beatGroups[i].rect.left())/helper->beatGroups[i].rect.width())*helper->beatGroups[i].beats.size();
                    this->helper->anchorFrom = QPoint(i,selected);
                    helper->pressed=true;
                    break;
                }
            }
            break;
        case Helper::TM_Move:
            this->setCursor(Qt::SizeAllCursor);
            helper->movingTargets.clear();
            helper->selectionDiffs.clear();
            foreach(QPoint sel,this->helper->selection)
            {
                if (helper->movingTargets.indexOf(sel.x())<0)
                {
                    helper->movingTargets.push_back(sel.x());                                        
                    this->helper->selectionDiffs.push_back(helper->beatGroups[sel.x()].rect.topLeft()-helper->cursorPos);
                }
            }


            break;
        }
    }
    */
}
//! [3]

//! [3]
void GLWidget::mouseReleaseEvent(QMouseEvent  * event)
{  

    const QPointF& from = this->helper->dragFrom;
    const QPointF& to = this->helper->cursorPos;
    qreal left = std::min(from.x(),to.x());
    qreal right = std::max(from.x(),to.x());
    qreal top = std::min(from.y(),to.y());
    qreal bottom = std::max(from.y(),to.y());
    QRectF dragRect = QRect(QPoint(left,top),QPoint(right,bottom));

    switch (event->button())
    {
    case Qt::LeftButton:
        {
            switch (this->helper->toolmode)
            {
            case Helper::TM_Drag:
                {
                    this->setCursor(Qt::OpenHandCursor);
                }
                break;
            case Helper::TM_Select:
                {
                    if (this->helper->pressed)
                    {                                                
                        for (int i=0;i<this->helper->beatGroups.size();i++)
                        {
                            if (helper->beatGroups[i].rect.intersects(dragRect))
                            {
                                qreal width = helper->beatGroups[i].rect.width()/helper->beatGroups[i].beats.size();

                                for (int j=0;j<this->helper->beatGroups[i].beats.size();j++)
                                {
                                    QRectF innerRect(this->helper->beatGroups[i].rect.left()+j*width,this->helper->beatGroups[i].rect.top(),width,40);
                                    if (innerRect.intersects(dragRect))
                                    {
                                        helper->selection.push_back(QPoint(i,j));
                                    }
                                }
                            }
                        }
                        this->setCursor(Qt::ArrowCursor);
                    }
                }
                break;
            case Helper::TM_Anchor:
                {
                    if (!helper->pressed)
                        break;

                    if (this->helper->hovering.type==Highlight::HT_Edge)
                    {
                        QPair<QPoint,QPoint> anchor;
                        anchor.first=this->helper->anchorFrom;
                        anchor.second=helper->hovering.edge;

                        //only allow downward arrows

                        if (this->helper->cursorPos.y()>this->helper->dragFrom.y())
                        {
                            qSwap(anchor.first,anchor.second);
                        }

                        if (this->helper->canAdd(anchor))
                        {
                            this->helper->anchors.push_back(anchor);
                            this->helper->onAnchorsChanged();
                        }
                    }
                }
                break;
            case Helper::TM_Move:
                {
                    this->setCursor(Qt::SizeAllCursor);
                }
                break;
            default:
                break;
            }
        }
        break;
    case Qt::RightButton:
        {
            switch (this->helper->toolmode)
            {                
            case Helper::TM_Create:
                {
                    BeatGroup bg;
                    bg.rect=dragRect;
                    bg.rect.setHeight(40);
                    for (int i=0;i<this->helper->defaultBeatCount;i++)
                    {
                        Beat beat=this->helper->defaultBeat;
                        bg.beats.push_back(beat);
                    }
                    this->helper->beatGroups.push_back(bg);
                    this->helper->onBeatGroupsChanged();

                    this->setCursor(Qt::CrossCursor);
                }
                break;
            default:
                break;
            }
        }
        break;
    default:
        break;
    }
    this->helper->toolmode=Helper::TM_None;
    helper->pressed=false;
}
//! [3]

//! [4]
void GLWidget::wheelEvent ( QWheelEvent * event  )
{
    const qreal scalespeed = 1.05;

    int numDegrees = event->delta() / 8;

    QPointF oldCursorPosScreen = (helper->focus+helper->cursorPos)*helper->scale;

    this->helper->scale=std::min(std::max(this->helper->scale*pow(scalespeed,numDegrees),0.1),10.0);
    helper->focus = oldCursorPosScreen/helper->scale-helper->cursorPos;


    event->accept();
}
//! [4]

