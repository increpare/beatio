#ifndef GENERATOR_H
#define GENERATOR_H

#include <QIODevice>

#define BUFFER_SIZE 32768
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SYSTEM_FREQ 44100
//
///*
//class Generator : public QIODevice
//{
//    Q_OBJECT
//public:
//    Generator(QObject *parent,int* _buffer, int _buffer_size);
//    ~Generator();
//
//    void start();
//    void stop();
//
//    char *t;
//    int  len;
//    int  pos;
//    int  total;
//    char *buffer;
//    bool finished;
//    int  chunk_size;
//    int  total_size;
//
//
//    qint64 readData(char *data, qint64 maxlen);
//    qint64 writeData(const char *data, qint64 len);
//
//private:
//    int putShort(char *t, unsigned int value);
//    int fillData(char *start,int* _intbuffer,int _buffer_size);
//};*/
//
//class Generator : public QIODevice
//{
//    Q_OBJECT
//public:
//    Generator(QObject *parent,int* _buffer, int _buffer_size);
//    ~Generator();
//
//    void start();
//    void stop();
//
//    char *t;
//    int  len;
//    int  pos;
//    int  total;
//    char *buffer;
//    bool finished;
//    int  chunk_size;
//    int total_len;
//
//    qint64 readData(char *data, qint64 maxlen);
//    qint64 writeData(const char *data, qint64 len);
//
//private:
//    int putShort(char *t, unsigned int value);
//    int fillData(char *start, int frequency, int seconds,int* _buffer, int buffer_size);
//};

class Helper;

class Generator : public QIODevice
{
    Q_OBJECT
public:
    Generator(QObject *parent, Helper* helper, int* buffer,int framecount);
    ~Generator();

    char *buffer;
    int  pos;
    int  framecount;

    bool done;

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

    void start();
    void stop();
private:
    Helper* helper;
};

#endif // GENERATOR_H
