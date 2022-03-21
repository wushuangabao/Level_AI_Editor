#ifndef DLGEDITVALUE_H
#define DLGEDITVALUE_H

#include <QDialog>
#include "../Values/valuemanager.h"

const int FUNC_TEXT_NUM = 10;

class FunctionClass;
class NodeInfo;
class TreeItemModel;
class QLabel;

namespace Ui {
class DlgEditValue;
}

class DlgEditValue : public QDialog
{
    Q_OBJECT

public:
    explicit DlgEditValue(QWidget *parent = 0);
    ~DlgEditValue();

    void ModifyValue(NodeInfo* node, int node_type);
    void ModifyInitVarValue(BaseValueClass* v, QString var_type);
    void CreateValueForParentIfNode(NodeInfo* parent_node, const QString &var_type); //给CONDITION节点创建的Compare类型的子节点上的值
    void CreateNewValue(const QString &var_type, NodeInfo *parent_node);

    void ModifyCallNode(NodeInfo* function_node);
    void CreateCallNode();
    void SetUpforFunction();

    void SetModel(TreeItemModel* m);

    QString GetValueText();
    BaseValueClass* GetValuePointer();

    bool IsAccepted();

private slots:
    void on_DlgEditValue_accepted();

    void on_radioVariable_toggled(bool checked);
    void on_radioFunction_toggled(bool checked);
    void on_radioCustom_toggled(bool checked);
    void on_radioPreset_clicked(bool checked);
    void on_radioEvtParam_clicked(bool checked);

    // 多选框 - 选择函数
    void on_comboBoxFunction_currentIndexChanged(int index);
    // 编辑框 - 筛选函数
    void on_lineEdit_Func_textChanged(const QString &arg1);

    // 编辑函数的某个参数的值
    void onBtnParam1_clicked();
    void onBtnParam2_clicked();
    void onBtnParam3_clicked();
    void onBtnParam4_clicked();
    void onBtnParam5_clicked();
    void onBtnParam6_clicked();
    void onBtnParam7_clicked();
    void onBtnParam8_clicked();
    void onBtnParam9_clicked();
    void onBtnParam10_clicked();

private:
    void initEvtParamComboBox();
    void initVariableComboBox();
    void initPresetComboBox();
    void initFunctionComboBox();

    void resetFuncComboBox();

    void initUIforValue(const QString &var_type);
    void initUI_SetInitvalue(BaseValueClass* v);

    void setUIByValue(BaseValueClass* v);

    void setUIVisible_Var(bool can_see);
    void setUIVisible_EvtParam(bool can_see);
    void setUIVisible_Enum(bool can_see);

    void onBtnParam_clicked(int idx);
    void clearFuncTextUI();
    void updateFuncTextUI(FunctionClass* func);
    void addFuncParam(QPushButton* btn, FunctionClass* func, int param_id);
    void addFuncText(QLabel* lbl, FunctionClass* func, int text_id);

    void modifyUIParamValue(int uiparamid);
    FunctionClass* getFunctionInfoByUI();
    int findComboBoxFuncId(FunctionClass* f);

    Ui::DlgEditValue *ui;
    TreeItemModel* model;
    bool is_for_function;
    QVector<int> vectorFunctionInfo;

    NodeInfo* node;
    int value_position;

    VALUE_TYPE value_type;
    BaseValueClass* value;
    QString var_type;

    QVector<QPushButton*> funcParamBtns;
    QVector<QLabel*> funcTextLabels;
    QVector<BaseValueClass*> funcParams;

    bool is_accepted;
};

#endif // DLGEDITVALUE_H
