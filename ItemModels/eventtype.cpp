#include <QDir>
#include <QCoreApplication>
#include <QTextStream>
#include "eventtype.h"

EventType* EventType::instance = nullptr;

EventType::EventType()
{
    InitEventTypes();
}

EventType::~EventType()
{
    ClearData();
}

EventType *EventType::GetInstance()
{
    if(instance == nullptr)
    {
        instance = new EventType();
    }
    return instance;
}

void EventType::InitEventTypes()
{
    QString config_path = QCoreApplication::applicationFilePath();
    config_path.replace("LevelEditor.exe", "config");

    QDir dir(config_path);
    if(!dir.exists())
    {
        createFakeData();
    }
    else
    {
        bool success = createDateByConfig(config_path + "/event_type.txt");
        if(!success)
        {
            ClearData();
            createFakeData();
        }
    }
}

void EventType::ClearData()
{
    if(paramNamesVector.size() > 0)
    {
        for(int i = 0; i < paramNamesVector.size(); i++)
        {
            paramNamesVector[i].clear();
        }
    }

    if(paramTypesVector.size() > 0)
    {
        for(int i = 0; i < paramTypesVector.size(); i++)
        {
            paramTypesVector[i].clear();
        }
    }

    paramNamesVector.clear();
    eventIdVector.clear();
    eventNameVector.clear();
}

void EventType::createFakeData()
{
    QStringList event_list;
    event_list << "空事件" //（任何触发AI的动作都将检测一次）
               << "关卡初始化"
               << "自己出生"
               << "自己死亡"
               << "---"
               << "回合X开始"
               << "回合X结束"
               << "回合计数器值到达"
               << "---"
               << "某棋子出生"
               << "某棋子死亡"
               << "某棋子受伤"
               << "---"
               << "某棋子在可被攻击范围内"
               << "某棋子离开其他人的攻击范围"
               << "---"
               << "某棋子获得了某BUFF"
               << "---"
               << "棋子被点击"
               << "---"
               << "某棋子进入某区域"
               << "---"
               << "CG动画开始"
               << "CG动画结束"
               << "---"
               << "巡逻路径结束"
               << "巡逻路径到达第X个点"
               << "---"
               << "某棋子处于X状态"//（找分类ID）
               << "flag参数发生改变"
               << "自己行动开始"
               << "自己行动结束";

    int n = event_list.size();

    for(int i = 0; i < n; i++)
    {
        if(event_list[i] == "---")
            continue;

        QStringList str_list;
        QStringList type_list;
        switch(i)
        {
        case 1:
        case 2:
        case 8:
        case 9:
        case 10:
            str_list << "棋子ID";
            type_list << "number";
            break;
        case 4:
        case 5:
        case 6:
            str_list << "回合数";
            type_list << "number";
            break;
        }

        paramNamesVector.append(str_list);
        paramTypesVector.append(type_list);
        eventIdVector.append(i + 1000);
        eventNameVector.append(event_list[i]);
    }


}

bool EventType::createDateByConfig(QString file_path)
{
    QFile file(file_path);
    if(!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QTextStream data(&file);

    while (!data.atEnd())
    {
        QString line = data.readLine();

        if(line.left(1) == "#")
            continue;

        QStringList sl = line.split('\t', QString::SkipEmptyParts);
        int n = sl.size();
        if(n < 2)
            continue;

        eventIdVector << static_cast<EVENT_TYPE_ID>(sl[0].toInt());
        eventNameVector << sl[1];

        QStringList params;
        QStringList types;
        for(int i = 2; i < n; i++)
        {
            QStringList pname_type = sl[i].split('|', QString::SkipEmptyParts);
            if(pname_type.size() == 2)
            {
                params << pname_type[0];
                types << pname_type[1];
            }
            else
            {
                types << "number";
                params << sl[i];
            }
        }
        paramNamesVector << params;
        paramTypesVector << types;
    }

    file.close();
    return true;
}
