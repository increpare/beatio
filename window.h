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

#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAudioOutput>
#include <QAudioDeviceInfo>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QListWidget>
#include <QComboBox>
#include <QListWidgetItem>

#include "helper.h"
#include "glwidget.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QWidget;
QT_END_NAMESPACE

//! [0]
class Window : public QWidget
{
    Q_OBJECT

public:
    Window();




    QPushButton* plusButton;
    QLabel *cellCountLabel;
    QPushButton* minusButton;

    QSlider *volumeSlider;
    QSlider *panningSlider;
    QListWidget *soundSelector;
    QPushButton *playButton;
    QListWidgetItem *previousSelectedListWidgetItem;

    void repopulateInstrumentList();

protected:
    void keyPressEvent( QKeyEvent * event);

public slots:

    void setPlay();
    void setSave();
    void setViewAll();
    void selectInstrument();

private slots:
    void status();
    void writeMore();
    void moreBeats();
    void fewerBeats();
    void addInstrument();
    void state(QAudio::State s);
    void listWidgetItemClicked( QListWidgetItem *item );

private:
    QAudioDeviceInfo  device;
    Generator*        gen;
    QAudioOutput*     audioOutput;
    QIODevice*        output;
    QAudioFormat      settings;
    QTimer*           timer;
    char*             buffer;

    QMenuBar *menuBar;
    QMenu *fileMenu;
    QAction *exitAction;
    GLWidget *openGL;

    Helper helper;

    void createMenu();
};
//! [0]

#endif
