#ifndef RELEASEAPPDIALOG_H
#define RELEASEAPPDIALOG_H

#include "servermanager.h"

#include <QDialog>
#include <QProgressDialog>

namespace Ui {
class ReleaseAppDialog;
}

class ReleaseAppDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReleaseAppDialog(QWidget *parent = nullptr);
    ~ReleaseAppDialog();

private slots:
    void on_pbCancel_clicked();

    void on_pbRelease_clicked();

    void on_pbBrowse_clicked();

    void on_leVersion_textChanged(const QString &arg1);

    void onUploadProgress(float percentage);

private:
    Ui::ReleaseAppDialog *ui;

    ServerManager m_sm;
    QProgressDialog m_progressDialog;

    void initializeProgressDialog();
    void createProgressDialog(const QString &labelText, const QString &cancelButtonText);

    void validateReleaseParameters();
};

#endif // RELEASEAPPDIALOG_H
