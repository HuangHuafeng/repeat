#include "browserwindow.h"
#include "ui_browserwindow.h"
#include "worddb.h"
#include "mysettings.h"

#include <QSplitter>
#include <QMutex>
#include <QSettings>

BrowserWindow::BrowserWindow(QWidget *parent) : QDialog(parent),
                                                ui(new Ui::BrowserWindow),
                                                m_wordView(parent),
                                                m_mutex()
{
    ui->setupUi(this);
    setMyTitle();
    loadSetting();

    QStringList header;
    header.append(BrowserWindow::tr("Word"));
    header.append(BrowserWindow::tr("Expire"));
    header.append(BrowserWindow::tr("Repetition"));
    header.append(BrowserWindow::tr("Easiness"));
    header.append(BrowserWindow::tr("Interval (day)"));
    header.append(BrowserWindow::tr("Reviewed at"));
    ui->treeWidget->setHeaderLabels(header);
    ui->treeWidget->sortByColumn(1, Qt::SortOrder::AscendingOrder);
    connect(ui->treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(onItemSelectionChanged()));

    QSplitter *splitter = new QSplitter(Qt::Orientation::Vertical, this);
    splitter->addWidget(&m_wordView);
    splitter->addWidget(ui->widgetBottom);
    ui->verticalLayout->addWidget(splitter);
    //ui->gridLayout->setSizeConstraint(QLayout::SetFixedSize);
}

BrowserWindow::~BrowserWindow()
{
    delete ui;
}

void BrowserWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    event->accept();
}

void BrowserWindow::saveSettings()
{
    QSettings settings;
    settings.beginGroup("BrowserWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.setValue("wordListHide", ui->checkHideTreeview->isChecked());
    settings.setValue("showDefinitionDirectly", ui->checkShowDefinitionDirectly->isChecked());
    settings.endGroup();
}

void BrowserWindow::loadSetting()
{
    QSettings settings;
    settings.beginGroup("BrowserWindow");
    resize(settings.value("size", QSize(640, 480)).toSize());
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    ui->checkHideTreeview->setChecked(settings.value("wordListHide", false).toBool());
    ui->checkShowDefinitionDirectly->setChecked(settings.value("showDefinitionDirectly", true).toBool());
    settings.endGroup();
}

void BrowserWindow::onItemSelectionChanged()
{
    auto ci = ui->treeWidget->currentItem();
    if (ci == nullptr)
    {
        return;
    }

    auto spelling = ci->text(0);
    auto word = Word::getWord(spelling);
    Q_ASSERT(word.get() != nullptr);
    m_wordView.setWord(word);

    auto showDefinitionDirectly = ui->checkShowDefinitionDirectly->isChecked();
    if (showDefinitionDirectly)
    {
        m_wordView.setShowSetting(WordView::ShowAll);
        showHideButtons(true);
    }
    else
    {
        m_wordView.setShowSetting(WordView::ShowSpell);
        showHideButtons(false);
    }
}

void BrowserWindow::addWordsToTreeView(sptr<StudyList> studyList)
{
    if (studyList.get() == nullptr)
    {
        return;
    }

    auto wordList = studyList->getWordList();
    for (int i = 0; i < wordList.size(); i++)
    {
        auto word = wordList.at(i);
        QStringList infoList;

        // spelling
        infoList.append(word);

        auto card = WordCard::getCard(word);
        if (card.get() && card->isNew() == false)
        {
            // expire
            infoList.append(card->getExpireTime().toString("yyyy-MM-dd"));

            // repetition
            auto repetition = card->getRepetition();
            if (repetition < 100)
            {
                // less than 999 days, display number
                QString repetitionText = QString("%1").arg(repetition, 2, 10, QChar('0'));
                infoList.append(repetitionText);
            }
            else
            {
                infoList.append(BrowserWindow::tr(">99"));
            }

            // easiness
            double easiness = static_cast<double>(card->getEasiness());
            QString easinessText = QString("%1").arg(easiness, 0, 'f', 2);
            infoList.append(easinessText);

            // interval
            auto im = card->getIntervalInMinute() / (60 * 24);
            if (im == 0)
            {
                infoList.append(BrowserWindow::tr("<1"));
            }
            else if (im < 999)
            {
                // less than 999 days, display number
                QString interval = QString("%1").arg(im, 3, 10, QChar('0'));
                infoList.append(interval);
            }
            else
            {
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

bool BrowserWindow::setWordList(sptr<StudyList> studyList)
{
    if (studyList.get() == nullptr || studyList->size() == 0)
    {
        return false;
    }

    // remove all the items
    ui->treeWidget->clear();
    ui->treeWidget->setSortingEnabled(false);
    addWordsToTreeView(studyList);
    ui->treeWidget->setSortingEnabled(true);

    // select the first item
    QTreeWidgetItemIterator it(ui->treeWidget);
    if (*it)
    {
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
    if (hideTreeWidget)
    {
        ui->treeWidget->hide();
    }
    else
    {
        ui->treeWidget->show();
    }
}

void BrowserWindow::on_pushPrevious_clicked()
{
    auto current = ui->treeWidget->currentItem();
    auto previous = ui->treeWidget->itemAbove(current);
    if (previous)
    {
        ui->treeWidget->setCurrentItem(previous);
    }
}

void BrowserWindow::on_pushNext_clicked()
{
    auto current = ui->treeWidget->currentItem();
    auto next = ui->treeWidget->itemBelow(current);
    if (next)
    {
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
    if (definitionIsShown)
    {
        ui->pushShow->hide();
        ui->pushNext->show();
        ui->pushPrevious->show();
    }
    else
    {
        ui->pushNext->hide();
        ui->pushPrevious->hide();
        ui->pushShow->show();
    }

    //ui->widgetBottom->adjustSize();
}

void BrowserWindow::on_checkShowDefinitionDirectly_stateChanged(int /*arg1*/)
{
    onItemSelectionChanged();
}

void BrowserWindow::setMyTitle()
{
    QString title = MySettings::appName() + " - " + BrowserWindow::tr("browse words");
    setWindowTitle(title);
}
