#include "mdxdict.h"
#include "golddict/instances.hh"
#include "golddict/article_maker.hh"
#include "golddict/article_netmgr.hh"

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QString>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setCentralWidget(ui->textEdit);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Dictionary");
    ui->textEdit->setText("<html><body><h1>What?</h1></body></html>" + fileName);

    std::vector< sptr< Dictionary::Class > > dictionaries;
    loadMdx(this, fileName, dictionaries);

    Instances::Groups groupInstances;

    ArticleMaker am(dictionaries, groupInstances, "", "");

    ArticleNetworkAccessManager articleNetMgr(this, dictionaries, am, true, false);

    QString contentType;
    //QUrl blankPage( "gdlookup://localhost?blank=1" );
    //sptr< Dictionary::DataRequest > r = articleNetMgr.getResource( blankPage,
    //                                                                 contentType );

    QString req("gdlookup://localhost?word=residual&dictionaries=");
    QString dic(dictionaries[0]->getId().c_str());
    QUrl testWordPage( req+dic );
    sptr< Dictionary::DataRequest > r = articleNetMgr.getResource( testWordPage,
                                                                     contentType );

    QEventLoop localLoop;

    QObject::connect( r.get(), SIGNAL( finished() ),
                      &localLoop, SLOT( quit() ) );

    localLoop.exec();

    if (r.get()) {
        r->finished();
        QString pageContent = QString::fromUtf8( &( r->getFullData().front() ),
                                                 r->getFullData().size() );
        ui->textEdit->append(pageContent);
    }
}

