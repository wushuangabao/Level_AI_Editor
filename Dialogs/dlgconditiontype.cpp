#include "../ItemModels/nodeinfo.h"
#include "../ItemModels/treeitemmodel.h"
#include "dlgeditvalue.h"
#include "dlgconditiontype.h"
#include "ui_dlgconditiontype.h"

DlgConditionType::DlgConditionType(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgConditionType)
{
    ui->setupUi(this);

    type = CONDITION_OP::AND;
    ui->btnText_1->setText("未定义值");
    ui->btnText_2->setText("未定义值");

    m_dlgEditValueLeft = new DlgEditValue(this);
    m_dlgEditValueRight = new DlgEditValue(this);
}

DlgConditionType::~DlgConditionType()
{
    delete ui;
}

void DlgConditionType::SetModelPointer(TreeItemModel* m)
{
    model = m;

    m_dlgEditValueLeft->SetModel(m);
    m_dlgEditValueRight->SetModel(m);
}

void DlgConditionType::CreateCondition(NodeInfo* parent, QString default_s)
{
    initUI(INVALID);
    this->node = parent;

    for(int i = 0; i < ui->comboBox->count(); i++)
        if(ui->comboBox->itemText(i) == default_s)
        {
            ui->comboBox->setCurrentIndex(i);
            break;
        }

    exec();
}

void DlgConditionType::ModifyCondition(NodeInfo *node)
{
    Q_ASSERT(node != nullptr);
    Q_ASSERT(node->type == CONDITION);
    Q_ASSERT(node->getValuesCount() == 1);

    initUI(CONDITION);

    initConditionType(node);

    exec();
}

void DlgConditionType::ModifyCompareNode(NodeInfo *node)
{
    Q_ASSERT(node != nullptr);
    Q_ASSERT(node->type == COMPARE);
    Q_ASSERT(node->getValuesCount() == 3);

    initUI(COMPARE);

    initConditionType(node);

    ui->btnText_1->setText(node->getValue(1));
    ui->btnText_2->setText(node->getValue(2));

    exec();
}

void DlgConditionType::on_comboBox_currentIndexChanged(int index)
{
    if(node_type == CONDITION || node_type == INVALID)
        type = static_cast<CONDITION_OP>(index);
    else if(node_type == COMPARE)
        type = static_cast<CONDITION_OP>(index + 2);

    if(node_type == INVALID)
    {
        if(index >= 2)
        {
            ui->btnText_1->show();
            ui->btnText_2->show();
        }
        else
        {
            ui->btnText_1->hide();
            ui->btnText_2->hide();
        }
    }
}

void DlgConditionType::on_buttonBox_rejected()
{
    type = CONDITION_OP::INVALID_CONDITION;
}

void DlgConditionType::on_buttonBox_accepted()
{
    if(node_type != INVALID)
    {
        node->modifyValue(type);

        if(node_type == CONDITION)
            node->updateCondionText();
        else if(node_type == COMPARE)
        {
            // todo: 修改value_left和value_right
            node->modifyValue(1, ui->btnText_1->text());
            node->modifyValue(2, ui->btnText_2->text());
            node->updateCompareText();
        }
    }
    else //创建Condition或者Compare节点
    {
        if(type == CONDITION_OP::AND)
        {
            NodeInfo* new_node = model->createNode("", NODE_TYPE::CONDITION, node);
            Q_ASSERT(new_node != nullptr);
            new_node->modifyValue(CONDITION_OP::AND);
            new_node->updateCondionText();
        }
        else if(type == CONDITION_OP::OR)
        {
            NodeInfo* new_node = model->createNode("", NODE_TYPE::CONDITION, node);
            Q_ASSERT(new_node != nullptr);
            new_node->modifyValue(CONDITION_OP::OR);
            new_node->updateCondionText();
        }
        else
        {
            NodeInfo* new_node = model->createNode("", NODE_TYPE::COMPARE, node);
            Q_ASSERT(node != nullptr);
            new_node->modifyValue(type);
            // todo: 把value_left和value_right赋值给new_node
            new_node->modifyValue(1, m_dlgEditValueLeft->GetValueText());
            new_node->modifyValue(2, m_dlgEditValueRight->GetValueText());
            new_node->updateCompareText();
        }
    }
}

void DlgConditionType::on_btnText_1_clicked()
{
    m_dlgEditValueLeft->ModifyValue(node, 1);

    ui->btnText_1->setText(m_dlgEditValueLeft->GetValueText());
}

void DlgConditionType::on_btnText_2_clicked()
{
    m_dlgEditValueRight->ModifyValue(node, 2);

    ui->btnText_2->setText(m_dlgEditValueRight->GetValueText());
}

void DlgConditionType::initUI(NODE_TYPE node_type)
{
    this->node_type = node_type;

    ui->comboBox->clear();
    ui->btnText_1->hide();
    ui->btnText_2->hide();

    QStringList list;
    if(node_type == CONDITION)
    {
        list << "AND" << "OR";
        ui->btnText_1->hide();
        ui->btnText_2->hide();
    }
    else if(node_type == COMPARE)
    {
        list << "==" << ">" << "<" << ">=" << "<=";
        ui->btnText_1->show();
        ui->btnText_2->show();
    }
    else if(node_type == INVALID)
    {
        list << "AND" << "OR" << "==" << ">" << "<" << ">=" << "<=";
    }

    ui->comboBox->addItems(list);
}

void DlgConditionType::initConditionType(NodeInfo *node)
{
    int n = ui->comboBox->count();
    for(int i = 0; i < n; i++)
        if(ui->comboBox->itemText(i) == node->getValue(0))
        {
            ui->comboBox->setCurrentIndex(i);
            break;
        }

    this->node = node;
    type = getConditionEnum(node->getValue(0));
    Q_ASSERT(type != INVALID_CONDITION);
}
