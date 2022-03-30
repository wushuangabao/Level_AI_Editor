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

    void CreateNewEvent();
    void EditEventName(QString name = "");

    void CreateNewCustomSeq();
    void EditCustomSeqName(QString name = "");

    void EditLevelName(const QString& name);
    void EditLevelPrefix(const QString& name);

    int ChoseCustActSeqNameIn(QStringList names);
    int ChoseEventNameIn(QStringList names);
    void EditEventType();

    int index = -1;
    QString event_name;
    QString text = "";

private slots:
    void on_lineEdit_textChanged(const QString &arg1);

    void on_DlgChoseEType_accepted();
    void on_DlgChoseEType_rejected();

    void on_comboBox_currentIndexChanged(int index);

private:
    Ui::DlgChoseEType *ui;
};

#endif // DLGCHOSEETYPE_H
