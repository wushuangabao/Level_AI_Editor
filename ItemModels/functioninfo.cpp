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
        info("????????? config ?????????");
    }
    else
    {
        bool success = createDateByConfig(config_path + "/function_info.txt");
        if(!success)
        {
            ClearData();
            info("????????? config/function_info.txt ???");
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
        if(n < 7)
        {
            info(line + "????????????");
            continue;
        }

        // ???????????????????????????????????????????????????????????????????????????
        if(sl[2] != "" && GetFunctionInfoByLuaName(sl[2]) != nullptr)
            continue;
        FunctionClass func;

        // ????????????
        func.name_lua = sl[2];
        func.name_ui = sl[3];

        // 1=??????AI?????????0=??????AI??????
        bool ok;
        int i = sl[1].toInt(&ok);
        if(!ok || i == 0) continue;

        // ??????
        parseTextsAndParams(&func, sl[4]);

        // ???????????????
        if(sl[5] != "0")
        {
            if(sl[5] == "1")
                func.values.append("number");
            else
                func.values = sl[5].split(',');
        }

        // ?????????Function???????????????????????????
        if(sl[6] == "0")
            func.can_be_call = false;
        else
            func.can_be_call = true;

        // ??????
        if(n >= 8)
            func.note = sl[7];
        else
            func.note = "";

        // ??????
        if(n >= 9 && func.name_lua != "")
            parseTags(sl[8], func.name_lua);

        addFunctionInfo(func, false);
    }

    file.close();
    return true;
}

bool FunctionInfo::createDataByLuaFile(const QString &lua_path)
{
    QString abs_path = config_path + lua_path;
    QFile file(abs_path.remove('\t'));
    if(!file.open(QIODevice::ReadOnly))
    {
        info(lua_path + "????????????");
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

            // ??????????????????
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
            // ??????????????????function??????????????????
            if(!ok || func.name_ui.isEmpty() || func.params.size() + func.texts.size() < 1)
            {
                info("function " + func.name_lua + "????????????");
                continue;
            }

            // ?????? function ??? Lua ??????
            QString temp_str = line.mid(8).remove(' ');
            int pos = temp_str.indexOf('(');
            func.name_lua = temp_str.left(pos);

            // ??????????????????
            temp_str = temp_str.mid(pos + 1).remove("flowController").remove(')');
            QStringList params_lua = temp_str.split(',', QString::SkipEmptyParts);
            int params_number = params_lua.size();
            if(params_number > 0 && func.params.size() < params_number)
            {
                info(func.name_lua + "????????????????????????");
                continue;
            }

            addFunctionInfo(func, true);
            if(tags_str != "") //????????????tags
                parseTags(line, func.name_lua);
        }
    }
    file.close();
    return true;
}

// ??????????????????function_info???????????????????????????????????????
//-- This function will be parsed by LevelAIEditor --
//-- can_be_call ???????????????call function???????????????????????????????????????
//-- type: ?????????????????????????????????????????????
//-- name: ???????????????UI???????????????
//-- desc: ???????????????????????????{{n}}?????????{{number}}???{{s}}?????????{{string}}???
//-- notes: ??????????????????????????????
bool FunctionInfo::parseLuaLine(FunctionClass *func, QString line)
{
    if(line.left(8) == "-- type:")
    {
        line = line.mid(8).remove(' ');
        if(line == "string" || line == "number" ||
           StructInfo::GetInstance()->CheckIsStruct(line) ||
           EnumInfo::GetInstance()->CheckVarTypeIsEnum(line))
        {
            func->values.append(line); //????????????????????????1????????????????????????
        }
        else if(line != "")
        {
            info(line + "??????????????????????????????");
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
            info(str + QString("??????{{???}}?????????????????????"));
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
    QRegExp rx("(,|???)");
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
    // ???????????????????????????????????????
    FunctionClass* func_old = GetFunctionInfoByLuaName(func.name_lua);
    if(func.name_lua != "" && func_old != nullptr)
    {
        if(fugai)
            *func_old = func; //???????????????????????????????????????????????????????????????????????????
        else
            return;
    }
    // ??????infoList??????????????????
    else
    {
        int idx = findIdxInList(func.name_ui);
        infoList.insert(idx, func);
    }
}

// ??????????????????????????????????????????????????????????????????findIdxInList?????????
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
