#include "../ItemModels/enumdefine.h"
#include "../Values/structinfo.h"
#include "dlgeditvalue.h"
#include "dlgeditstructvalue.h"
#include "ui_dlgeditstructvalue.h"

DlgEditStructValue::DlgEditStructValue(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgEditStructValue)
{
    is_init_value = false;
    ui->setupUi(this);
    ui->tableWidget->setColumnCount(2);
    QStringList header;
    header << "Key" << "Value";
    ui->tableWidget->setHorizontalHeaderLabels(header);
    m_value = new StructValueClass();
}

DlgEditStructValue::~DlgEditStructValue()
{
    if(m_value != nullptr)
    {
        delete m_value;
        m_value = nullptr;
    }
    delete ui;
}

void DlgEditStructValue::EditStructValue(StructValueClass *v)
{
    if(v == nullptr || !StructInfo::GetInstance()->CheckIsStruct(v->GetVarType()))
    {
        info("EditStructValue参数错误");
        return;
    }
    is_accepted = false;
    *m_value = *v;
    updateTable();
    this->exec();
}

StructValueClass *DlgEditStructValue::GetValuePointer()
{
    return m_value;
}

void DlgEditStructValue::on_DlgEditStructValue_accepted()
{
    is_accepted = true;
}

void DlgEditStructValue::on_tableWidget_itemDoubleClicked(QTableWidgetItem *item)
{
    int idx = item->row();
    QString key_name = ui->tableWidget->item(idx, 0)->text();
    CommonValueClass* comv = m_value->GetValueByKey(key_name);

    DlgEditValue* dlg = new DlgEditValue();
    dlg->SetModel(model);
    dlg->SetUpforInitValue(is_init_value);
    QPoint widget_pos = this->mapToGlobal(QPoint(0, 0));
    dlg->move(widget_pos.x() + 5, widget_pos.y() + 5);
    dlg->ModifyValue(comv);

    CommonValueClass* retv = dlg->GetValuePointer_Common();
    m_value->UpdateMember_Copy(key_name, retv);

    updateTable();
    delete dlg;
}

void DlgEditStructValue::updateTable()
{
    QTableWidgetItem* item;
    ui->tableWidget->clearContents();
    QStringList keys = m_value->GetAllKeys();
    int n = keys.size();
    ui->tableWidget->setRowCount(n);
    for(int i = 0; i < n; i++)
    {
        item = new QTableWidgetItem(keys[i]);
        ui->tableWidget->setItem(i, 0, item);
        if(m_value->GetValueByKey(keys[i]) != nullptr)
            item = new QTableWidgetItem(m_value->GetValueByKey(keys[i])->GetText());
        else
            item = new QTableWidgetItem("NULL");
        ui->tableWidget->setItem(i, 1, item);
    }
}
