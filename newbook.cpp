#include "newbook.h"
#include "ui_newbook.h"
#include "golddict/gddebug.hh"

#include <QMessageBox>
#include <QFileDialog>
#include <QProgressDialog>
#include <QFile>

NewBook::NewBook(GDHelper &gdhelper, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewBook),
    m_gdhelper(gdhelper)
{
    ui->setupUi(this);

    loadLemmas();
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

    QStringList wordsToAdd;
    do {
        char buf[1024];
        qint64 lineLength = wordFile.readLine(buf, sizeof(buf));

        if (lineLength == -1) {
            break;
        }

        QString spelling = QString(buf).trimmed();
        if (spelling.isEmpty() == false) {
            // rule out the empty lines
            wordsToAdd.append(spelling);
        }
    } while(1);

    addWordListToBook(book, wordsToAdd);
}

void NewBook::addWordListToBook(WordBook &book, const QStringList wordList)
{
    QProgressDialog progress("Creating the book ...", "Abort", 0, wordList.size(), this);
    QStringList failedWords;

    for (int i = 0;i < wordList.size();i ++) {
        auto spelling = wordList.at(i);
        auto repd = spelling.replace(QRegExp("[\\(\\)]"), "");
        if (m_gdhelper.saveWord(repd) == false) {
            auto lemma = lemmaWord(repd);
            if (lemma.isEmpty() == false) {
                if (m_gdhelper.saveWord(lemma) == false) {
                    failedWords.append(spelling);
                    //gdDebug("still failed after using lemma \"%s\"", lemma.toStdString().c_str());
                } else {
                    //gdDebug("added using lemma \"%s\"", lemma.toStdString().c_str());
                }
            } else {
                failedWords.append(spelling);
                //gdDebug("no leamma!");
            }
        } else {
            if (book.addWord(spelling) == false) {
                failedWords.append(spelling);
                gdDebug("STRANGE!! Cannot add \"%s\" to book!", spelling.toStdString().c_str());
            }
        }

        progress.setValue(i);
        if (progress.wasCanceled()) {
            break;
        }
    }

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

void NewBook::loadLemmas()
{
    load_e_lemma();
    loadAntLemma();
}

void NewBook::load_e_lemma()
{
    QFile lemmaFile(":/e_lemma.txt");
    if (!lemmaFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        gdDebug("failed to load e_lemma.txt");
        return;
    }

    do {
        char buf[1024];
        qint64 lineLength = lemmaFile.readLine(buf, sizeof(buf));

        if (lineLength == -1) {
            break;
        }

        QString line = QString(buf).trimmed();
        if (line.startsWith("[") == true) {
            // comment line
            continue;
        }

        QStringList temp = line.split("->", QString::SkipEmptyParts);
        if (temp.size() >= 2) {
            QString v = temp.at(0).trimmed();
            QStringList varies = temp.at(1).split(",", QString::SkipEmptyParts);
            for (int i = 0;i < varies.size();i ++) {
                m_lemmaMap.insert(varies.at(i).trimmed(), v);
            }
        }
    } while(1);
}

void NewBook::loadAntLemma()
{
    QFile lemmaFile(":/AntBNC_lemmas_ver_001.txt");
    if (!lemmaFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        gdDebug("failed to load antbnc_lemmas_ver_003.txt");
        return;
    }

    do {
        char buf[1024];
        qint64 lineLength = lemmaFile.readLine(buf, sizeof(buf));

        if (lineLength == -1) {
            break;
        }

        QString line = QString(buf).trimmed();
        QStringList temp = line.split("->", QString::SkipEmptyParts);
        if (temp.size() >= 2) {
            QString v = temp.at(0).trimmed();
            QStringList varies = temp.at(1).split('\t', QString::SkipEmptyParts);
            for (int i = 0;i < varies.size();i ++) {
                m_lemmaMap.insert(varies.at(i).trimmed(), v);
            }
        }
    } while(1);
}

QString NewBook::lemmaWord(QString spelling)
{
    return m_lemmaMap.value(spelling);
}
