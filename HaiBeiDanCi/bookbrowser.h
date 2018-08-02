#ifndef BOOKBROWSER_H
#define BOOKBROWSER_H

#include "wordbook.h"

#include <QDialog>
#include <QWebEngineView>

namespace Ui {
class BookBrowser;
}

class BookBrowser : public QDialog
{
    Q_OBJECT

public:
    explicit BookBrowser(QWidget *parent = 0);
    ~BookBrowser();

    bool setBook(const QString &bookName);

private:
    Ui::BookBrowser *ui;

    void addBookToTheView(WordBook &book);
};

#endif // BOOKBROWSER_H
