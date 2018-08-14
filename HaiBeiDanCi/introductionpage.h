#ifndef INTRODUCTIONPAGE_H
#define INTRODUCTIONPAGE_H

#include <QObject>
#include <QWebEnginePage>
#include <QDesktopServices>

class IntroductionPage : public QWebEnginePage
{
    Q_OBJECT

  public:
    IntroductionPage(QObject *parent = nullptr);

    virtual bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool /*isMainFrame*/) override
    {
        if (type == QWebEnginePage::NavigationTypeLinkClicked)
        {
            QDesktopServices::openUrl(url);
            return false;
        }

        return true;
    }
};

#endif // INTRODUCTIONPAGE_H
