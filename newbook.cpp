#include "newbook.h"
#include "ui_newbook.h"
#include "golddict/gddebug.hh"

#include <QMessageBox>
#include <QFileDialog>

NewBook::NewBook(GDHelper &gdhelper, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewBook),
    m_gdhelper(gdhelper)
{
    ui->setupUi(this);
}

NewBook::~NewBook()
{
    delete ui;
}

void NewBook::on_buttonBox_accepted()
{
    auto name = ui->leName->text();
    auto intro = ui->teIntroduction->toPlainText();
    auto wordFile = ui->lineSelectedFile->text();

    if (name.isEmpty() || wordFile.isEmpty()) {
        QMessageBox::information(this, NewBook::tr(""), NewBook::tr("Please set Name and Word file!"));
        return;
    }

    WordBook book(name, intro);
    book.dbsave();
    addWordsToBook(book, wordFile);
}

void NewBook::on_pushSelectFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Word File");
    ui->lineSelectedFile->setText(fileName);
}

void NewBook::addWordsToBook(WordBook &book, const QString fileName)
{
    QFile wordFile(fileName);
    if (!wordFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        gdDebug("failed to load file");
        return;
    }

    QStringList failedWords;
    do {
        char buf[1024];
        qint64 lineLength = wordFile.readLine(buf, sizeof(buf));

        if (lineLength == -1) {
            break;
        }

        QString spelling = QString(buf).trimmed();
        if (spelling.isEmpty() == false) {
            if (m_gdhelper.saveWord(spelling) == false) {
                gdDebug("No definition for word: %s", spelling.toStdString().c_str());
                failedWords.append(spelling);
            } else {
                if (book.addWord(spelling) == false) {
                    failedWords.append(spelling);
                }
            }
        }
    } while(1);

    if (failedWords.isEmpty() == false) {
        QMessageBox::information(this,
                                 "",
                                 "Something wrong! \n Not all words are added");
        for (int i = 0;i < failedWords.size();i ++) {
            QString fw = failedWords.at(i);
            gdDebug("FAILED WORD:%s", fw.toStdString().c_str());
        }
    }
}

