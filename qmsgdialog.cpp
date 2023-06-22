#include "qmsgdialog.h"
#include "ui_qmsgdialog.h"

QMsgDialog::QMsgDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QMsgDialog)
{
    ui->setupUi(this);

    setWindowTitle("人员进入提示");
}

QMsgDialog::~QMsgDialog()
{
    delete ui;
}

void QMsgDialog::setText(QString txt, QString time)
{
    ui->label_msg->setText(txt);
    ui->label_msg_time->setText(time);
}
