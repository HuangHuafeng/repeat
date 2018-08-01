#ifndef NEWBOOK_H
#define NEWBOOK_H

#include "HaiBeiDanCi/wordbook.h"
#include "gdhelper.h"

#include <QString>
#include <QDialog>

namespace Ui {
class NewBook;
}

class NewBook : public QDialog
{
    Q_OBJECT

public:
    explicit NewBook(GDHelper &gdhelper, QWidget *parent = 0);
    ~NewBook();

private slots:
    void on_buttonBox_accepted();

    void on_pushSelectFile_clicked();

private:
    Ui::NewBook *ui;
    GDHelper &m_gdhelper;

    void addWordsToBook(WordBook &book, const QString fileName);
};

#endif // NEWBOOK_H
