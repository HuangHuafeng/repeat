#ifndef RELEASEAPPDIALOG_H
#define RELEASEAPPDIALOG_H

#include <QDialog>

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

private:
    Ui::ReleaseAppDialog *ui;

    void validateReleaseParameters();
};

#endif // RELEASEAPPDIALOG_H
