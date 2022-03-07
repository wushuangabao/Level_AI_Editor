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
    void CreateValueForParentIfNode(NodeInfo* parent_node);

    void ModifyCallNode(NodeInfo* function_node);
    void SetUpforFunction();

    void SetModel(TreeItemModel* m);

    QString GetValueText();
    BaseValueClass* GetValuePointer();

private slots:
    void on_DlgEditValue_accepted();

    void on_radioVariable_toggled(bool checked);
    void on_radioFunction_toggled(bool checked);
    void on_radioCustom_toggled(bool checked);
    void on_radioPreset_clicked(bool checked);
    void on_radioEvtParam_clicked(bool checked);

    // 创建变量
    void on_btnSetValue_clicked();

    // 多选框
    void on_comboBoxFunction_currentIndexChanged(int index);
    void on_comboBox_Var_currentIndexChanged(int index);

    // 文本编辑框
    void on_lineEdit_textChanged(const QString &arg1);

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
    void initVariableComboBox(QString var_type);
    void initFunctionComboBox(QString var_type);

    void initUIforValue(QString var_type);
    void initUI_SetInitvalue(BaseValueClass* v);
    void setUIByValue(BaseValueClass* v);
    void setUIVisible_Var(bool can_see);
    void setUIVisible_EvtParam(bool can_see);
    void setUIVisible_Enum(bool can_see);

    void onBtnParam_clicked(int idx);
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

    VALUE_TYPE v_type;
    BaseValueClass* value;

    QVector<QPushButton*> funcParamBtns;
    QVector<QLabel*> funcTextLabels;
    QVector<BaseValueClass*> funcParams;
};

#endif // DLGEDITVALUE_H
