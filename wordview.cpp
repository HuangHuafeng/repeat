#include "wordview.h"
#include "golddict/gddebug.hh"

#include <QWebChannel>
#include <QFileInfo>
#include <QFile>
#include <QDir>

WordView::WordView(QWidget *parent) : QWebEngineView(parent),
    m_channel(parent)
{
    initialize();
    setWord("");
}

QSize WordView::sizeHint() const
{
    return m_sizeHint;
}

void WordView::initialize()
{
    m_channel.registerObject(QString("wordview"), this);
    page()->setWebChannel(&m_channel);

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

    setHtml(htmlFile.readAll().data());
}

QString WordView::getWord()
{
    return m_word;
}

void WordView::setWord(QString word)
{
    m_word = word;
    emit wordChanged(m_word);
}

int WordView::updateSizeHint(int w, int h)
{
    m_sizeHint.setWidth(w);
    m_sizeHint.setHeight(h);
    setMaximumHeight(h);
    setMinimumHeight(h);
    gdDebug("%d x %d", w, h);
    return 0;
}
