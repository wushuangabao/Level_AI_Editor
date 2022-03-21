#ifndef DLGSETVARIABLE_H
#define DLGSETVARIABLE_H

#include <QDialog>

namespace Ui {
class DlgSetVariable;
}

class DlgEditValue;
class TreeItemModel;
class NodeInfo;
class BaseValueClass;

class DlgSetVariable : public QDialog
{
    Q_OBJECT

public:
    explicit DlgSetVariable(QWidget *parent = 0);
    ~DlgSetVariable();

    void SetModel(TreeItemModel* m);

    void EditSetVarNode(NodeInfo* set_var_node);
    void CreateSetVarNode(NodeInfo *seq_node);

    QString GetNodeText();
    QString GetValueName();
    BaseValueClass* GetValuePointer();

    bool IsAccepted();

private slots:
    void on_pushButton_clicked();
    void on_DlgSetVariable_accepted();

    void on_comboBox_currentTextChanged(const QString &arg1);

private:
    bool initVariableComboBox();

    Ui::DlgSetVariable *ui;

    DlgEditValue* m_dlgEditValue;

    TreeItemModel* model;

    NodeInfo* node;

    QString var_type;

    bool is_accepted;
};

#endif // DLGSETVARIABLE_H
