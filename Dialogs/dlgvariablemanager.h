#ifndef DLGVARIABLEMANAGER_H
#define DLGVARIABLEMANAGER_H

#include <QDialog>
#include "dlgeditvalue.h"

namespace Ui {
class DlgVariableManager;
}

class TreeItemModel;
class NodeInfo;

class DlgVariableManager : public QDialog
{
    Q_OBJECT

public:
    explicit DlgVariableManager(QWidget *parent = 0);
    ~DlgVariableManager();

    void SetModel(TreeItemModel *m);

    void CreateVar();
    void ModifyVar(int id_var);
    void SetLoopVar(NodeInfo* loop_node);

private slots:
    // 确认创建变量
    void on_pushButton_clicked();

    // 编辑变量初始值
    void on_pushButton_2_clicked();

    void on_comboBox_currentIndexChanged(const QString &arg1);

private:
    Ui::DlgVariableManager *ui;
    TreeItemModel* model;
    NodeInfo* node;
    DlgEditValue* m_dlgEditValue;
    BaseValueClass* init_v;
    QString var_type;
    int global_var_id;
};

#endif // DLGVARIABLEMANAGER_H
