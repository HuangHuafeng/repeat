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

private:
    Ui::Upgrader *ui;
};

#endif // UPGRADER_H
