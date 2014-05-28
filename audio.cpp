#include "audio.h"

#include <QDebug>
#include <QAudioOutput>
#include <QAudioDeviceInfo>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SECONDS     1
#define FREQ        600
#define SYSTEM_FREQ 44100

Generator::Generator(QObject *parent)
    :QIODevice( parent )
{
    finished = false;
    buffer = new char[SECONDS*SYSTEM_FREQ*4+1000];
    t=buffer;
    len=fillData(t,FREQ,SECONDS); /* mono FREQHz sine */
    pos   = 0;
    total = len;
}

Generator::~Generator()
{
    delete [] buffer;
}

void Generator::start()
{
    open(QIODevice::ReadOnly);
}

void Generator::stop()
{
    close();
}

int Generator::putShort(char *t, unsigned int value)
{
    *(unsigned char *)(t++)=value&255;
    *(unsigned char *)(t)=(value/256)&255;
    return 2;
}

int Generator::fillData(char *start, int frequency, int seconds)
{
    int i, len=0;
    int value;
    for(i=0; i<seconds*SYSTEM_FREQ; i++) {
        value=(int)(32767.0*sin(2.0*M_PI*((double)(i))*(double)(frequency)/SYSTEM_FREQ));
        putShort(start, value);
        start += 4;
        len+=2;
    }
    return len;
}

qint64 Generator::readData(char *data, qint64 maxlen)
{
    int len = maxlen;
    if (len > 16384)
        len = 16384;

    if (len < (SECONDS*SYSTEM_FREQ*2)-pos) {
        // Normal
        memcpy(data,t+pos,len);
        pos+=len;
        return len;
    } else {
        // Whats left and reset to start
        qint64 left = (SECONDS*SYSTEM_FREQ*2)-pos;
        memcpy(data,t+pos,left);
        pos=0;
        return left;
    }
}

qint64 Generator::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

Audio::Audio(QWidget *parent)
{
    this->parent=parent;

    buffer = new char[BUFFER_SIZE];

    gen = new Generator(parent);

    pullMode = parent;

    gen->start();

    settings.setFrequency(SYSTEM_FREQ);
    settings.setChannels(1);
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
        button->setDisabled(true);
        button2->setDisabled(true);
        audioOutput = 0;
        return;
    }

    audioOutput = new QAudioOutput(settings,parent);
    parent->connect(audioOutput,SIGNAL(notify()),SLOT(status()));
    parent->connect(audioOutput,SIGNAL(stateChanged(QAudio::State)),SLOT(state(QAudio::State)));

    audioOutput->start(gen);
}

Audio::~Audio()
{
    delete [] buffer;
}

void Audio::deviceChanged(int idx)
{
    timer->stop();
    gen->stop();
    audioOutput->stop();
    audioOutput->disconnect(this);
    delete audioOutput;

    device = deviceBox->itemData(idx).value<QAudioDeviceInfo>();
    audioOutput = new QAudioOutput(device,settings,this);
    connect(audioOutput,SIGNAL(notify()),SLOT(status()));
    connect(audioOutput,SIGNAL(stateChanged(QAudio::State)),SLOT(state(QAudio::State)));
    gen->start();
    audioOutput->start(gen);
}

void Audio::status()
{
    qWarning() << "byteFree = " << audioOutput->bytesFree() << " bytes, elapsedUSecs = " << audioOutput->elapsedUSecs() << ", processedUSecs = " << audioOutput->processedUSecs();
}

void Audio::writeMore()
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

void Audio::toggle()
{
    // Change between pull and push modes

    timer->stop();
    audioOutput->stop();

    if (pullMode) {
        button->setText("Click for Pull Mode");
        output = audioOutput->start();
        pullMode = false;
        timer->start(20);
    } else {
        button->setText("Click for Push Mode");
        pullMode = true;
        audioOutput->start(gen);
    }
}

void Audio::togglePlay()
{
    // toggle suspend/resume
    if (audioOutput->state() == QAudio::SuspendedState) {
        qWarning() << "status: Suspended, resume()";
        audioOutput->resume();
        button2->setText("Click To Suspend");
    } else if (audioOutput->state() == QAudio::ActiveState) {
        qWarning() << "status: Active, suspend()";
        audioOutput->suspend();
        button2->setText("Click To Resume");
    } else if (audioOutput->state() == QAudio::StoppedState) {
        qWarning() << "status: Stopped, resume()";
        audioOutput->resume();
        button2->setText("Click To Suspend");
    } else if (audioOutput->state() == QAudio::IdleState) {
        qWarning() << "status: IdleState";
    }
}

void Audio::state(QAudio::State state)
{
    qWarning() << " state=" << state;
}
