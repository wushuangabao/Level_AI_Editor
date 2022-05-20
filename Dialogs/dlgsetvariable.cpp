#include <QMenu>
#include "../Values/valuemanager.h"
#include "../Values/structinfo.h"
#include "../ItemModels/treeitemmodel.h"
#include "dlgeditvalue.h"
#include "dlgsetvariable.h"
#include "multilevelcombobox.h"
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

    CommonValueClass* v = model->GetValueManager()->GetValueOnNode_SetVar(node);
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

    m_dlgEditValue->ResetNilValue();
    ui->pushButton->setText("nil");

    this->exec();
}

QString DlgSetVariable::GetNodeText()
{
    QString s = ui->comboBox->currentText();
    s = s + " = " + m_dlgEditValue->GetValueText();

    return s;
}

QString DlgSetVariable::GetCurVarName()
{
    QString text = ui->comboBox->currentText();
    int pos = text.indexOf('.');
    if(pos != -1)
        text = text.left(pos);
    return text;
}

CommonValueClass *DlgSetVariable::GetValuePointer()
{
    return m_dlgEditValue->GetValuePointer_Common();
}

bool DlgSetVariable::IsAccepted()
{
    return is_accepted;
}

void DlgSetVariable::on_pushButton_clicked()
{
    if(node != nullptr && node->type == SET_VAR)
    {
        m_dlgEditValue->ModifyValueOnNode(node, 0);
    }
    else if(node != nullptr && node->type == SEQUENCE)
    {
        QString var_text = ui->comboBox->currentText();
        QString edit_var_type = ValueManager::GetValueManager()->GetVarTypeOf_Key(var_text);
        m_dlgEditValue->CreateNewValueForParentNode(edit_var_type, node);
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
        QString var_text = vm->GetVarNameAt(node->getValue(0).toInt());
        for(int i = 1; i < node->getValuesCount(); i++)
            var_text += QString(".%1").arg(node->getValue(i));
        ui->comboBox->SetLineText(var_text);
    }
    return true;
}

void DlgSetVariable::on_DlgSetVariable_accepted()
{
    MY_ASSERT(model != nullptr);
    QString var_text = ui->comboBox->currentText();
    if(var_text.isEmpty())
    {
        info("没有选择任何变量");
        return;
    }
    var_type = model->GetValueManager()->GetVarTypeOf_Key(var_text);

    CommonValueClass* value = m_dlgEditValue->GetValuePointer_Common();
    if(var_type != value->GetVarType())
    {
        if(value->GetValueType() != VT_STR)
            info("变量类型和值的类型不匹配！");
        else
            static_cast<BaseValueClass*>(value)->SetVarType(var_type);
    }

    // 编辑 setvar 节点
    if(node != nullptr && node->type == SET_VAR)
    {
        int id_var = model->GetValueManager()->FindIdOfVarName(GetCurVarName());
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
//    StructInfo* struct_info = StructInfo::GetInstance();
    ValueManager* vm = model->GetValueManager();
    var_type = vm->GetVarTypeOf_Key(arg1);
}
