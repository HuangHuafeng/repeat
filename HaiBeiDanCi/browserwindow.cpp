#include "browserwindow.h"
#include "ui_browserwindow.h"
#include "../golddict/gddebug.hh"
#include "worddb.h"

#include <QSplitter>
#include <QMutex>

BrowserWindow::BrowserWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BrowserWindow),
    m_wordView(parent),
    m_updaterThread(nullptr),
    m_mutex()
{
    ui->setupUi(this);

    ui->treeWidget->sortByColumn(1, Qt::SortOrder::AscendingOrder);
    connect(ui->treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(onItemSelectionChanged()));

    QStringList header;
    header.append(BrowserWindow::tr("Word"));
    header.append(BrowserWindow::tr("Expire"));
    header.append(BrowserWindow::tr("Repetition"));
    header.append(BrowserWindow::tr("Easiness"));
    header.append(BrowserWindow::tr("Interval (day)"));
    header.append(BrowserWindow::tr("Reviewed at"));
    ui->treeWidget->setHeaderLabels(header);

    QSplitter *splitter = new QSplitter(Qt::Orientation::Vertical, this);
    splitter->addWidget(&m_wordView);
    splitter->addWidget(ui->widgetBottom);

    ui->verticalLayout->addWidget(splitter);
}

BrowserWindow::~BrowserWindow()
{
    stopUpdater();
    delete ui;
}

void BrowserWindow::onItemSelectionChanged()
{
    auto spelling = ui->treeWidget->currentItem()->text(0);
    auto word = Word::getWordFromDatabase(spelling);
    m_wordView.setWord(word);

    auto showDefinitionDirectly = ui->checkShowDefinitionDirectly->isChecked();
    if (showDefinitionDirectly) {
        m_wordView.setShowSetting(WordView::ShowAll);
        showHideButtons(true);
    } else {
        m_wordView.setShowSetting(WordView::ShowSpell);
        showHideButtons(false);
    }
}

void BrowserWindow::addWordsToTreeView(sptr<StudyList> studyList)
{
    if (studyList.get() == nullptr) {
        return;
    }

    auto wordList = studyList->getWordList();
    for (int i = 0;i < wordList.size();i ++) {
        auto word = wordList.at(i);
        QStringList infoList;

        // spelling
        infoList.append(word);

        auto card = WordCard::getCardForWord(word);
        if (card.get()) {
            // expire
            infoList.append(card->getExpireTime().toString("yyyy-MM-dd"));

            // repetition
            auto repetition = card->getRepitition();
            if (repetition < 100) {
                // less than 999 days, display number
                QString repetitionText = QString("%1").arg(repetition, 2, 10, QChar('0'));
                infoList.append(repetitionText);
            } else {
                infoList.append(BrowserWindow::tr(">99"));
            }

            // easiness
            infoList.append(QString::number(static_cast<double>(card->getEasiness())));

            // interval
            auto im = card->getIntervalInMinute() / (60 * 24);
            if (im < 999) {
                // less than 999 days, display number
                QString interval = QString("%1").arg(im, 3, 10, QChar('0'));
                infoList.append(interval);
            } else {
                infoList.append(BrowserWindow::tr(">999"));
            }

            // reviewed at
            infoList.append(card->getLastStudyTime().toString("yyyy-MM-dd"));
        }

        // add the item to the tree widget
        QTreeWidgetItem *item = new QTreeWidgetItem(infoList);
        ui->treeWidget->addTopLevelItem(item);
    }
}

void BrowserWindow::onTreeWidgetUpdated()
{
    m_updaterThread = nullptr;
}

void BrowserWindow::stopUpdater()
{
    if (m_updaterThread != nullptr && m_updaterThread->isRunning()) {
        m_updaterThread->requestInterruption();
        m_updaterThread->wait();
        m_updaterThread = nullptr;
    }
}

void BrowserWindow::startUpdater()
{
    if (m_updaterThread != nullptr)
    {
        return;
    }

    m_updaterThread = new TreeWidgetUpdater(*this, ui->treeWidget, this);
    connect(m_updaterThread, SIGNAL(updateFinished()), this, SLOT(onTreeWidgetUpdated()));
    connect(m_updaterThread, SIGNAL(finished()), m_updaterThread, SLOT(deleteLater()));
    m_updaterThread->start();
}

bool BrowserWindow::setWordList(sptr<StudyList> studyList)
{
    if (studyList.get() == nullptr
            || studyList->size() == 0) {
        return false;
    }

    // remove all the items
    ui->treeWidget->clear();
    ui->treeWidget->setSortingEnabled(false);
    addWordsToTreeView(studyList);
    ui->treeWidget->setSortingEnabled(true);

    // select the first item
    QTreeWidgetItemIterator it(ui->treeWidget);
    if (*it) {
        ui->treeWidget->setCurrentItem(*it);
    }

    return true;
}

void BrowserWindow::reloadView()
{
    m_wordView.reloadHtml();
}

void BrowserWindow::on_checkHideTreeview_stateChanged(int /* arg1 */)
{
    auto hideTreeWidget = ui->checkHideTreeview->isChecked();
    if (hideTreeWidget) {
        ui->treeWidget->hide();
    } else {
        ui->treeWidget->show();
    }
}

void BrowserWindow::on_pushPrevious_clicked()
{
    auto current = ui->treeWidget->currentItem();
    auto previous = ui->treeWidget->itemAbove(current);
    if (previous) {
        ui->treeWidget->setCurrentItem(previous);
    }
}

void BrowserWindow::on_pushNext_clicked()
{
    auto current = ui->treeWidget->currentItem();
    auto next = ui->treeWidget->itemBelow(current);
    if (next) {
        ui->treeWidget->setCurrentItem(next);
    }
}

void BrowserWindow::on_pushShow_clicked()
{
    m_wordView.setShowSetting(WordView::ShowAll);
    showHideButtons(true);
}

void BrowserWindow::showHideButtons(bool definitionIsShown)
{
    if (definitionIsShown) {
        ui->pushShow->hide();
        ui->pushNext->show();
        ui->pushPrevious->show();
    } else {
        ui->pushNext->hide();
        ui->pushPrevious->hide();
        ui->pushShow->show();
    }

    //ui->widgetBottom->adjustSize();
}

TreeWidgetUpdater::TreeWidgetUpdater(BrowserWindow &bw, QTreeWidget *treeWidget, QObject *parent) :
    QThread (parent),
    m_bw(bw),
    m_treeWidget(treeWidget)
{

}

void TreeWidgetUpdater::run()
{
    // https://stackoverflow.com/questions/20793689/correctly-using-qsqldatabase-in-multi-threaded-programs
    WordDB appDb;
    QString dbConnName = "TreeWidgetUpdater" + QDateTime::currentDateTime().toString();
    if (appDb.connectDB(dbConnName)) {
        updateTreeWidget();
    }

    emit updateFinished();
}

void TreeWidgetUpdater::updateTreeWidget()
{
    if (m_treeWidget == nullptr) {
        return;
    }

    m_bw.lockTree();

    int updatedItems = 0;
    QTreeWidgetItemIterator it(m_treeWidget);
    while (*it) {
        updatedItems ++;
        if (updatedItems % 100 == 0) {
            // can this help the thread to be interrupted eaiser???
            //msleep(50);
            break;
        }

        if (isInterruptionRequested()) {
            break;
        }
        auto spelling = (*it)->text(0);
        auto wordcard = WordCard::generateCardForWord(spelling);
        if (wordcard.get()) {
            QString newText;
            if (wordcard->getStudyHistory().isEmpty() == false) {
                auto expire = wordcard->getExpireTime().toLocalTime();
                newText = expire.toString("yyyy/MM/dd");
            } else {
                newText = BrowserWindow::tr("Not set");
            }

            (*it)->setText(1, newText);
        }

        it ++;
    }

    m_bw.unlockTree();
}

void BrowserWindow::on_checkShowDefinitionDirectly_stateChanged(int /*arg1*/)
{
    onItemSelectionChanged();
}
