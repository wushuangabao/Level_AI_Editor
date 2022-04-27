#include "enuminfo.h"

EnumInfo::~EnumInfo()
{
    dataMapUI.clear();
    dataMapLua.clear();
}

EnumInfo *EnumInfo::GetInstance()
{
    static EnumInfo* instance = nullptr;
    if(instance == nullptr)
        instance = new EnumInfo();
    return instance;
}

QStringList EnumInfo::GetEnumsOfType(const QString &var_type)
{
    QStringList enums;
    if(dataMapUI.contains(var_type))
    {
        enums.append(dataMapUI[var_type]);
    }
    return enums;
}

QStringList EnumInfo::GetAllTypes()
{
    QStringList items;
    QMap<QString, QStringList>::iterator it;
    for(it = dataMapUI.begin(); it != dataMapUI.end(); ++it)
    {
        items.push_back(QString(it.key()));
    }
    return items;
}

QString EnumInfo::GetLuaStr(const QString &var_type, const QString &s_ui)
{
    if(var_type == "number" || var_type == "string")
    {
        info("预设的值，类型不能是" + var_type);
        return "???";
    }
    if(!dataMapUI.contains(var_type) || !dataMapLua.contains(var_type))
    {
        info("找不到" + var_type + "类型！");
        return "???";
    }
    int id = dataMapUI[var_type].indexOf(s_ui);
    if(id == -1)
    {
        info("找不到" + var_type + "类型中的" + s_ui);
        return s_ui;
    }
    if(dataMapLua[var_type].size() <= id)
    {
        info(var_type + "类型的" + s_ui + "没有对应的Lua表示！");
        return s_ui;
    }
    return dataMapLua[var_type][id];
}

EnumInfo::EnumInfo()
{
    QString config_path = QCoreApplication::applicationFilePath();
    config_path.replace("LevelEditor.exe", "config");

    QDir dir(config_path);
    if(!dir.exists())
    {
        info("找不到config目录！");
    }
    else
    {
        bool success = createDateByConfig(config_path + "/enum_info.txt");
        if(!success)
        {
            info("config目录中找不到enum_info配置文件！");
        }
    }
}

bool EnumInfo::createDateByConfig(const QString &file_path)
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

        QStringList params_ui;
        QStringList params_lua;
        for(int i = 1; i < n; i++)
        {
            QStringList param = sl[i].split('|');
            MY_ASSERT(param.size() == 2);
            params_ui.push_back(param[0]);
            params_lua.push_back(param[1]);
        }

        dataMapUI.insert(sl[0], params_ui);
        dataMapLua.insert(sl[0], params_lua);
    }

    file.close();
    return true;
}
