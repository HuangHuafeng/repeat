#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QAbstractButton>

namespace Ui
{
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

    void on_pushUpdateInfo_clicked();

    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::PreferencesDialog *ui;

  private:
    bool saveCardSettings();
    bool saveDataSettings();

    void exitPreferencesDialog();
    bool saveDataDirectory();
    bool saveUpdateInterval();
    bool saveDefaultEasiness();
    bool saveCardDefaultInterval();
    bool saveCardMaximumInterval();
    bool savePerfectIncrease();
    bool saveCorrectIncrease();
    bool saveVagueDecrease();
    bool saveCardIntervalForIncorrect();
    void restoreDefaults();
    int restoreCardSettings();
    int restoreDataSettings();
    void loadCardSettings();
    void loadDataSettings();
};

#endif // PREFERENCESDIALOG_H
