#ifndef RELEASEUPGRADERDIALOG_H
#define RELEASEUPGRADERDIALOG_H

#include "servermanager.h"

#include <QDialog>
#include <QProgressDialog>

namespace Ui {
class ReleaseUpgraderDialog;
}

class ReleaseUpgraderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReleaseUpgraderDialog(QWidget *parent = nullptr);
    ~ReleaseUpgraderDialog();

private slots:
    void on_pbBrowse_clicked();

    void on_leVersion_textChanged(const QString &arg1);

    void on_pbCancel_clicked();

    void on_pbRelease_clicked();

    void onUploadProgress(float percentage);

    void onFileUploaded(QString fileName);

    void onUpgraderReleased(bool succeed);

private:
    Ui::ReleaseUpgraderDialog *ui;

    ServerManager m_sm;
    QProgressDialog m_progressDialog;

    void initializeProgressDialog();
    void createProgressDialog(const QString &labelText, const QString &cancelButtonText);

    void validateReleaseParameters();
};

#endif // RELEASEUPGRADERDIALOG_H
