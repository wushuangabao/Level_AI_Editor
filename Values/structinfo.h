#ifndef STRUCTINFO_H
#define STRUCTINFO_H

#include <QStringList>
#include <QMap>

class StructInfo
{
public:
    static StructInfo* GetInstance();
    ~StructInfo();

    QStringList GetAllStructNames();
    bool CheckIsStruct(QString name);

    int GetKeyNumberOf(QString name);
    QStringList GetKeysOf(QString name);
    QStringList GetValueTypesOf(QString name);

    QString GetValueTypeOf(QString name, QString key);
    QString GetValueTypeOfKeyLua(const QString &name, const QString &key_lua);
    QString GetKeyInLua(QString name, QString key);

private:
    StructInfo();
    bool createDateByConfig(const QString& path);

    QMap<QString, QStringList> dataMapKeyName; // key names of struct (show on UI)
    QMap<QString, QStringList> dataMapVType; // var type of keys
    QMap<QString, QStringList> dataMapKeyLua; // key names in lua table (generate Lua)
};

#endif // STRUCTINFO_H
