#ifndef INTRODUCTIONVIEW_H
#define INTRODUCTIONVIEW_H

#include <QWebEngineView>
#include <QWebChannel>
#include <QString>
#include <QContextMenuEvent>

class IntroductionView : public QWebEngineView
{
public:
    IntroductionView(QWidget *parent = nullptr);

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
};



#endif // INTRODUCTIONVIEW_H
