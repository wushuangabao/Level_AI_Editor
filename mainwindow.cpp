#include <QFileDialog>
#include <QDateTime>
#include <QStyleFactory>
#include <QStandardItemModel>

#include <QJsonParseError>
#include <QVariantMap>
#include <QJsonObject>
#include <QJsonArray>

#include "Dialogs/dlgchoseetype.h"
#include "Dialogs/dlgconditiontype.h"
#include "Dialogs/dlgeditvalue.h"
#include "Dialogs/dlgsetvariable.h"
#include "Dialogs/dlgvariablemanager.h"
#include "Dialogs/dlgchoseactiontype.h"
#include "ItemModels/functioninfo.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    m_curETNode = nullptr;
    event_args = nullptr;

    ui->setupUi(this);

    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    m_dlgChoseEvtType = new DlgChoseEType(this);
    m_dlgConditionType = new DlgConditionType(this);
    m_dlgSetVar = new DlgSetVariable(this);
    m_dlgEditFunction = new DlgEditValue(this);
    m_dlgManageVar = new DlgVariableManager(this);
    m_dlgChoseActionType = new DlgChoseActionType(this);

    InitEventTree();

    // 设置 treeView 的连接线
    ui->eventTreeView->setStyle(QStyleFactory::create("windows"));

    // 自动调整第2列的宽度
    ui->tableWidget->resizeColumnToContents(1);

    InitLevelTree();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotTreeMenu(const QPoint &pos)
{
    QString qss = "QMenu::item{padding:3px 20px 3px 20px;}QMenu::indicator{width:13px;height:13px;}";
    // 参考 https://blog.csdn.net/dpsying/article/details/80149462

    QMenu menu;
    menu.setStyleSheet(qss);

    QModelIndex curIndex = ui->eventTreeView->indexAt(pos); //当前点击的元素的index
    QModelIndex index = curIndex.sibling(curIndex.row(),0); //该行的第1列元素的index

    if (index.isValid())
    {
        m_curETNode = reinterpret_cast<NodeInfo*>(curIndex.internalPointer());

        menu.addAction(QStringLiteral("编辑节点"), this, SLOT(slotEditNode(bool)));
        menu.addSeparator();
        QAction* actCutNode = menu.addAction(QStringLiteral("剪切"), this, SLOT(slotCutNode(bool)));
        menu.addAction(QStringLiteral("复制"), this, SLOT(slotCopyNode(bool)));
        menu.addAction(QStringLiteral("粘贴"), this, SLOT(slotPasteNode(bool)));
        QAction* actDeleteNode = menu.addAction(QStringLiteral("删除"), this, SLOT(slotDeleteNode(bool)));
        menu.addSeparator();

        if(m_curETNode->type == EVENT)
            menu.addAction(QStringLiteral("新增事件"), this, SLOT(slotNewEvent(bool)));
        QAction* actCondition = menu.addAction(QStringLiteral("新增条件"), this, SLOT(slotNewCondition(bool)));
        QAction* actAddAction = menu.addAction(QStringLiteral("新增动作"), this, SLOT(slotNewAction(bool)));

        // 第一层节点不能剪切
        if(m_curETNode->parent == m_eventTreeModel->m_pRootNode)
        {
            actCutNode->setEnabled(false);
        }
        // 第二层节点不能剪切、删除
        else if(m_curETNode->parent->parent == m_eventTreeModel->m_pRootNode)
        {
            actCutNode->setEnabled(false);
            actDeleteNode->setEnabled(false);
        }

        // 只有 condition 节点可以新增条件
        if(m_curETNode->type != CONDITION)
        {
            actCondition->setEnabled(false);
        }

        // 只有“序列”节点可以新增动作（暂定）
        if(m_curETNode->type != SEQUENCE)
        {
            actAddAction->setEnabled(false);
        }
    }
    // 空白处点击右键，能新增事件
    else
    {
        menu.addAction(QStringLiteral("新增事件"), this, SLOT(slotNewEvent(bool)));
    }
    menu.exec(QCursor::pos());  //显示菜单
}

void MainWindow::slotTreeMenuExpand(bool b)
{
    Q_UNUSED(b);
    QModelIndex curIndex = ui->eventTreeView->currentIndex();
    QModelIndex index = curIndex.sibling(curIndex.row(),0); //同一行第一列元素的index
    if(index.isValid())
    {
        ui->eventTreeView->expand(index);
    }
}

void MainWindow::slotTreeMenuCollapse(bool b)
{
    Q_UNUSED(b);
    QModelIndex curIndex = ui->eventTreeView->currentIndex();
    QModelIndex index = curIndex.sibling(curIndex.row(),0); //同一行第一列元素的index
    if(index.isValid())
    {
        ui->eventTreeView->collapse(index);
    }
}

void MainWindow::slotEditNode(bool b)
{
    Q_UNUSED(b);
//    ui->eventTreeView->viewport()->update();

    if(m_curETNode != nullptr)
    {
        switch (m_curETNode->type) {

        // 一个事件
        case EVENT:
            editEventName(m_curETNode);
            break;
        // 事件类型
        case ETYPE:
            editEventType(m_curETNode);
            ui->eventTreeView->expandAll();
            updateVarTable();
            break;

        // 条件
        case CONDITION:
            m_dlgConditionType->ModifyCondition(m_curETNode);
            break;
        case COMPARE:
            m_dlgConditionType->ModifyCompareNode(m_curETNode);
            break;

        // 流程控制
        case SEQUENCE:
        case CHOICE:
        case END:
            info("不能编辑该节点。");
            break;
        case LOOP:
            // 编辑循环次数
//            m_dlgManageVar todo
            break;

        // 设置变量
        case SET_VAR:
        // 执行函数
        case FUNCTION:
        // 开启事件监听
        case OPEN_EVENT:
        // 关闭事件监听
        case CLOSE_EVENT:
            editActionNode(m_curETNode);

        default:
            break;
        }
    }

}

void MainWindow::slotCutNode(bool b)
{
    Q_UNUSED(b);
}

void MainWindow::slotCopyNode(bool b)
{
    Q_UNUSED(b);
}

void MainWindow::slotPasteNode(bool b)
{
    Q_UNUSED(b);
}

void MainWindow::slotDeleteNode(bool b)
{
    Q_UNUSED(b);
    if(m_eventTreeModel->deleteNode(m_curETNode))
    {
        m_curETNode = nullptr;
        ui->eventTreeView->expandAll();
    }
}

void MainWindow::slotNewEvent(bool b)
{
    Q_UNUSED(b);

    m_dlgChoseEvtType->CreateNewEvent();
    if(m_dlgChoseEvtType->index >= 0)
    {
        QString new_name = m_dlgChoseEvtType->event_name;
        if(new_name == "")
            info("事件名不能为空！");
        else
        {
            NodeInfo* event_node = m_eventTreeModel->FindEventByName(new_name);
            if(event_node != nullptr)
                info("已存在事件名：" + new_name);
            else
            {
                if(EventType::GetInstance()->eventIdVector.size() > m_dlgChoseEvtType->index)
                    createNewEventOnTree(EventType::GetInstance()->eventIdVector[m_dlgChoseEvtType->index], new_name);
                else
                    info("不存在这个事件类型！");
            }
        }
    }
}

void MainWindow::slotNewCondition(bool b)
{
    Q_UNUSED(b);
    if(m_curETNode->type == CONDITION)
    {
        m_dlgConditionType->CreateCondition(m_curETNode, "AND");
        ui->eventTreeView->expandAll();
    }
}

void MainWindow::slotNewAction(bool b)
{
    Q_UNUSED(b);

    m_dlgChoseActionType->CreateActionType();
    QString node_text;
    NODE_TYPE type = m_dlgChoseActionType->GetNodeTypeAndText(node_text);
    if(type != INVALID)
    {
        NodeInfo* new_node = m_eventTreeModel->createNode(node_text, type, m_curETNode);
        m_curETNode = new_node;
        ui->eventTreeView->expandAll();
        editActionNode(new_node);
    }
}

// 初始化m_eventTreeModel
void MainWindow::InitEventTree()
{
    m_eventTreeModel = new TreeItemModel(ui->eventTreeView);
    ui->eventTreeView->setModel(m_eventTreeModel);

//    m_eventTreeModel->createNode("默认事件", NODE_TYPE::EVENT);
//    ui->eventTreeView->expandAll();
//    ui->eventTreeView->setItemsExpandable(false); //暂时禁止折叠

    ui->eventTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->eventTreeView, &QTreeView::customContextMenuRequested, this, &MainWindow::slotTreeMenu);

    m_dlgConditionType->SetModelPointer(m_eventTreeModel);
    m_dlgSetVar->SetModel(m_eventTreeModel);
    m_dlgEditFunction->SetModel(m_eventTreeModel);
    m_dlgEditFunction->SetUpforFunction();
    m_dlgManageVar->SetModel(m_eventTreeModel);
    m_dlgChoseActionType->SetModel(m_eventTreeModel);
}

NodeInfo* MainWindow::createNewEventOnTree(int event_type_id, const QString &event_name)
{
    EVENT_TYPE_ID event_id = static_cast<EVENT_TYPE_ID>(event_type_id);
    if(!EventType::GetInstance()->eventIdVector.contains(event_id))
    {
        qDebug() << "ERROR param in createNewEventOnTree!" << endl;
        return nullptr;
    }

    NodeInfo* new_node = m_eventTreeModel->createNode(event_name, NODE_TYPE::EVENT, m_eventTreeModel->m_pRootNode);
    if(new_node == nullptr)
        return nullptr;

    new_node->childs[0]->UpdateEventType(event_id);
    m_eventTreeModel->GetValueManager()->UpdateEventParams(new_node, event_type_id);

    ui->eventTreeView->expandAll();
    return new_node;
}

void MainWindow::editEventName(NodeInfo *node)
{
    if(node->type != EVENT)
        return;

    m_dlgChoseEvtType->EditEventName(node->text);

    if(m_dlgChoseEvtType->index == -1)
        return;

    QString new_name = m_dlgChoseEvtType->event_name;
    NodeInfo* event_node = m_eventTreeModel->FindEventByName(new_name);
    if(new_name != "" && event_node == nullptr)
    {
        m_eventTreeModel->UpdateEventName(node, new_name);
    }
    else
    {
        if(event_node != nullptr && event_node != m_curETNode)
            info("已存在事件名：" + new_name);
        if(new_name == "")
            info("不能使用空事件名！");
    }
}

void MainWindow::editEventType(NodeInfo *node)
{
    if(node->type != ETYPE)
        return;

    // 选择事件类型（EventType）
    m_dlgChoseEvtType->EditEventType();
    if(m_dlgChoseEvtType->index > -1  && m_dlgChoseEvtType->index < EventType::GetInstance()->eventIdVector.size())
    {
        EVENT_TYPE_ID event_id = EventType::GetInstance()->eventIdVector[m_dlgChoseEvtType->index];
        node->UpdateEventType(event_id);
        m_eventTreeModel->GetValueManager()->UpdateEventParams(node->parent, event_id);
    }
}

void MainWindow::editActionNode(NodeInfo *node)
{
    switch (node->type)
    {
    case SET_VAR:
        m_dlgSetVar->EditSetVarNode(node);
        break;
    case FUNCTION:
        m_dlgEditFunction->ModifyCallNode(node);
        break;
    case OPEN_EVENT:
    case CLOSE_EVENT:
    {
        QStringList enames = m_eventTreeModel->GetEventNames();
        int id_enames = m_dlgChoseEvtType->ChoseEventNameIn(enames);
        if(id_enames >= 0 && id_enames < enames.size())
            node->modifyValue(0, enames[id_enames]);
        node->UpdateText();
    }
        break;
    default:
        break;
    }
    ui->eventTreeView->expandAll();
}

void MainWindow::addOneRowInTable(unsigned int row, const QString& s1, const QString& s2, const QString& s3)
{
    QTableWidgetItem* item;

    unsigned int row_count = ui->tableWidget->rowCount();
    if(row_count <= row)
        for(unsigned int i = row_count; i <= row; i++)
            ui->tableWidget->insertRow(i);

    // 变量名
    item = new QTableWidgetItem(s1, 0);
    ui->tableWidget->setItem(row, 0, item);

    // 变量类型
    item = new QTableWidgetItem(s2, 1);
    ui->tableWidget->setItem(row, 1, item);

    // 初始值
    item = new QTableWidgetItem(s3, 2);
    ui->tableWidget->setItem(row, 2, item);
}

void MainWindow::updateVarTable()
{
    ui->tableWidget->clearContents();

//    NodeInfo* event_node = m_eventTreeModel->findUppestNodeOf(m_curETNode);

    unsigned int row = 0;
    ValueManager* vm = m_eventTreeModel->GetValueManager();
    int n = vm->GetGlobalVarList().size();
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
        {
            QString var_name = vm->GetGlobalVarList().at(i);
//            if(!varNameIsEventParam(var_name, event_node))
//            {
                //                    变量名     变量类型              初始值
                addOneRowInTable(row, var_name, vm->GetVarTypeAt(i), vm->GetInitValueOfVar(i)->GetText());
                row++;
//            }
        }
    }

    // 自动调整列的宽度
//    ui->tableWidget->resizeColumnToContents(0);
//    ui->tableWidget->resizeColumnToContents(1);
//    ui->tableWidget->resizeColumnToContents(2);
}

void MainWindow::generateJsonDocument(QFile *file)
{
    int trigger_num = m_eventTreeModel->m_pRootNode->childs.size();
    if(trigger_num <= 0)
        return;

    QJsonObject object;
    addVariablesToJsonObj(&object);

    QJsonArray evtArray;
    for(int i = 0; i < trigger_num; i++)
    {
        QJsonObject event_type;
        createEventTypeJsonObj(m_eventTreeModel->m_pRootNode->childs[i]->childs[0], &event_type);

        QJsonObject trigger;

        trigger["event_name"] = getTriggerNameAt(i);
        trigger["event_type"] = event_type;
        addConditionToJsonObj(m_eventTreeModel->m_pRootNode->childs[i]->childs[1], &trigger);
        addActionSeqToJsonObj(m_eventTreeModel->m_pRootNode->childs[i]->childs[2], &trigger);

        evtArray.push_back(trigger);
    }
    object.insert("Event", evtArray);

    QJsonDocument document(object);
    file->write(document.toJson());
}

void MainWindow::createEventTypeJsonObj(NodeInfo *node, QJsonObject *json)
{
    Q_ASSERT(node != nullptr);
    Q_ASSERT(json != nullptr);
    Q_ASSERT(node->type == ETYPE);
    Q_ASSERT(node->getValuesCount() >= 1);

    bool ok = false;
    int eid = node->getValue(0).toInt(&ok);
    Q_ASSERT(ok == true);

    EVENT_TYPE_ID e_eid = static_cast<EVENT_TYPE_ID>(eid);
    int index = EventType::GetInstance()->eventIdVector.indexOf(e_eid);
    Q_ASSERT(index != -1);

    json->insert("id", e_eid);
//    json->insert("name", EventType::GetInstance()->eventNameVector[index]);

//    int params_num = EventType::GetInstance()->paramNamesVector[index].size();
//    if(params_num > 0)
//    {
//        QJsonArray event_params;
//        for(int i = 0; i < params_num; i++)
//        {
//            QString pname = EventType::GetInstance()->paramNamesVector[index][i];
//            event_params.push_back(pname);
//        }
//        json->insert("params", event_params);
//    }
}

void MainWindow::addVariablesToJsonObj(QJsonObject *json)
{
    Q_ASSERT(json != nullptr);

    QJsonArray var_array;
    ValueManager* vm = m_eventTreeModel->GetValueManager();
    int n = vm->GetGlobalVarList().size();
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
        {
            QString var_name = vm->GetGlobalVarList().at(i);

            QJsonObject var_obj;
            var_obj.insert("id", i);
            var_obj.insert("name", var_name);
            var_obj.insert("type", vm->GetVarTypeAt(i));

            QJsonObject value_obj;
            addValueToJsonObj(vm->GetInitValueOfVar(i), &value_obj);
            var_obj.insert("initValue", value_obj);

            var_array.push_back(var_obj);
        }
    }
    json->insert("Var", var_array);
}

// key_name : [
//   {
//      "ACTION" : "xxx",
//      ...
//   },
//   {
//      "ACTION" : "xxx",
//      ...
//   },
//   ...
// ]
void MainWindow::addActionSeqToJsonObj(NodeInfo *node, QJsonObject *json, QString key_name)
{
    Q_ASSERT(node != nullptr);
    Q_ASSERT(json != nullptr);
    Q_ASSERT(node->type == SEQUENCE);

    QJsonArray node_list;
    int n = node->childs.size();
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
        {
            NodeInfo* c_node = node->childs[i];
            QJsonObject node_obj;
            node_obj.insert("ACTION", getNodeTypeStr(c_node->type));

            switch(c_node->type)
            {
            case CHOICE:
                if(c_node->childs.size() >= 3)
                {
                    addConditionToJsonObj(c_node->childs[0], &node_obj);
                    addActionSeqToJsonObj(c_node->childs[1], &node_obj, "then");
                    addActionSeqToJsonObj(c_node->childs[2], &node_obj, "else");
                }
                break;
            case LOOP:
                if(c_node->childs.size() >= 1)
                {
//                    node_obj.insert("times", "5"); // todo 循环次数
                    addActionSeqToJsonObj(c_node->childs[0], &node_obj);
                }
                break;
            case END:
                break;
            case SET_VAR:
            {
                ValueManager* vm = m_eventTreeModel->GetValueManager();
                QJsonObject value_obj;
                addValueToJsonObj(vm->GetValueOnNode_SetVar(c_node), &value_obj);
                node_obj.insert("value", value_obj);
                node_obj.insert("name", c_node->getValue(0));
                node_obj.insert("id", vm->FindIdOfVarName(c_node->getValue(0)));
            }
                break;
            case FUNCTION:
                addFunctionToJsonObj(m_eventTreeModel->GetValueManager()->GetValueOnNode_Function(c_node), &node_obj);
                break;
            case OPEN_EVENT:
            case CLOSE_EVENT:
                node_obj.insert("name", c_node->getValue(0));
                break;
            default:
                break;
            }

            node_list.push_back(node_obj);
        }
    }


    json->insert(key_name, node_list);
}

// "AND/OR" : [conditions]
void MainWindow::addConditionToJsonObj(NodeInfo *node, QJsonObject *json)
{
    Q_ASSERT(node != nullptr);
    Q_ASSERT(json != nullptr);
    Q_ASSERT(node->type == CONDITION);

    int num = node->childs.size();

    QJsonArray conditions;
    if(num > 0)
    {
        for(int i = 0; i < num; i++)
        {
            if(node->childs[i]->type == CONDITION)
            {
                QJsonObject obj;
                addConditionToJsonObj(node->childs[i], &obj);
                conditions.push_back(obj);
            }
            else if(node->childs[i]->type == COMPARE)
            {
                addComparationToJsonArrary(node->childs[i], &conditions);
            }
        }
    }

    json->insert(node->getValue(0), conditions);
}

// {
//   type  :  "==",
//   value_left  : obj,
//   value_right : obj
// }
void MainWindow::addComparationToJsonArrary(NodeInfo *node, QJsonArray *conditions)
{
    Q_ASSERT(node != nullptr);
    Q_ASSERT(conditions != nullptr);
    Q_ASSERT(node->type == COMPARE);
    Q_ASSERT(node->getValuesCount() >= 3);

    QJsonObject comparation;
    comparation.insert("type", node->getValue(0));

    ValueManager* vm = m_eventTreeModel->GetValueManager();

    QJsonObject value_left;
    addValueToJsonObj(vm->GetValueOnNode_Compare_Left(node), &value_left);
    comparation.insert("value_left", value_left);

    QJsonObject value_right;
    addValueToJsonObj(vm->GetValueOnNode_Compare_Right(node), &value_right);
    comparation.insert("value_right", value_right);

    conditions->push_back(comparation);
}

void MainWindow::addValueToJsonObj(BaseValueClass* value, QJsonObject *json)
{
    Q_ASSERT(value != nullptr);
    Q_ASSERT(json != nullptr);

    switch (value->GetValueType()) {
    case VT_VAR:
        json->insert("name", value->GetText());
        json->insert("id", m_eventTreeModel->GetValueManager()->GetIdOfVariable(value)); // id=-1说明这个变量不存在dataList表中
        break;
    case VT_FUNC:
        {
            QJsonObject function;
            addFunctionToJsonObj(value, &function);
            json->insert("call", function);
        }
        break;
    case VT_STR:
        json->insert("code", value->GetText());
        break;
    }
}

void MainWindow::addFunctionToJsonObj(BaseValueClass *value, QJsonObject *json)
{
    Q_ASSERT(value != nullptr);
    Q_ASSERT(json != nullptr);
    Q_ASSERT(value->GetValueType() == VT_FUNC);

    json->insert("function", value->GetFunctionName());
    json->insert("id", value->GetFunctionInfo()->GetID());

    QJsonArray params;
    int n = value->GetFunctionParamsNum();
    for(int i = 0; i < n; i++)
    {
        QJsonObject param;
        addValueToJsonObj(value->GetFunctionParamAt(i), &param);
        params.push_back(param);
    }
    json->insert("params", params);
}

bool MainWindow::openJsonFile(QString fileName)
{
    QFile file( fileName );
    if ( !file.open( QIODevice::ReadOnly ) ) {
        info("文件打开失败！");
        return false;
    }

    QJsonParseError jsonParserError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson( file.readAll(), &jsonParserError );

    bool success = true;

    if ( !jsonDocument.isNull() && jsonParserError.error == QJsonParseError::NoError )
    {
        qDebug() << "open json file: " << fileName << "success!" << endl;

        if ( jsonDocument.isObject() ) {
            QJsonObject jsonObject = jsonDocument.object();

            // 变量
            if(jsonObject.contains("Var") && jsonObject.value("Var").isArray())
            {
                QJsonArray varJsonArray = jsonObject.value("Var").toArray();
                parseJsonArray_Var( &varJsonArray );
            }

            // 事件
            if(jsonObject.contains("Event") && jsonObject.value("Event").isArray())
            {
                QJsonArray evtArray = jsonObject.value("Event").toArray();
                int n = evtArray.size();
                for(int i = 0; i < n; i++)
                {
                    if(evtArray.at(i).isObject())
                    {
                        QJsonObject eventJsonObj = evtArray.at(i).toObject();
                        if(eventJsonObj.contains("event_name") && eventJsonObj.value("event_name").isString())
                        {
                            QString event_name = eventJsonObj.value("event_name").toString();
                            if(!parseJsonObj_Event( &eventJsonObj, event_name ))
                                success = false;
                        }
                    }
                    else
                        success = false;
                }
            }
        }
    }

    file.close();
    return success;
}

bool MainWindow::parseJsonArray_Var(QJsonArray *varJsonArray)
{
    m_eventTreeModel->ClearAllEvents();

    int num = varJsonArray->size();
    if(num > 0)
    {
        ValueManager* vm = m_eventTreeModel->GetValueManager();
        for(int i = 0; i < num; i++)
        {
            if(varJsonArray->at(i).isObject())
            {
                QJsonObject var = varJsonArray->at(i).toObject();
                if(var.contains("id") && var.value("id").isDouble() && var.value("id").toInt() == i)
                {
                    if(var.contains("name") && var.value("name").isString() &&
                       var.contains("type") && var.value("type").isString() &&
                       var.contains("initValue") && var.value("initValue").isObject())
                    {
                        QString name = var.value("name").toString();
                        QString type = var.value("type").toString();
                        QJsonObject init_v = var.value("initValue").toObject();
                        BaseValueClass* v = parseJsonObj_Value(&init_v);
                        if(v != nullptr)
                        {
                            vm->AddNewVariable(name, v);
                            delete v;
                        }
                        else
                            info("变量" + name + "的初始值解析失败！");
                        continue;
                    }
                }
            }
        }
    }
    updateVarTable();
    return true;
}

bool MainWindow::parseJsonObj_Event(QJsonObject *eventJsonObj, QString event_name)
{
    // 解析事件类型，创建事件树的根节点
    NodeInfo* event_node = nullptr;
    if(eventJsonObj->contains("event_type") && eventJsonObj->value("event_type").isObject())
    {
        QJsonObject eventTypeObj = eventJsonObj->value("event_type").toObject();
        EVENT_TYPE_ID eid;
        if(eventTypeObj.contains("id") && eventTypeObj.value("id").isDouble())
            eid = eventTypeObj.value("id").toInt();
        else
            return false;
        event_node = createNewEventOnTree(eid, event_name);
        if(event_node == nullptr)
            return false;
    }
    else
        return false;

    // 修改事件树的条件节点
    NodeInfo* condition_node = event_node->childs[1];
    if(eventJsonObj->contains("AND") && eventJsonObj->value("AND").isArray())
    {
        QJsonArray conditions = eventJsonObj->value("AND").toArray();
        parseJsonArray_Condition(&conditions, condition_node);
    }
    else
        return false;

    // 动作序列
    if ( eventJsonObj->contains("SEQUENCE") && eventJsonObj->value("SEQUENCE").isArray() )
    {
        QJsonArray jsonArray = eventJsonObj->value("SEQUENCE").toArray();
        if(!parseJsonArray_Sequence(&jsonArray, event_node->childs[2]))
        {
            m_eventTreeModel->deleteNode(event_node);
            return false;
        }
    }
    else
    {
        m_eventTreeModel->deleteNode(event_node); // todo: 如果 value manager 中有node和对应的value，也要删掉
        return false;
    }
    ui->eventTreeView->expandAll();
    return true;
}

bool MainWindow::parseJsonArray_Condition(QJsonArray *conditions, NodeInfo *condition_node)
{
    Q_ASSERT(condition_node->type == CONDITION);
    Q_ASSERT(condition_node->getValuesCount() == 1);
    Q_ASSERT(condition_node->getValue(0) == "AND" || condition_node->getValue(0) == "OR"); //最顶层的条件节点必须是AND或OR类型
    bool ok = false;

    int num = conditions->size();
    if(num > 0)
    {
        for ( int i = 0; i < num; i++ )
        {
            if (conditions->at(i).isObject())
            {
                QJsonObject condition = conditions->at(i).toObject();
                // AND 节点
                if(condition.contains("AND") && condition.value("AND").isArray())
                {
                    NodeInfo* new_node = condition_node->addNewChild(CONDITION, "AND");
                    new_node->modifyValue(AND);
                    QJsonArray andArr = condition.value("AND").toArray();
                    ok = parseJsonArray_Condition(&andArr, new_node);
                }
                // OR 节点
                else if(condition.contains("OR") && condition.value("OR").isArray())
                {
                    NodeInfo* new_node = condition_node->addNewChild(CONDITION, "OR");
                    new_node->modifyValue(OR);
                    QJsonArray andArr = condition.value("OR").toArray();
                    ok = parseJsonArray_Condition(&andArr, new_node);
                }
                // compare 节点
                else if(condition.contains("type") && condition.value("type").isString() &&
                        condition.contains("value_left") && condition.value("value_left").isObject() &&
                        condition.contains("value_right") && condition.value("value_right").isObject())
                {
                    QJsonObject lv_obj = condition.value("value_left").toObject();
                    QJsonObject rv_obj = condition.value("value_right").toObject();
                    BaseValueClass* left_value = parseJsonObj_Value(&lv_obj);
                    BaseValueClass* right_value = parseJsonObj_Value(&rv_obj);
                    if(left_value != nullptr && right_value != nullptr)
                    {
                        QString compare_type = condition.value("type").toString();
                        NodeInfo* new_node = condition_node->addNewChild_Compare(compare_type, left_value->GetText(), right_value->GetText());
                        m_eventTreeModel->GetValueManager()->AddValueOnNode_Compare_Left(new_node, left_value);
                        m_eventTreeModel->GetValueManager()->AddValueOnNode_Compare_Right(new_node, right_value);
                        ok = true;
                    }
                    else
                    {
                        if(left_value != nullptr) delete left_value;
                        if(right_value != nullptr) delete right_value;
                    }
                }
            }
        }
    }
    else
        return true; // 条件为空
    return ok;
}

bool MainWindow::parseJsonObj_ActionNode(QJsonObject *actionJsonObj, NodeInfo *parent_node)
{
    if(actionJsonObj->contains( "ACTION" ) && actionJsonObj->value( "ACTION" ).isString())
    {
        NODE_TYPE node_type = getNodeTypeEnum(actionJsonObj->value( "ACTION" ).toString());
        switch (node_type)
        {
        case CHOICE:
        {
            if(actionJsonObj->contains("AND") && actionJsonObj->value("AND").isArray() &&
               actionJsonObj->contains("then") && actionJsonObj->value("then").isArray() &&
               actionJsonObj->contains("else") && actionJsonObj->value("else").isArray())
            {
                QJsonArray if_cond = actionJsonObj->value("AND").toArray();
                QJsonArray then_seq = actionJsonObj->value("then").toArray();
                QJsonArray else_seq = actionJsonObj->value("else").toArray();

                NodeInfo* new_node = m_eventTreeModel->createNode("if", node_type, parent_node);
                if(new_node != nullptr)
                {
                    if(parseJsonArray_Condition(&if_cond, new_node->childs[0]) &&
                       parseJsonArray_Sequence(&then_seq, new_node->childs[1]) &&
                       parseJsonArray_Sequence(&else_seq, new_node->childs[2]))
                    {
                        return true;
                    }
                }
            }
        }
            break;
        case LOOP:
        {
            if(actionJsonObj->contains("SEQUENCE") && actionJsonObj->value("SEQUENCE").isArray())
            {
                QJsonArray loop_seq = actionJsonObj->value("SEQUENCE").toArray();
                NodeInfo* new_node = m_eventTreeModel->createNode("循环", node_type, parent_node);
                if(new_node != nullptr)
                {
                    if(parseJsonArray_Sequence(&loop_seq, new_node->childs[0]))
                    {
                        return true;
                    }
                }
            }
        }
            break;
        case END:
        {
            NodeInfo* new_node = m_eventTreeModel->createNode("跳出", node_type, parent_node);
            if(new_node != nullptr)
                return true;
        }
        case FUNCTION:
        {
            BaseValueClass* v_func = parseJsonObj_Function(actionJsonObj);
            if(v_func != nullptr)
            {
                NodeInfo* new_node = m_eventTreeModel->createNode("", node_type, parent_node);
                if(new_node != nullptr)
                {
                    m_eventTreeModel->GetValueManager()->UpdateValueOnNode_Function(new_node, v_func);
                    new_node->text = v_func->GetText();
                    return true;
                }
            }
        }
            break;
        case SET_VAR:
        {
            if(actionJsonObj->contains("name") && actionJsonObj->value("name").isString() &&
               actionJsonObj->contains("value") && actionJsonObj->value("value").isObject() &&
               actionJsonObj->contains("id") && actionJsonObj->value("id").isDouble())
            {
                QString var_name = actionJsonObj->value("name").toString();
                int var_id = actionJsonObj->value("id").toInt();

                // 检查全局变量表中是否存在这个变量
                QStringList var_list = m_eventTreeModel->GetValueManager()->GetGlobalVarList();
                if(var_id >= 0 && var_id < var_list.size() && var_list[var_id] == var_name)
                {
                    // 使用 new 创建 value
                    QJsonObject var_obj = actionJsonObj->value("value").toObject();
                    BaseValueClass* var_value = parseJsonObj_Value(&var_obj);
                    if(var_value != nullptr)
                    {
                        // 给父节点创建 set value 类型的子节点
                        NodeInfo* new_node = m_eventTreeModel->createNode("", node_type, parent_node);
                        if(new_node != nullptr)
                        {
                            new_node->addNewValue(var_name);
                            new_node->text = var_name + " = " + var_value->GetText();
                            m_eventTreeModel->GetValueManager()->UpdateValueOnNode_SetValue(new_node, var_value);
                            return true;
                        }
                        else
                            delete var_value;
                    }
                }
            }
        }
            break;
        case OPEN_EVENT:
        case CLOSE_EVENT:
        {
            if(actionJsonObj->contains("name") && actionJsonObj->value("name").isString())
            {
                NodeInfo* new_node = m_eventTreeModel->createNode("", node_type, parent_node);
                if(new_node != nullptr)
                {
                    new_node->modifyValue(0, actionJsonObj->value("name").toString());
                    new_node->UpdateText();
                    return true;
                }
            }
        }
        default:
            info(node_type + "是无法识别的动作类型！");
            return false;
        }
    }

    return false;
}

bool MainWindow::parseJsonArray_Sequence(QJsonArray *seqJsonArray, NodeInfo *seq_node)
{
    int action_num = seqJsonArray->size();
    for ( int i = 0; i < action_num; i++ )
    {
        if ( seqJsonArray->at(i).isObject() )
        {
            QJsonObject actionJsonObj = seqJsonArray->at(i).toObject();
            if(!parseJsonObj_ActionNode(&actionJsonObj, seq_node))
            {
               return false;
            }
        }
    }
    return true;
}

BaseValueClass *MainWindow::parseJsonObj_Value(QJsonObject *valueJsonObj)
{
    BaseValueClass* value = nullptr;
    if(valueJsonObj->contains("name") && valueJsonObj->value("name").isString() &&
       valueJsonObj->contains("id") && valueJsonObj->value("id").isDouble())
    {
        // 检查变量管理器中是否存在这个变量
        QString name = valueJsonObj->value("name").toString();
        int id = valueJsonObj->value("id").toInt();
        QStringList var_list = m_eventTreeModel->GetValueManager()->GetGlobalVarList();
        if(id == var_list.indexOf(name))
        {
            value = new BaseValueClass();
            value->SetVarName(name, m_eventTreeModel->GetValueManager()->GetVarTypeAt(id), id);
        }
    }
    else if(valueJsonObj->contains("code") && valueJsonObj->value("code").isString())
    {
        QString code = valueJsonObj->value("code").toString();
        value = new BaseValueClass(code);
        value->SetLuaStr(code);
    }
    else if(valueJsonObj->contains("call") && valueJsonObj->value("call").isObject())
    {
        QJsonObject func_obj = valueJsonObj->value("call").toObject();
        value = parseJsonObj_Function(&func_obj);
    }
    return value;
}

BaseValueClass *MainWindow::parseJsonObj_Function(QJsonObject *func_obj)
{
    BaseValueClass* value = nullptr;

    if(func_obj->contains("function") && func_obj->value("function").isString() &&
       func_obj->contains("id") && func_obj->value("id").isDouble() &&
       func_obj->contains("params") && func_obj->value("params").isArray())
    {
        QString func_name = func_obj->value("function").toString();
        FUNCTION_ID func_id = func_obj->value("id").toInt();
        FunctionClass* func = FunctionInfo::GetInstance()->GetFunctionInfoByID(func_id);
        if(func != nullptr && func->GetName() == func_name)
        {
            value = new BaseValueClass();
            value->SetFunction(func); //这里已经new指定数量的参数了
            bool success = true; //如果失败要把value释放掉

            QJsonArray params = func_obj->value("params").toArray();
            int param_num = params.size();
            if(param_num != func->GetParamNum())
                success = false;

            for(int i = 0; i < param_num; i++)
            {
                if(params[i].isObject())
                {
                    QJsonObject param = params[i].toObject();
                    BaseValueClass* param_v = parseJsonObj_Value(&param);
                    if(param_v != nullptr)
                    {
                        value->SetParamAt(i, param_v);
                        delete param_v;
                        param_v = nullptr;
                    }
                    else
                    {
                        info("解析Json时，设置函数参数值失败");
                        success = false;
                        break;
                    }
                }
                else {
                    success = false;
                    break;
                }
            }
            if(!success)
            {
                delete value;
                value = nullptr;
            }
        }
    }

    return value;
}

void MainWindow::generateLuaDocument(QFile *file)
{
    int trigger_num = m_eventTreeModel->m_pRootNode->childs.size();
    if(trigger_num <= 0)
        return;

    // 定义所有变量
    writeLuaVariables(file);
    file->write("\n");

    // 事件列表
    // 先把同一种事件类型的trigger放到一个vector里面
    event_pos_in_table.clear();
    QMap<int, QVector<int>> event_map;
    for(int i = 0; i < trigger_num; i++)
    {
        NodeInfo* node = m_eventTreeModel->m_pRootNode->childs[i];
        int event_id = node->childs[0]->getValue(0).toInt();
        if(!event_map.contains(event_id))
        {
            event_map.insert(event_id, QVector<int>());
        }
        event_pos_in_table.insert(i, event_map[event_id].size());
        event_map[event_id].append(i);
    }
    // 然后创建一个lua table
    file->write("local EventFunc = {\n");
    QMap<int, QVector<int>>::iterator itr;
    for(itr = event_map.begin(); itr != event_map.end(); ++itr)
    {
        QString line = QString("    [%1] = {\n").arg(itr.key()); //[事件类型ID] = { {}, {}, ... }
        file->write(line.toStdString().c_str());
        space_num = 8;
        for(int j = 0; j < itr.value().size(); j++)
        {
            writeLuaEventInfo(file, m_eventTreeModel->m_pRootNode->childs[itr.value().at(j)]);
        }
        file->write("    },\n");
    }
    file->write("}\n\n");

    // 生成一个Excute函数
    file->write("function Excute(event)\n"
                "    if EventFunc[event.id] ~= nil then\n"
                "        for _, func in EventFunc[event.id] do\n"
                "            if func and func.check and func.call and func.enable and func.enable == 1 and func.check(event) == 1 then\n"
                "                func.call()\n"
                "                -- break\n"
                "            end\n"
                "        end\n"
                "    end\n"
                "end\n\n");

    // 生成条件检查函数、动作序列函数
    space_num = 0;
    for(int i = 0; i < trigger_num; i++)
    {
        NodeInfo* event_node = m_eventTreeModel->m_pRootNode->childs[i];
        writeLuaEventCheckFunc(file, event_node->childs[1]);
        writeLuaEventActionFunc(file, event_node->childs[2]);
    }
}

void MainWindow::writeLuaVariables(QFile *file)
{
    ValueManager* vm = m_eventTreeModel->GetValueManager();
    int n = vm->GetGlobalVarList().size();
    for(int i = 0; i < n; i++)
    {
        // 用ID生成格式化的变量名
        QString line = QString("local g_var_%1 = ").arg(i);
        // 变量初始值 (vm->GetInitValueOfVar(i));
        line += getLuaValueString(vm->GetInitValueOfVar(i));
        // 注释自定义的变量名、变量类型
        line = line + "\t\t-- " + vm->GetGlobalVarList().at(i) + "\t\t" + vm->GetVarTypeAt(i) + "\n";

        file->write(line.toStdString().c_str());
    }
}

QString MainWindow::getLuaValueString(BaseValueClass* value)
{
    VALUE_TYPE vtype = value->GetValueType();
    switch (vtype) {
    case VT_STR:
        return value->GetText();
        break;
    case VT_VAR: // todo: 检查变量名是否正确
    {
        int id = m_eventTreeModel->GetValueManager()->GetIdOfVariable(value);
        if(id == -1)
        {
            if(event_args != nullptr)
            {
                int id_arg = event_args->indexOf(value->GetText());
                if(id_arg != -1)
                    return QString("event.arg%1").arg(id_arg + 1);
            }
            return "";
        }
        else
            return QString("g_var_%1").arg(id);
    }
        break;
    case VT_FUNC:
    {
        return getLuaCallString(value);
    }
        break;
    default:
        return "";
        break;
    }
}

QString MainWindow::getLuaCallString(BaseValueClass *value_func)
{
    if(value_func == nullptr)
    {
        return "";
    }
    if(value_func->GetValueType() != VT_FUNC)
    {
        info("这个值没有调用Function！");
        return "";
    }

    QString str = value_func->GetFunctionName() + "(";
    // value_func->GetFunctionInfo()->GetID();

    int n = value_func->GetFunctionParamsNum();
    for(int i = 0; i < n; i++)
    {
        if(i > 0)
        {
            str += ", ";
        }
        str = str + getLuaValueString(value_func->GetFunctionParamAt(i)); // todo 容错处理
    }

    str += ")";
    return str;
}

// {
//     "事件名称",
//     function(args) return xxx_Check(args) end,
//     function(sceneId, args) xxx_Execute(sceneId, args) end
// }
bool MainWindow::writeLuaEventInfo(QFile *file, NodeInfo *event_node)
{
    Q_ASSERT(event_node != nullptr);
    Q_ASSERT(event_node->type == EVENT);

    int id = findLuaIndexOfEvent(event_node);
    Q_ASSERT(id != -1);
    QString index = QString::number(id);

    QString str_space = "";
    for(int i = 0; i < space_num; i++)
        str_space += " ";

    QString line = str_space + "{   --";
    file->write(line.toStdString().c_str());

    // 事件名称
    line = event_node->text + "\n";
    file->write(line.toStdString().c_str());
    // 条件检查函数
    line = str_space + "    check = xxx_Check_" + index + ",\n";
    file->write(line.toStdString().c_str());
    // 执行动作函数
    line = str_space + "    call = xxx_Execute_" + index + ",\n";
    file->write(line.toStdString().c_str());
    // enable
    line = str_space + "    enable = 1\n";
    file->write(line.toStdString().c_str());

    line = str_space + "},\n";
    file->write(line.toStdString().c_str());

    return true;
}

// function xxx_Check(args)
// end
bool MainWindow::writeLuaEventCheckFunc(QFile *file, NodeInfo *condition_node)
{
    Q_ASSERT(condition_node->parent->type == EVENT);
    Q_ASSERT(condition_node->type = CONDITION);

    int id = findLuaIndexOfEvent(condition_node);
    Q_ASSERT(id != -1);
    QString index = QString::number(id);

    QString line = "function xxx_Check_" + index + "(event)\n";
    file->write(line.toStdString().c_str());

    if(condition_node->childs.size() == 0)
    {
        file->write("    return 1\nend\n\n");
        return true;
    }
    else
    {
        file->write("    if ");
        bool success = writeLuaCondition(file, condition_node);
        file->write(" then\n        return 1\n    else\n        return 0\n    end\nend\n\n");
        return success;
    }
}

bool MainWindow::writeLuaCondition(QFile *file, NodeInfo *condition_node)
{
    Q_ASSERT(condition_node != nullptr);
    Q_ASSERT(condition_node->type == CONDITION || condition_node->type == COMPARE);

    if(condition_node->parent->type == EVENT)
    {
        event_args = m_eventTreeModel->GetEventParamsOf(condition_node);
    }

    CONDITION_OP opt_enum = getConditionEnum(condition_node->getValue(0));
    switch (opt_enum) {
    case AND:
    {
        file->write("(");
        int children_num = condition_node->childs.size();
        for(int i = 0; i < children_num; i++)
        {
            if(i != 0)
                file->write(" and ");
            writeLuaCondition(file, condition_node->childs[i]);
        }
        if(children_num < 1)
        {
            if(condition_node->parent->type == CONDITION && condition_node->parent->getValue(0) == "OR")
                file->write("0");
            else
                file->write("1");
        }
        file->write(")");
    }
        break;
    case OR:
    {
        file->write("(");
        int children_num = condition_node->childs.size();
        for(int i = 0; i < children_num; i++)
        {
            if(i != 0)
                file->write(" or ");
            writeLuaCondition(file, condition_node->childs[i]);
        }
        if(children_num < 1)
        {
            if(condition_node->parent->type == CONDITION && condition_node->parent->getValue(0) == "OR")
                file->write("0");
            else
                file->write("1");
        }
        file->write(")");
    }
        break;
    case EQUAL_TO:
    case GREATER_THAN:
    case LESS_THAN:
    case EQUAL_GREATER:
    case EQUAL_LESS:
    {
        Q_ASSERT(condition_node->getValuesCount() == 3);
        ValueManager* vm = m_eventTreeModel->GetValueManager();
        BaseValueClass* v_left = vm->GetValueOnNode_Compare_Left(condition_node);
        BaseValueClass* v_right = vm->GetValueOnNode_Compare_Right(condition_node);
        if(v_left != nullptr && v_right != nullptr)
        {
            QString line = getLuaValueString(v_left) + " " + condition_node->getValue(0) + " " + getLuaValueString(v_right);
            file->write(line.toStdString().c_str());
        }
        else
            return false;
    }
        break;
    default:
        return false;
        break;
    }
    return true;
}

bool MainWindow::writeLuaEventActionFunc(QFile *file, NodeInfo *sequence_node)
{
    Q_ASSERT(sequence_node->parent->type == EVENT);
    Q_ASSERT(sequence_node->type = SEQUENCE);

    int id = findLuaIndexOfEvent(sequence_node);
    Q_ASSERT(id != -1);
    QString index = QString::number(id);

    QString line = "function xxx_Execute_" + index + "()\n";
    file->write(line.toStdString().c_str());

    if(sequence_node->childs.size() <= 0)
    {
        file->write("end\n\n");
        return true;
    }
    else
    {
        space_num = 4;
        if(writeLuaSequence(file, sequence_node))
        {
            file->write("end\n\n");
            return true;
        }
        else
        {
            space_num = 0;
            info("生成动作序列的Lua函数失败！");
            return false;
        }
    }
}

bool MainWindow::writeLuaSequence(QFile *file, NodeInfo *sequence_node)
{
    QString str_space = "";
    for(int i =0; i < space_num; i++)
        str_space += " ";

    bool success = true;
    int children_num = sequence_node->childs.size();

    for(int i = 0; i < children_num; i++)
    {
        NodeInfo* node = sequence_node->childs[i];
        switch (node->type) {
        case SET_VAR:
            success = writeLuaSetVar(file, node);
            break;
        case FUNCTION:
        {
            ValueManager* vm = m_eventTreeModel->GetValueManager();
            QString str_call = getLuaCallString(vm->GetValueOnNode_Function(node));
            if(str_call == "")
                success = false;
            else
            {
                QString line = str_space + str_call + "\n";
                file->write(line.toStdString().c_str());
            }
        }
            break;
        case CHOICE:
            if(node->childs.size() != 3 || node->childs[0]->type != CONDITION || node->childs[1]->type != SEQUENCE || node->childs[2]->type != SEQUENCE)
            {
                info("CHOICE节点的子节点数量和类型错误！");
                success = false;
            }
            else
            {
                // if ... then
                QString line = str_space + "if ";
                file->write(line.toStdString().c_str());
                if(!writeLuaCondition(file, node->childs[0]))
                    success = false;
                file->write(" then\n");

                // 一系列动作
                space_num += 4;
                if(!writeLuaSequence(file, node->childs[1]))
                    success = false;
                space_num -= 4;

                // else
                line = str_space + "else\n";
                file->write(line.toStdString().c_str());

                // 一系列动作
                space_num += 4;
                if(!writeLuaSequence(file, node->childs[2]))
                    success = false;
                space_num -= 4;

                // end
                line = str_space + "end\n";
                file->write(line.toStdString().c_str());
            }
            break;
        case LOOP:
            if(node->childs.size() != 1 || node->childs[0]->type != SEQUENCE)
            {
                info("LOOP节点的子节点数量和类型错误！");
                success = false;
            }
            else
            {
                // while 1 do
                QString line = str_space + "while 1 do\n";
                file->write(line.toStdString().c_str());

                // 一系列动作
                space_num += 4;
                if(!writeLuaSequence(file, node->childs[0]))
                    success = false;
                space_num -= 4;

                // end
                line = str_space + "end\n";
                file->write(line.toStdString().c_str());
            }
            break;
        case END:
            if(node->parent->type == SEQUENCE)
            {
                QString line = str_space;
                bool flag = false;

                NodeInfo* cur_node = node->parent;
                while(cur_node->type != EVENT)
                {
                    if(cur_node->type == LOOP)
                    {
                        flag = true;
                        break;
                    }
                    cur_node = cur_node->parent;
                }

                if(flag)
                {
                    line = line + "break\n";
                    file->write(line.toStdString().c_str());
                }
                else
                {
                    line = line + "return\n";
                    file->write(line.toStdString().c_str());
                }
            }
            else
            {
                info("END跳出节点的位置有错误！");
            }
            break;
        case OPEN_EVENT:
        case CLOSE_EVENT:
        {
            int id = m_eventTreeModel->FindEventPosByName(node->getValue(0));
            if(id == -1)
                success = false;
            else if(event_pos_in_table.contains(id))
            {
                NodeInfo* evt_node = m_eventTreeModel->m_pRootNode->childs[id];
                int pos = event_pos_in_table[id] + 1;
                QString line = str_space;
                line = line + "EventFunc[" + evt_node->childs[0]->getValue(0) + "][" + QString::number(pos) + "].enable = ";
                if(node->type == OPEN_EVENT)
                    line = line + "1\n";
                else
                    line = line + "0\n";
                file->write(line.toStdString().c_str());
            }
            else
                success = false;
        }
            break;
        default:
            break;
        }
    }

    return success;
}

bool MainWindow::writeLuaSetVar(QFile *file, NodeInfo *setvar_node)
{
    Q_ASSERT(setvar_node != nullptr);
    Q_ASSERT(setvar_node->type == SET_VAR);
    Q_ASSERT(setvar_node->getValuesCount() >= 1);

    if(setvar_node->getValue(0) == "")
        return false;

    ValueManager* vm = m_eventTreeModel->GetValueManager();
    BaseValueClass* value = vm->GetValueOnNode_SetVar(setvar_node);

    int id = vm->GetGlobalVarList().indexOf(setvar_node->getValue(0));
    if(id == -1)
        return false;

    QString str_value = getLuaValueString(value);
    if(str_value == "")
        return false;

    QString line = "";
    for(int i = 0; i < space_num; i++)
    {
        line += " ";
    }
    line = line + QString("g_var_%1").arg(id) + " = " + str_value + "\n";

    file->write(line.toStdString().c_str());
    return true;
}

int MainWindow::findLuaIndexOfEvent(NodeInfo *node)
{
    for(int i = 0; i < m_eventTreeModel->m_pRootNode->childs.size(); i++)
    {
        if(m_eventTreeModel->findUppestNodeOf(node) == m_eventTreeModel->m_pRootNode->childs[i])
            return i;
    }
    return -1;
}

QString MainWindow::getTriggerNameAt(int id)
{
    int event_num = m_eventTreeModel->m_pRootNode->childs.size();
    Q_ASSERT(event_num > id && id >= 0);

    return m_eventTreeModel->m_pRootNode->childs[id]->text;
}

void MainWindow::on_eventTreeView_clicked(const QModelIndex &index)
{
    if (index.isValid())
    {
        m_curETNode = reinterpret_cast<NodeInfo*>(index.internalPointer());
    }
}

void MainWindow::on_eventTreeView_doubleClicked(const QModelIndex &index)
{
    on_eventTreeView_clicked(index);
    slotEditNode();
}

void MainWindow::on_btnAddVar_clicked()
{

    m_dlgManageVar->CreateVar();

    updateVarTable();
}

void MainWindow::on_actionSave_triggered()
{
    QString file_path = QCoreApplication::applicationFilePath();
    file_path.replace("LevelEditor.exe", "config/");

    QString level_str = ui->levelList->currentItem()->text();
    if(level_str != "" && level_str.contains("level_"))
    {
        QString level_file_path = file_path + level_str + ".json";

        QFile file_level(level_file_path);
        if(!file_level.open(QIODevice::ReadWrite))
        {
            qDebug() << "File open error: " << level_file_path << endl;
        }
        else
        {
            file_level.resize(0);
            generateJsonDocument(&file_level);
            file_level.close();
        }
    }

    // 再存一个备份的文件
    QString time_str = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString backup_file_path = file_path + "backup/saved_" + time_str + ".json";

    QFile file(backup_file_path);
    if(!file.open(QIODevice::ReadWrite))
    {
        qDebug() << "File open error: " << backup_file_path << endl;
    }
    else
    {
        file.resize(0);
        generateJsonDocument(&file);
        file.close();
    }

}

void MainWindow::on_actionOpen_triggered()
{
    QString file_path = QCoreApplication::applicationFilePath();
    file_path.replace("LevelEditor.exe", "config/");
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", file_path, "Json (*.json)");
    if(!fileName.isNull())
    {
        openJsonFile(fileName);
    }
}

void MainWindow::on_actionLua_triggered()
{
    QString file_path = QCoreApplication::applicationFilePath();
    file_path.replace("LevelEditor.exe", "");
    QString fileName = QFileDialog::getSaveFileName(this, "Save Lua", file_path, "Lua (*.lua)");
    if(!fileName.isEmpty())
    {
        QFile file(fileName);
        if(!file.open(QIODevice::ReadWrite))
        {
            info("无效路径！");
        }
        else
        {
            file.resize(0);
            generateLuaDocument(&file);
            file.close();
        }
    }
}

void MainWindow::InitLevelTree()
{
    m_levelList = QStringList();

    for(int i = 1; i <= 30; i++)
    {
        m_levelList << QString("level_%1").arg(i);
    }

    ui->levelList->addItems(m_levelList);

    if(m_levelList.size() > 0)
    {
        ui->levelList->setCurrentRow(0);
        on_levelList_itemClicked(ui->levelList->item(0));
    }
}

void MainWindow::on_levelList_itemClicked(QListWidgetItem *item)
{
    QString file_path = QCoreApplication::applicationFilePath();
    file_path.replace("LevelEditor.exe", "config/");
    file_path = file_path + item->text() + ".json";
    openJsonFile(file_path);
}

void MainWindow::on_btnDeleteVar_clicked()
{

}

void MainWindow::on_tableWidget_itemDoubleClicked(QTableWidgetItem *item)
{
    int idx = item->row();

    m_dlgManageVar->ModifyVar(idx);

    updateVarTable();
}
