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
    void closeEvent(QCloseEvent* event);

    void SetModel(TreeItemModel* m);
    void BeginResetModel();
    void EndResetModel();

    void CreateActionType(NodeInfo *seq_node);
    NODE_TYPE GetNodeTypeAndText(QString& node_text);
    BaseValueClass* GetValue_SetVar();
    BaseValueClass* GetValue_CallFunc();

private slots:
    void on_btnChoice_clicked();
    void on_btnLoop_clicked();
    void on_btnEnd_clicked();
    void on_btnSetVar_clicked();
    void on_btnCallFunc_clicked();
    void on_btnCloseEvent_clicked();
    void on_btnOpenEvent_clicked();
    void on_btnCancel_clicked();
    void on_btnCustomAction_clicked();

private:
    Ui::DlgChoseActionType *ui;

    DlgChoseEType* m_dlgChoseEvent;
    DlgSetVariable* m_dlgSetVar;
    DlgEditValue* m_dlgCallFunc;

    TreeItemModel* model;
    NodeInfo* node;
    NODE_TYPE index;
};

#endif // DLGCHOSEACTIONTYPE_H
