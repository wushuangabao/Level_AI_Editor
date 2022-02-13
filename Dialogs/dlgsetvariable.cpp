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

void DlgSetVariable::SetModelPointer(TreeItemModel *m)
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

void DlgSetVariable::on_pushButton_clicked()
{
    m_dlgEditValue->ModifyValue(node, -1);

    ui->pushButton->setText(m_dlgEditValue->GetValueText());
}

void DlgSetVariable::initVariableComboBox()
{
    ValueManager* vm = model->GetValueManagerOf(node);
    if(vm->GetGlobalVarList().size() <= 0)
    {
        ui->comboBox->setEnabled(false);
    }
    else
    {
        ui->comboBox->addItems(vm->GetGlobalVarList());
    }
}

void DlgSetVariable::on_DlgSetVariable_accepted()
{

}
