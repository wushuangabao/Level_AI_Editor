#ifndef DLGCHOSEETYPE_H
#define DLGCHOSEETYPE_H

#include <QDialog>
#include <QMap>
#include "../ItemModels/enumdefine.h"
#include "../ItemModels/eventtype.h"

namespace Ui {
class DlgChoseEType;
}

class DlgChoseEType : public QDialog
{
    Q_OBJECT

public:
    explicit DlgChoseEType(QWidget *parent = 0);
    ~DlgChoseEType();

    void ShowWithEventType(QString name = "");
    void ShowWithActionType();

    QStringList action_list;
    QMap<QString, NODE_TYPE> action_map;

    int index = -1;
    QString event_name;
    QString text = "";

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

    void on_lineEdit_textChanged(const QString &arg1);

private:
    Ui::DlgChoseEType *ui;
};

#endif // DLGCHOSEETYPE_H
