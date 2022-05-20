#include "enuminfo.h"
#include "structinfo.h"

StructInfo *StructInfo::GetInstance()
{
    static StructInfo* instance = nullptr;
    if(instance == nullptr)
        instance = new StructInfo();
    return instance;
}

StructInfo::~StructInfo()
{
    dataMapKeyName.clear();
    dataMapVType.clear();
}

QStringList StructInfo::GetAllStructNames()
{
    QStringList list;
    QMap<QString, QStringList>::iterator i;
    for(i = dataMapKeyName.begin(); i != dataMapKeyName.end(); ++i)
    {
        list.append(i.key());
    }
    return list;
}

bool StructInfo::CheckIsStruct(QString name)
{
    return dataMapKeyName.contains(name);
}

int StructInfo::GetKeyNumberOf(QString name)
{
    if(dataMapKeyName.contains(name))
        return dataMapKeyName[name].size();
    else
    {
        info("StructInfo中不存在" + name);
        return 0;
    }
}

QStringList StructInfo::GetKeysOf(QString name)
{
    QStringList list;
    if(dataMapKeyName.contains(name))
        list = dataMapKeyName[name];
    return list;
}

QStringList StructInfo::GetValueTypesOf(QString name)
{
    QStringList list;
    if(dataMapVType.contains(name))
        list = dataMapVType[name];
    return list;
}

QString StructInfo::GetValueTypeOf(QString name, QString key, bool no_info)
{
    if(dataMapVType.contains(name))
    {
        int i = dataMapKeyName[name].indexOf(key);
        if(i != -1)
            return dataMapVType[name][i];
    }
    if(!no_info)
        info("GetValueType发生错误");
    return "";
}

QString StructInfo::GetValueTypeOfKeyLua(const QString &name, const QString &key_lua)
{
    if(dataMapVType.contains(name))
    {
        int i = dataMapKeyLua[name].indexOf(key_lua);
        if(i != -1)
            return dataMapVType[name][i];
    }
    return "";
}

QString StructInfo::GetKeyInLua(QString name, QString key)
{
    if(dataMapKeyLua.contains(name))
    {
        int i = dataMapKeyName[name].indexOf(key);
        if(i != -1)
            return dataMapKeyLua[name][i];
    }
    info("GetKeyInLua发生错误");
    return "ERROR";
}

StructInfo::StructInfo()
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
        bool success = createDateByConfig(config_path + "/struct_info.txt");
        if(!success)
        {
            info("config目录中找不到struct_info配置文件！");
        }
    }
}

bool StructInfo::createDateByConfig(const QString &path)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QTextStream data(&file);

    while(!data.atEnd())
    {
        QString line = data.readLine();

        if(line.left(1) == "#")
            continue;

        QStringList sl = line.split('\t', QString::SkipEmptyParts);
        int n = sl.size();
        if(n < 2)
            continue;

        QStringList keys;
        QStringList vtypes;
        QStringList keyslua;
        for(int i = 1; i < n; i++)
        {
            QStringList param = sl[i].split('|');
            MY_ASSERT(param.size() == 3);
            keys.push_back(param[0]);
            vtypes.push_back(param[1]);
            keyslua.push_back(param[2]);
        }

        dataMapKeyName.insert(sl[0], keys);
        dataMapVType.insert(sl[0], vtypes);
        dataMapKeyLua.insert(sl[0], keyslua);
    }

    file.close();
    return true;
}
