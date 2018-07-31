#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QMediaPlayer>

class MediaPlayer : public QObject
{
    Q_OBJECT

    QMediaPlayer m_mediaPlayer;
    QVector<QString> m_filesToPlay;

    void playNextFile();

public:
    MediaPlayer();

    void play(QString fileName);

public slots:
    void stateChanged(QMediaPlayer::State state);
    void error(QMediaPlayer::Error error);
};

#endif // MEDIAPLAYER_H
