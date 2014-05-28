#ifndef AUDIO_H
#define AUDIO_H




#define BUFFER_SIZE 32768

#include <QObject>
#include <QMainWindow>
#include <QIODevice>
#include <QTimer>
#include <QPushButton>
#include <QComboBox>

#include <QAudioOutput>
#define BUFFER_SIZE 32768

class Generator : public QIODevice
{
public:
    Generator(QObject *parent);
    ~Generator();

    void start();
    void stop();

    char *t;
    int  len;
    int  pos;
    int  total;
    char *buffer;
    bool finished;
    int  chunk_size;

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

private:
    int putShort(char *t, unsigned int value);
    int fillData(char *start, int frequency, int seconds);
};


class Audio
{
public:
    Audio(QWidget*);
    ~Audio();
    void tick(qreal elapsed);
    void addsound();

public:

    QAudioDeviceInfo  device;
    Generator*        gen;
    QAudioOutput*     audioOutput;
    QIODevice*        output;
    QTimer*           timer;
    QAudioFormat      settings;

    bool              pullMode;
    char*             buffer;

    QPushButton*      button;
    QPushButton*      button2;
    QComboBox*     deviceBox;

    void writeMore();
    void togglePlay();

    QWidget *parent;
};

#endif // AUDIO_H
