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

    void ModifyValue(NodeInfo* node, int value_id);
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
    void initVariableComboBox();
    void initFunctionComboBox();

    void setUpforValue();
    void updateFuncTextUI(FunctionClass* func);

    void onBtnParam_clicked(int idx);

    void addFuncParam(QPushButton* btn, FunctionClass* func, int param_id);
    void addFuncText(QLabel* lbl, FunctionClass* func, int text_id);

    Ui::DlgEditValue *ui;
    TreeItemModel* model;
    bool is_for_function;

    NodeInfo* node;
    int value_idx;

    VALUE_TYPE v_type;
    BaseValueClass* value;

    QVector<QPushButton*> funcParamBtns;
    QVector<QLabel*> funcTextLabels;
    QVector<BaseValueClass*> funcParams;
};

#endif // DLGEDITVALUE_H
