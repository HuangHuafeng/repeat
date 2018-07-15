#include "mediaplayer.h"
#include "golddict/gddebug.hh"

MediaPlayer::MediaPlayer(): m_mediaPlayer(),
    m_filesToPlay()
{
    connect(&m_mediaPlayer,
            &QMediaPlayer::stateChanged,
            this,
            &MediaPlayer::stateChanged);
}

void MediaPlayer::play(QString fileName)
{
    m_filesToPlay.append(fileName);
    if (m_filesToPlay.size() == 1)
    {
        m_mediaPlayer.setMedia(QUrl::fromLocalFile(fileName));
        m_mediaPlayer.play();
    } else {
        // nothing to do, fileName will be played later in stateChanged()
    }
}

void MediaPlayer::stateChanged(QMediaPlayer::State state)
{
    if (state == QMediaPlayer::StoppedState)
    {
        // remove the one just finished from the list
        m_filesToPlay.pop_front();

        if (m_filesToPlay.size() > 0) {
            // play the next one if exits
            m_mediaPlayer.setMedia(QUrl::fromLocalFile(m_filesToPlay.at(0)));
            m_mediaPlayer.play();
        }
    }
}
