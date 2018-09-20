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

private:
    Ui::ReleaseAppDialog *ui;
};

#endif // RELEASEAPPDIALOG_H
