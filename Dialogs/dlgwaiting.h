#ifndef DLGWAITING_H
#define DLGWAITING_H

#include <QDialog>

namespace Ui {
class DlgWaiting;
}

class DlgWaiting : public QDialog
{
    Q_OBJECT

public:
    explicit DlgWaiting(QWidget *parent = 0);
    ~DlgWaiting();

private:
    Ui::DlgWaiting *ui;
};

#endif // DLGWAITING_H
