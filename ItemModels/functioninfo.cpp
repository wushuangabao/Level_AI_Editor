#include <QDir>
#include <QCoreApplication>
#include <QTextStream>
#include <QDebug>
#include "enumdefine.h"
#include "../Values/structinfo.h"
#include "../Values/enuminfo.h"
#include "../Utils/pinyin.h"
#include "functioninfo.h"

FunctionInfo* FunctionInfo::data = nullptr;

FunctionInfo::~FunctionInfo()
{
    ClearData();
}

FunctionInfo *FunctionInfo::GetInstance()
{
    if(data == nullptr)
    {
        data = new FunctionInfo();
    }
    return data;
}

int FunctionInfo::GetFunctionInfoCount()
{
    return infoList.size();
}

FunctionClass *FunctionInfo::GetFunctionInfoByLuaName(const QString &name)
{
    int n = infoList.size();
    for(int i = 0; i < n; i++)
    {
        if(infoList[i].GetNameLua() == name)
            return &(infoList[i]);
    }
    return nullptr;
}

FunctionClass *FunctionInfo::GetFunctionInfoAt(int idx)
{
    int n = infoList.size();
    MY_ASSERT(n > idx && idx >= 0);

    return &(infoList[idx]);
}

int FunctionInfo::CheckFunctionNameLuaExist(const QString &name)
{
    int n = infoList.size();
    for(int i = 0; i < n; i++)
    {
        if(infoList[i].GetNameLua() == name)
        {
            return i;
        }
    }
    return -1;
}

QStringList FunctionInfo::GetTagList()
{
    QStringList list;
    list.clear();

    QMap<QString, QStringList>::iterator i = tagMap.begin();
    for(; i != tagMap.end(); ++i)
    {
        list.append(i.key());
    }

    return list;
}

bool FunctionInfo::CheckFunctionInTag(FunctionClass *func, const QString &tag)
{
    if(tagMap.contains(tag))
    {
        return tagMap[tag].contains(func->GetNameLua());
    }
    else
    {
        info("CheckFunctionInTag Error tag: " + tag);
        return false;
    }
}

FunctionInfo::FunctionInfo()
{
    config_path = QCoreApplication::applicationFilePath();
    config_path.replace("LevelEditor.exe", "config");

    parse_flag = "-- This function will be parsed by LevelAIEditor --";

    ClearData();
    InitFuncInfo();
}

void FunctionInfo::InitFuncInfo()
{
    QDir dir(config_path);
    if(!dir.exists())
    {
        createFakeData();
    }
    else
    {
        bool success = createDateByConfig(config_path + "/function_info.txt");
        if(!success)
        {
            ClearData();
            info("找不到 config/function_info.txt ！");
            createFakeData();
        }
    }
}

void FunctionInfo::ClearData()
{
    int n = infoList.size();
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
        {
            infoList[i].Clear();
        }
    }

    tagMap.clear();
}

void FunctionInfo::createFakeData()
{

}

bool FunctionInfo::createDateByConfig(QString file_path)
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
        if(line.left(5) == "Path=")
        {
            QString path = line.mid(5);
            createDataByLuaFile(path);
            continue;
        }

        QStringList sl = line.split('\t');
        int n = sl.size();
        MY_ASSERT(n >= 7);

        // 检查函数名称是否已经有了。如果已经有了，跳过这一行
        if(GetFunctionInfoByLuaName(sl[2]) != nullptr)
            continue;
        FunctionClass func;

        // 函数名称
        func.name_lua = sl[2];
        func.name_ui = sl[3];

        // 1=场景AI专属，0=棋子AI专属
        bool ok;
        int i = sl[1].toInt(&ok);
        if(!ok || i == 0) continue;

        // 描述
        parseTextsAndParams(&func, sl[4]);

        // 返回值类型
        if(sl[5] != "0")
        {
            if(sl[5] == "1")
                func.values.append("number");
            else
                func.values = sl[5].split(',');
        }

        // 是否在Function动作节点设置时可选
        if(sl[6] == "0")
            func.can_be_call = false;
        else
            func.can_be_call = true;

        // 注释
        if(n >= 8)
            func.note = sl[7];
        else
            func.note = "";

        // 标签
        if(n >= 9)
            parseTags(sl[8], func.name_lua);

        addFunctionInfo(func, false);
    }

    file.close();
    return true;
}

bool FunctionInfo::createDataByLuaFile(const QString &lua_path)
{
    QString abs_path = config_path + lua_path;
    QFile file(abs_path);
    if(!file.open(QIODevice::ReadOnly))
    {
        info(lua_path + "打开失败");
        return false;
    }
    QTextStream data(&file);
    while (!data.atEnd())
    {
        QString line = data.readLine();
        if(line.contains(parse_flag))
        {
            bool ok = true;
            FunctionClass func;
            func.Clear();

            // 解析各种属性
            line = data.readLine();
            QString tags_str = "";
            while(line.left(9) != "function ")
            {
                if(line.left(8) == "-- tags:")
                {
                    tags_str = line.mid(8).remove(' ');
                }
                else if(!parseLuaLine(&func, line))
                {
                    ok = false;
                    break;
                }
                line = data.readLine();
            }
            // 检查解析出的function内容是否合法
            if(!ok || func.name_ui.isEmpty() || func.params.size() + func.texts.size() < 1)
            {
                info("function " + func.name_lua + "解析出错");
                continue;
            }

            // 解析 function 的 Lua 名称
            QString temp_str = line.mid(8).remove(' ');
            int pos = temp_str.indexOf('(');
            func.name_lua = temp_str.left(pos);

            // 检验参数个数
            temp_str = temp_str.mid(pos + 1).remove("flowController").remove(')');
            QStringList params_lua = temp_str.split(',', QString::SkipEmptyParts);
            int params_number = params_lua.size();
            if(params_number > 0 && func.params.size() < params_number)
            {
                info(func.name_lua + "的参数数量不正确");
                continue;
            }

            addFunctionInfo(func, true);
            if(tags_str != "") //最后解析tags
                parseTags(line, func.name_lua);
        }
    }
    file.close();
    return true;
}

// 这是开始解析function_info的标志，写在函数定义之前：
//-- This function will be parsed by LevelAIEditor --
//-- can_be_call 表示可以在call function动作中调用，不填的话就不行
//-- type: 返回值类型，不填表示没有返回值
//-- name: 函数名称（UI中显示的）
//-- desc: 函数描述以及参数，{{n}}可表示{{number}}，{{s}}可表示{{string}}。
//-- notes: 写点注释，给策划看的
bool FunctionInfo::parseLuaLine(FunctionClass *func, QString line)
{
    if(line.left(8) == "-- type:")
    {
        line = line.mid(8).remove(' ');
        if(line == "string" || line == "number" ||
           StructInfo::GetInstance()->CheckIsStruct(line) ||
           EnumInfo::GetInstance()->CheckVarTypeIsEnum(line))
        {
            func->values.append(line); //返回值类型目前只1个，以后可能多个
        }
        else if(line != "")
        {
            info(line + "不是合法的返回值类型");
            return false;
        }
    }
    else if(line.left(8) == "-- name:")
    {
        if(line[8] == ' ')
            func->name_ui = line.mid(9);
        else
            func->name_ui = line.mid(8);
    }
    else if(line.left(8) == "-- desc:")
    {
        if(line[8] == ' ')
            line = line.mid(9);
        else
            line = line.mid(8);
        line.replace("{{n}}", "{{number}}");
        line.replace("{{s}}", "{{string}}");
        parseTextsAndParams(func, line);
    }
    else if(line.left(9) == "-- notes:")
    {
        if(line[9] == ' ')
            func->note = line.mid(10);
        else
            func->note = line.mid(9);
    }
    else if(line.contains("can_be_call"))
    {
        func->can_be_call = true;
    }
    return true;
}

void FunctionInfo::parseTextsAndParams(FunctionClass *func, QString str)
{
    QStringList str_list_1 = str.split("{{", QString::SkipEmptyParts);
    if(str_list_1.size() == 1)
    {
        func->texts.append(str);
        func->param_is_before_text = false;
        return;
    }

    int i = 0;
    if(str.left(2) != "{{")
    {
        func->texts.append(str_list_1[0]);
        func->param_is_before_text = false;
        i = 1;
    }
    else
        func->param_is_before_text = true;

    QString endbkt = "}}";
    int n = str_list_1.size();
    while(i < n)
    {
        int pos = str_list_1[i].indexOf(endbkt);
        if(pos == -1)
        {
            info(str + QString("中的{{和}}没有一一配对！"));
        }
        else
        {
            func->params.append(str_list_1[i].left(pos));
            if(pos + 2 < str_list_1[i].size())
                func->texts.append(str_list_1[i].mid(pos + 2));
        }
        i++;
    }
}

void FunctionInfo::parseTags(QString str, QString func_name_lua)
{
    QRegExp rx("(,|，)");
    QStringList sl = str.split(rx, QString::SkipEmptyParts);
    int n = sl.size();

    for(int i = 0; i < n; i++)
    {
        sl[i].remove(' ');
        if(sl[i] == "")
            continue;
        if(!tagMap.contains(sl[i]))
        {
            QStringList v;
            v.clear();
            tagMap.insert(sl[i], v);
        }
        tagMap[sl[i]].append(func_name_lua);
    }
}

void FunctionInfo::addFunctionInfo(const FunctionClass &func, bool fugai)
{
    // 检查是否已经有同名的函数了
    FunctionClass* func_old = GetFunctionInfoByLuaName(func.name_lua);
    if(func_old != nullptr)
    {
        if(fugai)
            *func_old = func; //由于没有指针类型的成员变量，貌似不用重写一个深拷贝
        else
            return;
    }
    // 插到infoList中合适的位置
    else
    {
        int idx = findIdxInList(func.name_ui);
        infoList.insert(idx, func);
    }
}

// 这个算法写得有问题，还需要改。暂时用更简单的findIdxInList来替代
int FunctionInfo::findIdxInRange(int fst_i, int size, const QString& name_ui)
{
    if(fst_i >= size)
        return size;

    int idx = (size + fst_i) / 2;
    int res = PinYin::GetInstance()->Compare(name_ui, infoList[idx].name_ui);
    if(res == 1)
    {
        return findIdxInRange(idx + 1, size, name_ui);
    }
    else if(res == 2)
    {
        if(idx == 0)
            return 0;
        else
            return findIdxInRange(fst_i, idx - 1, name_ui);
    }
    else
        return idx;
}

int FunctionInfo::findIdxInList(const QString &name_ui)
{
    int n = infoList.size();
    for(int i = 0; i < n; i++)
    {
        int res = PinYin::GetInstance()->Compare(infoList[i].name_ui, name_ui);
        if(res == 1 || res == 0)
            return i;
    }
    return n;
}
