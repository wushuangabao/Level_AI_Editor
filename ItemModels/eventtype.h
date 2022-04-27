#ifndef EVENTTYPE_H
#define EVENTTYPE_H

#include <QVector>
#include <QMap>

class EventType
{
public:
    ~EventType();

    static EventType* GetInstance();

    inline int GetCount() { return eventTypeNameVector.size(); }
    inline bool IsEventIdValid(QString etype_lua) { return eventTypeNameVector.contains(etype_lua); }
    inline QStringList GetEventTypeList() { return eventNameVector; }

    inline QString GetEventNameAt(int idx)
    {
        if(idx > -1 && idx < eventNameVector.size())
            return eventNameVector[idx];
        else
            return "";
    }

    inline QString GetEventLuaType(int idx)
    {
        if(idx > -1 && idx < eventTypeNameVector.size())
            return eventTypeNameVector[idx];
        else
            return "???";
    }

    int GetIndexOf(const QString &event_id);
    int GetIndexOfName(const QString &event_name);

    QStringList* GetEventParamsUIAt(int idx);
    QStringList* GetEventParamsLuaAt(int idx);
    QStringList* GetEventParamTypes(QStringList* params);

    QStringList GetTagList();
    bool CheckEventInTag(const QString &etype, const QString &tag);

private:
    EventType();

    void InitEventTypes();
    void ClearData();

    void createFakeData();
    bool createDateByConfig(QString path);

    void parseTags(QString str);

    // 参数名ui
    QVector<QStringList> paramNamesVector;
    // 参数的变量类型
    QVector<QStringList> paramTypesVector;
    // 参数名lua
    QVector<QStringList> paramNameInLuaVector;

    // 事件ENUM
    QVector<QString> eventTypeNameVector;
    // 事件名
    QStringList eventNameVector;

    // 事件标签
    QMap<QString, QStringList> eventTagMap;
};

#endif // EVENTTYPE_H
