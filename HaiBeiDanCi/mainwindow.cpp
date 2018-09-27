#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mysettings.h"
#include "wordcard.h"
#include "word.h"
#include "worddb.h"
#include "wordbook.h"
#include "preferencesdialog.h"
#include "serverdatadialog.h"
#include "aboutdialog.h"
#include "serverdatadownloader.h"
#include "mediafilemanager.h"
#include "registerdialog.h"
#include "logindialog.h"
#include "clienttoken.h"

#include <QTreeWidgetItem>
#include <QMessageBox>
#include <QSettings>
#include <QFuture>
#include <QFutureWatcher>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow),
                                          m_studyWindow(nullptr),
                                          m_browserWindow(nullptr)
{
    ui->setupUi(this);

    QStringList header;
    header.append(QObject::tr("Book Name"));
    ui->twBooks->setHeaderLabels(header);

    setMyTitle();

    ui->tbIntro->setOpenExternalLinks(true);

    if (WordDB::initialize() == false)
    {
        QMessageBox::critical(this, MySettings::appName(), MainWindow::tr("database error"));
    }

    connect(ui->twBooks, SIGNAL(itemSelectionChanged()), this, SLOT(onItemSelectionChanged()));

    connect(&m_studyWindow, SIGNAL(wordStudied(QString)), this, SLOT(onWordStudied(QString)));

    loadFont();
    loadSetting();
    reloadBooks();
    updateAllBooksData();

    // call MediaFileManager::instance() so it starts to initialize the existing file list
    MediaFileManager::instance();

    ClientToken::instance()->setLoginAction(ui->actionLogin);
}

MainWindow::~MainWindow()
{
    WordDB::shutdown();
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    event->accept();
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();
}

void MainWindow::loadSetting()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    resize(settings.value("size", QSize(640, 480)).toSize());
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    settings.endGroup();
}

void MainWindow::loadFont()
{
    QString fontString = MySettings::applicationFont();
    if (fontString.isEmpty() == false)
    {
        QFont font;
        if (font.fromString(fontString) == true)
        {
            QApplication::setFont(font);
        }
    }
}

void MainWindow::reloadBooks()
{
    ui->twBooks->clear();

    auto bookList = WordBook::getAllBooks();

    for (int i = 0; i < bookList.size(); i++)
    {
        auto book = WordBook::getBook(bookList.at(i));
        if (book.get())
        {
            addBookToTheView(*book);
        }
    }

    ui->twBooks->sortItems(0, Qt::SortOrder::AscendingOrder);

    // select the first book
    QTreeWidgetItemIterator it(ui->twBooks);
    if (*it)
    {
        ui->twBooks->setCurrentItem(*it);
    }
    else
    {
        // no book, call onItemSelectionChanged() manually
        onItemSelectionChanged();
    }
}

void MainWindow::addBookToTheView(WordBook &book)
{
    QStringList infoList;
    infoList.append(book.getName());
    QTreeWidgetItem *item = new QTreeWidgetItem(infoList);
    ui->twBooks->addTopLevelItem(item);
}

void MainWindow::onItemSelectionChanged()
{
    showCurrentBookIntroduction();
    updateCurrentBookData();
}

void MainWindow::onWordStudied(QString /*spelling*/)
{
    updateAllBooksData();
    updateCurrentBookData();
}

void MainWindow::onBookDownloaded(QString /*bookName*/)
{
    reloadBooks();
    updateAllBooksData();
}

void MainWindow::updateCurrentBookData()
{
    // expired words
    auto expired = expiredWordsFromCurrentBook();
    int numOfWords = 0;
    if (expired.get())
    {
        numOfWords = expired->size();
    }
    ui->labelExpired->setText(QString::number(numOfWords) + MainWindow::tr(" words"));
    ui->pushBrowseExpiredWords->setEnabled(numOfWords > 0);
    ui->pushStudyExpiredWords->setEnabled(numOfWords > 0);

    // old words
    auto old = oldWordsFromCurrentBook();
    numOfWords = 0;
    if (old.get())
    {
        numOfWords = old->size();
    }
    ui->labelOld->setText(QString::number(numOfWords) + MainWindow::tr(" words"));
    ui->pushBrowseOldWords->setEnabled(numOfWords > 0);
    ui->pushStudyOldWords->setEnabled(numOfWords > 0);

    // new words
    auto newWords = newWordsFromCurrentBook();
    numOfWords = 0;
    if (newWords.get())
    {
        numOfWords = newWords->size();
    }
    ui->labelNew->setText(QString::number(numOfWords) + MainWindow::tr(" words"));
    ui->pushBrowseNewWords->setEnabled(numOfWords > 0);
    ui->pushStudyNewWords->setEnabled(numOfWords > 0);

    // all words
    auto all = allWordsFromCurrentBook();
    numOfWords = 0;
    if (all.get())
    {
        numOfWords = all->size();
    }
    ui->labelAll->setText(QString::number(numOfWords) + MainWindow::tr(" words"));
    ui->pushBrowseAllWords->setEnabled(numOfWords > 0);
    ui->pushStudyAllWords->setEnabled(numOfWords > 0);
}

void MainWindow::updateAllBooksData()
{
    // expired words
    auto expired = StudyList::allExpiredWords();
    auto numOfWords = 0;
    if (expired.get())
    {
        numOfWords = expired->size();
    }
    ui->labelGlobalExpired->setText(QString::number(numOfWords) + MainWindow::tr(" words"));
    ui->pushGlobalBrowseExpiredWords->setEnabled(numOfWords > 0);
    ui->pushGlobalStudyExpiredWords->setEnabled(numOfWords > 0);

    // old words
    auto old = StudyList::allOldWords();
    numOfWords = 0;
    if (old.get())
    {
        numOfWords = old->size();
    }
    ui->labelGlobalOld->setText(QString::number(numOfWords) + MainWindow::tr(" words"));
    ui->pushGlobalBrowseOldWords->setEnabled(numOfWords > 0);
    ui->pushGlobalStudyOldWords->setEnabled(numOfWords > 0);

    // new words
    auto newWords = StudyList::allNewWords();
    numOfWords = 0;
    if (newWords.get())
    {
        numOfWords = newWords->size();
    }
    ui->labelGlobalNew->setText(QString::number(numOfWords) + MainWindow::tr(" words"));
    ui->pushGlobalBrowseNewWords->setEnabled(numOfWords > 0);
    ui->pushGlobalStudyNewWords->setEnabled(numOfWords > 0);

    // all words
    auto all = StudyList::allWords();
    numOfWords = 0;
    if (all.get())
    {
        numOfWords = all->size();
    }
    ui->labelGlobalAll->setText(QString::number(numOfWords) + MainWindow::tr(" words"));
    ui->pushGlobalBrowseAllWords->setEnabled(numOfWords > 0);
    ui->pushGlobalStudyAllWords->setEnabled(numOfWords > 0);
}

void MainWindow::startStudy(sptr<StudyList> studyList)
{
    if (studyList.get() == nullptr || studyList->size() == 0)
    {
        QMessageBox::information(this,
                                 MainWindow::tr(""),
                                 MainWindow::tr("No word to study!"));
        return;
    }

    auto setRestul = m_studyWindow.setStudyList(studyList);
    if (setRestul)
    {
        m_studyWindow.reloadView();
        m_studyWindow.show();
    }
    else
    {
        QMessageBox::critical(this,
                              MainWindow::tr(""),
                              MainWindow::tr("failed to set the word list to study!"));
    }
}

void MainWindow::startBrowse(sptr<StudyList> studyList)
{
    if (studyList.get() == nullptr || studyList->size() == 0)
    {
        QMessageBox::information(this,
                                 MainWindow::tr(""),
                                 MainWindow::tr("No word to Browse!"));
        return;
    }

    auto setRestul = m_browserWindow.setWordList(studyList);
    if (setRestul)
    {
        m_browserWindow.reloadView();
        m_browserWindow.show();
    }
    else
    {
        QMessageBox::critical(this,
                              MainWindow::tr(""),
                              MainWindow::tr("failed to set the word list to browse!"));
    }
}

sptr<StudyList> MainWindow::expiredWordsFromCurrentBook()
{
    auto ci = ui->twBooks->currentItem();
    if (ci == nullptr)
    {
        return sptr<StudyList>();
    }

    auto bookName = ci->text(0);
    auto expire = QDateTime::currentDateTime();
    return StudyList::allExpiredWordsInBook(bookName, expire);
}

void MainWindow::on_pushStudyExpiredWords_clicked()
{
    startStudy(expiredWordsFromCurrentBook());
}

void MainWindow::on_pushBrowseExpiredWords_clicked()
{
    startBrowse(expiredWordsFromCurrentBook());
}

sptr<StudyList> MainWindow::oldWordsFromCurrentBook()
{
    auto ci = ui->twBooks->currentItem();
    if (ci == nullptr)
    {
        return sptr<StudyList>();
    }

    auto bookName = ci->text(0);
    return StudyList::allOldWordsInBook(bookName);
}

void MainWindow::on_pushStudyOldWords_clicked()
{
    startStudy(oldWordsFromCurrentBook());
}

void MainWindow::on_pushBrowseOldWords_clicked()
{
    startBrowse(oldWordsFromCurrentBook());
}

sptr<StudyList> MainWindow::newWordsFromCurrentBook()
{
    auto ci = ui->twBooks->currentItem();
    if (ci == nullptr)
    {
        return sptr<StudyList>();
    }

    auto bookName = ci->text(0);
    return StudyList::allNewWordsInBook(bookName);
}

void MainWindow::on_pushStudyNewWords_clicked()
{
    startStudy(newWordsFromCurrentBook());
}

void MainWindow::on_pushBrowseNewWords_clicked()
{
    startBrowse(newWordsFromCurrentBook());
}

void MainWindow::showCurrentBookIntroduction()
{
    auto ci = ui->twBooks->currentItem();
    if (ci == nullptr)
    {
        return;
    }

    auto bookName = ci->text(0);
    auto book = WordBook::getBook(bookName);
    if (book.get())
    {
        ui->tbIntro->setHtml(book->getIntroduction());
    }
}

sptr<StudyList> MainWindow::allWordsFromCurrentBook()
{
    auto ci = ui->twBooks->currentItem();
    if (ci == nullptr)
    {
        return sptr<StudyList>();
    }

    auto bookName = ci->text(0);
    return StudyList::allWordsInBook(bookName);
}

void MainWindow::on_pushStudyAllWords_clicked()
{
    startStudy(allWordsFromCurrentBook());
}

void MainWindow::on_pushBrowseAllWords_clicked()
{
    startBrowse(allWordsFromCurrentBook());
}

void MainWindow::on_pushGlobalStudyAllWords_clicked()
{
    auto studyList = StudyList::allWords();
    startStudy(studyList);
}

void MainWindow::on_pushGlobalBrowseAllWords_clicked()
{
    auto studyList = StudyList::allWords();
    startBrowse(studyList);
}

void MainWindow::on_pushGlobalStudyNewWords_clicked()
{
    auto studyList = StudyList::allNewWords();
    startStudy(studyList);
}

void MainWindow::on_pushGlobalBrowseNewWords_clicked()
{
    auto studyList = StudyList::allNewWords();
    startBrowse(studyList);
}

void MainWindow::on_pushGlobalStudyOldWords_clicked()
{
    auto studyList = StudyList::allOldWords();
    startStudy(studyList);
}

void MainWindow::on_pushGlobalBrowseOldWords_clicked()
{
    auto studyList = StudyList::allOldWords();
    startBrowse(studyList);
}

void MainWindow::on_pushGlobalStudyExpiredWords_clicked()
{
    auto studyList = StudyList::allExpiredWords();
    startStudy(studyList);
}

void MainWindow::on_pushGlobalBrowseExpiredWords_clicked()
{
    auto studyList = StudyList::allExpiredWords();
    startBrowse(studyList);
}

void MainWindow::setMyTitle()
{
    QString title = MySettings::appName();
    setWindowTitle(title);
}

void MainWindow::on_action_About_triggered()
{
    AboutDialog ad(this);
    ad.exec();
}

void MainWindow::on_actionPreferences_triggered()
{
    PreferencesDialog pd(this);
    pd.exec();
}

void MainWindow::on_actionBooks_triggered()
{
    ServerDataDialog sdd(this);
    connect(&sdd, SIGNAL(bookDownloaded(QString)), this, SLOT(onBookDownloaded(QString)));
    sdd.exec();
}


void MainWindow::on_actionexit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_actionRegister_User_triggered()
{
    RegisterDialog rd(this);
    auto result = rd.exec();
    if (result == QDialog::Accepted)
    {
        // a new user is registered, here we can conintue logging in with the newly created user
    }
    else
    {
        // no user is registered and the dialog is cancelled
    }
}

void MainWindow::on_actionLogin_triggered()
{
    auto ct = ClientToken::instance();
    if (ct->hasAliveToken() == true
            && ct->hasValidUser() == true)
    {
        auto answer = QMessageBox::warning(this,
                                           MySettings::appName(),
                                           "\"" + ct->user().name() + "\" " + QObject::tr("already logged in. Would you like to logout?"),
                                           QMessageBox::Yes,
                                           QMessageBox::No);
        if (answer == QMessageBox::Yes)
        {
            ui->actionLogout->trigger();
        }
        else
        {
            return;
        }
    }

    LoginDialog ld(this);
    auto result = ld.exec();
    if (result == QDialog::Accepted)
    {
        // a new user is registered, here we can conintue logging in with the newly created user
    }
    else
    {
        // no user is registered and the dialog is cancelled
    }
}

void MainWindow::on_actionLogout_triggered()
{
    auto ct = ClientToken::instance();
    if (ct->hasAliveToken() == true
            && ct->hasValidUser() == true)
    {
        ServerUserAgent *sua = new ServerUserAgent(this);
        connect(sua, &ServerUserAgent::logoutSucceeded, [sua] (QString name) { sua->deleteLater(); qDebug() << "sua->deleteLater() called for" << name; });
        sua->logoutUser(ct->user().name());
        ct->setToken(Token::invalidToken);
        ct->setUser(ApplicationUser::invalidUser);
    }
}

void MainWindow::on_actionCheck_for_Updates_triggered()
{
    SvrAgt *sa = new SvrAgt(MySettings::serverHostName(), MySettings::serverPort(), this);
    connect(sa, SIGNAL(appVersion(ApplicationVersion, QString, QString, QDateTime)), this, SLOT(onAppVersion(ApplicationVersion, QString, QString, QDateTime)));
    connect(sa, &SvrAgt::appVersion, [sa] () {
        sa->deleteLater();
        qDebug() << "sa->deleteLater() called!";
    });
    sa->sendRequestAppVersion();
}

void MainWindow::onAppVersion(ApplicationVersion version, QString fileName, QString info, QDateTime releaseTime)
{
    ApplicationVersion myVer = ApplicationVersion::fromString(APP_VERSION);
    if (version.toInt() > myVer.toInt())
    {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText(QObject::tr("Version %1 is available!").arg(version.toString()));
        QString infoText = info.arg(releaseTime.toString())
                + "<br></br>"
                + QObject::tr("Would you like to download it now?");
        msgBox.setInformativeText(infoText);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Yes)
        {
            downloadLatestVersion(fileName);
        }
    }
    else
    {
        QMessageBox::information(this,
                                 MySettings::appName(),
                                 QObject::tr("You are using the latest version!"));
    }
}

void MainWindow::downloadLatestVersion(QString fileName)
{
    QProgressDialog *pd = new QProgressDialog(QObject::tr("downloading %1 ...").arg(fileName),
                                              QObject::tr("Cancel"),
                                              0,
                                              100,
                                              this);
    pd->setModal(true);
    pd->setValue(0);
    ServerDataDownloader *sdd = new ServerDataDownloader(this);

    // update the downloading progress
    connect(sdd, &ServerDataDownloader::downloadProgress, [pd] (float percentage) {
        int progress = static_cast<int>(100 * percentage);
        pd->setValue(progress);
    });

    // delete pd and sdd when downloading finishes
    connect(sdd, &ServerDataDownloader::fileDownloaded, [pd, sdd] () {
        sdd->deleteLater();
        pd->deleteLater();
        qDebug() << "sdd, pd deleted as the downloading finished!";
    });

    // delete pd and sdd when downloading is cancelled
    connect(pd, &QProgressDialog::canceled, [pd, sdd] () {
        sdd->cancelDownloading();
        sdd->deleteLater();
        pd->deleteLater();
        qDebug() << "sdd, pd deleted as the downloading is cancelled!";
    });

    sdd->downloadApp(fileName);
}
