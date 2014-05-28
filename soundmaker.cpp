#include "soundmaker.h"

extern "C" {
    #include <sndfile.h>
}

#include <math.h>
#include <iostream>
#include <QFileInfo>

using namespace std;

#ifndef		M_PI
#define		M_PI		3.14159265358979323846264338
#endif

#define		SAMPLE_RATE			44100
#define		AMPLITUDE			(0.5 * 0x7F000000)
#define		LEFT_FREQ			(344.0 / SAMPLE_RATE)
#define		RIGHT_FREQ			(466.0 / SAMPLE_RATE)

SoundMaker::SoundMaker()
{

}

SoundMaker::~SoundMaker()
{

}

bool SoundMaker::SaveWave(QVector<Note> onsets,const QString& filename)
{
    int temp;
    int* buffer = this->GenerateWave(onsets,temp);
    int length = onsets.size()>0 ? ceil(onsets.last().onset)+1 : 0;

    int sample_count  = SAMPLE_RATE*length;

    SNDFILE	*file ;
    SF_INFO	sfinfo ;

    memset (&sfinfo, 0, sizeof (sfinfo)) ;
    sfinfo.samplerate	= SAMPLE_RATE ;
    sfinfo.frames		= sample_count ;
    sfinfo.channels		= 2 ;
    sfinfo.format		= (SF_FORMAT_WAV | SF_FORMAT_PCM_16);

    // open file for writing
    if (! (file = sf_open (filename.toLatin1().data(), SFM_WRITE, &sfinfo)))
    {	cout << "Error : Not able to open output file.\n";
        return false ;
    } ;

    if (sf_write_int (file, buffer, sfinfo.channels * sample_count) !=
        sfinfo.channels * sample_count)
        puts (sf_strerror (file)) ;

    sf_close (file) ;

    std::cout<<"DELETE BUFFER5"<<std::endl;
    delete [] buffer;
    buffer=0;

    return true;
}

void SoundMaker::unloadInstrument(int i)
{
    instruments.remove(i);
}

void SoundMaker::unloadInstruments()
{
    instruments.empty();
}

bool SoundMaker::loadInstrument(const QString& samplefilename)
{
    SNDFILE		*infile;

    SF_INFO sfinfo;
    memset (&sfinfo, 0, sizeof (sfinfo)) ;

    if (! (infile = sf_open (samplefilename.toLatin1(), SFM_READ, &sfinfo)))
    {
        printf ("Error : could not open file : %s\n", samplefilename.toLatin1().data()) ;
        puts (sf_strerror (NULL)) ;
        exit (1) ;
    }

    if (! sf_format_check (&sfinfo) || !(sfinfo.channels==1||sfinfo.channels==2) || (sfinfo.samplerate!=SAMPLE_RATE))
    {
        sf_close (infile) ;

        printf ("Invalid encoding\n") ;
        return false;
    }

    QVector<int> sample;

    int *buf = new int[sfinfo.frames*2];

    int num = sf_read_int(infile,buf,sfinfo.frames*sfinfo.channels);

    if (sfinfo.samplerate==2)
    {
        for (int i=0;i<num;i++)
        {
            sample.push_back(buf[i]);
        }
    }
    else //(sfinfo.samplerate==1)
    {
        for (int i=0;i<num;i++)
        {
            sample.push_back(buf[i]);
            sample.push_back(buf[i]);
        }
    }

    std::cout<<"DELETE BUFFER4"<<std::endl;
    delete [] buf;
    buf=0;

    sf_close (infile);

    this->instruments.push_back(sample);
    QFileInfo fileInfo = QFileInfo(samplefilename);
    this->names.push_back(fileInfo.baseName());
    return true;
}

int* SoundMaker::GenerateWave(QVector<Note> onsets, int& framecount)
{
    /* very nasty...load sample into wav */
    if (onsets.size()==0)
        return false;

    int length = ceil(onsets.last().onset)+1;
    int frame_count  = SAMPLE_RATE*length;
    framecount=frame_count;

    SF_INFO	sfinfo ;
    int		k ;
    int	*buffer ;

    buffer = new int[2 * frame_count];

    memset (&sfinfo, 0, sizeof (sfinfo)) ;
    sfinfo.samplerate	= SAMPLE_RATE ;
    sfinfo.frames		= frame_count ;
    sfinfo.channels		= 2 ;
    sfinfo.format		= (SF_FORMAT_WAV | SF_FORMAT_PCM_24);

    int cur=0;

    QVector< NoteN > notePointers;

    for (k = 0 ; k < frame_count ; k++)
    {

        for (int i=notePointers.size()-1;i>=0;i--)
        {
            NoteN& point = notePointers[i];
            point.onset++;
            if (2*notePointers[i].onset>=instruments[point.instrument].size()-1)
            {
                notePointers.remove(i);
            }
        }

        while (cur<onsets.size() && (onsets[cur].onset*SAMPLE_RATE)<k)
        {
            if (onsets[cur].instrument>=0)
            {
                NoteN newNote = onsets[cur];
                newNote.onset=0;
                notePointers.push_back(newNote);
            }
            cur++;
        }

        int l=0;
        int r=0;

        for (int i=notePointers.size()-1;i>=0;i--)
        {
            int oldl=l;
            l+=instruments[notePointers[i].instrument][2*notePointers[i].onset];
            if ((l<0)!=(oldl<0) && ((oldl<0) == (instruments[notePointers[i].instrument][2*notePointers[i].onset]<0)))
            {
                if (l!=0 && oldl!=0)
                {
                    //std::cout<<"L"<<l<<","<<oldl<<","<<instruments[notePointers[i].instrument][2*notePointers[i].onset]<<std::endl;
                    if (oldl>0)
                        l=INT_MAX;
                    else
                        l=INT_MIN;
                }
            }
            int oldr=r;
            r+=instruments[notePointers[i].instrument][2*notePointers[i].onset+1];
            if ((r<0)!=(oldr<0) && ((oldr<0) == (instruments[notePointers[i].instrument][2*notePointers[i].onset+1]<0)))
            {
                if (r!=0 && oldr!=0)
                {
                    //std::cout<<"R"<<r<<","<<oldr<<","<<instruments[notePointers[i].instrument][2*notePointers[i].onset+1]<<std::endl;
                    if (oldr>0)
                        r=INT_MAX;
                    else
                        r=INT_MIN;
                }
            }
        }

        buffer [2 * k] = l;
        buffer [2 * k + 1] = r;
    }

    std::cout<<"BUFFER4"<<buffer<<std::endl;
    return buffer;
}

