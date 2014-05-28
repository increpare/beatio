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

#ifndef HELPER_H
#define HELPER_H

#include <QBrush>
#include <QFont>
#include <QPen>
#include <QStack>
#include <QSound>
#include <iostream>

#include "soundmaker.h"

QT_BEGIN_NAMESPACE
class QPainter;
class QPaintEvent;
QT_END_NAMESPACE


class BeatGroup;

class Beat {
public:
    qreal vol;
    qreal pan;
    int sample;
    QVector<BeatGroup*> ancestry();    
    Beat(qreal _vol, qreal _pan, int _sample):
            vol(_vol),pan(_pan),sample(_sample){};
    Beat():vol(1),pan(0.5),sample(-1){};
};

class BeatGroup {
public:
    QRectF rect;
    qreal y;
    QVector<Beat> beats;
    int nLevel;


    qreal cellWidth()
    {
        return rect.width()/beats.size();
    }

    qreal getCellX(int n)
    {
        return rect.x() + cellWidth()*n;
    }
};

class GLWidget;

typedef QPair<QPoint, QPoint> Anchor;


struct AnchorTree {
   int label;

   QVector<AnchorTree*> children;

   AnchorTree* searchFor(int toFind)
   {
       if (this->label==toFind)
       {
           return this;
       }
       else
       {
           for (int i=0;i<children.size();i++)
           {
               AnchorTree* found = children[i]->searchFor(toFind);
               if (found)
                   return found;
           }
       }

       return 0;
   }

   AnchorTree() {};
   AnchorTree(int _label)  : label(_label){};
};


struct State {
    QVector<BeatGroup> beatGroups;
    QVector<QPair<QPoint,QPoint> > anchors;
};


struct Highlight {
    enum { HT_None, HT_Edge, HT_Cell, HT_Anchor } type;
    QPoint cell;
    QPoint edge;
    Anchor* anchor;
};

class Helper;

class SelectionData
{
public:
    SelectionData(Helper* parent)
        :_parent(parent){};

    void set(const QVector<QPoint>& selection);
    void push_back(const QPoint& newPoint);

    void clear() {set(QVector<QPoint>()); };

    QVector<QPoint> getSelection()
    {
        return selection;
    }

private:
    QVector<QPoint> selection;
    Helper* _parent;
    void update();
};

//! [0]
class Helper
{
public:
    Helper();


    Beat defaultBeat;
    int defaultBeatCount;

public:
    SelectionData selection;
    SoundMaker soundMaker;

    QVector<Note> generateOnsets();
    QStack<State> stack;

    bool performingoperation;

    void beginTransaction();
    void endTransaction();
    void terminateTransaction();
    void cacheState();
    void undo();

    Highlight hovering;

    void paint(QPainter *painter, QPaintEvent *event, qreal elapsed);

    enum ToolMode {TM_None,TM_Select,TM_Create,TM_Drag, TM_Anchor, TM_Move} toolmode;

    void onBeatGroupsChanged();
    void onAnchorsChanged();
    bool onAnchorsChangedInner();
    bool setDockingPositions(AnchorTree& tree);

    bool canAdd(const Anchor& anchor);
    bool dragging;

    void deleteSelection();

    bool tryGenForest();

    QVector<BeatGroup> beatGroups;

    void InsertBeatAfter(const QPoint& p);
    void RemoveBeat(const QPoint& p);
    void ClearEmptyBeatGroups();

    QVector<QPointF> selectionDiffs;

    QVector <int> movingTargets;

    QVector<AnchorTree> forest;

    QVector<QPair<QPoint,QPoint> > anchors;

    QPointF dragFrom;
    QPointF cursorPos;

    QPointF focus;
    qreal scale;
    qreal bpm;

    QRectF bounds;

    QPoint anchorFrom;

    bool pressed;

    qreal playpos;
    bool playing;

    GLWidget* parent;

    QRectF getEdgeHitBox(int i,int j);
    QRectF getCellHitBox(int i,int j);
    QRectF getAnchorHitBox(int n);

private:
    QBrush background;
    QBrush circleBrush;
    QPen circlePen;
    QBrush boxBrush;
    QBrush boxSelectedBrush;
    QBrush clearBrush;
    QPen clearPen;
    QPen boxPen;

    QPen boundsPen;
    QPen playPen;
    QPen anchorPen;

    QBrush highlightBrush;

    QBrush createBrush;
    QPen createPen;
    QFont textFont;
    QPen scorePen;
    QPen textPen;
    
};
//! [0]

#endif
