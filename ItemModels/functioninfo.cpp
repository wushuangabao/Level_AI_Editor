#include <QDir>
#include <QCoreApplication>
#include <QTextStream>
#include <QDebug>
#include "enumdefine.h"
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

FunctionClass *FunctionInfo::GetFunctionInfoByID(FUNCTION_ID id)
{
    int n = infoList.size();
    for(int i = 0; i < n; i++)
    {
        if(infoList[i].GetID() == id)
            return &(infoList[i]);
    }
    return nullptr;
}

FunctionClass *FunctionInfo::GetFunctionInfoAt(int idx)
{
    int n = infoList.size();
    Q_ASSERT(n > idx && idx >= 0);

    return &(infoList[idx]);
}

FunctionInfo::FunctionInfo()
{
    ClearData();
    InitFuncInfo();
}

void FunctionInfo::InitFuncInfo()
{
    QString config_path = QCoreApplication::applicationFilePath();
    config_path.replace("LevelEditor.exe", "config");

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

        QStringList sl = line.split('\t'/*, QString::SkipEmptyParts*/);
        int n = sl.size();
        Q_ASSERT(n == 6);
        FunctionClass func;

        // 函数ID
        bool ok;
        func.id = sl[0].toInt(&ok);
        if(!ok) continue;

        // 1=场景AI专属，0=棋子AI专属
        int i = sl[1].toInt(&ok);
        if(!ok || i == 0) continue;

        // 函数名称
        func.name = sl[2];

        // 描述
        parseTextsAndParams(&func, sl[3]);

        // 返回值类型
        if(sl[4] != "0")
        {
            if(sl[4] == "1")
                func.values.append("number");
            else
                func.values = sl[4].split(',');
        }

        // 注释
        func.note = sl[5];

        infoList << func;
    }

    file.close();
    return true;
}

void FunctionInfo::parseTextsAndParams(FunctionClass *func, QString str)
{
    QStringList str_list_1 = str.split("{{");
    if(str_list_1.size() == 1)
    {
        func->texts.append(str);
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
            qDebug() << str << "中的{{和}}没有一一配对！" << endl;
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
