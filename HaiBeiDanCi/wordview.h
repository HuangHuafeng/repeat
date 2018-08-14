#ifndef WORDVIEW_H
#define WORDVIEW_H

#include "temporaryfilemanager.h"
#include "word.h"

#include <QWebEngineView>
#include <QWebChannel>
#include <QString>
#include <QContextMenuEvent>

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

    void setWord(sptr<Word> word = sptr<Word>());
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

  protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

  private:
    QWebChannel m_channel;
    sptr<Word> m_word;
    QSize m_sizeHint;
    TemporaryFileManager m_tfm;
    ShowOptions m_showSetting;

    void loadHtml();
    void toHtmlCallback(QString html);
};

#endif // WORDVIEW_H
