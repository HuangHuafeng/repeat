#ifndef NEWBOOK_H
#define NEWBOOK_H

#include "HaiBeiDanCi/wordbook.h"
#include "gdhelper.h"

#include <QString>
#include <QDialog>
#include <QMap>

namespace Ui {
class NewBook;
}

class NewBook : public QDialog
{
    Q_OBJECT

public:
    explicit NewBook(GDHelper &gdhelper, QWidget *parent = nullptr);
    ~NewBook();

private slots:
    void on_buttonBox_accepted();

    void on_pushSelectFile_clicked();

private:
    Ui::NewBook *ui;
    GDHelper &m_gdhelper;

    QMap<QString, QString> m_lemmaMap;

    void addWordsToBook(WordBook &book, const QString fileName);
    void addWordListToBook(WordBook &book, const QStringList wordList);
    bool addWord(QString spelling);
    QString lemmaWord(QString spelling);
    void loadLemmas();
    void load_e_lemma();
    void loadAntLemma();
};

#endif // NEWBOOK_H
