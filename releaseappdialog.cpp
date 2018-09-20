#include "releaseappdialog.h"
#include "ui_releaseappdialog.h"

ReleaseAppDialog::ReleaseAppDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReleaseAppDialog)
{
    ui->setupUi(this);
}

ReleaseAppDialog::~ReleaseAppDialog()
{
    delete ui;
}
