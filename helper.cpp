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
#include <QtGlobal>
#include "helper.h"
#include <iostream>
#include <QFileDialog>
#include "glwidget.h"
#include "window.h"

void SelectionData::set(const QVector<QPoint>& selection)
{
    this->selection=selection;
    if (this->selection.size()>0)
    {
//        this->sel
    }
    update();
}

void SelectionData::push_back(const QPoint &newPoint)
{
    this->selection.push_back(newPoint);
    if (this->selection.size()>0)
    {
//        this->selection.s
    }
    update();
}

void SelectionData::update()
{
    QListWidget* myListWidget = this->_parent->parent->_parent->soundSelector;

    QItemSelectionModel * selectionModel = myListWidget->selectionModel();

    // clear the selection
    selectionModel->clearSelection();    // or QItemSelectionModel::clear()

    QSet<int> selectedInsts;
    for (int i=0;i<this->selection.size();i++)
    {
        int sample = this->_parent->beatGroups[this->selection[i].x()].beats[this->selection[i].y()].sample;
        selectedInsts.insert(sample);
    // make a multiple selection (i'm not sure, maybe you need to use QItemSelection)        
    }

    if (selectedInsts.size()==1)
    {
        int selected=*selectedInsts.begin();
        myListWidget->setCurrentItem(myListWidget->item(selected));
    }
    else
    {
        myListWidget->setCurrentItem(0);
    }
}

//! [0]
Helper::Helper():
        defaultBeat(),
        selection(this)
{
    playing=false;
    if (QMultimedia::Available!=0)
    {
        std::cerr<<"ERROR - sound notavailable"<<std::endl;
    }

    background = QBrush(QColor(100, 100, 140));
    scorePen = QPen(QColor(110, 110, 200));
    circleBrush = QBrush(QColor(100,100,255,30));
    circlePen = QPen(Qt::black);
    circlePen.setWidth(1);
    circlePen.setCosmetic(true);
    defaultBeatCount=4;


    clearBrush = QBrush();
    clearPen = QPen(Qt::transparent);

    boxBrush = QBrush(QColor(235,235,150,200));
    boxSelectedBrush = QBrush(QColor(255,255,200,255));
    boxPen = QPen(QColor(30,30,100,255));
    boxPen.setWidth(1);
    boxPen.setCosmetic(true);

    highlightBrush = QBrush(QColor(0,255,255,30));

    boundsPen = QPen(QColor(20,255,255,100));
    boundsPen.setWidth(1);
    boundsPen.setCosmetic(true);

    playPen = QPen(QColor(255,20,20,200));
    playPen.setWidth(1);
    playPen.setCosmetic(true);


    QVector<qreal> dashVals;
    dashVals.push_back(2.0f);
    dashVals.push_back(2.0f);

    anchorPen = QPen(QColor(255,255,50,120),1);
    anchorPen.setWidth(1);
    anchorPen.setCosmetic(true);
    //anchorPen.setDashPattern(dashVals);


    createBrush = QBrush(QColor(255,255,100,30));
    createPen = QPen(Qt::yellow);
    createPen.setWidth(1);
    createPen.setCosmetic(true);

    circlePen.setDashPattern(dashVals);
    textPen = QPen(Qt::white);
    textFont.setPixelSize(50);

    this->scale=1;
    this->toolmode=TM_Select;
    this->focus=QPointF(0.0f,0.0f);
    this->pressed=false;
    this->performingoperation=false;
}

QVector<Note> Helper::generateOnsets()
{
    QVector<Note> onsets;

    qreal leftmost = this->bounds.left();

    for (int i=0;i<this->beatGroups.size();i++)
    {
        BeatGroup& bg = this->beatGroups[i];
        for (int j=0;j<bg.beats.size();j++)
        {
            Note s;
            s.instrument=bg.beats[j].sample;
            s.pan=bg.beats[j].pan;
            s.volumn=bg.beats[j].vol;
            s.onset=(bg.getCellX(j)-leftmost)/100;
            onsets.push_back(s);
        }
    }

    qSort(onsets);
    return onsets;
}

void Helper::InsertBeatAfter(const QPoint& p)
{
    Q_ASSERT(p.x()<this->beatGroups.size() && p.y()<this->beatGroups[p.x()].beats.size());

    //change anchors
    for (int i=0;i<this->anchors.size();i++)
    {
        Anchor& a=anchors[i];
        if (a.first.x()==p.x()&&a.first.y()>p.y())
        {
            a.first.setY(a.first.y()+1);
        }
        if (a.second.x()==p.x()&&a.second.y()>p.y())
        {
            a.second.setY(a.second.y()+1);
        }
    }
    QVector<Beat>& beats = this->beatGroups[p.x()].beats;
    beats.insert(beats.begin()+p.y(),this->defaultBeat);
}

//after calling this, should try delete all beatgroups with no members
void Helper::RemoveBeat(const QPoint& p)
{
    Q_ASSERT(p.x()<this->beatGroups.size() && p.y()<this->beatGroups[p.x()].beats.size());

    //change anchors
    for (int i=this->anchors.size()-1;i>=0;i--)
    {
        bool bShouldRemove=false;
        Anchor& a=anchors[i];
        if (a.first.x()==p.x()&&a.first.y()>p.y())
        {
            a.first.setY(a.first.y()+1);
            bShouldRemove=true;
        }
        if (a.second.x()==p.x()&&a.second.y()>p.y())
        {
            a.second.setY(a.second.y()+1);
            bShouldRemove=true;
        }

        if (bShouldRemove)
        {
            this->anchors.remove(i);
        }
    }
    this->beatGroups[p.x()].beats.remove(p.y());
}

//need to call 'tidy up after alteration' functions after here
void Helper::ClearEmptyBeatGroups()
{
    for (int i=this->beatGroups.size()-1;i>=0;i--)
    {
        BeatGroup& bg = this->beatGroups[i];
        if (bg.beats.size()==0)
        {

        }
    }
}

void Helper::beginTransaction()
{
    if (performingoperation)
    {
        std::cerr<<"BEGINNING A TRANSACTION BEFORE HAVING ENDED THE PREVIOUS ONE"<<std::endl;
    }
    {
        performingoperation=true;
        State s;
        s.anchors=this->anchors;
        s.beatGroups=this->beatGroups;
        this->stack.push(s);
    }
}

void Helper::endTransaction()
{
    if (performingoperation)
    {
        performingoperation=false;
    }
}

void Helper::terminateTransaction()
{
    if (performingoperation)
    {
        performingoperation=false;
        State s = this->stack.pop();
        this->anchors=s.anchors;
        this->beatGroups=s.beatGroups;
        this->onAnchorsChanged();
    }
    else
    {
        std::cerr<<"Terminating a transaction that was never begun."<<std::endl;
    }
}

void Helper::undo()
{
    if (!performingoperation)
    {
        if (this->stack.size()>0)
        {
            State s = this->stack.pop();
            this->anchors=s.anchors;
            this->beatGroups=s.beatGroups;
            this->onAnchorsChanged();
        }
    }
    else
    {
        std::cerr<<"Undoing mid-operation."<<std::endl;
    }

}

void Helper::onBeatGroupsChanged()
{
    bool set=false;
    for (int i=0;i<this->beatGroups.size();i++)
    {
        if (!set)
        {
            set=true;
            this->bounds.setLeft(this->beatGroups[i].rect.left());
            this->bounds.setRight(this->beatGroups[i].rect.right());
            this->bounds.setTop(this->beatGroups[i].rect.top());
            this->bounds.setBottom(this->beatGroups[i].rect.bottom());
        }
        else
        {
            this->bounds.setLeft(std::min(this->bounds.left(),this->beatGroups[i].rect.left()));
            this->bounds.setRight(std::max(this->bounds.right(),this->beatGroups[i].rect.right()));
            this->bounds.setTop(std::min(this->bounds.top(),this->beatGroups[i].rect.top()));
            this->bounds.setBottom(std::max(this->bounds.bottom(),this->beatGroups[i].rect.bottom()));
        }
    }
}

// A little weird...should probably keep the forest around allways instead of
// recalculating it each time.
bool Helper::canAdd(const Anchor& anchor)
{
    QPoint from = anchor.first;
    QPoint to = anchor.second;

    //1: can't add if there are already two lines coming from here
    bool bFound=false;
    for (int i=0;i<this->anchors.size();i++)
    {
        if (this->anchors[i].first==from)
        {
            std::cout<<"this cell is already linked  : "<<std::endl;
            return false;
        }
        if (this->anchors[i].first.x()==from.x())
        {
            if (!bFound)
            {
                //4: ensure pairs are in right order
                qreal a1 = this->beatGroups[from.x()].getCellX(from.y());
                qreal a2 = this->beatGroups[to.x()].getCellX(to.y());

                qreal b1 =  this->beatGroups[anchors[i].first.x()].getCellX(anchors[i].first.y());
                qreal b2 =  this->beatGroups[anchors[i].second.x()].getCellX(anchors[i].second.y());

                if ( (a1<b1) != (a2<b2))
                {
                    std::cout<<"not order-preerving : "<<a1<<","<<a2<<","<<b1<<","<<b2<<std::endl;
                    return false;
                }
                bFound=true;
            }
            else
            {
                std::cout<<"can't have more than two constraints"<<std::endl;

                return false;
            }
        }
    }

    //2: check no cycles AND
    //3: arrange into tree
    anchors.push_back(anchor);
    bool bResult = tryGenForest();
    anchors.pop_back();

    return bResult;
}

bool Helper::tryGenForest()
{
    forest.clear();

    for (int i=0;i<this->beatGroups.size();i++)
    {
        forest.push_back(AnchorTree(i));
    }

    for (int i=0;i<this->anchors.size();i++)
    {
        int constrained = this->anchors[i].first.x();
        int constraining = this->anchors[i].second.x();

        if (forest[constrained].searchFor(constraining))
        {
            std::cout<<"recursive constraints"<<std::endl;
            return false;
        }

        if (!forest[constraining].searchFor(constrained))
        {
            forest[constraining].children.push_back(&forest[constrained]);
        }
    }

    //from here on, index does not correspond to label

    return true;
}

void Helper::deleteSelection()
{
    QVector<QPoint> selection = this->selection.getSelection();

    for (int i=anchors.size()-1;i>=0;i--)
    {
        for (int j=0;j<selection.size();j++)
        {
            if (anchors[i].first.x()==selection[j].x() || anchors[i].second.x()==selection[j].x())
            {
                anchors.remove(i);
                break;
            }
        }
    }

    QVector<int> toDelete;

    for (int i=0;i<selection.size();i++)
    {
        if (toDelete.indexOf(selection[i].x())<0)
        {
            toDelete.push_back(selection[i].x());
        }
    }

    qSort(toDelete);

    for (int i=toDelete.size()-1;i>=0;i--)
    {
        std::cout<<"deleting"<<toDelete.size()<<","<<toDelete[i]<<std::endl;
        int n = toDelete[i];
        this->beatGroups.remove(n);
        for (int j=0;j<anchors.size();j++)
        {
            Anchor& a = anchors[j];
            if (a.first.x()>n)
            {
                a.first.setX(a.first.x()-1);
            }
            if (a.second.x()>n)
            {
                a.second.setX(a.second.x()-1);
            }
        }
    }

    this->selection.clear();

    this->onAnchorsChanged();
}

bool Helper::setDockingPositions(AnchorTree& tree)
{
    QPair<int,qreal> anchor1;
    QPair<int,qreal> anchor2;

    Anchor* pAnchor1=0;
    Anchor* pAnchor2=0;

    anchor1.first=-1;
    anchor2.first=-1;

    for (int j=0;j<anchors.size();j++)
    {
        if (anchors[j].first.x()==tree.label)
        {
            //link 'em
            BeatGroup& bgConstraining = this->beatGroups[anchors[j].second.x()];
            //BeatGroup& bgConstrained = this->beatGroups[tree.label];
            QPair<int,qreal>& anch = anchor1.first==-1 ? anchor1 : anchor2;
            Anchor*& pAnchor = pAnchor1==0  ? pAnchor1 : pAnchor2;
            anch.first=anchors[j].first.y();
            anch.second=bgConstraining.getCellX(anchors[j].second.y());
            pAnchor=&anchors[j];
        }
    }

    if (anchor2.first==-1)
    {
        if (anchor1.first!=-1)
        {
            //just translate
            qreal box_x = this->beatGroups[tree.label].rect.x();
            qreal cell_x = this->beatGroups[tree.label].getCellX(anchor1.first);
            this->beatGroups[tree.label].rect.moveLeft(box_x + (anchor1.second-cell_x));

            BeatGroup& bg = this->beatGroups[tree.label];
            QRectF& rect = bg.rect;
            rect.moveLeft(anchor1.second-bg.cellWidth()*anchor1.first);
        }
        else
        {
            //found nothing
        }

    }
    else
    {
        //found two anchors - some scaling will be needed
        if (anchor1.first>anchor2.first)
            qSwap(anchor1,anchor2);

        if (anchor1.second>anchor2.second)
        {
            //two anchors in the wrong order
            std::cerr<<"wrong order"<<anchor1.second<<","<<anchor2.second<<std::endl;
            qSwap(anchor1.second,anchor2.second);
            // swap anchor components aswell
            {
                int y1 = pAnchor1->first.y();
                int y2 = pAnchor2->first.y();
                pAnchor1->first.setY(y2);
                pAnchor2->first.setY(y1);
            }
            return false;
        }

        BeatGroup& bg = this->beatGroups[tree.label];
        QRectF& rect = bg.rect;

        qreal cell_dist = bg.getCellX(anchor2.first) - bg.getCellX(anchor1.first);
        rect.setWidth(rect.width()*(anchor2.second-anchor1.second)/cell_dist);
        rect.moveLeft(anchor1.second-bg.cellWidth()*anchor1.first);
    }

    QRectF& parentRect = this->beatGroups[tree.label].rect;

    for (int i=0;i<tree.children.size();i++)
    {
        QRectF& childRect = this->beatGroups[tree.children[i]->label].rect;

        if (parentRect.bottom()+12>childRect.top())
        {
            childRect.moveTop(parentRect.bottom()+12);
        }

        if (!setDockingPositions(*tree.children[i]))
            return false;
    }

    return true;
}

void Helper::onAnchorsChanged()
{
    while (!onAnchorsChangedInner()){}

    this->onBeatGroupsChanged();
}


bool Helper::onAnchorsChangedInner()
{
    tryGenForest();

    for (int i=0;i<forest.size();i++)
    {
        if (!setDockingPositions(forest[i]))
            return false;
    }

    return true;
}



QRectF Helper::getEdgeHitBox(int i,int j)
{
    switch (this->toolmode)
    {
    case Helper::TM_None:
        {            
            BeatGroup& bg = this->beatGroups[i];
            qreal about = bg.rect.left()+bg.cellWidth()*j;
            qreal radius = bg.cellWidth()/5;
            return QRectF(about-radius,bg.rect.top()-10, 2*radius,60);
        }
        break;
    case Helper::TM_Anchor:
        {            
            BeatGroup& bg = this->beatGroups[i];
            qreal about = bg.rect.left()+bg.cellWidth()*j;
            qreal w = bg.cellWidth();
            return QRectF(j==0 ? about-10 : about-w/2,bg.rect.top()-10, (j==0 || j==bg.beats.size()) ? 10+w/2 : w ,60);
        }
        break;
    case Helper::TM_Create:
        {
            return QRectF();
        }
        break;
    case Helper::TM_Drag:
        {
            return QRectF();
        }
        break;
    case Helper::TM_Move:
        {
            return QRectF();
        }
        break;
    case Helper::TM_Select:
        {
            return QRectF();
        }
        break;
    default:
        {
            return QRectF();
        }
    }

}

QRectF Helper::getCellHitBox(int i,int j)
{
    switch (this->toolmode)
    {
    case Helper::TM_None:
        {
            BeatGroup& bg = this->beatGroups[i];
            qreal about = bg.rect.left();
            qreal w = bg.cellWidth();
            qreal radius = bg.cellWidth()/5;
            return QRectF(about+w*j+radius,bg.rect.top()-10, w-2*radius,60);
        }
        break;
    case Helper::TM_Anchor:
        {
            return QRectF();
        }
        break;
    case Helper::TM_Create:
        {
            return QRectF();
        }
        break;
    case Helper::TM_Drag:
        {
            return QRectF();
        }
        break;
    case Helper::TM_Move:
        {
            return QRectF();
        }
        break;
    case Helper::TM_Select:
        {
            if (!this->pressed)
            {
                BeatGroup& bg = this->beatGroups[i];
                qreal about = bg.rect.left();
                qreal w = bg.cellWidth();
                return QRectF(about+w*j,bg.rect.top(), w,40);
            }
            else
            {
                return QRectF();
            }
        }
        break;
    default:
        {
            return QRectF();
        }
        break;
    }
}

QRectF Helper::getAnchorHitBox(int n)
{
    n++;//to stop warning
    return QRectF();
}

void Helper::paint(QPainter *painter, QPaintEvent *event, qreal elapsed)
{
    painter->fillRect(event->rect(), background);
    int w = this->parent->width();
    int h = this->parent->height();


    //painter->translate(100, 100);

    painter->setPen(scorePen);

    qreal diff=scale*100;
    {
        QVector<QPointF> lines;
        for (qreal f=fmod(this->focus.x()*scale,diff);f<w;f+=diff)
        {
            lines.push_back(QPointF(f,0));
            lines.push_back(QPointF(f,h));
        }

        for (qreal f=fmod(this->focus.y()*scale,diff);f<h;f+=diff)
        {
            lines.push_back(QPointF(0,f));
            lines.push_back(QPointF(w,f));
        }

        painter->drawLines(lines);
    }


    painter->setTransform(QTransform(scale,0,0,scale,this->focus.x()*scale,this->focus.y()*scale));

    //draw bounds
    if (this->beatGroups.size()>0)
    {
        painter->setPen(boundsPen);
        painter->setBrush(clearBrush);

        painter->drawRect(bounds);


        if (playing)
        {
            qreal oldplaypos=this->playpos;
            this->playpos+=elapsed/100.0;
            /*
            for (int i=0;i<this->beatGroups.size();i++)
            {
                BeatGroup& bg = this->beatGroups[i];
                for (int j=0;j<bg.beats.size();j++)
                {
                    qreal t = bg.getCellX(j);
                    if (oldplaypos<t && t<=playpos)
                    {

                    }
                }
            }*/
            painter->setPen(playPen);
            painter->drawLine(QPoint(this->playpos+this->bounds.left(),-this->focus.y()),QPoint(this->playpos+this->bounds.left(),-this->focus.y() + this->parent->height()/this->scale));

//            if (this->playpos>this->bounds.width())
//                playpos=0;
        }
    }

    {
        //draw anchors
        QVector<QPointF> lines;
        for (int i=0;i<this->anchors.size();i++)
        {
                QPoint a1 = anchors[i].first;
                QPoint a2 = anchors[i].second;
                qreal fromx  = this->beatGroups[a1.x()].getCellX(a1.y());

                qreal y1 = this->beatGroups[a1.x()].rect.y();
                qreal y2 = this->beatGroups[a2.x()].rect.y();
                lines.push_back(QPointF(fromx,y1));
                lines.push_back(QPointF(fromx,y2));
                if (y1<y2)
                {
                    y1+=40.0;
                    qreal arrHeight=std::min(8.0/this->scale,y2-y1);
                    lines.push_back(QPointF(fromx,y2));
                    lines.push_back(QPointF(fromx-3/this->scale,y2-arrHeight));
                    lines.push_back(QPointF(fromx,y2));
                    lines.push_back(QPointF(fromx+3/this->scale,y2-arrHeight));
                }
                else if (y1>y2)
                {
                    y2+=40;
                    qreal arrHeight=std::min(8.0/this->scale,y1-y2);
                    lines.push_back(QPointF(fromx,y2));
                    lines.push_back(QPointF(fromx-3/this->scale,y2+arrHeight));
                    lines.push_back(QPointF(fromx,y2));
                    lines.push_back(QPointF(fromx+3/this->scale,y2+arrHeight));
                }
        }
        painter->setPen(anchorPen);
        painter->drawLines(lines);
    }

    //draw boxes

    QVector<QPoint> locselection = this->selection.getSelection();

    QVector<QRectF> rects;
    QVector<QRectF> selectedRects;
    QVector<QRectF> filledRects;
    for (int i=0;i<beatGroups.size();i++)
    {        
        QRectF rect = beatGroups[i].rect;
        qreal left = rect.x();
        qreal cellWidth = rect.width()/beatGroups[i].beats.size();
        rect.setWidth(cellWidth);
        for (int j=0;j<beatGroups[i].beats.size();j++)
        {
            rect.setX(left+cellWidth*j);
            rect.setWidth(cellWidth);

            if (locselection.contains(QPoint(i,j)))
            {
                selectedRects.push_back(rect);
            }
            else
            {
                rects.push_back(rect);
            }

            if (beatGroups[i].beats[j].sample>=0)
            {
                QRectF newrect=rect;
                newrect.moveTopLeft(newrect.topLeft()+(newrect.bottomRight()-newrect.topLeft())/4);
                newrect.setWidth(newrect.width()/2);
                newrect.setHeight(newrect.height()/2);
                filledRects.push_back(newrect);
            }
        }
    }

    painter->setBrush(boxBrush);
    painter->setPen(boxPen);
    painter->drawRects(rects);

    painter->setBrush(boxSelectedBrush);
    painter->drawRects(selectedRects);

    painter->setBrush(clearBrush);
    painter->setPen(boxPen);
    painter->drawRects(filledRects);

    if (this->pressed)
    {        
        QRectF dragRect = QRectF(this->dragFrom,this->cursorPos);

        switch (toolmode)
        {
        case TM_Select:
            {
                painter->setBrush(circleBrush);
                painter->setPen(circlePen);

                painter->drawRect(dragRect);
            }
            break;
        case TM_Create:
            {
                painter->setBrush(createBrush);
                painter->setPen(createPen);
                dragRect.setHeight(40);
                painter->drawRect(dragRect);
            }
            break;
        case TM_Drag:
            {

            }
            break;
        case TM_Anchor:
            {
                painter->setPen(circlePen);
                BeatGroup bg = this->beatGroups[this->anchorFrom.x()];
                qreal cellWidth = bg.cellWidth();
                qreal x = bg.rect.x()+cellWidth*this->anchorFrom.y();
                qreal y = bg.rect.y()>this->cursorPos.y() ? bg.rect.y() : bg.rect.y()+bg.rect.height();
                painter->drawLine(QPoint(x,y),QPoint(x,this->cursorPos.y()));
            }
            break;
        case TM_Move:
            {

            }
            break;
        case TM_None:
            {

            }
            break;
        }

    }


    //draw highlights
    switch (this->hovering.type)
    {
    case Highlight::HT_None:
        break;
    case Highlight::HT_Edge:
        {
            painter->setBrush(highlightBrush);
            painter->setPen(clearPen);
            QRectF rect = this->getEdgeHitBox(this->hovering.edge.x(),this->hovering.edge.y());
            painter->drawRect(rect);
        }
        break;
    case Highlight::HT_Cell:
        {
            painter->setBrush(highlightBrush);
            painter->setPen(clearPen);
            QRectF rect = this->getCellHitBox(this->hovering.cell.x(),this->hovering.cell.y());
            painter->drawRect(rect);
        }
        break;
    case Highlight::HT_Anchor:
        break;
    }

//    painter->setPen(textPen);
//    painter->setFont(textFont);
//    painter->drawText(QRect(-50, -50, 100, 100), Qt::AlignCenter, "Qt");
}
