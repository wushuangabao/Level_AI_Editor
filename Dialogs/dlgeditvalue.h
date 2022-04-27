#ifndef DLGEDITVALUE_H
#define DLGEDITVALUE_H

#include <QDialog>
#include "../Values/valuemanager.h"

#define DEFINE_FUNCTION_ON_CLICKED(i) void onBtnParam##i##_clicked(){onBtnParam_clicked(i-1);}
const int FUNC_TEXT_NUM = 30;

class FunctionClass;
class NodeInfo;
class TreeItemModel;
class QLabel;
class DlgEditStructValue;

namespace Ui {
class DlgEditValue;
}

class DlgEditValue : public QDialog
{
    Q_OBJECT

public:
    explicit DlgEditValue(QWidget *parent = 0);
    ~DlgEditValue();

    void ModifyValueOnNode(NodeInfo* node, int node_type);
    void ModifyValue(CommonValueClass *v);
    void ModifyInitVarValue(BaseValueClass* b_v, StructValueClass* s_v, QString var_type);
    void CreateNewValueForParentNode(const QString &var_type, NodeInfo *parent_node);

    void ModifyCallNode(NodeInfo* function_node);
    void CreateCallNode();
    void SetUpforFunction();

    void SetModel(TreeItemModel* m);
    inline void SetValueType(VALUE_TYPE vt){value_type = vt;}

    void ResetNilValue();
    QString GetValueText();
    BaseValueClass *GetValuePointer_Base();
    StructValueClass *GetValuePointer_Struct();
    CommonValueClass *GetValuePointer_Common(bool *is_base_v = nullptr);

    bool IsAccepted();

private slots:
    void on_DlgEditValue_accepted();
    void on_DlgEditValue_rejected();

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
    DEFINE_FUNCTION_ON_CLICKED(1)
    DEFINE_FUNCTION_ON_CLICKED(2)
    DEFINE_FUNCTION_ON_CLICKED(3)
    DEFINE_FUNCTION_ON_CLICKED(4)
    DEFINE_FUNCTION_ON_CLICKED(5)
    DEFINE_FUNCTION_ON_CLICKED(6)
    DEFINE_FUNCTION_ON_CLICKED(7)
    DEFINE_FUNCTION_ON_CLICKED(8)
    DEFINE_FUNCTION_ON_CLICKED(9)
    DEFINE_FUNCTION_ON_CLICKED(10)
    DEFINE_FUNCTION_ON_CLICKED(11)
    DEFINE_FUNCTION_ON_CLICKED(12)
    DEFINE_FUNCTION_ON_CLICKED(13)
    DEFINE_FUNCTION_ON_CLICKED(14)
    DEFINE_FUNCTION_ON_CLICKED(15)
    DEFINE_FUNCTION_ON_CLICKED(16)
    DEFINE_FUNCTION_ON_CLICKED(17)
    DEFINE_FUNCTION_ON_CLICKED(18)
    DEFINE_FUNCTION_ON_CLICKED(19)
    DEFINE_FUNCTION_ON_CLICKED(20)
    DEFINE_FUNCTION_ON_CLICKED(21)
    DEFINE_FUNCTION_ON_CLICKED(22)
    DEFINE_FUNCTION_ON_CLICKED(23)
    DEFINE_FUNCTION_ON_CLICKED(24)
    DEFINE_FUNCTION_ON_CLICKED(25)
    DEFINE_FUNCTION_ON_CLICKED(26)
    DEFINE_FUNCTION_ON_CLICKED(27)
    DEFINE_FUNCTION_ON_CLICKED(28)
    DEFINE_FUNCTION_ON_CLICKED(29)
    DEFINE_FUNCTION_ON_CLICKED(30)

    void on_multiComboBox_Func_editTextChanged(const QString &arg1);

    void on_btnEditStructValue_clicked();

private:
    void initEvtParamComboBox();
    void initVariableComboBox();
    void initPresetComboBox();
    void initFunctionComboBox();

    void resetFuncComboBox();

    void initUIforValue(const QString &var_type, bool is_init_value = false);

    void setValueAndUI_Common(CommonValueClass* v);
    void setValueAndUI_Base(BaseValueClass* v);
    void setValueAndUI_Struct(StructValueClass* v);

    void setUIVisible_Var(bool can_see);
    void setUIVisible_EvtParam(bool can_see);
    void setUIVisible_Enum(bool can_see);

    void updateValueType();

    void setFuncParamValue(int idx, CommonValueClass* v);
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
    VALUE_TYPE value_type;
    QString var_type;

    BaseValueClass* value_base;
    StructValueClass* value_struct;

    QVector<QPushButton*> funcParamBtns;
    QVector<QLabel*> funcTextLabels;
    QVector<CommonValueClass*> funcParams_Value;

    bool is_accepted;

    DlgEditStructValue* dlgEditStruct;
};

#endif // DLGEDITVALUE_H
