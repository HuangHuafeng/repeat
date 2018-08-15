#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = nullptr);
    ~PreferencesDialog();

private slots:
    void on_pushBrowse_clicked();

    void on_buttonBox_accepted();

private:
    Ui::PreferencesDialog *ui;

private:
    bool saveDataDirectory();
};

#endif // PREFERENCESDIALOG_H
