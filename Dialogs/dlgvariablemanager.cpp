#include "../ItemModels/treeitemmodel.h"
#include "../Values/enuminfo.h"
#include "../Values/structinfo.h"
#include "../Values/valuemanager.h"
#include "dlgvariablemanager.h"
#include "ui_dlgvariablemanager.h"

DlgVariableManager::DlgVariableManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgVariableManager)
{
    model = nullptr;
    node = nullptr;
    init_v_struct = new StructValueClass();
    m_dlgEditValue = new DlgEditValue(this);
    init_v = new BaseValueClass(m_dlgEditValue->GetValuePointer_Base());

    ui->setupUi(this);
    ui->pushButton_2->setText(m_dlgEditValue->GetValueText());

    // [0]=number [1]=string [2]=------ ... [pos_struct_begin]=------ ...
    pos_enum_begin = 2;
    QStringList items = EnumInfo::GetInstance()->GetAllTypes();
    items.push_back("-----");
    pos_struct_begin = pos_enum_begin + items.size();
    items.push_front("-----");

    ui->comboBox->addItems(items);
    ui->comboBox->addItems(StructInfo::GetInstance()->GetAllStructNames());
}

DlgVariableManager::~DlgVariableManager()
{
    if(init_v != nullptr)
    {
        delete init_v;
        init_v = nullptr;
    }
    if(init_v_struct != nullptr)
    {
        delete init_v_struct;
        init_v_struct = nullptr;
    }
    delete ui;
}

void DlgVariableManager::SetModel(TreeItemModel *m)
{
    if(model == m)
        return;
    model = m;
    m_dlgEditValue->SetModel(m);
}

void DlgVariableManager::CreateVar()
{
    MY_ASSERT(model != nullptr);

    node = nullptr;
    if(ui->comboBox->currentIndex() != pos_enum_begin && ui->comboBox->currentIndex() != pos_struct_begin)
        var_type = ui->comboBox->currentText();
    else
        var_type = "";

    init_v->SetVarType(var_type);
    this->setWindowTitle("创建新变量");
    ui->checkBox->setChecked(false);

    exec();
}

void DlgVariableManager::ModifyVar(int id_var)
{
    MY_ASSERT(model != nullptr);
    node = nullptr;

    ValueManager* vm = model->GetValueManager();
    QStringList var_list = vm->GetGlobalVarList();

    global_var_id = vm->GetIdOfVariable(var_list[id_var]);
    CommonValueClass* comv = vm->GetInitValueOfVar(global_var_id);
    var_type = comv->GetVarType();
    m_dlgEditValue->SetValueType(comv->GetValueType());
    if(comv->GetValueType() < VT_TABLE)
    {
        *init_v = *((BaseValueClass*)comv);
        *(m_dlgEditValue->GetValuePointer_Base()) = *init_v;
        ui->pushButton_2->setText(init_v->GetText());
    }
    else
    {
        init_v->ClearData();
        *init_v_struct = *((StructValueClass*)comv);
        *(m_dlgEditValue->GetValuePointer_Struct()) = *init_v_struct;
        ui->pushButton_2->setText(init_v_struct->GetText());
    }

    this->setWindowTitle("编辑变量");
    ui->comboBox->setCurrentText(vm->GetVarTypeAt(global_var_id));
    ui->lineEdit->setText(vm->GetGlobalVarList().at(id_var));
    ui->checkBox->setChecked(vm->CheckVarIsLevelParam(global_var_id));

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
    else if(!isValidVarName(var_name))
        return;

    MY_ASSERT(model != nullptr);
    ValueManager* vm = model->GetValueManager();
    MY_ASSERT(vm != nullptr);

    bool is_base_v;
    m_dlgEditValue->GetValuePointer_Common(&is_base_v);
    if(is_base_v)
    {
        if(init_v->GetValueType() != VT_STR && var_type != init_v->GetVarType())
            info("变量类型与初始值的类型不一致！");
        else if(init_v->GetValueType() == VT_STR)
            // 强行断定lua_str值的变量类型吧。。
            init_v->SetVarType(var_type);
    }

    CommonValueClass* value;
    if(is_base_v)
        value = init_v;
    else
        value = init_v_struct;

    if(this->windowTitle() == "创建新变量")
    {
        if(!vm->AddNewVariable(var_name, value, ui->checkBox->isChecked()))
            return;
    }
    else if(this->windowTitle() == "编辑变量")
    {
        if(vm->GetVarNameAt(global_var_id) != var_name && vm->GetIdOfVariable(var_name) != -1)
        {
            info("已经存在这个变量名！");
            return;
        }
        vm->ModifyVarValueAt(global_var_id, var_name, value, ui->checkBox->isChecked());
    }

    this->hide();
}

void DlgVariableManager::on_pushButton_2_clicked()
{
    m_dlgEditValue->ModifyInitVarValue(init_v, init_v_struct, var_type);

    if(m_dlgEditValue->IsAccepted())
    {
        bool is_base_v;
        CommonValueClass* init_v_com = m_dlgEditValue->GetValuePointer_Common(&is_base_v);
        if(is_base_v)
        {
            *init_v = *(static_cast<BaseValueClass*>(init_v_com));
            ui->pushButton_2->setText(init_v->GetText());
        }
        else
        {
            *init_v_struct = *(static_cast<StructValueClass*>(init_v_com));
            ui->pushButton_2->setText(init_v_struct->GetText());
        }
    }
}

void DlgVariableManager::on_comboBox_currentIndexChanged(const QString &arg1)
{
    int cur_id = ui->comboBox->currentIndex();
    if(cur_id != pos_enum_begin && cur_id != pos_struct_begin)
        var_type = arg1;

    if(cur_id < pos_struct_begin && init_v->GetVarType() != var_type)
    {
        init_v->ClearData();
        init_v->SetVarType(var_type);
        ui->pushButton_2->setText(init_v->GetText());
//        info("初始值已重置！");
    }
    else if(cur_id > pos_struct_begin && init_v_struct->GetVarType() != var_type)
    {
        init_v_struct->SetVarType(var_type);
        ui->pushButton_2->setText(init_v_struct->GetText());
        //        info("初始值已重置！");
    }
}

bool DlgVariableManager::isValidVarName(const QString &name)
{
    int n = name.size();
    if(n < 1) return false;

    bool ok = (name[0] >= 'a' && name[0] <= 'z') || (name[0] >= 'A' && name[0] <= 'Z') || name[0] == '_';
    if(!ok)
    {
        info("变量名首字符只能是英文字母或下划线！");
        return false;
    }

    for(int i = 1; i < n; i++)
    {
        ok = (name[i] >= 'a' && name[i] <= 'z') || (name[i] >= 'A' && name[i] <= 'Z') ||
             (name[i] >= '0' && name[i] <= '9') || name[i] == '_';
        if(!ok)
        {
            info("变量名第" + QString::number(i + 1) + "个字符非法！");
            return false;
        }
    }

    return true;
}
