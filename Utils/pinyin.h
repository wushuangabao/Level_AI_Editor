#ifndef PINYIN_H
#define PINYIN_H

#include <QMap>

class PinYin
{
public:
    ~PinYin();
    static PinYin* GetInstance();

    // str是否为中文
    static bool IsZh(const QString& str);

    // 汉字转为拼音
    QString Zh2PY(const QString& zh_str);

    // 比较两个字符串的顺序
    int Compare(const QString& s1, const QString& s2);

private:
    PinYin();
    bool createDateByConfig(const QString &file_path);

    QMap<QString, QString> m_map;
};

#endif // PINYIN_H
