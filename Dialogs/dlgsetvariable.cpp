#include "../Values/valuemanager.h"
#include "../ItemModels/treeitemmodel.h"
#include "dlgeditvalue.h"
#include "dlgsetvariable.h"
#include "ui_dlgsetvariable.h"

DlgSetVariable::DlgSetVariable(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgSetVariable)
{
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

    initVariableComboBox();

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

void DlgSetVariable::on_pushButton_clicked()
{
    m_dlgEditValue->ModifyValue(node, 0);

    ui->pushButton->setText(m_dlgEditValue->GetValueText());
}

void DlgSetVariable::initVariableComboBox()
{
    Q_ASSERT(model != nullptr);
    Q_ASSERT(node != nullptr);

    ui->comboBox->clear();

    ValueManager* vm = model->GetValueManager();

    if(vm->GetGlobalVarList().size() > 0)
    {
        ui->comboBox->addItems(vm->GetGlobalVarList());
    }

    // 顶层条件节点，在变量中添加事件参数
//    if(node->parent->type == EVENT)
//    {
//        QStringList* event_params = vm->GetEventParams(node->parent);
//        if(event_params != nullptr && event_params->size() > 0)
//        {
//            for(int i = 0; i < event_params->size(); i++)
//            {
//                QString param_name = event_params->at(i);
//                ui->comboBox->addItem(param_name);
//            }
//        }
//    }

    if(ui->comboBox->count() <= 0)
    {
       ui->comboBox->setEnabled(false);
       info("没有可用的变量！请先创建变量。");
    }
    else
    {
        ui->comboBox->setEnabled(true);
    }
}

void DlgSetVariable::on_DlgSetVariable_accepted()
{
    Q_ASSERT(model != nullptr);
    Q_ASSERT(node != nullptr);

    if(ui->comboBox->isEnabled() == false)
    {
        node->text = "未知变量 = 未定义值";
        return;
    }

    if(node->getValuesCount() > 0)
        node->modifyValue(0, GetValueName());
    else
        node->addNewValue(GetValueName());

    model->GetValueManager()->UpdateValueOnNode_SetValue(node, GetValuePointer());
    node->text = GetNodeText();
}
