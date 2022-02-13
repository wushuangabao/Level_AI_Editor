#ifndef DLGSETVARIABLE_H
#define DLGSETVARIABLE_H

#include <QDialog>

namespace Ui {
class DlgSetVariable;
}

class DlgEditValue;
class TreeItemModel;
class NodeInfo;

class DlgSetVariable : public QDialog
{
    Q_OBJECT

public:
    explicit DlgSetVariable(QWidget *parent = 0);
    ~DlgSetVariable();

    void SetModelPointer(TreeItemModel* m);

    void EditSetVarNode(NodeInfo* set_var_node);

    QString GetNodeText();

private slots:
    void on_pushButton_clicked();

    void on_DlgSetVariable_accepted();

private:
    void initVariableComboBox();

    Ui::DlgSetVariable *ui;

    DlgEditValue* m_dlgEditValue;

    TreeItemModel* model;

    NodeInfo* node;
};

#endif // DLGSETVARIABLE_H
