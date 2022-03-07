#include "dlgchoseactiontype.h"
#include "ui_dlgchoseactiontype.h"

DlgChoseActionType::DlgChoseActionType(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgChoseActionType)
{
    model = nullptr;
    ui->setupUi(this);

    m_dlgCallFunc = new DlgEditValue(this);
    m_dlgChoseEvent = new DlgChoseEType(this);
    m_dlgSetVar = new DlgSetVariable(this);
}

DlgChoseActionType::~DlgChoseActionType()
{
    delete ui;
}

void DlgChoseActionType::SetModel(TreeItemModel *m)
{
    model = m;
    m_dlgCallFunc->SetModel(m);
    m_dlgCallFunc->SetUpforFunction();
    m_dlgSetVar->SetModel(m);
}

void DlgChoseActionType::CreateActionType()
{
    index = INVALID;
    exec();
}

NODE_TYPE DlgChoseActionType::GetNodeTypeAndText(QString& node_text)
{
    switch (index) {
    case SET_VAR:
        node_text = ui->lblSetVar->text();
        break;
    case FUNCTION:
        node_text = ui->lblCallFunc->text();
        break;
    case CHOICE:
        node_text = "if";
        break;
    case LOOP:
        node_text = "循环";
        break;
    case END:
        node_text = "跳出";
        break;
    case CLOSE_EVENT:
        node_text = "停止监听..";
        break;
    case OPEN_EVENT:
        node_text = "开始监听..";
        break;
    default:
        node_text = "";
        break;
    }
    return index;
}

void DlgChoseActionType::on_btnChoice_clicked()
{
    index = CHOICE;
    hide();
}

void DlgChoseActionType::on_btnLoop_clicked()
{
    index = LOOP;
    hide();
}

void DlgChoseActionType::on_btnEnd_clicked()
{
    index = END;
    hide();
}

void DlgChoseActionType::on_btnSetVar_clicked()
{
    index = SET_VAR;
    hide();
}

void DlgChoseActionType::on_btnCallFunc_clicked()
{
    index = FUNCTION;
    hide();
}

void DlgChoseActionType::on_btnCloseEvent_clicked()
{
    index = CLOSE_EVENT;
    hide();
}

void DlgChoseActionType::on_btnOpenEvent_clicked()
{
    index = OPEN_EVENT;
    hide();
}

void DlgChoseActionType::on_btnCancel_clicked()
{
    index = INVALID;
    hide();
}
