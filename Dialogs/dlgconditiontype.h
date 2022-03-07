#ifndef DLGCONDITIONTYPE_H
#define DLGCONDITIONTYPE_H

#include <QDialog>
#include "../ItemModels/enumdefine.h"

class DlgEditValue;
class NodeInfo;
class TreeItemModel;
class BaseValueClass;

namespace Ui {
class DlgConditionType;
}

class BaseValueClass;

class DlgConditionType : public QDialog
{
    Q_OBJECT

public:
    explicit DlgConditionType(QWidget *parent = 0);
    ~DlgConditionType();

    void SetModelPointer(TreeItemModel* m);

    void CreateCondition(NodeInfo* parent_node, QString default_s);

    void ModifyCondition(NodeInfo* node);
    void ModifyCompareNode(NodeInfo* node);

    BaseValueClass* GetValue_Left();
    BaseValueClass* GetValue_Right();

private slots:
    void on_comboBox_currentIndexChanged(int index);

    void on_buttonBox_rejected();
    void on_buttonBox_accepted();

    void on_btnText_1_clicked();
    void on_btnText_2_clicked();

private:
    void initUI(NODE_TYPE node_type);
    void initConditionType(NodeInfo* node);
    void initComparationValues(NodeInfo* node);

    // 比较左右值的类型是否一致
    bool checkCompareValuesType();

    Ui::DlgConditionType *ui;

    CONDITION_OP type;
    NODE_TYPE node_type;
    NodeInfo* node;

    DlgEditValue* m_dlgEditValueLeft;
    DlgEditValue* m_dlgEditValueRight;

    TreeItemModel* model;
};

#endif // DLGCONDITIONTYPE_H
