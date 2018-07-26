#ifndef STUDYWINDOW_H
#define STUDYWINDOW_H

#include "gdhelper.h"
#include "wordview.h"
#include "wordcard.h"

#include <QDialog>

namespace Ui {
class StudyWindow;
}

class StudyWindow : public QDialog
{
    Q_OBJECT

public:
    explicit StudyWindow(GDHelper &gdhelper, QWidget *parent = 0);
    ~StudyWindow();

private slots:
    void on_pushButton_clicked();

private:
    Ui::StudyWindow *ui;
    GDHelper &m_gdhelper;
    WordView m_wordView;
    QWebEngineView m_definitionView;
    WordCard m_wordCard;

    // the list of words to learn
};

#endif // STUDYWINDOW_H
