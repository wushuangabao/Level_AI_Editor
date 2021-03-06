#include <QPushButton>
#include "../Values/enuminfo.h"
#include "../Values/structinfo.h"
#include "../ItemModels/nodeinfo.h"
#include "../ItemModels/treeitemmodel.h"
#include "../ItemModels/functioninfo.h"
#include "multiselectcombobox.h"
#include "multilevelcombobox.h"
#include "dlgeditstructvalue.h"
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
    is_init_value = false;
    value_base = new BaseValueClass("nil");
    value_struct = nullptr;
    is_accepted = false;

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
        funcParams_Value.append(new BaseValueClass("nil"));
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
    connect(funcParamBtns[10], SIGNAL(clicked()), this, SLOT(onBtnParam11_clicked()));
    connect(funcParamBtns[11], SIGNAL(clicked()), this, SLOT(onBtnParam12_clicked()));
    connect(funcParamBtns[12], SIGNAL(clicked()), this, SLOT(onBtnParam13_clicked()));
    connect(funcParamBtns[13], SIGNAL(clicked()), this, SLOT(onBtnParam14_clicked()));
    connect(funcParamBtns[14], SIGNAL(clicked()), this, SLOT(onBtnParam15_clicked()));
    connect(funcParamBtns[15], SIGNAL(clicked()), this, SLOT(onBtnParam16_clicked()));
    connect(funcParamBtns[16], SIGNAL(clicked()), this, SLOT(onBtnParam17_clicked()));
    connect(funcParamBtns[17], SIGNAL(clicked()), this, SLOT(onBtnParam18_clicked()));
    connect(funcParamBtns[18], SIGNAL(clicked()), this, SLOT(onBtnParam19_clicked()));
    connect(funcParamBtns[19], SIGNAL(clicked()), this, SLOT(onBtnParam20_clicked()));
    connect(funcParamBtns[20], SIGNAL(clicked()), this, SLOT(onBtnParam21_clicked()));
    connect(funcParamBtns[21], SIGNAL(clicked()), this, SLOT(onBtnParam22_clicked()));
    connect(funcParamBtns[22], SIGNAL(clicked()), this, SLOT(onBtnParam23_clicked()));
    connect(funcParamBtns[23], SIGNAL(clicked()), this, SLOT(onBtnParam24_clicked()));
    connect(funcParamBtns[24], SIGNAL(clicked()), this, SLOT(onBtnParam25_clicked()));
    connect(funcParamBtns[25], SIGNAL(clicked()), this, SLOT(onBtnParam26_clicked()));
    connect(funcParamBtns[26], SIGNAL(clicked()), this, SLOT(onBtnParam27_clicked()));
    connect(funcParamBtns[27], SIGNAL(clicked()), this, SLOT(onBtnParam28_clicked()));
    connect(funcParamBtns[28], SIGNAL(clicked()), this, SLOT(onBtnParam29_clicked()));
    connect(funcParamBtns[29], SIGNAL(clicked()), this, SLOT(onBtnParam30_clicked()));

    ui->multiComboBox_Func->addItems(FunctionInfo::GetInstance()->GetTagList());
    ui->multiComboBox_Func->stateChange(0);

    dlgEditStruct = new DlgEditStructValue(this);
}

DlgEditValue::~DlgEditValue()
{
//    qDebug() << "Delete DlgEditValue at" << (uintptr_t)this << endl;

    for(int i = 0; i < FUNC_TEXT_NUM; i++)
    {
        delete funcParamBtns[i];
        delete funcTextLabels[i];
        delete funcParams_Value[i];
    }
    funcParamBtns.clear();
    funcTextLabels.clear();
    funcParams_Value.clear();

    if(value_base != nullptr)
    {
        delete value_base;
        value_base = nullptr;
    }
    if(value_struct != nullptr)
    {
        delete value_struct;
        value_struct = nullptr;
    }
    delete ui;
}

void DlgEditValue::ModifyValueOnNode(NodeInfo *node, int value_position)
{
    MY_ASSERT(node != nullptr);
    MY_ASSERT(model != nullptr);

    this->node = node;

    // 初始化value
    CommonValueClass* v = nullptr;
    QString var_type = "";
    if(value_position == 0)
    {
        // set_value节点
        v = model->GetValueManager()->GetValueOnNode_SetVar(node);
        var_type = v->GetVarType();
    }
    else if(value_position == 1)
    {
        // compare节点的左值
        v = model->GetValueManager()->GetValueOnNode_Compare_Left(node);
        var_type = model->GetValueManager()->GetValueOnNode_Compare_Right(node)->GetVarType();
        this->var_type = var_type;
    }
    else if(value_position == 2)
    {
        // compare节点的右值
        v = model->GetValueManager()->GetValueOnNode_Compare_Right(node);
        var_type = model->GetValueManager()->GetValueOnNode_Compare_Left(node)->GetVarType();
        this->var_type = var_type;
    }
    else
    {
        MY_ASSERT(false);
    }

    if(v != nullptr)
    {
        initUIforValue(var_type);
        setValueAndUI_Common(v);
    }
    else
        info("Modify Value 找不到编辑的值！");

    this->exec();
}

void DlgEditValue::ModifyValue(CommonValueClass *v)
{
    MY_ASSERT(model != nullptr);
    MY_ASSERT(v != nullptr);
    this->node = nullptr;
    initUIforValue(v->GetVarType());

    setValueAndUI_Common(v);

    value_base->SetVarType(var_type);
    this->exec();
}

void DlgEditValue::ModifyInitVarValue(BaseValueClass *b_v, StructValueClass *s_v, QString var_type)
{
    MY_ASSERT(model != nullptr);
    MY_ASSERT(b_v != nullptr);
    MY_ASSERT(s_v != nullptr);
    MY_ASSERT(is_init_value == true);
    node = nullptr;

    if(var_type != "" && var_type != b_v->GetVarType())
        b_v->ClearData();

    initUIforValue(var_type);
    setValueAndUI_Base(b_v);

    if(StructInfo::GetInstance()->CheckIsStruct(var_type))
    {
        if(s_v->GetVarType() != var_type)
            s_v->SetVarType(var_type);
        setValueAndUI_Struct(s_v);
    }

    this->exec();
}

void DlgEditValue::CreateNewValueForParentNode(const QString& var_type, NodeInfo* p_node)
{
    MY_ASSERT(model != nullptr);
    node = p_node;

    value_base->SetLuaStr("nil");
    value_base->SetVarType(var_type);
    if(value_struct == nullptr)
        value_struct = new StructValueClass();
    if(StructInfo::GetInstance()->CheckIsStruct(var_type))
        value_struct->SetVarType(var_type);

    initUIforValue(var_type);
    this->exec();
}

void DlgEditValue::initUIforValue(const QString &var_type)
{
    MY_ASSERT(model != nullptr);

    is_for_function = false;
    this->var_type = var_type;

    if(!is_init_value)
    {
        setUIVisible_EvtParam(true);
        setUIVisible_Var(true);

        initEvtParamComboBox();
        initVariableComboBox();
    }

    if(var_type != "")
        setWindowTitle("设置：" + var_type);
    else
        setWindowTitle("设置：Value");

    bool is_struct = StructInfo::GetInstance()->CheckIsStruct(var_type);
    setUIVisible_Enum(!is_struct);
    if(!is_struct)
        initPresetComboBox();
    ui->btnEditStructValue->setVisible(is_struct);

    initFunctionComboBox();

    if(ui->radioEvtParam->isEnabled())
        ui->radioEvtParam->setChecked(true);
    else if(ui->radioCustom->isEnabled())
        ui->radioCustom->setChecked(true);
    else if(ui->radioPreset->isEnabled())
        ui->radioPreset->setChecked(true);
    else if(ui->radioVariable->isEnabled())
        ui->radioVariable->setChecked(true);
    else if(ui->radioFunction->isEnabled())
        ui->radioFunction->setChecked(true);

    updateValueType();
}

void DlgEditValue::setValueAndUI_Common(CommonValueClass *v)
{
    if(v->GetValueType() < VT_TABLE)
        setValueAndUI_Base((BaseValueClass*)v);
    else
        setValueAndUI_Struct((StructValueClass*)v);
}

void DlgEditValue::SetUpforFunction()
{
    is_for_function = true;
    var_type = "";

    setWindowTitle("编辑Function");

    setUIVisible_Var(false);
    setUIVisible_EvtParam(false);
    setUIVisible_Enum(false);

    ui->radioCustom->setText("lua脚本：");
    ui->radioFunction->setChecked(true);

    initFunctionComboBox();
}

void DlgEditValue::SetUpforInitValue(bool ok)
{
    is_init_value = ok;
    setUIVisible_EvtParam(!ok);
    setUIVisible_Var(!ok);
    dlgEditStruct->SetUpForInitValue(ok);
}

void DlgEditValue::updateFuncTextUI(FunctionClass* func)
{
    clearFuncTextUI();

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
    ui->radioVariable->setEnabled(can_see);
    ui->radioVariable->setVisible(can_see);
    ui->comboBox_Var->setVisible(can_see);
}

void DlgEditValue::setUIVisible_EvtParam(bool can_see)
{
    ui->radioEvtParam->setEnabled(can_see);
    ui->radioEvtParam->setVisible(can_see);
    ui->comboBox_EvtParam->setVisible(can_see);
}

void DlgEditValue::setUIVisible_Enum(bool can_see)
{
    ui->radioPreset->setEnabled(can_see);
    ui->radioPreset->setVisible(can_see);
    ui->comboBox_Enum->setVisible(can_see);
}

void DlgEditValue::updateValueType()
{
    if(ui->radioEvtParam->isChecked())
        value_type = VT_PARAM;
    else if(ui->radioCustom->isChecked())
    {
        if(StructInfo::GetInstance()->CheckIsStruct(var_type))
        {
            QString line = ui->lineEdit->text();
            line.replace(" ", "");
            if(line == "nil" || (line.size() >= 2 && line.at(0) == '{'))
                value_type = VT_TABLE;
            else
                value_type = VT_STR;
        }
        else
            value_type = VT_STR;
    }
    else if(ui->radioPreset->isChecked())
        value_type = VT_ENUM;
    else if(ui->radioVariable->isChecked())
        value_type = VT_VAR;
    else if(ui->radioFunction->isChecked())
        value_type = VT_FUNC;
}

void DlgEditValue::setFuncParamValue(int idx, CommonValueClass *v)
{
    delete funcParams_Value[idx];
    if(v->GetValueType() < VT_TABLE)
        funcParams_Value[idx] = new BaseValueClass((BaseValueClass*)v);
    else if(v->GetValueType() == VT_TABLE)
        funcParams_Value[idx] = new StructValueClass((StructValueClass*)v);
}

void DlgEditValue::ModifyCallNode(NodeInfo *function_node)
{
    MY_ASSERT(function_node != nullptr);
    MY_ASSERT(model != nullptr);
    MY_ASSERT(function_node->type == FUNCTION);

    node = function_node;
    ui->lineEdit_Func->setText("");
    ui->multiComboBox_Func->ResetSelection();

    // 初始化value
    BaseValueClass* v = model->GetValueManager()->GetValueOnNode_Function(function_node);
    if(v != nullptr)
    {
        value_type = v->GetValueType();
        MY_ASSERT(value_type == VT_FUNC || value_type == VT_STR);
        setValueAndUI_Base(v);
    }
    else
    {
        value_type = VT_FUNC;
        ui->comboBoxFunction->setCurrentIndex(0);
        on_comboBoxFunction_currentIndexChanged(0);
        value_base->SetFunction(getFunctionInfoByUI());
    }

    this->exec();
}

void DlgEditValue::CreateCallNode()
{
    MY_ASSERT(model != nullptr);
    value_type = VT_FUNC;

    ui->lineEdit->setText("");
    ui->lineEdit_Func->setText("");
    ui->radioFunction->setChecked(true);
    ui->comboBoxFunction->setCurrentIndex(0);
    on_comboBoxFunction_currentIndexChanged(0);

    value_base->SetFunction(getFunctionInfoByUI());

    this->exec();
}

void DlgEditValue::SetModel(TreeItemModel *m)
{
    if(model == m)
        return;
    model = m;
}

void DlgEditValue::ResetNilValue()
{
    if(value_struct != nullptr)
    {
        delete value_struct;
        value_struct = nullptr;
    }
    value_base->ClearData();
    setValueAndUI_Base(value_base);
}

QString DlgEditValue::GetValueText()
{
    if(value_type < VT_TABLE)
        return value_base->GetText();
    else if(value_type == VT_TABLE)
        return value_struct->GetText();
    else
    {
        info("GetValueText: value_type error!");
        return "nil";
    }
}

BaseValueClass *DlgEditValue::GetValuePointer_Base()
{
    return value_base;
}

StructValueClass *DlgEditValue::GetValuePointer_Struct()
{
    if(value_struct == nullptr)
        value_struct = new StructValueClass();
    return value_struct;
}

CommonValueClass *DlgEditValue::GetValuePointer_Common(bool* is_base_v)
{
    if(value_type == VT_TABLE)
    {
        if(is_base_v != nullptr)
            *is_base_v = false;
        return value_struct;
    }
    else
    {
        if(is_base_v != nullptr)
            *is_base_v = true;
        return value_base;
    }
}

bool DlgEditValue::IsAccepted()
{
    return is_accepted;
}

void DlgEditValue::on_DlgEditValue_accepted()
{
    MY_ASSERT(model != nullptr);

    updateValueType();

    // 给成员变量value赋值
    switch (value_type) {
    case VT_FUNC:
    {
        FunctionClass* func = getFunctionInfoByUI();
        if(func == nullptr)
        {
            info("选择了无效函数");
            is_accepted = false;
            return;
        }
        else
        {
            value_base->SetFunction(func);
            // 将funcParams中的参数值赋值给value的params成员
            int n = func->GetParamNum();
            if(n > 0)
            {
                int dlg_param_id = 0;
                if(!func->param_is_before_text)
                    dlg_param_id++; //因为这种情况下UI的idx比实际参数的idx多1
                for(int i = 0; i < n; i++)
                {
                    value_base->SetParamAt(i, funcParams_Value[dlg_param_id]);
                    dlg_param_id++;
                }
            }
            MY_ASSERT(value_base->GetFunctionParamsNum() == func->GetParamNum());
        }
    }
        break;
    case VT_STR:
    {
        value_base->SetLuaStr(ui->lineEdit->text(), var_type);
    }
        break;
    case VT_VAR:
    {
        QString v_name = ui->comboBox_Var->currentText();
        if(v_name.isEmpty())
        {
            info("你没有选择任何变量");
            return;
        }
        ValueManager* vm = model->GetValueManager();
        QString var_name = ValueManager::GetVarNameInKeyStr(v_name);
        int i = vm->FindIdOfVarName(var_name);
        if(i != -1)
        {
            QString var_type = vm->GetVarTypeOf_Key(v_name);
            value_base->SetVarName(var_name, var_type, i, ValueManager::GetKeyNameInKeyStr(v_name));
        }
    }
        break;
    case VT_PARAM:
    {
        QString str = ui->comboBox_EvtParam->currentText();
        QStringList* event_params = model->GetEventParamsUIOf(node);
        if(event_params == nullptr)
        {
            info("不存在事件参数！");
            return;
        }
        bool ok = false;
        int eid = model->findUppestNodeOf(node)->childs[0]->childs[0]->getValue(0).toInt(&ok);
        MY_ASSERT(ok);
        int pid = event_params->indexOf(str);
        value_base->SetEvtParam(eid, pid);
    }
        break;
    case VT_ENUM:
    {
        QString str = ui->comboBox_Enum->currentText();
        if(value_base->GetVarType() == "")
            value_base->SetVarType(var_type);
        value_base->SetEnumValue(str);
    }
        break;
    case VT_TABLE:
    {
        // 什么也不做，让调用方判断IsAccepted之后自己取value_struct的值
    }
        break;
    default:
        break;
    }

    // ModifyCallNode结束时更新ValueManager
    if(is_for_function && node != nullptr)
    {
        // function 节点
        MY_ASSERT(node->type == FUNCTION);
        if(value_type == VT_FUNC || value_type == VT_STR)
        {
            model->GetValueManager()->UpdateValueOnNode_Function(node, value_base);
            node->text = value_base->GetText();
        }
        else
            info("错误的值类型");
    }

    // 将funcParams还原为初始值
    for(int i = 0; i < FUNC_TEXT_NUM; i++)
    {
        delete funcParams_Value[i];
        funcParams_Value[i] = new BaseValueClass("nil");
    }

    is_accepted = true;
}

void DlgEditValue::on_radioVariable_toggled(bool checked)
{
    if(checked && value_type != VT_VAR)
    {
        value_type = VT_VAR;
    }
}

void DlgEditValue::on_radioFunction_toggled(bool checked)
{
    if(checked && value_type != VT_FUNC)
    {
        value_type = VT_FUNC;
    }
}

void DlgEditValue::on_radioCustom_toggled(bool checked)
{
    if(checked && value_type != VT_STR)
    {
        value_type = VT_STR;
    }
}

void DlgEditValue::initVariableComboBox()
{
    MY_ASSERT(model != nullptr);

    // init comboBox_Var
    ValueManager* vm = model->GetValueManager();
    QStringList v_list = vm->GetGlobalVarList();
    QStringList items;
    int n = v_list.size();
    for(int i = 0; i < n; i++)
    {
        QString my_var_type = vm->GetVarTypeOf_Key(v_list[i]);
        if(var_type == "" || my_var_type == var_type || StructInfo::GetInstance()->CheckIsStruct(my_var_type))
            items.append(QString(v_list.at(i)));
    }
    if(items.size() <= 0)
    {
        ui->comboBox_Var->setEnabled(false);
        ui->radioVariable->setEnabled(false);
    }
    else
    {
        ui->comboBox_Var->SetVarType(var_type);
        ui->comboBox_Var->clear();
        ui->comboBox_Var->addItems(items);
        ui->comboBox_Var->setEnabled(true);
        ui->radioVariable->setEnabled(true);
    }
}

void DlgEditValue::initPresetComboBox()
{
    EnumInfo* enum_info = EnumInfo::GetInstance();
    QStringList str_list = enum_info->GetEnumsOfType(var_type);
    if(str_list.size() <= 0)
    {
        ui->comboBox_Enum->setEnabled(false);
        ui->radioPreset->setEnabled(false);
    }
    else
    {
//        disconnect(ui->comboBox_Enum, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBox_Enum_currentIndexChanged(int)));
        ui->comboBox_Enum->clear();
//        connect(ui->comboBox_Enum, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBox_Enum_currentIndexChanged(int)));
        ui->comboBox_Enum->addItems(str_list);
        ui->comboBox_Enum->setEnabled(true);
        ui->radioPreset->setEnabled(true);
    }
}

void DlgEditValue::initFunctionComboBox()
{
    ui->lineEdit_Func->setText("");

    resetFuncComboBox();
}

void DlgEditValue::resetFuncComboBox()
{
    disconnect(ui->comboBoxFunction, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxFunction_currentIndexChanged(int)));
    ui->comboBoxFunction->clear();
    connect(ui->comboBoxFunction, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxFunction_currentIndexChanged(int)));

    FunctionInfo* functions = FunctionInfo::GetInstance();
    int n = functions->GetFunctionInfoCount();
    if(n <= 0)
    {
        info("没有读取到任何函数信息！");
        return;
    }

    vectorFunctionInfo.clear();
    QStringList items;
    QStringList filters = ui->lineEdit_Func->text().split(' ', QString::SkipEmptyParts);
    QStringList tags = ui->multiComboBox_Func->currentText();

    for(int i = 0; i < n; i++)
    {
        FunctionClass* func = functions->GetFunctionInfoAt(i);
        // 编辑 value 节点时，只添加 特定类型返回值 的函数
        // 这就导致 infoList 中函数和 comboBoxFunction 中的函数名不是一一对应的了
        // 所以这里添加函数名时，要记录一下两者的对应关系
        if(!is_for_function && func->GetReturnNum() <= 0)
            continue;
        if(is_for_function && !func->CanBeCall())
            continue;
        if(var_type != "" && func->GetReturnTypeAt(0) != var_type)
            continue;

        // 过滤函数名
        QString func_name = func->GetNameUI();
        bool ok = true;
        for(int j = 0; j < filters.size(); j++)
        {
            if(!func_name.contains(filters[j]))
            {
                ok = false;
                break;
            }
        }

        // 过滤标签
        int tags_n = tags.size();
        bool ok_tag = false;
        if(!(tags_n == 1 && tags[0] == "All"))
        {
            for(int j = 0; j < tags_n; j++)
            {
                if(FunctionInfo::GetInstance()->CheckFunctionInTag(func, tags[j]))
                {
                    ok_tag = true;
                    break;
                }
            }
        }
        else
            ok_tag = true;

        if(ok && ok_tag)
        {
            vectorFunctionInfo.push_back(i);
            items.push_back(func_name);
        }
    }
    ui->comboBoxFunction->addItems(items);

    if(items.size() <= 0)
    {
        ui->radioFunction->setChecked(false);
        ui->radioFunction->setEnabled(false);
        ui->comboBoxFunction->setEnabled(false);
        if(ui->lineEdit_Func->text() == "")
        {
            ui->lineEdit_Func->setEnabled(false);
        }
        clearFuncTextUI();
    }
    else
    {
        ui->radioFunction->setChecked(true);
        ui->radioFunction->setEnabled(true);
        ui->comboBoxFunction->setEnabled(true);
        ui->lineEdit_Func->setEnabled(true);
        ui->comboBoxFunction->setCurrentIndex(-1);
        on_comboBoxFunction_currentIndexChanged(-1);
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
    FunctionClass* func = getFunctionInfoByUI();
    if(func == nullptr)
    {
        MY_ASSERT(func != nullptr);
        return;
    }

    QString param_type;
    if(!func->param_is_before_text)
        param_type = func->GetParamTypeAt(idx - 1); //因为这种情况下UI的idx比实际参数的idx多1
    else
        param_type = func->GetParamTypeAt(idx);

    DlgEditValue* dlg = new DlgEditValue();
    dlg->SetUpforInitValue(is_init_value);
    dlg->SetModel(model);
    dlg->node = node;
    QPoint widget_pos = this->mapToGlobal(QPoint(0, 0));
    dlg->move(widget_pos.x() + 5, widget_pos.y() + 5);
    dlg->setWindowTitle("设置：" + param_type);

    CommonValueClass* param = funcParams_Value[idx];
    dlg->initUIforValue(param_type);
    dlg->ResetNilValue();
    dlg->setValueAndUI_Common(param);
    dlg->exec();
    if(dlg->IsAccepted())
    {
        setFuncParamValue(idx, dlg->GetValuePointer_Common());
        funcParamBtns[idx]->setText(funcParams_Value[idx]->GetText());
    }
    delete dlg;
}

FunctionClass *DlgEditValue::getFunctionInfoByUI()
{
    int idx = ui->comboBoxFunction->currentIndex();
    if(idx < 0)
        return nullptr;
    return FunctionInfo::GetInstance()->GetFunctionInfoAt(vectorFunctionInfo[idx]);
}

int DlgEditValue::findComboBoxFuncId(FunctionClass* f)
{
    FunctionInfo* functions = FunctionInfo::GetInstance();
    for(int i = 0; i < vectorFunctionInfo.size(); i++)
    {
        if( functions->GetFunctionInfoAt(vectorFunctionInfo[i]) == f)
        {
            return i;
        }
    }
    return -1;
}

void DlgEditValue::on_comboBoxFunction_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    if(index == -1)
    {
        clearFuncTextUI();
        ui->label->setText("");
    }
    else
    {
        FunctionClass* func = getFunctionInfoByUI();
        MY_ASSERT(func != nullptr);

        updateFuncTextUI(func);
    }
}

void DlgEditValue::onBtnParam_clicked(int idx)
{
    MY_ASSERT(idx < FUNC_TEXT_NUM);
    MY_ASSERT(idx >= 0);

    modifyUIParamValue(idx);
}

void DlgEditValue::clearFuncTextUI()
{
    for(int i = 0; i < FUNC_TEXT_NUM; i++)
    {
        delete funcParams_Value[i];
        funcParams_Value[i] = new BaseValueClass("nil");
        funcParamBtns[i]->setVisible(false);
        funcTextLabels[i]->setVisible(false);
    }
}

void DlgEditValue::setValueAndUI_Base(BaseValueClass *v)
{
    value_type = v->GetValueType();
    switch (value_type) {
    case VT_VAR:
    {
        ui->radioVariable->setChecked(true);
        ui->comboBox_Var->SetLineText(v->GetText());
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
                    CommonValueClass* param = v->GetFunctionParamAt(i);
                    int id_ui = i;
                    if(!func->param_is_before_text)
                        id_ui++; //这种情况下UI的idx比实际参数的idx多1
                    setFuncParamValue(id_ui, param);
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
    case VT_TABLE:
    {
        info("setValueAndUI类型错误");
        return;
    }
    default:
        info("未知的值类型！");
        break;
    }

    *value_base = *v;
}

void DlgEditValue::setValueAndUI_Struct(StructValueClass *v)
{
    MY_ASSERT(v->GetValueType() == VT_TABLE);
    value_type = VT_TABLE;

    ui->lineEdit->setText(v->GetLuaValueString(""));
    ui->radioCustom->setChecked(true);

    if(value_struct == nullptr)
        value_struct = new StructValueClass(v);
    else
        *value_struct = *v;
}

void DlgEditValue::initEvtParamComboBox()
{
    if(node == nullptr)
    {
        setUIVisible_EvtParam(false);
        return;
    }

    // init comboBox_EvtParam
    QStringList* event_params = model->GetEventParamsUIOf(node);
    MY_ASSERT(event_params != nullptr);
    QStringList* params_types = EventType::GetInstance()->GetEventParamTypes(event_params);
    QStringList items;

    if(params_types != nullptr)
    {
        for(int i = 0; i < event_params->size(); i++)
        {
            if(params_types->at(i) == var_type || var_type == "")
                items.push_back(QString(event_params->at(i)));
        }
    }

    // 如果没有符合条件的参数可选，就禁用控件
    if(items.size() <= 0)
    {
        ui->comboBox_EvtParam->setEnabled(false);
        ui->radioEvtParam->setEnabled(false);
    }
    else
    {
//        disconnect(ui->comboBox_EvtParam, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBox_EvtParam_currentIndexChanged(int)));
        ui->comboBox_EvtParam->clear();
//        connect(ui->comboBox_EvtParam, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBox_EvtParam_currentIndexChanged(int)));
        ui->comboBox_EvtParam->addItems(items);
        ui->comboBox_EvtParam->setEnabled(true);
        ui->radioEvtParam->setEnabled(true);
    }
}

void DlgEditValue::on_radioPreset_clicked(bool checked)
{
    if(checked && value_type != VT_ENUM)
    {
        value_type = VT_ENUM;
//        on_comboBox_Preset_currentIndexChanged(ui->comboBox_Preset->currentIndex());
    }
}

void DlgEditValue::on_radioEvtParam_clicked(bool checked)
{
    if(checked && value_type != VT_PARAM)
    {
        value_type = VT_PARAM;
    }
}

void DlgEditValue::on_lineEdit_Func_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    resetFuncComboBox();
}

void DlgEditValue::on_DlgEditValue_rejected()
{
    is_accepted = false;
}

void DlgEditValue::on_multiComboBox_Func_editTextChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    resetFuncComboBox();
}

void DlgEditValue::on_btnEditStructValue_clicked()
{
    dlgEditStruct->SetModelAndNode(model, node);
    dlgEditStruct->setWindowTitle("设置：" + var_type);

    if(value_struct == nullptr)
        value_struct = new StructValueClass();
    value_struct->SetVarType(var_type);
    dlgEditStruct->EditStructValue(value_struct);

    if(dlgEditStruct->CheckIsAccepted())
    {
        *value_struct = *(dlgEditStruct->GetValuePointer());
        ui->lineEdit->setText(value_struct->GetLuaValueString(""));
    }
}
