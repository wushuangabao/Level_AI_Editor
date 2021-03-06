#include "../ItemModels/nodeinfo.h"
#include "../ItemModels/treeitemmodel.h"
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

void DlgChoseActionType::closeEvent(QCloseEvent *event)
{
    index = INVALID;
    QDialog::closeEvent(event);
}

void DlgChoseActionType::SetModel(TreeItemModel *m)
{
    model = m;
    m_dlgCallFunc->SetModel(m);
    m_dlgCallFunc->SetUpforFunction();
    m_dlgSetVar->SetModel(m);
}

void DlgChoseActionType::BeginResetModel()
{
    model->beginResetModel();
}

void DlgChoseActionType::EndResetModel()
{
    model->endResetModel();
}

void DlgChoseActionType::CreateActionType(NodeInfo* seq_node)
{
    index = INVALID;

    node = seq_node;
    if(node->type != SEQUENCE)
    {
        info("只能在序列中插入动作！");
        return;
    }

    exec();
}

NODE_TYPE DlgChoseActionType::GetNodeTypeAndText(QString& node_text)
{
    switch (index) {
    case SET_VAR:
        node_text = m_dlgSetVar->GetNodeText();
        break;
    case FUNCTION:
        node_text = m_dlgCallFunc->GetValueText();
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
    case OPEN_EVENT:
    {
        QStringList enames = model->getEventNames();
        int id = m_dlgChoseEvent->index;
        if(id >= 0 && id < enames.size())
            node_text = enames[id];
        else
        {
            info("GetNodeTypeAndText ERROR");
            node_text = getNodeTypeStr(index);
        }
    }
        break;
    case SEQUENCE:
    {
        QStringList names = model->getCustActSeqNames();
        int id = m_dlgChoseEvent->index;
        if(id >= 0 && id < names.size())
        {
            node_text = BaseValueClass::custom_name_prefix + names[id];
        }
        else
        {
            node_text = "-- ERROR Custom Action";
        }
    }
        break;
    default:
        node_text = "";
        break;
    }
    return index;
}

CommonValueClass *DlgChoseActionType::GetValue_SetVar()
{
    return m_dlgSetVar->GetValuePointer();
}

BaseValueClass *DlgChoseActionType::GetValue_CallFunc()
{
    return m_dlgCallFunc->GetValuePointer_Base();
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

    m_dlgSetVar->CreateSetVarNode(node);
    if(m_dlgSetVar->IsAccepted())
    {
//        ui->lblSetVar->setText(m_dlgSetVar->GetNodeText());
        hide();
    }
}

void DlgChoseActionType::on_btnCallFunc_clicked()
{
    index = FUNCTION;

    m_dlgCallFunc->CreateCallNode();
    if(m_dlgCallFunc->IsAccepted())
        hide();
}

void DlgChoseActionType::on_btnCloseEvent_clicked()
{
    index = CLOSE_EVENT;

    int id_ename = m_dlgChoseEvent->ChoseEventNameIn(model->getEventNames());
    if(id_ename != -1)
       hide();
}

void DlgChoseActionType::on_btnOpenEvent_clicked()
{
    index = OPEN_EVENT;

    int id_ename = m_dlgChoseEvent->ChoseEventNameIn(model->getEventNames());
    if(id_ename != -1)
       hide();
}

void DlgChoseActionType::on_btnCancel_clicked()
{
    index = INVALID;
    hide();
}

void DlgChoseActionType::on_btnCustomAction_clicked()
{
    index = SEQUENCE;
    if(m_dlgChoseEvent->ChoseCustActSeqNameIn(model->getCustActSeqNames()) != -1)
       hide();
}
