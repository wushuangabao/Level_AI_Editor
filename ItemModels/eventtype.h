#ifndef EVENTTYPE_H
#define EVENTTYPE_H

#include <QVector>
#include <QMap>

class EventType
{
public:
    ~EventType();

    static EventType* GetInstance();

    inline int GetCount() { return eventTypeNameLuaVector.size(); }
    inline bool IsEventTypeNameLuaValid(QString etype_lua) { return eventTypeNameLuaVector.contains(etype_lua); }
    inline QStringList GetEventTypeNameUIList() { return eventTypeNameUIVector; }

    QString GetEventTypeUIOf(const QString& event_type_lua);
    inline QString GetEventTypeUIAt(int idx)
    {
        if(idx > -1 && idx < eventTypeNameUIVector.size())
            return eventTypeNameUIVector[idx];
        else
            return "";
    }

    inline QString GetEventLuaTypeAt(int idx)
    {
        if(idx > -1 && idx < eventTypeNameLuaVector.size())
            return eventTypeNameLuaVector[idx];
        else
            return "???";
    }

    QString GetEventParamNameLua(int event_type_id, int param_id);
    QString GetEventParamNameUI(int event_type_id, int param_id);

    int GetIdOfParamNameLuaInEventTypeLua(const QString &param_lua, const QString &event_type);

    int GetIndexOfEventTypeLua(const QString &event_type);
    int GetIndexOfEventTypeName(const QString &event_name);

    QStringList* GetEventParamsUIAt(int idx);
    QStringList* GetEventParamsLuaAt(int idx);
    QStringList* GetEventParamsUIOf(const QString &event_type);
    QStringList* GetEventParamsLuaOf(const QString &event_type);
    QStringList* GetEventParamTypes(QStringList* params);

    QStringList GetTagList();
    bool CheckEventInTag(const QString &etype, const QString &tag);

private:
    EventType();

    void InitEventTypes();
    void ClearData();

    void createFakeData();
    bool createDateByConfig(QString path);

    void parseTags(QString str, const QString &etype_lua);

    // 参数名ui
    QList<QStringList> paramNamesInUIVector;
    // 参数的变量类型
    QList<QStringList> paramTypesVector;
    // 参数名lua
    QList<QStringList> paramNamesInLuaVector;

    // 事件ENUM
    QList<QString> eventTypeNameLuaVector;
    // 事件名
    QStringList eventTypeNameUIVector;

    // 事件标签
    QMap<QString, QStringList> eventTagMap;
};

#endif // EVENTTYPE_H
