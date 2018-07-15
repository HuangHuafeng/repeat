#include "mediaplayer.h"
#include "golddict/gddebug.hh"

MediaPlayer::MediaPlayer(): m_mediaPlayer(),
    m_filesToPlay()
{
    connect(&m_mediaPlayer,
            &QMediaPlayer::stateChanged,
            this,
            &MediaPlayer::stateChanged);
    connect(&m_mediaPlayer,
            QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
            this,
            &MediaPlayer::error);
}

void MediaPlayer::play(QString fileName)
{
    m_filesToPlay.append(fileName);
    playNextFile();
}

void MediaPlayer::playNextFile()
{
    if (m_filesToPlay.size() > 0 && m_mediaPlayer.state() == QMediaPlayer::StoppedState) {
        // play the next one if exits
        m_mediaPlayer.setMedia(QUrl::fromLocalFile(m_filesToPlay.at(0)));
        m_mediaPlayer.play();
    }
}

void MediaPlayer::stateChanged(QMediaPlayer::State state)
{
    //gdDebug("got state QMediaPlayer::stateChanged(%d)", state);
    //gdDebug("MediaStatus is %d in MediaPlayer::stateChanged()", m_mediaPlayer.mediaStatus());
    if (state == QMediaPlayer::StoppedState)
    {
        // remove the one just finished from the list
        m_filesToPlay.pop_front();
        playNextFile();
    } else {
        ;
    }
}

void MediaPlayer::error(QMediaPlayer::Error error)
{
    gdDebug("got error QMediaPlayer::error(%d)", error);
    m_mediaPlayer.stop();
}
