#ifndef WORDVIEW_H
#define WORDVIEW_H

#include "temporaryfilemanager.h"
#include "word.h"

#include <QWebEngineView>
#include <QWebChannel>
#include <QString>
#include <QContextMenuEvent>
#include <QElapsedTimer>

class WordView : public QWebEngineView
{
    Q_OBJECT

    Q_PROPERTY(ShowOptions showSetting READ showSetting WRITE setShowSetting NOTIFY showSettingChanged)
    Q_PROPERTY(QString spelling READ getSpelling NOTIFY wordChanged)
    Q_PROPERTY(QString definition READ getDefinition NOTIFY wordChanged)

  public:
    WordView(QWidget *parent = nullptr);

    typedef enum
    {
        ShowSpell = 1,
        ShowDefinition = 2,
        ShowAll = 3,
        ShowNothing = 4
    } ShowOptions;
    Q_ENUM(ShowOptions)

    void setWord(const Word *word);
    void reloadHtml();

    void setShowSetting(ShowOptions showSetting);
    ShowOptions showSetting() const
    {
        return m_showSetting;
    }

    QString getSpelling();
    QString getDefinition();

    virtual QSize sizeHint() const override;

    Q_INVOKABLE int updateSizeHint(int w, int h);

  signals:
    void wordChanged();
    void showSettingChanged(int showSetting);

  private slots:

    /// Inspect element
    void inspect();
    void onLoadFinished(bool ok);

  protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

  private:
    QWebChannel m_channel;
    const Word *m_word;
    QSize m_sizeHint;
    TemporaryFileManager m_tfm;
    ShowOptions m_showSetting;
    QElapsedTimer m_et;

    void loadHtml(QString fileName);
    void toHtmlCallback(QString html);
    void changeBackgroundToStyleInHtml(const QVariant &color);
};

#endif // WORDVIEW_H
