#include "../ItemModels/treeitemmodel.h"
#include "../Values/enuminfo.h"
#include "../Values/valuemanager.h"
#include "dlgvariablemanager.h"
#include "ui_dlgvariablemanager.h"

DlgVariableManager::DlgVariableManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgVariableManager)
{
    model = nullptr;
    node = nullptr;
    m_dlgEditValue = new DlgEditValue(this);
    init_v = new BaseValueClass(m_dlgEditValue->GetValuePointer());

    ui->setupUi(this);
    ui->pushButton_2->setText(m_dlgEditValue->GetValueText());

    QStringList items = EnumInfo::GetInstance()->GetAllTypes();
    ui->comboBox->addItems(items);
}

DlgVariableManager::~DlgVariableManager()
{
    delete init_v;
    delete ui;
}

void DlgVariableManager::SetModel(TreeItemModel *m)
{
    if(model == m)
        return;
    if(model != nullptr)
        delete model;
    model = m;
    m_dlgEditValue->SetModel(m);
}

void DlgVariableManager::CreateVar()
{
    MY_ASSERT(model != nullptr);

    node = nullptr;
    var_type = ui->comboBox->currentText();
    init_v->SetVarType(var_type);

    this->setWindowTitle("创建新变量");

    exec();
}

void DlgVariableManager::ModifyVar(int id_var)
{
    MY_ASSERT(model != nullptr);

    node = nullptr;
    global_var_id = id_var;
    *init_v = *(model->GetValueManager()->GetInitValueOfVar(id_var));
    var_type = init_v->GetVarType();

    this->setWindowTitle("编辑变量");
    ui->comboBox->setCurrentText(model->GetValueManager()->GetVarTypeAt(id_var));
    ui->lineEdit->setText(model->GetValueManager()->GetGlobalVarList().at(id_var));
    ui->pushButton_2->setText(init_v->GetText());

    exec();
}

void DlgVariableManager::SetLoopVar(NodeInfo *loop_node)
{
    MY_ASSERT(model != nullptr);
    MY_ASSERT(node != nullptr);
    MY_ASSERT(loop_node->type == LOOP);

    this->node = node;
    this->setWindowTitle("编辑循环变量");

    exec();
}

void DlgVariableManager::on_pushButton_clicked()
{
    QString var_name = ui->lineEdit->text();
    if(var_name == "")
    {
        info("变量名不能为空！");
        return;
    }
    MY_ASSERT(model != nullptr);

    if(init_v->GetValueType() != VT_STR && var_type != init_v->GetVarType())
    {
        info("变量类型与初始值的类型不一致！");
    }
    else if(init_v->GetValueType() == VT_STR)
    {
        // 强行断定lua_str值的变量类型吧。。
        init_v->SetVarType(var_type);
    }

    if(this->windowTitle() == "创建新变量")
    {
        ValueManager* vm = model->GetValueManager();
        MY_ASSERT(vm != nullptr);
        if(!vm->AddNewVariable(var_name, init_v))
            return;
    }
    else if(this->windowTitle() == "编辑变量")
    {
        ValueManager* vm = model->GetValueManager();
        MY_ASSERT(vm != nullptr);
        if(global_var_id >= 0 && global_var_id < vm->GetGlobalVarList().size())
            vm->ModifyVarValueAt(global_var_id, var_name, init_v);
        else
            info("不存在这个变量了..");
    }
    else if(node != nullptr && node->type == LOOP)
    {

    }

    this->hide();
}

void DlgVariableManager::on_pushButton_2_clicked()
{
    m_dlgEditValue->ModifyInitVarValue(init_v, var_type);
    init_v = m_dlgEditValue->GetValuePointer();
    ui->pushButton_2->setText(init_v->GetText());
}

void DlgVariableManager::on_comboBox_currentIndexChanged(const QString &arg1)
{
    var_type = arg1;
    if(init_v->GetVarType() != arg1)
    {
        init_v->ClearData();
        init_v->SetVarType(var_type);
        ui->pushButton_2->setText(init_v->GetText());
//        info("初始值已重置！");
    }
}
