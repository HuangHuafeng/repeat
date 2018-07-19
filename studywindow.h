#ifndef STUDYWINDOW_H
#define STUDYWINDOW_H

#include "gdhelper.h"
#include "wordview.h"

#include <QDialog>

namespace Ui {
class StudyWindow;
}

class StudyWindow : public QDialog
{
    Q_OBJECT

public:
    explicit StudyWindow(QWidget *parent = 0);
    ~StudyWindow();

private slots:
    void on_pushButton_clicked();

private:
    Ui::StudyWindow *ui;

    GDHelper m_gdhelper;
    WordView m_wordView;
};

#endif // STUDYWINDOW_H
