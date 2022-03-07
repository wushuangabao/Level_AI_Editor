#ifndef DLGCHOSEACTIONTYPE_H
#define DLGCHOSEACTIONTYPE_H

#include <QDialog>
#include "../ItemModels/enumdefine.h"
#include "dlgsetvariable.h"
#include "dlgeditvalue.h"
#include "dlgchoseetype.h"

namespace Ui {
class DlgChoseActionType;
}

class TreeItemModel;

class DlgChoseActionType : public QDialog
{
    Q_OBJECT

public:
    explicit DlgChoseActionType(QWidget *parent = 0);
    ~DlgChoseActionType();

    void SetModel(TreeItemModel* m);
    void CreateActionType();
    NODE_TYPE GetNodeTypeAndText(QString& node_text);

private slots:
    void on_btnChoice_clicked();
    void on_btnLoop_clicked();
    void on_btnEnd_clicked();
    void on_btnSetVar_clicked();
    void on_btnCallFunc_clicked();
    void on_btnCloseEvent_clicked();
    void on_btnOpenEvent_clicked();
    void on_btnCancel_clicked();

private:
    Ui::DlgChoseActionType *ui;

    DlgChoseEType* m_dlgChoseEvent;
    DlgSetVariable* m_dlgSetVar;
    DlgEditValue* m_dlgCallFunc;

    TreeItemModel* model;
    NODE_TYPE index;
};

#endif // DLGCHOSEACTIONTYPE_H
