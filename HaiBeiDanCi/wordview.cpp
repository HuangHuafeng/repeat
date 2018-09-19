#include "wordview.h"
#include "mysettings.h"

#include <QWebChannel>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QMenu>
#include <QDesktopServices>
#include <QMessageBox>
#include <QTemporaryFile>
#include <QCoreApplication>

WordView::WordView(QWidget *parent) : QWebEngineView(parent),
                                      m_channel(parent),
                                      m_tfm(parent)
{
    m_showSetting = WordView::ShowAll;
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(onLoadFinished(bool)));

    reloadHtml();
    setWord(nullptr);

    //auto action = page()->action(QWebEnginePage::ViewSource);
    auto action = pageAction(QWebEnginePage::ViewSource);
    if (action)
    {
        connect(action, SIGNAL(triggered()), this, SLOT(inspect()));
    }
}

QSize WordView::sizeHint() const
{
    return m_sizeHint;
}

void WordView::loadHtml(QString fileName)
{
    QFile htmlFile(fileName);
    if (htmlFile.open(QIODevice::ReadOnly | QIODevice::Text) == true)
    {
        m_channel.registerObject(QString("wordview"), this);
        page()->setWebChannel(&m_channel);

        QUrl baseUrl("file:////" + MySettings::dataDirectory() + "/");
        setHtml(htmlFile.readAll().data(), baseUrl);
    }
    else
    {
        qDebug("failed to open file %s", fileName.toUtf8().constData());
    }
}

void WordView::reloadHtml()
{
    bool needReload = false;
    QFileInfo wordHtmlFile(MySettings::dataDirectory() + "/wordview.html");

    if (m_et.isValid() == false)
    {
        needReload = true;
        m_et.start();
    }

    if (wordHtmlFile.exists() == false)
    {
        QFile::copy(":/wordview.html", wordHtmlFile.absoluteFilePath());
        qInfo("file copied to %s", wordHtmlFile.absoluteFilePath().toUtf8().constData());

        needReload = true;
    }
    else
    {
        QDateTime lmt = wordHtmlFile.lastModified();
        QDateTime current = QDateTime::currentDateTime();
        qint64 past = lmt.msecsTo(current);
        if (past < m_et.elapsed())
        {
            needReload = true;
        }
    }

    if (needReload == true)
    {
        loadHtml(wordHtmlFile.absoluteFilePath());
    }
}

void WordView::setWord(const Word *word)
{
    m_word = word;
    emit wordChanged();
}

void WordView::setShowSetting(ShowOptions showSetting)
{
    m_showSetting = showSetting;
    emit showSettingChanged(m_showSetting);
}

int WordView::updateSizeHint(int w, int h)
{
    m_sizeHint.setWidth(w);
    m_sizeHint.setHeight(h);
    //setMaximumHeight(h);
    //setMinimumHeight(h);
    return 0;
}

QString WordView::getSpelling()
{
    QString spelling = "";

    if (m_word != nullptr)
    {
        spelling = m_word->getSpelling();
    }

    return spelling;
}

/**
 * @brief WordView::getDefinition
 * @return
 * we can add <div> into return value if needed
 */
QString WordView::getDefinition()
{
    QString definition = "";

    if (m_word != nullptr)
    {
        definition = m_word->getDefinitionDIV();
    }

    return definition;
}

void WordView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = page()->createStandardContextMenu();

#ifdef QT_NO_DEBUG
    const QList<QAction *> actions = menu->actions();
    auto it = actions.begin();
    while (it != actions.end())
    {
        if (*it != page()->action(QWebEnginePage::Copy))
        {
            (*it)->setVisible(false);
        }

        it++;
    }
#endif

    menu->popup(event->globalPos());
}

void WordView::toHtmlCallback(QString html)
{
    /*
     * the following lines tells that
     * html.size() sometimes is smaller than html.toStdString().length() !!!!!
    qDebug("html.size(): %d", html.size());
    qDebug("strlen: %d", strlen(html.toStdString().c_str()));
    qDebug("html.toStdString().length(): %d", html.toStdString().length());
    */

    QString word(m_word != nullptr ? m_word->getSpelling() : "nullptr");
    QTemporaryFile tmp(QDir::temp().filePath("XXXXXX-" + word + ".html"));
    qint64 len = static_cast<qint64>(html.toStdString().length());

    if (!tmp.open() || tmp.write(html.toStdString().c_str(), len) != len)
    {
        qDebug("failed to write temporary file inWordView::toHtmlCallback()");
    }
    else
    {
        tmp.setAutoRemove(false);
        m_tfm.addTemporaryFile(tmp);
        const QUrl tempUrl("view-source:file:///" + tmp.fileName());
        setUrl(tempUrl);
    }
}

void WordView::inspect()
{
    page()->toHtml(std::bind(&WordView::toHtmlCallback, this, std::placeholders::_1));
}

void WordView::onLoadFinished(bool /*ok*/)
{
    page()->runJavaScript("getComputedStyle(document.body).backgroundColor", std::bind(&WordView::changeBackgroundToStyleInHtml, this, std::placeholders::_1));
}

void WordView::changeBackgroundToStyleInHtml(const QVariant &color)
{
    QStringList rgb = color.toString().replace(QRegExp("(rgb\\(|\\))"), "").split(",");

    if (rgb.size() == 3)
    {
        int r = rgb[0].toInt();
        int g = rgb[1].toInt();
        int b = rgb[2].toInt();
        page()->setBackgroundColor(QColor(r, g, b));
    }
}
