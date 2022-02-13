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
    value = new BaseValueClass("未定义变量", VT_VAR);

    qDebug() << "Create DlgEditValue at" << (uintptr_t)this << endl;

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
        funcParams.append(new BaseValueClass("未定义值", VT_VAR));
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
    qDebug() << "Delete DlgEditValue at" << (uintptr_t)this << endl;

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

void DlgEditValue::ModifyValue(NodeInfo *node, int value_id)
{
    Q_ASSERT(node != nullptr);
    Q_ASSERT(model != nullptr);

    if(this->node != node)
    {
        this->node = node;
        setUpforValue();
    }

//    if(node->getValuesCount() <= value_id || value_id < 0)
//        Q_ASSERT(false);

//    value_idx = value_id;
//    v_type =

    this->exec();
}

void DlgEditValue::setUpforValue()
{
    Q_ASSERT(model != nullptr);

    is_for_function = false;
    initVariableComboBox();
    initFunctionComboBox();
}

void DlgEditValue::SetUpforFunction()
{
    is_for_function = true;

    ui->radioVariable->setVisible(false);
    ui->comboBox_Var->setVisible(false);
    ui->btnSetValue->setVisible(false);

    setWindowTitle("编辑Function");

    ui->radioCustom->setText("lua脚本：");

    initFunctionComboBox();
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

void DlgEditValue::ModifyCallNode(NodeInfo *function_node)
{
    Q_ASSERT(function_node != nullptr);
    Q_ASSERT(model != nullptr);
    Q_ASSERT(function_node->type == FUNCTION);

    node = function_node;
    v_type = VT_FUNC;

    ui->comboBoxFunction->setCurrentIndex(0);
    on_comboBoxFunction_currentIndexChanged(0);

    value->SetFunction(&(FunctionInfo::GetInstance()->infoList[0]));

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
    if(v_type == VT_FUNC)
    {
        FunctionClass* func = &(FunctionInfo::GetInstance()->infoList[ui->comboBoxFunction->currentIndex()]);
        Q_ASSERT(func != nullptr);
        value->SetFunction(func);

        qDebug() << "on_DlgEditValue_accepted" << endl;

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
        value->SetLuaStr(ui->lineEdit->text());
    }
}

void DlgEditValue::on_btnSetValue_clicked()
{

}

void DlgEditValue::initVariableComboBox()
{
    Q_ASSERT(model != nullptr);
    Q_ASSERT(node != nullptr);

    ValueManager* vm = model->GetValueManagerOf(node);
    if(vm->GetGlobalVarList().size() <= 0)
    {
        ui->comboBox_Var->setEnabled(false);
        ui->radioVariable->setEnabled(false);
        ui->radioFunction->setChecked(true);
    }
    else
    {
        ui->comboBox_Var->clear();
        ui->comboBox_Var->addItems(vm->GetGlobalVarList());
        ui->radioVariable->setChecked(true);
    }
}

void DlgEditValue::initFunctionComboBox()
{
    ui->comboBoxFunction->clear();

    FunctionInfo* functions = FunctionInfo::GetInstance();
    QStringList items;

    int n = functions->infoList.size();
    Q_ASSERT(n > 0);
    if(n > 0)
    {
//        value->SetFunction(&(functions->infoList[0]));
        for(int i = 0; i < n; i++)
        {
            if(!is_for_function || functions->infoList[i].GetReturnNum() > 0)
            {
                items.append(functions->infoList[i].GetName());
            }
        }
    }

    ui->comboBoxFunction->addItems(items);
}

void DlgEditValue::addFuncParam(QPushButton* btn, FunctionClass *func, int param_id)
{
    btn->setText(func->GetParamNameAt(param_id));
    btn->show();
}

void DlgEditValue::addFuncText(QLabel *lbl, FunctionClass *func, int text_id)
{
    lbl->setText(func->GetTextAt(text_id));
    lbl->show();
}

void DlgEditValue::on_comboBoxFunction_currentIndexChanged(int index)
{
    FunctionClass* func = &(FunctionInfo::GetInstance()->infoList[index]);
    Q_ASSERT(func != nullptr);

    updateFuncTextUI(func);
}

void DlgEditValue::on_comboBox_Var_currentIndexChanged(int index)
{
    ValueManager* vm = model->GetValueManagerOf(node);
    int list_size = vm->GetGlobalVarList().size();
    if(index < list_size)
    {
        QString v_name = vm->GetGlobalVarList().at(index);
        value->SetVarName(v_name);
    }
    else
    {
        // index越界
        if(list_size == 0)
        {
            ui->radioVariable->setEnabled(false);
            ui->radioFunction->setChecked(true);
        }
        else
        {
            info("on_comboBox_Var_currentIndexChanged 参数越界");
        }
    }
}

void DlgEditValue::on_lineEdit_textChanged(const QString &arg1)
{
    if(v_type == VT_STR)
        value->SetLuaStr(arg1);
}

void DlgEditValue::onBtnParam_clicked(int idx)
{
    qDebug() << "--- onBtnParam_clicked ---" << endl;

    Q_ASSERT(idx < FUNC_TEXT_NUM);
    Q_ASSERT(idx >= 0);

    FunctionClass* func = &(FunctionInfo::GetInstance()->infoList[ui->comboBoxFunction->currentIndex()]);
    Q_ASSERT(func != nullptr);

    int param_id = idx;
    if(!func->param_is_before_text)
        param_id--; //因为这种情况下UI的idx比实际参数的idx多1

    QPoint widget_pos = this->mapToGlobal(QPoint(0, 0));
    DlgEditValue* dlg = new DlgEditValue();
    dlg->move(widget_pos.x() + 5, widget_pos.y() + 5);
    dlg->setWindowTitle("设置：" + func->GetParamNameAt(param_id));
    dlg->SetModel(model);
    dlg->ModifyValue(node, -1);

    qDebug() << (uintptr_t)(funcParams[idx]) << " = " << (uintptr_t)(dlg->GetValuePointer()) << endl;
    *(funcParams[idx]) = *(dlg->GetValuePointer());

    qDebug() << "--- begin delete dlg ---" << endl;
    delete dlg;
    // value->SetParamAt(param_id, funcParams[idx])在最后按下OK按钮时进行

    QString param_text = funcParams[idx]->GetText();
    funcParamBtns[idx]->setText(param_text);
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
