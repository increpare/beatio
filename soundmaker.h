#ifndef SOUNDMAKER_H
#define SOUNDMAKER_H

#include <QString>
#include <QVector>

#include "generator.h"

struct Note
{
    int instrument;
    qreal onset;
    qreal volumn;
    qreal pan;

    bool operator<(const Note& other) const
    {
        return this->onset<other.onset;
    }
};

struct NoteN
{
    int instrument;
    int onset;
    qreal volumn;
    qreal pan;

    bool operator<(const Note& other) const
    {
        return this->onset<other.onset;
    }

    NoteN()
    {
        instrument=0;
        onset=0;
        volumn=1;
        pan=0.5;
    };

    NoteN(Note n):
            instrument(n.instrument),
            onset(n.onset),
            volumn(n.volumn),
            pan(n.pan){};

};

class SoundMaker
{
public:
    bool SaveWave(QVector<Note> onsets,const QString& filename);

    bool loadInstrument(const QString& filename);
    void unloadInstrument(int i);
    void unloadInstruments();

    QVector<QVector<int> > instruments;
    QVector<QString > names;

private:
    Generator* gen;
public:
    int* GenerateWave(QVector<Note> onsets, int& framecount);
public:
    SoundMaker();
    ~SoundMaker();

};

#endif // SOUNDMAKER_H
