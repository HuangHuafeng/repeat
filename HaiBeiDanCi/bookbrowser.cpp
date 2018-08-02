#include "bookbrowser.h"
#include "ui_bookbrowser.h"
#include "wordcard.h"

BookBrowser::BookBrowser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BookBrowser)
{
    ui->setupUi(this);

    QStringList header;
    header.append(BookBrowser::tr("Word"));
    header.append(BookBrowser::tr("Expire"));
    ui->treeWidget->setHeaderLabels(header);
}

BookBrowser::~BookBrowser()
{
    delete ui;
}


bool BookBrowser::setBook(const QString &bookName)
{
    auto book = WordBook::getBook(bookName);
    if (book.get() == 0) {
        return false;
    }

    addBookToTheView(*book);

    return true;
}

void BookBrowser::addBookToTheView(WordBook &book)
{
    // remove all the items
    ui->treeWidget->clear();
    auto wordList = book.getAllWords();
    for (int i = 0;i < wordList.size();i ++) {
        auto spelling = wordList.at(i);
        QStringList infoList;
        infoList.append(spelling);
        auto wordcard = WordCard::generateCardForWord(spelling);
        if (wordcard.get() && wordcard->getStudyHistory().isEmpty() == false) {
            auto expire = wordcard->getExpireTime().toLocalTime();
            infoList.append(expire.toString());
        }
        QTreeWidgetItem *item = new QTreeWidgetItem(infoList);
        ui->treeWidget->addTopLevelItem(item);
    }
}
