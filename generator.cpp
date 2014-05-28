#include "generator.h"
#include <math.h>
#include <iostream>
#include "helper.h"
///*
//Generator::Generator(QObject *parent,int* _intbuffer, int _buffer_size)
//    :QIODevice( parent )
//{
//
//    total_size=_buffer_size;
//    finished = false;
//    buffer = new char[_buffer_size*2];//just try it out
//    t=buffer;
//    len=fillData(t,_intbuffer,_buffer_size);
//    pos   = 0;
//    total = len;
//}
//
//Generator::~Generator()
//{
//    delete [] buffer;
//}
//
//void Generator::start()
//{
//    open(QIODevice::ReadOnly);
//}
//
//void Generator::stop()
//{
//    close();
//}
//
//int Generator::putShort(char *t, unsigned int value)
//{
//    *(unsigned char *)(t++)=value&255;
//    *(unsigned char *)(t)=(value/256)&255;
//    return 2;
//}
//
//int Generator::fillData(char *start,int* _intbuffer,int _buffer_size)
//{
//
//    int i, len=0;
//    int value;
//    for(i=0; i<_buffer_size; i++) {
//        value=(int)(32767.0*sin(2.0*M_PI*((double)(i))*(double)(400.0)/SYSTEM_FREQ));
//        putShort(start, value);
//        start += 4;
//        len+=2;
//        _intbuffer++;
//    }
//    return len;
//
//}
//
//qint64 Generator::readData(char *data, qint64 maxlen)
//{
//    int len = maxlen;
//    if (len > 16384)
//        len = 16384;
//
//    if (len < (SECONDS*SYSTEM_FREQ*2)-pos) {
//        // Normal
//        memcpy(data,t+pos,len);
//        pos+=len;
//        return len;
//    } else {
//        // Whats left and reset to start
//        qint64 left = (SECONDS*SYSTEM_FREQ*2)-pos;
//        memcpy(data,t+pos,left);
//        pos=0;
//        return left;
//    }
//}
//
//qint64 Generator::writeData(const char *data, qint64 len)
//{
//    Q_UNUSED(data);
//    Q_UNUSED(len);
//
//    return 0;
//}*/
//
//
//
//Generator::Generator(QObject *parent,int* _buffer, int _buffer_size)
//
//    :QIODevice( parent )
//{
//    /*
//    finished = false;
//    buffer = new char[SECONDS*SYSTEM_FREQ*4+1000];
//    t=buffer;
//    len=fillData(t,FREQ,SECONDS,_buffer,_buffer_size); // mono FREQHz sine
//    pos   = 0;
//    total = len;*/
//    finished = false;
//    buffer = new char[SECONDS*SYSTEM_FREQ*4+1000];
//    t=buffer;
//    len=fillData(t,FREQ,SECONDS,0,0); /* mono FREQHz sine */
//    pos   = 0;
//    total = len;
//}
//
//Generator::~Generator()
//{
//    delete [] buffer;
//}
//
//void Generator::start()
//{
//    open(QIODevice::ReadOnly);
//}
//
//void Generator::stop()
//{
//    close();
//}
//
//int Generator::putShort(char *t, unsigned int value)
//{
//    *(unsigned char *)(t++)=value&255;
//    *(unsigned char *)(t)=(value/256)&255;
//    return 2;
//}
//
//int Generator::fillData(char *start, int frequency, int seconds,int* _buffer, int buffer_size)
//{
//    /*
//    std::cout<<"BUFFER2"<<_buffer<<std::endl;
//    int i, len=0;
//    int value;
//    for(i=0; i<buffer_size; i++) {
//        value=(int)(32767.0*sin(2.0*M_PI*((double)(i))*(double)(frequency)/SYSTEM_FREQ));
//        _buffer++;
//        putShort(start, value);
//        start += 4;
//        len+=2;
//    }
//    total_len=buffer_size;
//    return len;*/
//    int i, len=0;
//    int value;
//    for(i=0; i<seconds*SYSTEM_FREQ; i++) {
//        value=(int)(32767.0*sin(2.0*M_PI*((double)(i))*(double)(frequency)/SYSTEM_FREQ));
//        putShort(start, value);
//        start += 4;
//        len+=2;
//    }
//    return len;
//}
//
//qint64 Generator::readData(char *data, qint64 maxlen)
//{
//    int len = maxlen;
//    if (len > 16384)
//        len = 16384;
///*
//    if (len < (SECONDS*SYSTEM_FREQ*2)-pos) {
//        // Normal
//        memcpy(data,t+pos,len);
//        pos+=len;
//        return len;
//    } else {
//        // Whats left and reset to start
//        qint64 left = (SECONDS*SYSTEM_FREQ*2)-pos;
//        memcpy(data,t+pos,left);
//        pos=0;
//        return left;
//    }*/
//}
//
//qint64 Generator::writeData(const char *data, qint64 len)
//{
//    Q_UNUSED(data);
//    Q_UNUSED(len);
//
//    return 0;
//}
//


int putShort(char *t, unsigned int value)
{
    *(unsigned char *)(t++)=value&255;
    *(unsigned char *)(t)=(value/256)&255;
    return 2;
}

Generator::Generator(QObject *parent, Helper *helper, int* _buffer, int _framecount)
    :QIODevice( parent )
{
    this->helper=helper;
    done=false;
    pos=0;

    buffer = new char[_framecount*4];
    char* start=buffer;
    for (int i=0;i<_framecount*2;i++)
    {
        //int value=(int)(32767.0*sin(2.0*M_PI*((double)(i))*(double)(400)/SYSTEM_FREQ));
        int value = _buffer[i];
        //putShort(start, value&65535);
        putShort(start, (value/65536)&65535);
        start += 2;
    }

    std::cout<<"DELETE BUFFER2"<<std::endl;
    delete [] _buffer;
    _buffer=0;
    this->pos=0;
    this->framecount=_framecount;
}

Generator::~Generator()
{
    if (buffer)
    {
        std::cout<<"DELETE BUFFER3"<<std::endl;
        delete [] buffer;
        buffer=0;
    }
}

void Generator::start()
{
    open(QIODevice::ReadOnly);
}

void Generator::stop()
{
    close();
}

qint64 Generator::readData(char *data, qint64 maxlen)
{
    int len = maxlen;
    if (len > framecount*4)
        len = framecount*4;

    if (len < (framecount*4)-pos) {
        // Normal
        memcpy(data,buffer+pos,len);
        pos+=len;
        return len;
    } else {
        // Whats left and reset to start
        qint64 left = (framecount*4)-pos;
        memcpy(data,buffer+pos,left);
        pos=0;
        std::cout<<"BLIP";
        this->helper->playpos=BUFFER_SIZE/SYSTEM_FREQ;
        return left;
    }
}

qint64 Generator::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

