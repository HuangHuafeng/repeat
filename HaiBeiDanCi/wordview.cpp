#include "wordview.h"
#include "../golddict/gddebug.hh"

#include <QWebChannel>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QMenu>
#include <QDesktopServices>
#include <QMessageBox>
#include <QTemporaryFile>

WordView::WordView(QWidget *parent) : QWebEngineView(parent),
    m_channel(parent),
    m_tfm(parent)
{
    m_showSetting = WordView::ShowAll;
    setWord();
    loadHtml();

    //auto action = page()->action(QWebEnginePage::ViewSource);
    auto action = pageAction(QWebEnginePage::ViewSource);
    if (action) {
        connect( action, SIGNAL( triggered() ), this, SLOT( inspect() ) );
    }
}

QSize WordView::sizeHint() const
{
    return m_sizeHint;
}

void WordView::loadHtml()
{
    QFileInfo wordHtmlFile(QDir::currentPath() + "/wordview.html");

    if (!wordHtmlFile.exists())
    {
        QFile::copy(":/wordview.html", wordHtmlFile.absoluteFilePath());
        gdDebug("file copied to %s", wordHtmlFile.absoluteFilePath().toStdString().c_str());
    }

    QFile htmlFile(wordHtmlFile.absoluteFilePath());
    if (!htmlFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        gdDebug("failed to load file");
        return;
    }

    QUrl baseUrl("file:///Users/huafeng/Documents/GitHub/TextFinder/build-Repeat-Desktop_Qt_5_11_1_clang_64bit-Debug/Repeat.app/Contents/MacOS/");
    setHtml(htmlFile.readAll().data(), baseUrl);

    m_channel.registerObject(QString("wordview"), this);
    page()->setWebChannel(&m_channel);
}

void WordView::setWord(sptr<Word> word)
{
    m_word = word;
    emit wordChanged();
}

void WordView::reloadHtml()
{
    loadHtml();
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

    if (m_word.get()) {
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

    if (m_word.get()) {
        definition = m_word->getDefinitionDIV();
    }

    return definition;
}

void WordView::contextMenuEvent(QContextMenuEvent *event)
{
    QWebEngineView::contextMenuEvent(event);
}

void WordView::toHtmlCallback(QString html)
{
    /*
     * the following lines tells that
     * html.size() sometimes is smaller than html.toStdString().length() !!!!!
    gdDebug("html.size(): %d", html.size());
    gdDebug("strlen: %d", strlen(html.toStdString().c_str()));
    gdDebug("html.toStdString().length(): %d", html.toStdString().length());
    */

    QString word(m_word.get() ? m_word->getSpelling() : "nullptr");
    QTemporaryFile tmp(QDir::temp().filePath( "XXXXXX-" + word + ".html" ));
    qint64 len = static_cast<qint64>(html.toStdString().length());

    if ( !tmp.open() || tmp.write( html.toStdString().c_str(), len ) != len )
    {
        gdDebug("failed to write temporary file inWordView::toHtmlCallback()");
    } else {
        tmp.setAutoRemove(false);
        m_tfm.addTemporaryFile(tmp);
        const QUrl tempUrl("view-source:file://" + tmp.fileName());
        setUrl(tempUrl);
    }

}

void WordView::inspect()
{
    page()->toHtml(std::bind(&WordView::toHtmlCallback, this, std::placeholders::_1));
}
