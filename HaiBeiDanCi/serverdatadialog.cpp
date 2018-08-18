#include "serverdatadialog.h"
#include "ui_serverdatadialog.h"

ServerDataDialog::ServerDataDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerDataDialog)
{
    ui->setupUi(this);
}

ServerDataDialog::~ServerDataDialog()
{
    delete ui;
}
