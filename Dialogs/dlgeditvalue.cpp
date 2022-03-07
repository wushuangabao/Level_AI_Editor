#include "../ItemModels/nodeinfo.h"
#include "../ItemModels/treeitemmodel.h"
#include "../ItemModels/functioninfo.h"
#include "dlgeditvalue.h"
#include "ui_dlgeditvalue.h"

DlgEditValue::DlgEditValue(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgEditValue)
{
    ui->setupUi(this);

    model = nullptr;
    node = nullptr;
    is_for_function = false;
    value = new BaseValueClass("0");

//    qDebug() << "Create DlgEditValue at" << (uintptr_t)this << endl;

    QString btnStyle = "background-color:transparent;"
        "color:blue;"
        "border:none;"
        "border-bottom:1px solid blue;"
        "text-align: left;";

    // 最多9个参数\9段描述文字
    for(int i = 0; i < FUNC_TEXT_NUM; i++)
    {
        QPushButton* btnParam = new QPushButton(this);
        btnParam->setStyleSheet(btnStyle);
        ui->functionLayout->addWidget(btnParam);
        btnParam->hide();

        QLabel* lblText = new QLabel(this);
        ui->functionLayout->addWidget(lblText);
        lblText->hide();

        funcParamBtns.append(btnParam);
        funcTextLabels.append(lblText);
        funcParams.append(new BaseValueClass("0"));
    }
    ui->functionLayout->addStretch();

    connect(funcParamBtns[0], SIGNAL(clicked()), this, SLOT(onBtnParam1_clicked()));
    connect(funcParamBtns[1], SIGNAL(clicked()), this, SLOT(onBtnParam2_clicked()));
    connect(funcParamBtns[2], SIGNAL(clicked()), this, SLOT(onBtnParam3_clicked()));
    connect(funcParamBtns[3], SIGNAL(clicked()), this, SLOT(onBtnParam4_clicked()));
    connect(funcParamBtns[4], SIGNAL(clicked()), this, SLOT(onBtnParam5_clicked()));
    connect(funcParamBtns[5], SIGNAL(clicked()), this, SLOT(onBtnParam6_clicked()));
    connect(funcParamBtns[6], SIGNAL(clicked()), this, SLOT(onBtnParam7_clicked()));
    connect(funcParamBtns[7], SIGNAL(clicked()), this, SLOT(onBtnParam8_clicked()));
    connect(funcParamBtns[8], SIGNAL(clicked()), this, SLOT(onBtnParam9_clicked()));
    connect(funcParamBtns[9], SIGNAL(clicked()), this, SLOT(onBtnParam10_clicked()));
}

DlgEditValue::~DlgEditValue()
{
//    qDebug() << "Delete DlgEditValue at" << (uintptr_t)this << endl;

    for(int i = 0; i < FUNC_TEXT_NUM; i++)
    {
        delete funcParamBtns[i];
        delete funcTextLabels[i];
        delete funcParams[i];
    }
    funcParamBtns.clear();
    funcTextLabels.clear();
    funcParams.clear();

    delete value;
    delete ui;
}

void DlgEditValue::ModifyValue(NodeInfo *node, int node_type)
{
    Q_ASSERT(node != nullptr);
    Q_ASSERT(model != nullptr);

    this->node = node;
    value_position = node_type;

    // 初始化value
    BaseValueClass* v = nullptr;
    if(value_position == 0)
    {
        // set_value节点
        v = model->GetValueManager()->GetValueOnNode_SetVar(node);
    }
    else if(value_position == 1)
    {
        // compare节点的左值
        v = model->GetValueManager()->GetValueOnNode_Compare_Left(node);
    }
    else if(value_position == 2)
    {
        // compare节点的右值
        v = model->GetValueManager()->GetValueOnNode_Compare_Right(node);
    }
    else
    {
        Q_ASSERT(false);
    }

    if(v != nullptr)
    {
        if(value_position == 0)
            initUIforValue(v->GetVarType());
        else
            initUIforValue("");
        setUIByValue(v);
    }
    else
        info("Modify Value 找不到编辑的值！");

    this->exec();
}

void DlgEditValue::ModifyInitVarValue(BaseValueClass *v, QString var_type)
{
    Q_ASSERT(model != nullptr);
    node = nullptr;

    initVariableComboBox(var_type);
    initFunctionComboBox(var_type);

    initUI_SetInitvalue(v);
    this->exec();
}

void DlgEditValue::CreateValueForParentIfNode(NodeInfo *parent_node)
{
    node = parent_node;
    initUIforValue(""); //还未确定值的变量类型
    this->exec();
}

void DlgEditValue::initUIforValue(QString var_type)
{
    Q_ASSERT(model != nullptr);

    is_for_function = false;

    initVariableComboBox(var_type);
    initFunctionComboBox(var_type);
}

void DlgEditValue::initUI_SetInitvalue(BaseValueClass *v)
{
    setUIVisible_EvtParam(false);
    setUIVisible_Var(false);

    if(v != nullptr)
    {
        setUIByValue(v);
    }
}

void DlgEditValue::SetUpforFunction()
{
    is_for_function = true;
    setWindowTitle("编辑Function");

    setUIVisible_Var(false);
    setUIVisible_EvtParam(false);
    setUIVisible_Enum(false);

    ui->radioCustom->setText("lua脚本：");
    ui->radioFunction->setChecked(true);

    initFunctionComboBox("");
}

void DlgEditValue::updateFuncTextUI(FunctionClass* func)
{
    for(int i = 0; i < FUNC_TEXT_NUM; i++)
    {
        funcParamBtns[i]->setVisible(false);
        funcTextLabels[i]->setVisible(false);
    }

    int start_pos = 1;
    int param_n = func->GetParamNum();
    if(func->param_is_before_text)
    {
        start_pos = 0;
    }

    if(param_n > 0)
    {
        for(int i = 0; i < param_n; i++)
        {
            addFuncParam(funcParamBtns[start_pos], func, i);
            start_pos ++;
        }
    }

    int text_n = func->GetTextNum();
    if(text_n > 0)
    {
        for(int i = 0; i < text_n; i++)
        {
            addFuncText(funcTextLabels[i], func, i);
        }
    }

    ui->label->setText(func->GetNote());
}

void DlgEditValue::setUIVisible_Var(bool can_see)
{
    ui->radioVariable->setVisible(can_see);
    ui->comboBox_Var->setVisible(can_see);
    ui->btnSetValue->setVisible(can_see);
}

void DlgEditValue::setUIVisible_EvtParam(bool can_see)
{
    ui->radioEvtParam->setVisible(can_see);
    ui->comboBox_EvtParam->setVisible(can_see);
}

void DlgEditValue::setUIVisible_Enum(bool can_see)
{
    ui->radioPreset->setVisible(can_see);
    ui->comboBox_Enum->setVisible(can_see);
}

void DlgEditValue::ModifyCallNode(NodeInfo *function_node)
{
    Q_ASSERT(function_node != nullptr);
    Q_ASSERT(model != nullptr);
    Q_ASSERT(function_node->type == FUNCTION);

    node = function_node;
    v_type = VT_FUNC;

    // 初始化value
    BaseValueClass* v = model->GetValueManager()->GetValueOnNode_Function(function_node);
    if(v != nullptr)
    {
        Q_ASSERT(v->GetValueType() == VT_FUNC);
        setUIByValue(v);
    }
    else
    {
        ui->comboBoxFunction->setCurrentIndex(0);
        on_comboBoxFunction_currentIndexChanged(0);
        value->SetFunction(&(FunctionInfo::GetInstance()->infoList[0]));
    }

    this->exec();
}

void DlgEditValue::SetModel(TreeItemModel *m)
{
    if(model == m)
        return;
    if(model != nullptr)
        delete model;
    model = m;
}

QString DlgEditValue::GetValueText()
{
    return value->GetText();
}

BaseValueClass* DlgEditValue::GetValuePointer()
{
    return value;
}

void DlgEditValue::on_DlgEditValue_accepted()
{
    Q_ASSERT(model != nullptr);

    // 给成员变量value赋值
    if(v_type == VT_FUNC)
    {
        FunctionClass* func = getFunctionInfoByUI();
        Q_ASSERT(func != nullptr);
        value->SetFunction(func);

//        qDebug() << "on_DlgEditValue_accepted" << endl;

        // 将funcParams中的参数值赋值给value的params成员
        int n = func->GetParamNum();
        if(n > 0)
        {
            int dlg_param_id = 0;
            if(!func->param_is_before_text)
                dlg_param_id++; //因为这种情况下UI的idx比实际参数的idx多1
            for(int i = 0; i < n; i++)
            {
                value->SetParamAt(i, funcParams[dlg_param_id]);
                dlg_param_id++;
            }
        }

        Q_ASSERT(value->GetFunctionParamsNum() == func->GetParamNum());
    }
    else if(v_type == VT_STR)
    {
        value->SetLuaStr(ui->lineEdit->text());
    }
    else if(v_type == VT_VAR)
    {
        QString v_name = ui->comboBox_Var->currentText();
        ValueManager* vm = model->GetValueManager();
        int i = vm->FindIdOfVarName(v_name);
        if(i != -1)
        {
            QString var_type = vm->GetVarTypeAt(i);
            value->SetVarName(v_name, var_type, i);
        }
    }

    // 如果传入了node
    if(node != nullptr)
    {
        // 更新ValueManager
        if(is_for_function)
        {
            // function 节点
            Q_ASSERT(node->type == FUNCTION);
            if(v_type == VT_FUNC || v_type == VT_STR)
            {
                model->GetValueManager()->UpdateValueOnNode_Function(node, value);
                node->text = value->GetText();
            }
            else
                info("错误的值类型");
        }
        else
        {
            if(value_position == 0)
            {
                // set_value节点
                model->GetValueManager()->UpdateValueOnNode_SetValue(node, value);
            }
            if(value_position == 1)
            {
                // compare节点的左值
                model->GetValueManager()->UpdateValueOnNode_Compare_Left(node, value);
            }
            else if(value_position == 2)
            {
                // compare节点的右值
                model->GetValueManager()->UpdateValueOnNode_Compare_Right(node, value);
            }
        }
    }
}

void DlgEditValue::on_radioVariable_toggled(bool checked)
{
    if(checked && v_type != VT_VAR)
    {
        v_type = VT_VAR;
        on_comboBox_Var_currentIndexChanged(ui->comboBox_Var->currentIndex());
    }
}

void DlgEditValue::on_radioFunction_toggled(bool checked)
{
    if(checked && v_type != VT_FUNC)
    {
        v_type = VT_FUNC;
    }
}

void DlgEditValue::on_radioCustom_toggled(bool checked)
{
    if(checked && v_type != VT_STR)
    {
        v_type = VT_STR;
    }
}

void DlgEditValue::on_btnSetValue_clicked()
{

}

void DlgEditValue::initVariableComboBox(QString var_type)
{
    Q_ASSERT(model != nullptr);
    Q_ASSERT(node != nullptr);

    // init comboBox_EvtParam
    QStringList* event_params = model->GetEventParamsOf(node);
    if(event_params->size() <= 0)
    {
        ui->comboBox_EvtParam->setEnabled(false);
        ui->radioEvtParam->setEnabled(false);
    }
    else
    {
        disconnect(ui->comboBox_EvtParam, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBox_EvtParam_currentIndexChanged(int)));
        ui->comboBox_EvtParam->clear();
        connect(ui->comboBox_EvtParam, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBox_EvtParam_currentIndexChanged(int)));
        ui->comboBox_EvtParam->addItems(*event_params);
        ui->comboBox_EvtParam->setEnabled(true);
        ui->radioEvtParam->setEnabled(true);
    }

    // init comboBox_Var
    ValueManager* vm = model->GetValueManager();
    if(vm->GetGlobalVarList().size() <= 0)
    {
        ui->comboBox_Var->setEnabled(false);
        ui->radioVariable->setEnabled(false);
    }
    else
    {
        disconnect(ui->comboBox_Var, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBox_Var_currentIndexChanged(int)));
        ui->comboBox_Var->clear();
        connect(ui->comboBox_Var, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBox_Var_currentIndexChanged(int)));
        ui->comboBox_Var->addItems(vm->GetGlobalVarList());
        ui->comboBox_Var->setEnabled(true);
        ui->radioVariable->setEnabled(true);
        ui->radioVariable->setChecked(true);
    }


}

void DlgEditValue::initFunctionComboBox(QString var_type)
{
    disconnect(ui->comboBoxFunction, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxFunction_currentIndexChanged(int)));
    ui->comboBoxFunction->clear();
    connect(ui->comboBoxFunction, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxFunction_currentIndexChanged(int)));

    FunctionInfo* functions = FunctionInfo::GetInstance();
    int n = functions->infoList.size();
    if(n > 0)
    {
        vectorFunctionInfo.clear();
        QStringList items;
        for(int i = 0; i < n; i++)
        {
            // 编辑 value 节点时，只添加 特定类型返回值 的函数
            // 这就导致 infoList 中函数和 comboBoxFunction 中的函数名不是一一对应的了
            // 所以这里添加函数名时，要记录一下两者的对应关系
            if(!is_for_function && functions->infoList[i].GetReturnNum() <= 0)
                continue;
            if(var_type != "" && functions->infoList[i].GetReturnTypeAt(0) != var_type)
                continue;
            vectorFunctionInfo.push_back(i);
            items.push_back(functions->infoList[i].GetName());
        }
        ui->comboBoxFunction->addItems(items);
    }
    else
    {
        info("没有读取到任何函数信息！");
    }
}

void DlgEditValue::addFuncParam(QPushButton* btn, FunctionClass *func, int param_id)
{
    btn->setText(func->GetParamTypeAt(param_id));
    btn->show();
}

void DlgEditValue::addFuncText(QLabel *lbl, FunctionClass *func, int text_id)
{
    lbl->setText(func->GetTextAt(text_id));
    lbl->show();
}

void DlgEditValue::modifyUIParamValue(int idx)
{
    DlgEditValue* dlg = new DlgEditValue();
    dlg->SetModel(model);
    dlg->node = node;

    FunctionClass* func = getFunctionInfoByUI();
    if(func == nullptr)
    {
        delete dlg;
        Q_ASSERT(func != nullptr);
        return;
    }

    QPoint widget_pos = this->mapToGlobal(QPoint(0, 0));
    dlg->move(widget_pos.x() + 5, widget_pos.y() + 5);

    int param_id = idx;
    if(!func->param_is_before_text)
        param_id--; //因为这种情况下UI的idx比实际参数的idx多1
    dlg->setWindowTitle("设置：" + func->GetParamTypeAt(param_id));

    BaseValueClass* param = funcParams[idx];
    *(dlg->value) = *param;
    dlg->initUIforValue(func->GetParamTypeAt(param_id));
    dlg->setUIByValue(param);
    dlg->exec();
    *(funcParams[idx]) = *(dlg->GetValuePointer());

    delete dlg;
}

FunctionClass *DlgEditValue::getFunctionInfoByUI()
{
    int idx = ui->comboBoxFunction->currentIndex();
    Q_ASSERT(vectorFunctionInfo.size() > idx);
    return &(FunctionInfo::GetInstance()->infoList[vectorFunctionInfo[idx]]);
}

int DlgEditValue::findComboBoxFuncId(FunctionClass* f)
{
    FunctionInfo* functions = FunctionInfo::GetInstance();
    int n = functions->infoList.size();
    if(n > 0)
    {
        for(int i = 0; i < vectorFunctionInfo.size(); i++)
        {
            if( &(functions->infoList[vectorFunctionInfo[i]]) == f)
            {
                return i;
            }
        }
    }
    return -1;
}

void DlgEditValue::on_comboBoxFunction_currentIndexChanged(int index)
{
    Q_UNUSED(index);

    FunctionClass* func = getFunctionInfoByUI();
    Q_ASSERT(func != nullptr);

    updateFuncTextUI(func);
}

void DlgEditValue::on_comboBox_Var_currentIndexChanged(int index)
{
    // 可能需要检查 index 越界
}

void DlgEditValue::on_lineEdit_textChanged(const QString &arg1)
{
}

void DlgEditValue::onBtnParam_clicked(int idx)
{
    Q_ASSERT(idx < FUNC_TEXT_NUM);
    Q_ASSERT(idx >= 0);

    modifyUIParamValue(idx);

    QString param_text = funcParams[idx]->GetText();
    funcParamBtns[idx]->setText(param_text);
}

void DlgEditValue::setUIByValue(BaseValueClass *v)
{
    v_type = v->GetValueType();
    switch (v_type) {
    case VT_VAR:
    {
        ui->radioVariable->setChecked(true);
        int idx = ui->comboBox_Var->findText(v->GetText());
        if(idx != -1)
        {
            ui->comboBox_Var->setCurrentIndex(idx);
        }
        else
        {
            info("ComboBoxVar里找不到变量" + v->GetText());
            if(ui->comboBox_Var->count() > 0)
            {
                ui->comboBox_Var->setCurrentIndex(0);
            }
            else
            {
                info("没有可选的变量！");
                ui->radioFunction->setChecked(true);
                ui->radioVariable->setCheckable(false);
                ui->comboBox_Var->setDisabled(true);
            }
        }
        break;
    }
    case VT_FUNC:
    {
        ui->radioFunction->setChecked(true);
        int idx = findComboBoxFuncId(v->GetFunctionInfo());
        if(idx != -1)
        {
            ui->comboBoxFunction->setCurrentIndex(idx);
            on_comboBoxFunction_currentIndexChanged(idx);

            // 设置funcParamBtns
            FunctionClass* func = getFunctionInfoByUI();
            if(func == v->GetFunctionInfo())
            {
                int param_num = func->GetParamNum();
                for(int i = 0; i < param_num; i++)
                {
                    BaseValueClass* param = v->GetFunctionParamAt(i);
                    int id_ui = i;
                    if(!func->param_is_before_text)
                        id_ui++; //这种情况下UI的idx比实际参数的idx多1
                    *(funcParams[id_ui]) = *param;
                    funcParamBtns[id_ui]->setText(param->GetText());
                }
            }
            else
                info("初始化UI，设置函数时出错！");
        }
        else
        {
            info("ComboBoxFunc里找不到这个值中包含的函数！");
            ui->comboBoxFunction->setCurrentIndex(0);
            on_comboBoxFunction_currentIndexChanged(0);
        }
        break;
    }
    case VT_STR:
        ui->radioCustom->setChecked(true);
        ui->lineEdit->setText(v->GetText());
        break;
    case VT_ENUM:
    {
        ui->radioPreset->setChecked(true);
        int idx = ui->comboBox_Enum->findText(v->GetText());
        if(idx != -1)
        {
            ui->comboBox_Enum->setCurrentIndex(idx);
            // on_comboBox......
        }
        else
            info("ComboBox_Enum里找不到这个值中包含的预设值！");
    }
        break;
    case VT_PARAM:
    {
        ui->radioEvtParam->setChecked(true);
        int idx = ui->comboBox_EvtParam->findText(v->GetText());
        if(idx != -1)
        {
            ui->comboBox_EvtParam->setCurrentIndex(idx);
            // on_comboBox......
        }
        else
            info("ComboBox_EvtParam里找不到这个值中包含的参数！");
    }
        break;
    default:
        info("未知的值类型！");
        break;
    }

    *value = *v;
}

void DlgEditValue::onBtnParam1_clicked()
{
    onBtnParam_clicked(0);
}
void DlgEditValue::onBtnParam2_clicked()
{
    onBtnParam_clicked(1);
}
void DlgEditValue::onBtnParam3_clicked()
{
    onBtnParam_clicked(2);
}
void DlgEditValue::onBtnParam4_clicked()
{
    onBtnParam_clicked(3);
}
void DlgEditValue::onBtnParam5_clicked()
{
    onBtnParam_clicked(4);
}
void DlgEditValue::onBtnParam6_clicked()
{
    onBtnParam_clicked(5);
}
void DlgEditValue::onBtnParam7_clicked()
{
    onBtnParam_clicked(6);
}
void DlgEditValue::onBtnParam8_clicked()
{
    onBtnParam_clicked(7);
}
void DlgEditValue::onBtnParam9_clicked()
{
    onBtnParam_clicked(8);
}
void DlgEditValue::onBtnParam10_clicked()
{
    onBtnParam_clicked(9);
}

void DlgEditValue::on_radioPreset_clicked(bool checked)
{
    if(checked && v_type != VT_ENUM)
    {
        v_type = VT_ENUM;
//        on_comboBox_Preset_currentIndexChanged(ui->comboBox_Preset->currentIndex());
    }
}

void DlgEditValue::on_radioEvtParam_clicked(bool checked)
{
    if(checked && v_type != VT_PARAM)
    {
        v_type = VT_PARAM;
    }
}
