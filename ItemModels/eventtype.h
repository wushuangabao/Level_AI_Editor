#ifndef EVENTTYPE_H
#define EVENTTYPE_H

#include <QVector>

#define EVENT_TYPE_ID int

class EventType
{
public:
    ~EventType();

    static EventType* GetInstance();

    QVector<QStringList> paramNamesVector;
    QVector<EVENT_TYPE_ID> eventIdVector;
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
