#ifndef ENUMINFO_H
#define ENUMINFO_H

#include <QStringList>
#include <QMap>
#include <QDir>
#include <QCoreApplication>
#include <QTextStream>
#include "../ItemModels/enumdefine.h"

class EnumInfo
{
public:
    ~EnumInfo();
    static EnumInfo* GetInstance();

    QStringList GetEnumsOfType(const QString& var_type);
    QStringList GetAllTypes();

    QString GetLuaStr(const QString& var_type, const QString& s_ui);

    bool CheckVarTypeIsEnum(const QString& type);
private:
    EnumInfo();
    bool createDateByConfig(const QString& path);

    // <变量类型，枚举值列表>
    QMap<QString, QStringList> dataMapUI;
    QMap<QString, QStringList> dataMapLua;
};

#endif // ENUMINFO_H
