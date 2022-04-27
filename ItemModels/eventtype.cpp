#include <QDir>
#include <QCoreApplication>
#include <QTextStream>
#include "eventtype.h"

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
    static EventType* instance = nullptr;
    if(instance == nullptr)
    {
        instance = new EventType();
    }
    return instance;
}

int EventType::GetIndexOf(const QString& event_id)
{
    for(int i = 0; i < eventTypeNameVector.size(); i++)
        if(eventTypeNameVector[i] == event_id)
            return i;

    return -1;
}

int EventType::GetIndexOfName(const QString &event_name)
{
    for(int i = 0; i < eventNameVector.size(); i++)
        if(eventNameVector[i] == event_name)
            return i;

    return -1;
}

QStringList *EventType::GetEventParamsUIAt(int idx)
{
    if(idx < 0 || idx >= paramNamesVector.size())
        return nullptr;

    return &(paramNamesVector[idx]);
}

QStringList *EventType::GetEventParamsLuaAt(int idx)
{
    if(idx < 0 || idx >= paramNameInLuaVector.size())
        return nullptr;

    return &(paramNameInLuaVector[idx]);
}

QStringList *EventType::GetEventParamTypes(QStringList *params)
{
    int n = paramTypesVector.size();
    for(int i = 0; i < n; i++)
    {
        if(&(paramNamesVector[i]) == params)
            return &(paramTypesVector[i]);
    }

    return nullptr;
}

QStringList EventType::GetTagList()
{
    QStringList list;
    list.clear();

    QMap<QString, QStringList>::iterator i = eventTagMap.begin();
    for(; i != eventTagMap.end(); ++i)
    {
        list.append(i.key());
    }

    return list;
}

bool EventType::CheckEventInTag(const QString &etype, const QString &tag)
{
    if(eventTagMap.contains(tag))
    {
        return eventTagMap[tag].contains(etype);
    }
    else
    {
        return false;
    }
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
    eventTypeNameVector.clear();
    eventNameVector.clear();
    eventTagMap.clear();
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
        eventTypeNameVector.append(QString::number(i + 1000));
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

        QStringList sl = line.split('\t');
        int n = sl.size();
        if(n < 3)
            continue;

        QStringList params;
        QStringList types;
        QStringList lua;
        for(int i = 3; i < n; i++)
        {
            QStringList pname_type = sl[i].split('|', QString::SkipEmptyParts);
            if(pname_type.size() == 3)
            {
                params << pname_type[0];
                types << pname_type[1];
                lua << pname_type[2];
            }
            else
            {
                continue;
            }
        }

        eventTypeNameVector << sl[0];
        eventNameVector << sl[1];
        paramNamesVector << params;
        paramTypesVector << types;
        paramNameInLuaVector << lua;

        parseTags(sl[2]);
    }

    file.close();
    return true;
}

void EventType::parseTags(QString str)
{
    QRegExp rx("(,|，)");
    QStringList sl = str.split(rx, QString::SkipEmptyParts);
    int n = sl.size();

    for(int i = 0; i < n; i++)
    {
        sl[i].remove(' ');
        if(sl[i] == "")
            continue;
        if(!eventTagMap.contains(sl[i]))
        {
            QStringList v;
            v.clear();
            eventTagMap.insert(sl[i], v);
        }
        eventTagMap[sl[i]].append(eventTypeNameVector.last());
    }
}
