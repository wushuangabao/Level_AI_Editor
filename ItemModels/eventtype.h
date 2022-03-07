#ifndef EVENTTYPE_H
#define EVENTTYPE_H

#include <QVector>

#define EVENT_TYPE_ID int

class EventType
{
public:
    ~EventType();

    static EventType* GetInstance();

    // 参数名
    QVector<QStringList> paramNamesVector;
    // 参数的变量类型
    QVector<QStringList> paramTypesVector;

    // 事件ID
    QVector<EVENT_TYPE_ID> eventIdVector;
    // 事件名
    QStringList eventNameVector;

private:
    EventType();

    void InitEventTypes();
    void ClearData();

    void createFakeData();
    bool createDateByConfig(QString path);

    static EventType* instance;
};

#endif // EVENTTYPE_H
