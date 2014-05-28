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
#include <QSplitter>
#include <QDialog>

#include "widget.h"
#include "window.h"
#include <iostream>

#include <QAudioOutput>
#include <QBoxLayout>
#include <QSpacerItem>
#include <QGroupBox>
#include <QFileDialog>

int computeListViewMinimumWidth(QAbstractItemView* view) {
int minWidth = 0;
QAbstractItemModel* model = view->model();

// Too bad view->viewOptions() is protected
// FIXME: Handle the case where text is below icon
QStyleOptionViewItem option;
option.decorationSize = view->iconSize();

int rowCount = model->rowCount();
for (int row = 0; row<rowCount; ++row) {
QModelIndex index = model->index(row, 0);
QSize size = view->itemDelegate()->sizeHint(option, index);
minWidth = qMax(size.width(), minWidth);
}
minWidth += 2 * view->frameWidth();
return minWidth;
}

//! [0]
Window::Window()
    : QWidget()
{
        previousSelectedListWidgetItem=0;
    gen=0;

        settings.setSampleRate(SYSTEM_FREQ);
        settings.setChannelCount(2);
        settings.setSampleSize(16);
        settings.setCodec("audio/pcm");
        settings.setByteOrder(QAudioFormat::LittleEndian);
        settings.setSampleType(QAudioFormat::SignedInt);

        QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
        if (!info.isFormatSupported(settings)) {
            qWarning()<<"default format not supported try to use nearest";
            settings = info.nearestFormat(settings);
        }

        if(settings.sampleSize() != 16) {
            qWarning()<<"audio device doesn't support 16 bit samples, example cannot run";
            audioOutput = 0;
            return;
        }

        audioOutput = new QAudioOutput(settings,this);
        audioOutput->setBufferSize(BUFFER_SIZE);
        
        connect(audioOutput,SIGNAL(notify()),SLOT(status()));
        connect(audioOutput,SIGNAL(stateChanged(QAudio::State)),SLOT(state(QAudio::State)));

        timer = new QTimer(this);
        connect(timer,SIGNAL(timeout()),SLOT(writeMore()));









    // Set up overall layout
    QBoxLayout  *layout = new QBoxLayout(QBoxLayout::TopToBottom);
    layout->setSpacing(1);
    layout->setMargin(1);
    setLayout(layout);

    // Add menu bar

    // WHY ISN'T THIS WORKING?!??
    this->createMenu();
    layout->setMenuBar(this->menuBar);

    // Buttons on top
    QHBoxLayout *toolbar = new QHBoxLayout();
    toolbar->setSpacing(1);
    toolbar->setMargin(1);
    layout->addLayout(toolbar,0);

    QPushButton *button1 = new QPushButton("&new");
    toolbar->addWidget(button1,0);
    QPushButton *button2 = new QPushButton("&save");
    connect(button2,SIGNAL(clicked()),this,SLOT(setSave()));
    toolbar->addWidget(button2,0);
    playButton = new QPushButton("&play");
    connect(playButton, SIGNAL(clicked()), this,SLOT(setPlay()));
    toolbar->addWidget(playButton,0);
    QPushButton *button4 = new QPushButton("&view all");
    connect(button4, SIGNAL(clicked()), this,SLOT(setViewAll()));
    toolbar->addWidget(button4,0);


    QSpacerItem *toolbarSpacer = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Expanding);
    toolbar->addSpacerItem(toolbarSpacer);

    // Splitter main occupies main area
    QHBoxLayout *splitter = new QHBoxLayout;
    layout->addLayout(splitter,1);

    // main area on the left
    openGL = new GLWidget(&helper, this);

    openGL->setMouseTracking(true);
    openGL->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    splitter->addWidget(openGL);

    // panel on the right
    QGroupBox* rightPanelBox = new QGroupBox();
    rightPanelBox->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding);
    rightPanelBox->setFixedWidth(200);
    splitter->addWidget(rightPanelBox);


    QVBoxLayout* rightPanel = new QVBoxLayout;
    rightPanel->setSpacing(1);
    rightPanel->setMargin(1);
    rightPanelBox->setLayout(rightPanel);
/*
    // Piece Properties
    QGroupBox *piecePropertiesBox = new QGroupBox();
    rightPanel->addWidget(piecePropertiesBox);

    QVBoxLayout *pieceProperties = new QVBoxLayout;
    piecePropertiesBox->setLayout(pieceProperties);

    QLabel *bpmLabel = new QLabel("beats per minute");
    pieceProperties->addWidget(bpmLabel);
    QSlider *bpmSlider = new QSlider(Qt::Horizontal);
    pieceProperties->addWidget(bpmSlider);
*/            
    QHBoxLayout *plusMinusButtons = new QHBoxLayout;
    plusMinusButtons->setSpacing(1);
    plusMinusButtons->setMargin(1);
    rightPanel->addLayout(plusMinusButtons);

    minusButton = new QPushButton("&-");
    connect(minusButton,SIGNAL(clicked()),this,SLOT(fewerBeats()));
    plusMinusButtons->addWidget(minusButton);
    cellCountLabel = new QLabel("4");
    cellCountLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    cellCountLabel->setAlignment(Qt::AlignCenter);
    plusMinusButtons->addWidget(cellCountLabel);
    plusButton = new QPushButton("&+");
    connect(plusButton,SIGNAL(clicked()),this,SLOT(moreBeats()));
    plusMinusButtons->addWidget(plusButton);

    // Selection Properties
    QGroupBox *selectedPropertiesBox = new QGroupBox();
    //selectedPropertiesBox->setDisabled(true);
    rightPanel->addWidget(selectedPropertiesBox);

    QVBoxLayout *selectedProperties = new QVBoxLayout;
    selectedPropertiesBox->setLayout(selectedProperties);

    QLabel *volumeLabel = new QLabel("volume");
    selectedProperties->addWidget(volumeLabel);
    volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setStyleSheet("\
                                QSlider::groove:horizontal {\
                                    border: 1px solid #999999;\
                                    height: 8px; /* the groove expands to the size of the slider by default. by giving it a height, it has a fixed size */\
                                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B1B1B1, stop:1 #c4c4c4);\
                                    margin: 2px 0;\
                                } \
    ");
    selectedProperties->addWidget(volumeSlider);

    QLabel *panningLabel = new QLabel("panning");
    selectedProperties->addWidget(panningLabel);
    panningSlider = new QSlider(Qt::Horizontal);
    selectedProperties->addWidget(panningSlider);


    //    QLabel *sampleLabel = new QLabel("sound");
    //    selectedProperties->addWidget(sampleLabel);
    soundSelector = new QListWidget();
    soundSelector->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Expanding);
    //soundSelector->setFixedWidth(selectedProperties->geometry().width());
    rightPanel->addWidget(soundSelector);
    QPushButton *addInstrumentButton = new QPushButton("add new sample");
    rightPanel->addWidget(addInstrumentButton,0);
    connect(addInstrumentButton,SIGNAL(clicked()),this,SLOT(addInstrument()));

    connect( soundSelector, SIGNAL( itemClicked( QListWidgetItem * ) ),
                this, SLOT( listWidgetItemClicked( QListWidgetItem * )));

    //QSpacerItem *rightPanelSpacer = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Expanding);
    //rightPanel->addSpacerItem(rightPanelSpacer);


    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), openGL, SLOT(animate()));

    timer->start(50);

    setWindowTitle(tr("2D Painting on Native and OpenGL Widgets"));
}


#define SECONDS     1
#define FREQ        600
#define SYSTEM_FREQ 44100


void Window::setPlay()
{
    switch (audioOutput->state())
    {
    case QAudio::ActiveState:
        {
            if (gen)
            {
                this->helper.playing=false;
                std::cout<<"stopping=";
                audioOutput->stop();                
                delete gen;
                gen=0;
                playButton->setText("Play");
            }
            else
            {
                std::cout<<"OOPS1523"<<std::endl;
            }
        }
        break;
    default:
        {
            QVector<Note> onsets = this->helper.generateOnsets();
            if (onsets.size()>0)
            {
                if (gen)
                {
                    delete gen;
                    gen=0;
                }
                playButton->setText("Stop");

                std::cout<<"playing"<<std::endl;
                this->helper.playing=true;
                this->helper.playpos=0;
                //this->helper.soundMaker.loadInstrument("sound.wav");
                //std::cout<<"instrumentcount "<<helper.soundMaker.instruments.size()<<std::endl;

                int framecount;
                int* buffer = this->helper.soundMaker.GenerateWave(onsets,framecount);

                std::cout<<"NEW GEN1"<<std::endl;

                gen = new Generator(this,&this->helper,buffer,framecount);//,buffer,framecount*2);
                gen->start();
                audioOutput->start(gen);

            }
        }
        break;
    }

}




void Window::setSave()
{
    QVector<Note> onsets = this->helper.generateOnsets();
    if (onsets.size()>0)
    {
        QString fileName = QFileDialog::getSaveFileName(this,"Select samples to load","","(*.wav)");
        this->helper.soundMaker.SaveWave(onsets, fileName);
    }
}

void Window::keyPressEvent(QKeyEvent *event)
{
    if (!this->openGL)
        return;

    switch( event->key() )
    {
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
        this->openGL->helper->deleteSelection();
        break;
    default:
        break;
    }}

void Window::setViewAll()
{
    this->helper.focus = -this->helper.bounds.topLeft();
    this->helper.scale = std::min(static_cast<qreal>(this->openGL->width())/this->helper.bounds.width(),static_cast<qreal>(this->openGL->height())/this->helper.bounds.height());
    this->helper.playpos=0;
}

void Window::selectInstrument()
{

}

void Window::createMenu()
{
        menuBar = new QMenuBar;

        fileMenu = new QMenu(tr("&File"));
        exitAction = fileMenu->addAction(tr("E&xit"));
        menuBar->addMenu(fileMenu);

        //connect(exitAction, SIGNAL(triggered()), this, SLOT(accept()));
}
//! [1]

void Window::addInstrument()
{   /*
     dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter(trUtf8("Splits (*.wav)"));
    dialog.setLabelText("Select WAV file");
    QStringList fileNames;
    if (dialog.exec())*/
    {
        QStringList fileNames = QFileDialog::getOpenFileNames(this,"Select samples to load","","(*.wav)");
        for (int i=0;i<fileNames.size();i++)
        {
            QString selectedFile = fileNames[i];

            this->helper.soundMaker.loadInstrument(selectedFile);
        }
        repopulateInstrumentList();
    }
}

void Window::repopulateInstrumentList()
{
    this->soundSelector->clear();
    for (int i=0;i<this->helper.soundMaker.names.size();i++)
    {
        this->soundSelector->addItem(this->helper.soundMaker.names[i]);
    }
}


void Window::listWidgetItemClicked( QListWidgetItem *item )
{
    const QVector<QPoint> selection = this->helper.selection.getSelection();

    if( !item )
    {
        for (int i=0;i<this->soundSelector->selectedItems().size();i++)
        {
            this->soundSelector->selectedItems()[i]->setSelected(false);
        }
        previousSelectedListWidgetItem = 0;                
    }
    else
    {
        int index = this->helper.soundMaker.names.indexOf(item->text());

        if( previousSelectedListWidgetItem == item )
        {
            item->setSelected( false );
            previousSelectedListWidgetItem = 0;

            for (int i=0;i<selection.size();i++)
            {
                Q_ASSERT(index>=0);

                this->helper.beatGroups[selection[i].x()].beats[selection[i].y()].sample=-1;
            }
        }
        else
        {
            previousSelectedListWidgetItem = item;

            for (int i=0;i<selection.size();i++)
            {
                Q_ASSERT(index>=0);

                this->helper.beatGroups[selection[i].x()].beats[selection[i].y()].sample=index;
            }
        }
        this->helper.defaultBeat.sample=index;
    }
}


void Window::moreBeats()
{
    this->helper.defaultBeatCount++;
    this->cellCountLabel->setText(QString().setNum(this->helper.defaultBeatCount));
}


void Window::fewerBeats()
{
    this->helper.defaultBeatCount--;
    if (this->helper.defaultBeatCount<1)
            this->helper.defaultBeatCount=1;
    this->cellCountLabel->setText(QString().setNum(this->helper.defaultBeatCount));
}



void Window::status()
{
    qWarning() << "byteFree = " << audioOutput->bytesFree() << " bytes, elapsedUSecs = " << audioOutput->elapsedUSecs() << ", processedUSecs = " << audioOutput->processedUSecs();
}


void Window::writeMore()
{
    if (!audioOutput)
        return;

    if (audioOutput->state() == QAudio::StoppedState)
        return;

    int    l;
    int    out;

    int chunks = audioOutput->bytesFree()/audioOutput->periodSize();
    while(chunks) {
       l = gen->read(buffer,audioOutput->periodSize());
       if (l > 0)
           out = output->write(buffer,l);
       if (l != audioOutput->periodSize())
           break;
       chunks--;
    }
}

void Window::state(QAudio::State state)
{
    qWarning() << " state=" << state;
}






