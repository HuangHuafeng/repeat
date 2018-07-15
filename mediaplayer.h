#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QMediaPlayer>

class MediaPlayer : public QObject
{
    Q_OBJECT

    QMediaPlayer m_mediaPlayer;
    QVector<QString> m_filesToPlay;

public:
    MediaPlayer();

    void play(QString fileName);

public slots:
    void stateChanged(QMediaPlayer::State state);
};

#endif // MEDIAPLAYER_H
