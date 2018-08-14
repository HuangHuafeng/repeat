#include "introductionview.h"
#include "introductionpage.h"
#include <QMenu>

IntroductionView::IntroductionView(QWidget *parent) : QWebEngineView(parent)
{

    auto introPage = new IntroductionPage(this);
    setPage(introPage);
}

void IntroductionView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = page()->createStandardContextMenu();
    const QList<QAction *> actions = menu->actions();

    auto it = actions.begin();
    while (it != actions.end())
    {
        if (*it != page()->action(QWebEnginePage::Copy) && *it != page()->action(QWebEnginePage::SelectAll) && *it != page()->action(QWebEnginePage::CopyLinkToClipboard))
        {
            (*it)->setVisible(false);
        }

        it++;
    }

    menu->popup(event->globalPos());
}
