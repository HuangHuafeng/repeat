#include "upgrader.h"
#include "ui_upgrader.h"

#include <JlCompress.h>
#include <QtDebug>
#include <QTimer>
#include <QMessageBox>

Upgrader::Upgrader(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Upgrader)
{
    ui->setupUi(this);
}

Upgrader::~Upgrader()
{
    delete ui;
}

void Upgrader::setTarget(QString target)
{
    m_target = target;
}

void Upgrader::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    static bool started = false;
    if (started == true)
    {
        return;
    }

    started = true;
    QTimer *ptrTimer = new QTimer(this);
    connect(ptrTimer, &QTimer::timeout, [ptrTimer, this] () {
        if (this->targetIsRunning() == false) {
            ptrTimer->stop();
            ptrTimer->deleteLater();
            this->extract();
        }
        else
        {
            qDebug() << "waiting for target to exit.";
        }
    });
    ptrTimer->start(200);
}

bool Upgrader::targetIsRunning()
{
    UpgradeData ud(m_target);
    return QFile::exists(ud.targetRunningFile()) == true;
}

void Upgrader::extract()
{
    Q_ASSERT(m_target.isEmpty() == false);
    UpgradeData ud(m_target);
    Q_ASSERT(ud.hasUpgradeData() == true);
    ApplicationVersion version(0, 0, 0);
    QString zipFile, extractDir;
    if (ud.getUpgradeData(version, zipFile, extractDir) == true)
    {
        ui->label->setText(QObject::tr("Decompressing version %1 ...").arg(version.toString()));
        qDebug() << "extracting" << zipFile << "to" << extractDir;
        //JlCompress::extractDir(zipFile, extractDir);

        auto files = JlCompress::getFileList(zipFile);
        ui->pbExtracting->setMaximum(files.size() - 1);
        for (int i = 0;i < files.size();i ++)
        {
            auto fileName = files.at(i);
            auto destFileName = extractDir + "/" + fileName.section('/', 1);
            qDebug() << QString("extracting %1 to %2 ...").arg(fileName).arg(destFileName);
            JlCompress::extractFile(zipFile, fileName, destFileName);
            ui->pbExtracting->setValue(i);
            QCoreApplication::processEvents();
        }

        QMessageBox::information(this,
                                 QObject::tr("Upgrader"),
                                 QObject::tr("Decompressing completed! Press OK to start version %1.").arg(version.toString()));

        // start the target
        ud.startTarget();

        // exit this upgrader
        close();
    }

}
