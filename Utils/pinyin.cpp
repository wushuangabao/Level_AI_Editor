#include <QCoreApplication>
#include <QTextStream>
#include <QFile>
#include "../ItemModels/enumdefine.h"
#include "pinyin.h"

PinYin::~PinYin()
{

}

PinYin *PinYin::GetInstance()
{
    static PinYin* ins = nullptr;
    if(ins == nullptr)
        ins = new PinYin();
    return ins;
}

bool PinYin::IsZh(const QString &str)
{
    bool ok = true;
    int n = str.size();
    for(int i = 0; i < n; i++)
    {
        QChar ch = str.at(i);
        ushort code = ch.unicode();
        if(code < 0x4E00 || code > 0x9FA5)
        {
            ok = false;
            break;
        }
    }
    return ok;
}

QString PinYin::Zh2PY(const QString &zh_str)
{
    int n = zh_str.size();
    QString py_str = "";
    for(int i = 0; i < n; i++)
    {
        QString ch(zh_str.at(i));
        if(IsZh(ch) && m_map.contains(ch))
            py_str.push_back(m_map[ch]);
        else
            py_str.push_back(ch);
    }
    return py_str;
}

int PinYin::Compare(const QString &s1, const QString &s2)
{
    int n1 = s1.size();
    int n2 = s2.size();

    for(int i = 0; i < n1 && i < n2; i++)
    {
        bool b1 = PinYin::IsZh(s1.at(i));
        bool b2 = PinYin::IsZh(s2.at(i));
        if(b1 && !b2)
            return 1;
        else if(!b1 && b2)
            return 2;
        else if(b1 && b2)
        {
            QString temp_s1 = Zh2PY(s1.at(i));
            QString temp_s2 = Zh2PY(s2.at(i));
            if(temp_s1 > temp_s2)
                return 1;
            else if(temp_s1 < temp_s2)
                return 2;
        }
        else
        {
            ushort c1 = s1.at(i).unicode();
            ushort c2 = s2.at(i).unicode();
            if(c1 > c2)
                return 1;
            else if(c1 < c2)
                return 2;
        }
    }

    if(n1 > n2)
        return 1;
    else if(n1 < n2)
        return 2;

    return 0;
}

PinYin::PinYin()
{
    QString path = QCoreApplication::applicationFilePath();
    path.replace("LevelEditor.exe", "config/pinyin");
    if(!createDateByConfig(path))
        info("配置文件pinyin解析失败！");
}

bool PinYin::createDateByConfig(const QString &file_path)
{
    QFile file(file_path);
    if(!file.open(QIODevice::ReadOnly))
        return false;

    QTextStream data(&file);
    data.setCodec("UTF-8");

    while(!data.atEnd())
    {
        QString line = data.readLine();

        int pos = line.indexOf('=');
        if(pos == -1 || pos == line.size() - 1)
            continue;

        QString zh = line.left(pos);
        QString py = line.mid(pos + 1);
        pos = py.indexOf(',');
        if(pos != -1)
            py = py.left(pos);

        MY_ASSERT(!m_map.contains(zh));
        m_map.insert(zh, py);
    }

    file.close();
    return true;
}
