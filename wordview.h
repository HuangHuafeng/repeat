#ifndef WORDVIEW_H
#define WORDVIEW_H

#include <QWebEngineView>
#include <QWebChannel>
#include <QString>

class WordView : public QWebEngineView
{
    Q_OBJECT

public:
    WordView(QWidget *parent = Q_NULLPTR);

    void initialize();
    void setWord(QString word);
    QString getWord();
    virtual QSize sizeHint() const override;

    Q_INVOKABLE int updateSizeHint(int w, int h);

    Q_PROPERTY(QString word MEMBER m_word  NOTIFY wordChanged FINAL)

signals:
    void wordChanged(const QString & newWord);

private:
    QWebChannel m_channel;
    QString m_word;
    QSize m_sizeHint;
};

#endif // WORDVIEW_H
