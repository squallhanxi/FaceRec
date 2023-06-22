#ifndef QMSGDIALOG_H
#define QMSGDIALOG_H

#include <QDialog>

namespace Ui {
class QMsgDialog;
}

class QMsgDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QMsgDialog(QWidget *parent = nullptr);
    ~QMsgDialog();

    void setText(QString txt, QString time);

private:
    Ui::QMsgDialog *ui;
};

#endif // QMSGDIALOG_H
