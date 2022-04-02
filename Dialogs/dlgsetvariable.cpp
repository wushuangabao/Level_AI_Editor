#include "../Values/valuemanager.h"
#include "../ItemModels/treeitemmodel.h"
#include "dlgeditvalue.h"
#include "dlgsetvariable.h"
#include "ui_dlgsetvariable.h"

DlgSetVariable::DlgSetVariable(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgSetVariable)
{
    var_type = "";
    node = nullptr;

    ui->setupUi(this);

    m_dlgEditValue = new DlgEditValue(this);
}

DlgSetVariable::~DlgSetVariable()
{
    delete ui;
}

void DlgSetVariable::SetModel(TreeItemModel *m)
{
    model = m;
    m_dlgEditValue->SetModel(m);
}

void DlgSetVariable::EditSetVarNode(NodeInfo *set_var_node)
{
    node = set_var_node;
    is_accepted = false;

    if(!initVariableComboBox())
        return;
    ui->comboBox->setEnabled(false); //被赋值的是哪个变量，这个暂时禁止修改

    BaseValueClass* v = model->GetValueManager()->GetValueOnNode_SetVar(node);
    if(v != nullptr)
        ui->pushButton->setText(v->GetText());

    this->exec();
}

void DlgSetVariable::CreateSetVarNode(NodeInfo *seq_node)
{
    node = seq_node;
    is_accepted = false;

    ui->comboBox->setEnabled(true);
    if(!initVariableComboBox())
        return;

    this->exec();
}

QString DlgSetVariable::GetNodeText()
{
    QString s = ui->comboBox->currentText();
    s = s + " = " + m_dlgEditValue->GetValueText();

    return s;
}

QString DlgSetVariable::GetValueName()
{
    return ui->comboBox->currentText();
}

BaseValueClass *DlgSetVariable::GetValuePointer()
{
    return m_dlgEditValue->GetValuePointer();
}

bool DlgSetVariable::IsAccepted()
{
    return is_accepted;
}

void DlgSetVariable::on_pushButton_clicked()
{
    if(node != nullptr && node->type == SET_VAR)
    {
        m_dlgEditValue->ModifyValue(node, 0);
    }
    else if(node != nullptr && node->type == SEQUENCE)
    {
        m_dlgEditValue->CreateNewValue(model->GetValueManager()->GetVarTypeOf(ui->comboBox->currentText()), node);
    }

    if(m_dlgEditValue->IsAccepted())
        ui->pushButton->setText(m_dlgEditValue->GetValueText());
}

bool DlgSetVariable::initVariableComboBox()
{
    MY_ASSERT(model != nullptr);

    disconnect(ui->comboBox, SIGNAL(currentTextChanged(const QString &)), this, SLOT(on_comboBox_currentTextChanged(const QString &)));
    ui->comboBox->clear();
    connect(ui->comboBox, SIGNAL(currentTextChanged(const QString &)), this, SLOT(on_comboBox_currentTextChanged(const QString &)));

    ValueManager* vm = model->GetValueManager();

    if(vm->GetGlobalVarList().size() > 0)
    {
        ui->comboBox->addItems(vm->GetGlobalVarList());
    }

    if(ui->comboBox->count() <= 0)
    {
       ui->comboBox->setEnabled(false);
       info("没有可用的变量！请先创建变量。");
       return false;
    }
    else if(node != nullptr && node->type == SET_VAR)
    {
        // 根据node设置comboBox的初始选项
        int id = 0;
        if(node->getValuesCount() > 0)
        {
            int var_id = node->getValue(0).toInt();
            id = ui->comboBox->findText(vm->GetVarNameAt(var_id));
            if(id == -1)
                id = 0;
        }
        ui->comboBox->setCurrentIndex(id);
        ui->comboBox->setEnabled(true);
    }
    return true;
}

void DlgSetVariable::on_DlgSetVariable_accepted()
{
    MY_ASSERT(model != nullptr);

    BaseValueClass* value = m_dlgEditValue->GetValuePointer();
    if(var_type != value->GetVarType() && value->GetValueType() != VT_STR)
    {
        info("变量类型和值的类型不匹配！");
    }

    // 编辑 setvar 节点
    if(node != nullptr && node->type == SET_VAR)
    {
        int id_var = model->GetValueManager()->FindIdOfVarName(GetValueName());
        if(id_var != -1)
        {
            if(node->getValuesCount() > 0)
                node->modifyValue(0, QString::number(id_var));
            else
                node->addNewValue(QString::number(id_var));
            model->GetValueManager()->UpdateValueOnNode_SetValue(node, value);
            node->text = GetNodeText();
        }
        else
        {
            info("on_DlgSetVariable_accepted");
        }
    }
    // 新建 setvar 节点
    else if(node != nullptr && node->type == SEQUENCE)
    {
        if(ui->comboBox->isEnabled() == false)
            return;
        is_accepted = true;
    }
}

void DlgSetVariable::on_comboBox_currentTextChanged(const QString &arg1)
{
    if(arg1 == "")
        return;
    ValueManager* vm = model->GetValueManager();
    var_type = vm->GetVarTypeOf(arg1);
}
