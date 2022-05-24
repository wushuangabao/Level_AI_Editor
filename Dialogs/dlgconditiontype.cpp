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

    node = nullptr;

    type = CONDITION_OP::AND;

    m_dlgEditValueLeft = new DlgEditValue(this);
    m_dlgEditValueRight = new DlgEditValue(this);

    ui->btnText_1->setText(m_dlgEditValueLeft->GetValueText());
    ui->btnText_2->setText(m_dlgEditValueRight->GetValueText());
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

    m_dlgEditValueLeft->GetValuePointer_Base()->SetLuaStr("nil");
    m_dlgEditValueRight->GetValuePointer_Base()->SetLuaStr("nil");
    ui->btnText_1->setText(m_dlgEditValueLeft->GetValueText());
    ui->btnText_2->setText(m_dlgEditValueRight->GetValueText());

    exec();
}

NodeInfo *DlgConditionType::GetNewNode()
{
    return node;
}

void DlgConditionType::ModifyCondition(NodeInfo *node)
{
    MY_ASSERT(node != nullptr);
    MY_ASSERT(node->type == CONDITION);
    MY_ASSERT(node->getValuesCount() == 1);

    initUI(CONDITION);

    initConditionType(node);

    exec();
}

void DlgConditionType::ModifyCompareNode(NodeInfo *node)
{
    MY_ASSERT(node != nullptr);
    MY_ASSERT(node->type == COMPARE);
    MY_ASSERT(node->getValuesCount() == 3);

    initUI(COMPARE);

    initConditionType(node);
    initComparationValues(node);

    ui->btnText_1->setText(node->getValue(1));
    ui->btnText_2->setText(node->getValue(2));

    exec();
}

BaseValueClass *DlgConditionType::GetValue_Left()
{
    return m_dlgEditValueLeft->GetValuePointer_Base();
}

BaseValueClass *DlgConditionType::GetValue_Right()
{
    return m_dlgEditValueRight->GetValuePointer_Base();
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
    node = nullptr;
}

void DlgConditionType::on_buttonBox_accepted()
{
    // 修改Condition或者Compare节点
    if(node_type != INVALID)
    {
        node->modifyValue(type); // 比较运算符的类型

        if(node_type == CONDITION)
            node->UpdateText();
        else if(node_type == COMPARE && checkCompareValuesType())
        {
            // 修改value_left和value_right
            model->GetValueManager()->UpdateValueOnNode_Compare_Left(node, GetValue_Left());
            model->GetValueManager()->UpdateValueOnNode_Compare_Right(node, GetValue_Right());

            node->modifyValue(1, ui->btnText_1->text());
            node->modifyValue(2, ui->btnText_2->text());
            node->UpdateText();
        }
    }

    // 创建Condition或者Compare节点
    else
    {
        NodeInfo* new_node = nullptr;
        if(type == CONDITION_OP::AND)
        {
            new_node = model->createNode("", NODE_TYPE::CONDITION, node);
            MY_ASSERT(new_node != nullptr);
            new_node->modifyValue(CONDITION_OP::AND);
            new_node->UpdateText();
        }
        else if(type == CONDITION_OP::OR)
        {
            new_node = model->createNode("", NODE_TYPE::CONDITION, node);
            MY_ASSERT(new_node != nullptr);
            new_node->modifyValue(CONDITION_OP::OR);
            new_node->UpdateText();
        }
        else
        {
            if(!checkCompareValuesType())
                return;
            new_node = model->createNode("", NODE_TYPE::COMPARE, node);
            MY_ASSERT(new_node != nullptr);
            new_node->modifyValue(type);

            // 把value_left和value_right赋值给new_node
            model->GetValueManager()->UpdateValueOnNode_Compare_Left(new_node, GetValue_Left());
            model->GetValueManager()->UpdateValueOnNode_Compare_Right(new_node, GetValue_Right());

            new_node->modifyValue(1, m_dlgEditValueLeft->GetValueText());
            new_node->modifyValue(2, m_dlgEditValueRight->GetValueText());
            new_node->UpdateText();
        }
        node = new_node; //让GetNewNode可以取到新建的节点
    }
}

void DlgConditionType::on_btnText_1_clicked()
{
    if(node_type == INVALID) //创建Condition或者Compare节点
    {
        m_dlgEditValueLeft->CreateNewValueForParentNode(m_dlgEditValueRight->GetValuePointer_Base()->GetVarType(), node); //这里取的是另一边的变量类型
    }
    else // 修改Compare节点
    {
        m_dlgEditValueLeft->ModifyValueOnNode(node, 1);
    }

    if(m_dlgEditValueLeft->IsAccepted())
        ui->btnText_1->setText(m_dlgEditValueLeft->GetValueText());
}

void DlgConditionType::on_btnText_2_clicked()
{
    if(node_type == INVALID) //创建Condition或者Compare节点
    {
        m_dlgEditValueRight->CreateNewValueForParentNode(m_dlgEditValueLeft->GetValuePointer_Base()->GetVarType(), node); //这里取的是另一边的变量类型
    }
    else // 修改Compare节点
    {
        m_dlgEditValueRight->ModifyValueOnNode(node, 2);
    }

    if(m_dlgEditValueRight->IsAccepted())
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
    MY_ASSERT(type != INVALID_CONDITION);
}

void DlgConditionType::initComparationValues(NodeInfo *node)
{
    ValueManager* vm = model->GetValueManager();

    if(node->type == COMPARE)
    {
        CommonValueClass* value_left = vm->GetValueOnNode_Compare_Left(node);
        CommonValueClass* value_right = vm->GetValueOnNode_Compare_Right(node);

        // todo: 兼容 StructValueClass

        if(value_left != nullptr)
            *(m_dlgEditValueLeft->GetValuePointer_Base()) = *((BaseValueClass*)value_left);
        if(value_right != nullptr)
            *(m_dlgEditValueRight->GetValuePointer_Base()) = *((BaseValueClass*)value_right);
    }
    else
    {
        // todo
    }
}

bool DlgConditionType::checkCompareValuesType()
{
    if(CommonValueClass::AreSameVarType(GetValue_Left(), GetValue_Right()))
        return true;
    else
    {
        info("左右值的类型不一致！");
        return false;
    }
}
