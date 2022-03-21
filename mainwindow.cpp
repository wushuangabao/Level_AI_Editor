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
#include "Values/enuminfo.h"
#include "ItemModels/functioninfo.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    m_curETNode = nullptr;
    backupFilePaths.clear();

    // 检查config等目录是否存在
    QString config_path;
    getConfigPath(config_path);
    QDir dir_config(config_path.left(config_path.size() - 1));
    if(!dir_config.exists())
    {
        info("找不到" + config_path + "文件夹");
    }
    QString backup_path = config_path + "backup";
    QDir dir_backup;
    if(!dir_backup.exists(backup_path))
    {
        /*bool ok = */dir_backup.mkdir(backup_path);
    }

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

    InitEventTree(); //整个程序中只调用一次

    // 设置 treeView 的连接线
    ui->eventTreeView->setStyle(QStyleFactory::create("windows"));

    // 自动调整第2列的宽度
    ui->tableWidget->resizeColumnToContents(1);

    InitLevelTree(); //整个程序中只调用一次
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    bool need_save = false;
    foreach (bool already_saved, savedOrNot)
    {
        if(!already_saved)
        {
            need_save = true;
            break;
        }
    }
    if(need_save)
    {
        int ch = QMessageBox::warning(nullptr, "提示", "还有未保存的修改，确定直接退出？", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if(ch != QMessageBox::Yes)
        {
            event->ignore();
            return;
        }
    }

    // 删除所有backup文件
    QString backup_path;
    getConfigPath(backup_path);
    backup_path += "backup";
    QDir dir(backup_path);
    if(!dir.exists())
        return;
    dir.setFilter(QDir::Files);
    QStringList file_list = dir.entryList();
    int file_num = file_list.size();
    backup_path += "/";
    for(int i = 0; i < file_num; i++)
    {
        QString file_path = backup_path + file_list[i];
        deleteFile(file_path);
    }

    QMainWindow::closeEvent(event);
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

void MainWindow::saveEventItemState_Expanded(const QModelIndex &index)
{
    if(index.isValid())
    {
        if(m_itemState.contains(index))
        {
            m_itemState[index] = true;
        }
        else
            m_itemState.insert(index, true);
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

void MainWindow::saveEventItemState_Collapsed(const QModelIndex &index)
{
    if(index.isValid())
    {
        if(m_itemState.contains(index))
        {
            m_itemState[index] = false;
        }
        else
            m_itemState.insert(index, false);
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
            break;
        default:
            return;
        }
        saveBackupJsonFile();
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
        updateEventTreeState();
        saveBackupJsonFile();
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
                if(EventType::GetInstance()->GetCount() > m_dlgChoseEvtType->index)
                {
                    createNewEventOnTree(EventType::GetInstance()->GetEventLuaType(m_dlgChoseEvtType->index), new_name);
                    saveBackupJsonFile();
                }
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

        slotTreeMenuCollapse();
        slotTreeMenuExpand();
        updateEventTreeState();

        saveBackupJsonFile();
    }
}

void MainWindow::slotNewAction(bool b)
{
    Q_UNUSED(b);

    // 选择并新建一个动作节点
    m_dlgChoseActionType->CreateActionType(m_curETNode);
    QString node_text;
    NODE_TYPE type = m_dlgChoseActionType->GetNodeTypeAndText(node_text);

    switch (type)
    {
    case INVALID:
        return;
    case SET_VAR:
    {
        QStringList texts = node_text.split(" = ");
        if(texts.size() == 2)
        {
            int id_var = m_eventTreeModel->GetValueManager()->FindIdOfVarName(texts[0]);
            NodeInfo* new_node = m_curETNode->addNewChildNode_SetVar(texts[0], texts[1], id_var); //在动作序列中插入新建的setvar节点
            if(new_node != nullptr && m_dlgChoseActionType->GetValue_SetVar() != nullptr)
                m_eventTreeModel->GetValueManager()->UpdateValueOnNode_SetValue(new_node, m_dlgChoseActionType->GetValue_SetVar());
            else
            {
                info("创建SET_VAR节点失败");
                return;
            }
        }
        else
        {
            info("创建SET_VAR节点失败");
            return;
        }
        break;
    }
    case FUNCTION:
    {
        NodeInfo* new_node = m_curETNode->addNewChild(FUNCTION, node_text);
        if(new_node != nullptr)
        {
            m_eventTreeModel->GetValueManager()->UpdateValueOnNode_Function(new_node, m_dlgChoseActionType->GetValue_CallFunc());
        }
        else
        {
            info("创建FUNCTION节点失败");
            return;
        }
    }
        break;
    case OPEN_EVENT:
    case CLOSE_EVENT:
    {
        NodeInfo* new_node = m_eventTreeModel->createNode(node_text, type, m_curETNode);
        m_curETNode = new_node;
        editActionNode(new_node);
        return;
    }
        break;
    default:
        m_curETNode = m_eventTreeModel->createNode(node_text, type, m_curETNode);
        break;
    }

    slotTreeMenuCollapse();
    slotTreeMenuExpand();
    updateEventTreeState();

    saveBackupJsonFile();
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
    connect(ui->eventTreeView, SIGNAL(collapsed(const QModelIndex&)), this, SLOT(saveEventItemState_Collapsed(const QModelIndex&)));
    connect(ui->eventTreeView, SIGNAL(expanded(const QModelIndex&)), this, SLOT(saveEventItemState_Expanded(const QModelIndex&)));

    m_dlgConditionType->SetModelPointer(m_eventTreeModel);
    m_dlgSetVar->SetModel(m_eventTreeModel);
    m_dlgEditFunction->SetModel(m_eventTreeModel);
    m_dlgEditFunction->SetUpforFunction();
    m_dlgManageVar->SetModel(m_eventTreeModel);
    m_dlgChoseActionType->SetModel(m_eventTreeModel);
}

NodeInfo* MainWindow::createNewEventOnTree(QString event_type, const QString &event_name)
{
    if(!EventType::GetInstance()->IsEventIdValid(event_type))
    {
        info("ERROR param in createNewEventOnTree!");
        return nullptr;
    }

    NodeInfo* new_node = m_eventTreeModel->createNode(event_name, NODE_TYPE::EVENT, m_eventTreeModel->m_pRootNode);
    if(new_node == nullptr)
        return nullptr;

    new_node->childs[0]->UpdateEventType(EventType::GetInstance()->GetIndexOf(event_type));

    updateEventTreeState();

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
    if(m_dlgChoseEvtType->index > -1  && m_dlgChoseEvtType->index < EventType::GetInstance()->GetCount())
    {
        node->UpdateEventType(m_dlgChoseEvtType->index);
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

//    updateEventTreeState();
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
            int var_id = vm->GetIdOfVariable(var_name);
//            if(!varNameIsEventParam(var_name, event_node))
//            {
                //                    变量名     变量类型              初始值
                addOneRowInTable(row, var_name, vm->GetVarTypeAt(var_id), vm->GetInitValueOfVar(var_id)->GetText());
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

bool MainWindow::saveBackupJsonFile()
{
    if(lastLevelIndex == -1)
        return false;
    QString level_name = m_levelList[lastLevelIndex];

    QString config_path;
    getConfigPath(config_path);
    QString file_path = config_path + "backup/" + level_name + "_aoutosaved_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".json";

    QFile file(file_path);
    if(!file.open(QIODevice::ReadWrite))
    {
        qDebug() << "Bcakup file open error: " << file_path << endl;
        return false;
    }
    else
    {
        // 生成备份文件
        file.resize(0);
        generateJsonDocument(&file);
        file.close();

        // 存在旧的备份文件
        if(backupFilePaths.contains(level_name))
        {
            // 新旧备份文件路径不同，则需存储路径
            if(backupFilePaths[level_name].last() != file_path)
            {
                if(isSameFile(backupFilePaths[level_name].last(), file_path))
                {
                    // 如果内容一致，那么删掉原来那个文件，移除它的路径
                    deleteFile(backupFilePaths[level_name].last());
                    backupFilePaths[level_name].removeLast();
                }
                // 在最后插入新的备份文件路径
                backupFilePaths[level_name].push_back(file_path);
            }
            changeSavedFlag(level_name, isSameFile(config_path + level_name + ".json", file_path));
            // 路径相同，则内容已经被覆盖了，也无须再存储路径
            return true;
        }
        // 没有旧的备份文件时
        else
        {
            // 如果当前备份文件与原关卡文件的内容不同，则存储备份文件路径
            if(!isSameFile(config_path + level_name + ".json", file_path))
            {
                backupFilePaths.insert(level_name, QStringList());
                backupFilePaths[level_name].push_back(file_path);
                changeSavedFlag(level_name, false);
                return true;
            }
            // 没有改动，删除备份文件
            else
            {
                deleteFile(file_path);
                return false;
            }
        }
    }
}

void MainWindow::changeSavedFlag(const QString &level_name, bool already_saved)
{
    if(savedOrNot.contains(level_name))
    {
        if(savedOrNot[level_name] == already_saved)
            return;
        else
            savedOrNot[level_name] = already_saved;
    }
    else
    {
        savedOrNot.insert(level_name, already_saved);
    }

    int pos = m_levelList.indexOf(level_name);
    MY_ASSERT(pos != -1);
    QListWidgetItem* item = ui->levelList->item(pos);

    // 没保存，UI中显示星号
    if(!already_saved)
    {
        item->setText(level_name + " *");
    }
    // 已保存，去掉星号
    else
    {
        item->setText(level_name);
    }
}

bool MainWindow::isSameFile(const QString &path1, const QString &path2)
{
    bool same = false;
    QFile file1(path1);
    QFile file2(path2);
    if(file1.open(QIODevice::ReadOnly))
    {
        if(file2.open(QIODevice::ReadOnly))
        {
            QByteArray b1 = file1.readAll();
            QByteArray b2 = file2.readAll();
            int n = b1.size();
            if(n == b2.size())
            {
                same = true;
                for(int i = 0; i < n; i++)
                {
                    if(b1.at(i) != b2.at(i))
                    {
                        same = false;
                        break;
                    }
                }
            }
            file2.close();
        }
        else
        {
            info(path2 + "无法打开");
        }
        file1.close();
    }
    else
    {
        info(path1 + "无法打开");
    }
    return same;
}

void MainWindow::createEventTypeJsonObj(NodeInfo *node, QJsonObject *json)
{
    MY_ASSERT(node != nullptr);
    MY_ASSERT(json != nullptr);
    MY_ASSERT(node->type == ETYPE);
    MY_ASSERT(node->getValuesCount() >= 1);

    int index = node->getValue(0).toInt();
    MY_ASSERT(index != -1);

    json->insert("name", EventType::GetInstance()->GetEventLuaType(index));

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
    MY_ASSERT(json != nullptr);

    QJsonArray var_array;
    ValueManager* vm = m_eventTreeModel->GetValueManager();
    int n = vm->GetGlobalVarList().size();
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
        {
            QString var_name = vm->GetGlobalVarList().at(i);
            int var_id = vm->GetIdOfVariable(var_name);

            QJsonObject var_obj;
            var_obj.insert("id", var_id);
            var_obj.insert("name", var_name);
            var_obj.insert("type", vm->GetVarTypeAt(var_id));

            QJsonObject value_obj;
            addValueToJsonObj(vm->GetInitValueOfVar(var_id), &value_obj);
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
    MY_ASSERT(node != nullptr);
    MY_ASSERT(json != nullptr);
    MY_ASSERT(node->type == SEQUENCE);

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
                int var_id = c_node->getValue(0).toInt();
                QString var_name = m_eventTreeModel->GetValueManager()->GetVarNameAt(var_id);
                if(var_name != "")
                {
                    node_obj.insert("value", value_obj);
                    node_obj.insert("name", var_name);
                    node_obj.insert("id", var_id);
                }
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
    MY_ASSERT(node != nullptr);
    MY_ASSERT(json != nullptr);
    MY_ASSERT(node->type == CONDITION);

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
    MY_ASSERT(node != nullptr);
    MY_ASSERT(conditions != nullptr);
    MY_ASSERT(node->type == COMPARE);
    MY_ASSERT(node->getValuesCount() >= 3);

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
    MY_ASSERT(value != nullptr);
    MY_ASSERT(json != nullptr);

    switch (value->GetValueType()) {
    case VT_VAR:
    {
        int id = m_eventTreeModel->GetValueManager()->GetIdOfVariable(value);
        if(id == -1)
            info("Json提示：找不到变量" + value->GetText());
        else
            json->insert("type", m_eventTreeModel->GetValueManager()->GetVarTypeAt(id));
        json->insert("name", value->GetText());
        json->insert("id", id);
    }
        break;
    case VT_FUNC:
        {
            QJsonObject function;
            addFunctionToJsonObj(value, &function);
            json->insert("call", function);
            json->insert("type", value->GetFunctionInfo()->GetReturnTypeAt(0)); //可以是空值""，因为有些函数无返回值
        }
        break;
    case VT_STR:
        json->insert("code", value->GetText());
        json->insert("type", value->GetVarType());
        break;
    case VT_ENUM:
        json->insert("enum", value->GetText());
        json->insert("type", value->GetVarType());
        break;
    case VT_PARAM:
        json->insert("param", value->GetEventParamInLua());
        json->insert("name", value->GetText());
        json->insert("type", value->GetVarType()); //todo: 似乎应该在EventType管理器中取
        break;
    }
}

void MainWindow::addFunctionToJsonObj(BaseValueClass *value, QJsonObject *json)
{
    MY_ASSERT(value != nullptr);
    MY_ASSERT(json != nullptr);
    MY_ASSERT(value->GetValueType() == VT_FUNC);

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
//        qDebug() << "open json file: " << fileName << "success!" << endl;

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
                if(var.contains("id") && var.value("id").isDouble() &&
                   var.contains("name") && var.value("name").isString() &&
                   var.contains("type") && var.value("type").isString() &&
                   var.contains("initValue") && var.value("initValue").isObject())
                {
                    int id = var.value("id").toInt();
                    QString name = var.value("name").toString();
                    QString type = var.value("type").toString();
                    QJsonObject init_v = var.value("initValue").toObject();
                    BaseValueClass* v = parseJsonObj_Value(&init_v);
                    if(v != nullptr)
                    {
                        v->SetVarType(type);
                        if( !vm->AddNewVarAtPos(name, v, id) )
                            delete v;
                    }
                    else
                        info("变量" + name + "的初始值解析失败！");
                    continue;
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
        QString etype;
        if(eventTypeObj.contains("name") && eventTypeObj.value("name").isString())
            etype = eventTypeObj.value("name").toString();
        else
            return false;
        event_node = createNewEventOnTree(etype, event_name);
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
    // ui->eventTreeView->expandAll();
    return true;
}

bool MainWindow::parseJsonArray_Condition(QJsonArray *conditions, NodeInfo *condition_node)
{
    MY_ASSERT(condition_node->type == CONDITION);
    MY_ASSERT(condition_node->getValuesCount() == 1);
    MY_ASSERT(condition_node->getValue(0) == "AND" || condition_node->getValue(0) == "OR"); //最顶层的条件节点必须是AND或OR类型
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
                        m_eventTreeModel->GetValueManager()->UpdateValueOnNode_Compare_Left(new_node, left_value);
                        m_eventTreeModel->GetValueManager()->UpdateValueOnNode_Compare_Right(new_node, right_value);
                        ok = true;
                    }
                    if(left_value != nullptr) delete left_value;
                    if(right_value != nullptr) delete right_value;
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
                if(m_eventTreeModel->GetValueManager()->GetVarNameAt(var_id) == var_name && var_name != "")
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
                            new_node->addNewValue(QString::number(var_id));
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
        if(name == m_eventTreeModel->GetValueManager()->GetVarNameAt(id))
        {
            value = new BaseValueClass();
            value->SetVarName(name, m_eventTreeModel->GetValueManager()->GetVarTypeAt(id), id);
        }
        else
            info("Json解析：找不到变量" + name + " id=" + id);
    }
    else if(valueJsonObj->contains("code") && valueJsonObj->value("code").isString() &&
            valueJsonObj->contains("type") && valueJsonObj->value("type").isString())
    {
        QString code = valueJsonObj->value("code").toString();
        value = new BaseValueClass(code);
        QString type = valueJsonObj->value("type").toString();
        value->SetVarType(type);
    }
    else if(valueJsonObj->contains("call") && valueJsonObj->value("call").isObject() &&
            valueJsonObj->contains("type") && valueJsonObj->value("type").isString())
    {
        QJsonObject func_obj = valueJsonObj->value("call").toObject();
        value = parseJsonObj_Function(&func_obj);
        if(value != nullptr)
        {
            QString type = valueJsonObj->value("type").toString();
            value->SetVarType(type);
        }
        else
            info("Json解析：Function" + func_obj.value("function").toString() + "创建失败");
    }
    else if(valueJsonObj->contains("enum") && valueJsonObj->value("enum").isString() &&
            valueJsonObj->contains("type") && valueJsonObj->value("type").isString())
    {
        QString code = valueJsonObj->value("enum").toString();
        value = new BaseValueClass();
        QString type = valueJsonObj->value("type").toString();
        value->SetVarType(type);
        value->SetEnumValue(code);
    }
    else if(valueJsonObj->contains("param") && valueJsonObj->value("param").isString() &&
            valueJsonObj->contains("name") && valueJsonObj->value("name").isString() &&
            valueJsonObj->contains("type") && valueJsonObj->value("type").isString())
    {
        QString code = valueJsonObj->value("param").toString();
        QString name = valueJsonObj->value("name").toString();
        QString type = valueJsonObj->value("type").toString();
        value = new BaseValueClass();
        value->SetEvtParam(code, name, type);
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
        if(func != nullptr && func->GetNameLua() == func_name)
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

    // 先把同一种事件类型的trigger放到一个vector里面
    event_pos_in_table.clear();
    QMap<QString, QVector<int>> event_map;
    for(int i = 0; i < trigger_num; i++)
    {
        NodeInfo* node = m_eventTreeModel->m_pRootNode->childs[i];
        QString event_id = node->childs[0]->getValue(0);
        if(!event_map.contains(event_id))
        {
            event_map.insert(event_id, QVector<int>());
        }
        event_pos_in_table.insert(i, event_map[event_id].size());
        event_map[event_id].append(i);
    }

    file->write("local levelTable = {}\n\n");

    // 定义所有变量
    writeLuaVariables(file);
    file->write("\n");

    // 生成条件检查函数、动作序列函数
    space_num = 0;
    for(int i = 0; i < trigger_num; i++)
    {
        NodeInfo* event_node = m_eventTreeModel->m_pRootNode->childs[i];
        writeLuaEventCheckFunc(file, event_node->childs[1]);
        writeLuaEventActionFunc(file, event_node->childs[2]);
    }

    // 创建一个 lua table 事件列表
    file->write("levelTable.EventFunc = {\n");
    QMap<QString, QVector<int>>::iterator itr;
    for(itr = event_map.begin(); itr != event_map.end(); ++itr)
    {
        QString etype = EventType::GetInstance()->GetEventLuaType(itr.key().toInt());
        QString line = QString("    [E_LEVEL_TRIGGER_TYPE.%1] = {\n").arg(etype); //[事件类型ID] = { {}, {}, ... }
        file->write(line.toStdString().c_str());
        space_num = 8;
        for(int j = 0; j < itr.value().size(); j++)
        {
            writeLuaEventInfo(file, m_eventTreeModel->m_pRootNode->childs[itr.value().at(j)]);
        }
        file->write("    },\n");
    }
    file->write("}\n\n");

    // 生成一个Init函数
    writeLuaVarInitFunc(file);

    // 生成一个Excute函数
    file->write("function levelTable:Excute(event)\n"
                "    local ret = false\n"
                "    if self.EventFunc[event.id] ~= nil then\n"
                "        for _, func in self.EventFunc[event.id] do\n"
                "            if func and func.check and func.call and func.enable and func.enable == 1 and func.check(event) == 1 then\n"
                "                func.call(event, self.flowController)\n"
                "                ret = true\n"
                "                -- break\n"
                "            end\n"
                "        end\n"
                "    end\n"
                "    return ret\n"
                "end\n\n");

    file->write("return levelTable\n");
}

void MainWindow::writeLuaVariables(QFile *file)
{
    ValueManager* vm = m_eventTreeModel->GetValueManager();
    QStringList list = vm->GetGlobalVarList();
    int n = list.size();
    for(int i = 0; i < n; i++)
    {
        // 用ID生成格式化的变量名
        QString line = QString("local g_var_%1\n").arg(vm->GetIdOfVariable(list[i]));
        file->write(line.toStdString().c_str());
    }
}

void MainWindow::writeLuaVarInitFunc(QFile *file)
{
    file->write("function levelTable:init(flowController)\n    self.flowController = flowController\n");

    ValueManager* vm = m_eventTreeModel->GetValueManager();
    QStringList list = vm->GetGlobalVarList();
    int n = list.size();
    for(int i = 0; i < n; i++)
    {
        int var_id = vm->GetIdOfVariable(list[i]);
        // 格式化变量名
        QString line = QString("    g_var_%1 = ").arg(var_id);
        // 赋值为 initValue
        line += getLuaValueString(vm->GetInitValueOfVar(var_id));
        // 注释自定义的变量名、变量类型
        for(int si = line.length(); si < 24; si++)
        {
            line += " ";
        }
        line = line + "\t-- " + list.at(i) + "\t" + vm->GetVarTypeAt(var_id) + "\n";

        file->write(line.toStdString().c_str());
    }

    for(int i = 0; i < m_eventTreeModel->m_pRootNode->childs.size(); i++)
    {
        QString line = "    self.";
        writeLuaOpenOrCloseEvent(file, i, line, true);
    }

    file->write("end\n\n");
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
            info("Lua值错误：找不到" + value->GetText() + "所使用的变量");
            return "未知的值";
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
    case VT_PARAM:
        if(value->GetEventParamInLua() != "")
        {
            return QString("event.%1").arg(value->GetEventParamInLua());
        }
        else
        {
            info("Lua值错误：找不到" + value->GetText() + "所使用的事件参数");
            return QString("\""+value->GetText()+"\"");
        }
        break;
    case VT_ENUM:
    {
        return EnumInfo::GetInstance()->GetLuaStr(value->GetVarType(), value->GetText());
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

    QString str = value_func->GetFunctionName() + "(flowController";
    // value_func->GetFunctionInfo()->GetID();

    int n = value_func->GetFunctionParamsNum();
    for(int i = 0; i < n; i++)
    {
        str += ", ";
        BaseValueClass* p = value_func->GetFunctionParamAt(i);
        str = str + getLuaValueString(p); // todo 容错处理
        if(p->GetVarType() != value_func->GetFunctionInfo()->GetParamTypeAt(i) && p->GetValueType() != VT_STR)
            info("Lua提示：函数" + value_func->GetFunctionName() + "的第" + QString::number(i) + "个参数" + p->GetText() + "的数据类型不正确");
    }

    str += ")";
    return str;
}

// {
//     "事件名称",
//     function(args) return Check(args) end,
//     function(sceneId, args) Execute(sceneId, args) end
// }
bool MainWindow::writeLuaEventInfo(QFile *file, NodeInfo *event_node)
{
    MY_ASSERT(event_node != nullptr);
    MY_ASSERT(event_node->type == EVENT);

    int id = findLuaIndexOfEvent(event_node);
    MY_ASSERT(id != -1);
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
    line = str_space + "    check = Check_" + index + ",\n";
    file->write(line.toStdString().c_str());
    // 执行动作函数
    line = str_space + "    call = Execute_" + index + ",\n";
    file->write(line.toStdString().c_str());
    // enable
    line = str_space + "    enable = 1\n";
    file->write(line.toStdString().c_str());

    line = str_space + "},\n";
    file->write(line.toStdString().c_str());

    return true;
}

// function Check(args)
// end
bool MainWindow::writeLuaEventCheckFunc(QFile *file, NodeInfo *condition_node)
{
    MY_ASSERT(condition_node->parent->type == EVENT);
    MY_ASSERT(condition_node->type = CONDITION);

    int id = findLuaIndexOfEvent(condition_node);
    MY_ASSERT(id != -1);
    QString index = QString::number(id);

    QString line = "local function Check_" + index + "(event, flowController)\n";
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
    MY_ASSERT(condition_node != nullptr);
    MY_ASSERT(condition_node->type == CONDITION || condition_node->type == COMPARE);

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
        MY_ASSERT(condition_node->getValuesCount() == 3);
        ValueManager* vm = m_eventTreeModel->GetValueManager();
        BaseValueClass* v_left = vm->GetValueOnNode_Compare_Left(condition_node);
        BaseValueClass* v_right = vm->GetValueOnNode_Compare_Right(condition_node);
        if(v_left != nullptr && v_right != nullptr)
        {
            QString line = getLuaValueString(v_left) + " " + condition_node->getValue(0) + " " + getLuaValueString(v_right);
            file->write(line.toStdString().c_str());
            if(v_left->GetVarType() != v_right->GetVarType() && !(v_left->GetValueType() == VT_STR || v_right->GetValueType() == VT_STR))
                info("Lua提示：比较大小时，左值" + v_left->GetText() + "和右值" + v_right->GetText() + "的数据类型不一致");
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
    MY_ASSERT(sequence_node->parent->type == EVENT);
    MY_ASSERT(sequence_node->type = SEQUENCE);

    int id = findLuaIndexOfEvent(sequence_node);
    MY_ASSERT(id != -1);
    QString index = QString::number(id);

    QString line = "local function Execute_" + index + "(event, flowController)\n";
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
                bool flag = node->IsBreakButNotReturn();

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
                QString line = str_space + "levelTable.";
                success = writeLuaOpenOrCloseEvent(file, id, line, node->type == OPEN_EVENT ? true : false);
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
    MY_ASSERT(setvar_node != nullptr);
    MY_ASSERT(setvar_node->type == SET_VAR);
    MY_ASSERT(setvar_node->getValuesCount() >= 1);

    ValueManager* vm = m_eventTreeModel->GetValueManager();
    BaseValueClass* value = vm->GetValueOnNode_SetVar(setvar_node);

    bool ok = false;
    int id = setvar_node->getValue(0).toInt(&ok);
    if(!ok)
    {
        info("set_var节点的value[0]不是var_id");
        return false;
    }

    if(vm->GetVarTypeAt(id) != value->GetVarType() && value->GetValueType() != VT_STR)
        info("Lua提示：设置变量" + vm->GetVarNameAt(id) + "的类型与值" + value->GetText() + "的类型不一致");

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

bool MainWindow::writeLuaOpenOrCloseEvent(QFile *file, int event_pos, const QString &pre_str, bool is_open)
{
    // todo 判断 false
    NodeInfo* evt_node = m_eventTreeModel->m_pRootNode->childs[event_pos];
    int pos = event_pos_in_table[event_pos] + 1;
    QString etype = EventType::GetInstance()->GetEventLuaType(evt_node->childs[0]->getValue(0).toInt());
    QString line = pre_str;
    line = line + "EventFunc[E_LEVEL_TRIGGER_TYPE." + etype + "][" + QString::number(pos) + "].enable = ";
    if(is_open)
        line = line + "1\n";
    else
        line = line + "0\n";
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

void MainWindow::getConfigPath(QString &s)
{
    s = QCoreApplication::applicationFilePath();
    s.replace("LevelEditor.exe", "config/");
}

QString MainWindow::getTriggerNameAt(int id)
{
    int event_num = m_eventTreeModel->m_pRootNode->childs.size();
    MY_ASSERT(event_num > id && id >= 0);

    return m_eventTreeModel->m_pRootNode->childs[id]->text;
}

void MainWindow::on_eventTreeView_clicked(const QModelIndex &index)
{
    if (index.isValid())
    {
        m_curETNode = reinterpret_cast<NodeInfo*>(index.internalPointer());
    }
    else
    {
        m_curETNode = nullptr;
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
    saveBackupJsonFile();
}

void MainWindow::on_actionSave_triggered()
{
    QString file_path;
    getConfigPath(file_path);

    QString level_str = getLevelNameOnItem(ui->levelList->currentItem());
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

            changeSavedFlag(level_str, true);
        }
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QString file_path;
    getConfigPath(file_path);
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
    m_levelList.clear();
    savedOrNot.clear();

    QString config_path;
    getConfigPath(config_path);

    QDir dir(config_path.left(config_path.length() - 1));
    if(!dir.exists())
        return;
    dir.setFilter(QDir::Files);
    QStringList file_list = dir.entryList();

    int file_num = file_list.size();
    for(int i = 0; i < file_num; i++)
    {
        bool ok = false;
        int lvl_id = -1;
        QString name = file_list[i];
        int pos = name.indexOf(".json");
        if(pos != -1 && name.left(6) == "level_")
        {
            lvl_id = name.mid(6, pos - 6).toInt(&ok);
        }
        if(ok && lvl_id != -1)
            m_levelList.insert(lvl_id - 1, name.left(pos));
    }

    foreach (QString namelvl, m_levelList)
    {
        savedOrNot.insert(namelvl, true);
    }

    ui->levelList->addItems(m_levelList);

    lastLevelIndex = -1;
    if(m_levelList.size() > 0)
    {
        ui->levelList->setCurrentRow(0);
        on_levelList_itemClicked(ui->levelList->item(0));
    }
}

void MainWindow::on_levelList_itemClicked(QListWidgetItem *item)
{
    QString file_path;
    getConfigPath(file_path);

    // 存一个备份的文件
    if(lastLevelIndex != -1)
        saveBackupJsonFile();
    lastLevelIndex = ui->levelList->currentRow();

    // 打开新的关卡文件或者对应的备份文件
    QString level_name = getLevelNameOnItem(item);
    if(backupFilePaths.contains(level_name))
    {
        file_path = backupFilePaths[level_name].last();
    }
    else
    {
        file_path = file_path + level_name + ".json";
    }

    openJsonFile(file_path);

    m_itemState.clear();
    if(m_eventTreeModel->m_pRootNode->childs.size() <= 3)
        ui->eventTreeView->expandAll();
    else if(m_eventTreeModel->m_pRootNode->childs.size() <= 8)
        ui->eventTreeView->expandToDepth(1);
}

void MainWindow::on_btnDeleteVar_clicked()
{
    int idx = ui->tableWidget->currentRow();
    if(idx < 0)
        return;

    ValueManager* vm = m_eventTreeModel->GetValueManager();
    QString var_name = vm->GetGlobalVarList().at(idx);

    if(vm->CheckVarIsUsedOrNot(var_name))
    {
        info("这个变量正在被使用，无法删除");
        return;
    }

    if( !vm->DeleteVariable(var_name) )
        info("删除失败！");
    else
    {
        saveBackupJsonFile();
        updateVarTable();
    }
}

void MainWindow::on_tableWidget_itemDoubleClicked(QTableWidgetItem *item)
{
    int idx = item->row();
    m_dlgManageVar->ModifyVar(idx);
    updateVarTable();
    saveBackupJsonFile();
}

void MainWindow::deleteFile(const QString &path)
{
    QFile fileTemp(path);
    fileTemp.remove();
}

QString MainWindow::getLevelNameOnItem(QListWidgetItem *item)
{
    QString level_name = item->text();
    int pos = level_name.indexOf(" *");
    if(pos != -1)
    {
        level_name = level_name.left(level_name.size() - 2);
    }
    return level_name;
}

void MainWindow::on_levelList_customContextMenuRequested(const QPoint &pos)
{
    QListWidgetItem* cur_item = ui->levelList->itemAt(pos);
    if( cur_item == nullptr )
        return;
    QString level_name = getLevelNameOnItem(cur_item);

    QMenu *popMenu = new QMenu( this );
    QAction *copy_act = new QAction("拷贝" + level_name + "为新关卡", this);
    QAction *del_act = new QAction("删除关卡" + level_name, this);
    popMenu->addAction( copy_act );
    popMenu->addAction( del_act );
    connect( copy_act, SIGNAL(triggered()), this, SLOT(CreateNewLevel_CopyCurLvl()) );
    connect( del_act, SIGNAL(triggered()), this, SLOT(DeleteCurrentLevel()) );

    popMenu->exec( QCursor::pos() );

    delete popMenu;
    delete copy_act;
    delete del_act;
}

void MainWindow::CreateNewLevel_CopyCurLvl()
{
    QListWidgetItem* item = ui->levelList->currentItem();
    if( item == nullptr )
        return;

    QString level_name = getLevelNameOnItem(item);
    m_dlgChoseEvtType->EditLevelName(level_name);

    if(m_dlgChoseEvtType->event_name != "")
    {
        if(m_levelList.contains(m_dlgChoseEvtType->event_name))
        {
            info("已经存在这个名字的关卡！");
            return;
        }

        if(m_dlgChoseEvtType->event_name.left(6) != "level_")
        {
            info(QString("请命名为\"level_") + QString("数字\"的形式。"));
            return;
        }

        QString s_num = m_dlgChoseEvtType->event_name.mid(6);
        bool ok = false;
        s_num.toInt(&ok);
        if(!ok)
        {
            info(QString("请命名为\"level_") + QString("数字\"的形式。"));
            return;
        }

        QString path;
        getConfigPath(path);
        QString path1 = path + level_name + ".json";
        QString path2 = path + m_dlgChoseEvtType->event_name + ".json";
        QFile file1(path1);
        QFile file2(path2);

        bool success = false;

        if(file1.open(QIODevice::ReadOnly))
        {
            if(file2.open(QIODevice::WriteOnly))
            {
                file2.resize(0);
                file2.write(file1.readAll());
                file2.close();
            }
            else
            {
                info(m_dlgChoseEvtType->event_name + "是不合法的文件名");
            }
            file1.close();
            success = true;
        }
        else
        {
            info(level_name + ".json文件无法打开");
        }

        if(success)
        {
            int pos = m_levelList.size();
            m_levelList.push_back(m_dlgChoseEvtType->event_name);

            savedOrNot.insert(m_dlgChoseEvtType->event_name, true);

            ui->levelList->addItem(m_dlgChoseEvtType->event_name);
            ui->levelList->setCurrentRow(pos);
            on_levelList_itemClicked(ui->levelList->item(pos));
        }
    }
}

void MainWindow::DeleteCurrentLevel()
{
    QListWidgetItem* item = ui->levelList->currentItem();
    if( item == nullptr )
        return;
    QString level_name = getLevelNameOnItem(item);

    int ch = QMessageBox::warning(nullptr, "提示", "确定删除这个关卡？", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ch != QMessageBox::Yes)
        return;

    QString path;
    getConfigPath(path);
    path = path + level_name + ".json";

    // 删除文件
    deleteFile(path);

    // 更新数据结构
    int pos = m_levelList.indexOf(level_name);
    m_levelList.removeAt(pos);
    savedOrNot.remove(level_name);

    // UI中删除
    ui->levelList->removeItemWidget(item); //这个删除不彻底，还要delete
    delete item; item = nullptr;

    // 刷新UI选择的项
    if(lastLevelIndex == pos)
    {
        int new_pos = (pos == m_levelList.size()) ? pos - 1 : pos;
        lastLevelIndex = -1;
        ui->levelList->setCurrentRow(new_pos);
        on_levelList_itemClicked(ui->levelList->item(new_pos));
        return;
    }
    else if(lastLevelIndex > pos)
        lastLevelIndex --;
    else if(lastLevelIndex != -1)
        ui->levelList->setCurrentRow(lastLevelIndex);
}

void MainWindow::updateEventTreeState()
{
    QMap<QModelIndex, bool>::iterator itr;
    for(itr = m_itemState.begin(); itr != m_itemState.end(); ++itr)
    {
        ui->eventTreeView->setExpanded(itr.key(), itr.value());
    }
}

