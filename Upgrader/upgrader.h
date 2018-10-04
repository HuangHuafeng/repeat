#ifndef UPGRADER_H
#define UPGRADER_H

#include "upgradedata.h"

#include <QWidget>

namespace Ui {
class Upgrader;
}

class Upgrader : public QWidget
{
    Q_OBJECT

public:
    explicit Upgrader(QWidget *parent = nullptr);
    ~Upgrader();

    void setTarget(QString target);

protected:
    virtual void showEvent(QShowEvent *event);

private:
    Ui::Upgrader *ui;
    QString m_target;

    void extract();
    bool targetIsRunning();
};

#endif // UPGRADER_H
