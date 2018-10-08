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
    QString extractDir;
    QStringList zipFiles;
    if (ud.getUpgradeData(version, zipFiles, extractDir) == true)
    {
        ui->label->setText(QObject::tr("Decompressing version %1 ...").arg(version.toString()));
        qDebug() << "extracting" << zipFiles << "to" << extractDir;
        //JlCompress::extractDir(zipFile, extractDir);

        QStringList files;
        for (int i = 0;i < zipFiles.size();i ++)
        {
            files += JlCompress::getFileList(zipFiles.at(i));
        }
        ui->pbExtracting->setMaximum(files.size() - 1);
        int progress = 0;
        for (int i = 0;i < zipFiles.size();i ++)
        {
            QString zf = zipFiles.at(i);
            auto filesInOneZip = JlCompress::getFileList(zf);
            for (int j = 0;j < filesInOneZip.size();j ++)
            {
                auto fileName = filesInOneZip.at(i);
                auto destFileName = extractDir + "/" + fileName.section('/', 1);
                qDebug() << QString("extracting %1 from %2 to %3 ...").arg(fileName).arg(zf).arg(destFileName);
                JlCompress::extractFile(zf, fileName, destFileName);
                progress ++;
                ui->pbExtracting->setValue(progress);
                QCoreApplication::processEvents();
            }
        }
        //////

        QMessageBox::information(this,
                                 QObject::tr("Upgrader"),
                                 QObject::tr("Decompressing completed! Press OK to start version %1.").arg(version.toString()));

        // start the target
        ud.startTarget();

        // exit this upgrader
        close();
    }

}
