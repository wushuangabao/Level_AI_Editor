#ifndef VALUECLASS_H
#define VALUECLASS_H

#include <QString>
#include <QVector>
#include <QMap>

enum VALUE_TYPE
{
    // BaseValue:
    VT_VAR,  //用一个变量名来表示值，变量具有类型（VAR_TYPE），可以被赋为另一个相同VAR_TYPE的值
    VT_FUNC, //用一个函数（FunctionClass指针）来表示值
    VT_STR,  //用自定义的lua代码来表示值
    VT_ENUM, //在一系列预设的值中选择其中一个
    VT_PARAM,//这个值是事件触发时传入的参数

    // StructValue:
    VT_TABLE //用多个key=value组合成的表
};

class FunctionClass;

class CommonValueClass
{
public:
    CommonValueClass() {}
    virtual ~CommonValueClass() {}

    virtual VALUE_TYPE GetValueType() = 0; //变量类型，指值在逻辑层面表示哪一类东西，比如数值、点、棋子……
    virtual QString    GetVarType()   = 0; //值类型，指值的存储方式，是变量名、函数、lua代码还是其他

    // 用于编辑器中显示
    virtual QString GetText() {return "nil";}
    // 随变量修改而更新
    virtual bool UpdateVarNameAndType(int var_id, const QString& name, const QString &type, bool *need_update = nullptr) = 0;
    virtual bool IsUsingVar(const QString& vname) = 0;

    virtual QString GetLuaValueString(QString var_prefix) = 0;

    static bool AreSameVarType(CommonValueClass* v1, CommonValueClass* v2);
};

//
class BaseValueClass : public CommonValueClass
{
public:
    BaseValueClass();
    BaseValueClass(BaseValueClass* obj); //拷贝构造
    BaseValueClass(QString str);
    ~BaseValueClass();

    void ClearData();
    void operator=(const BaseValueClass& obj);

    virtual QString GetText();
    inline virtual VALUE_TYPE GetValueType() {return value_type;}
    inline virtual QString    GetVarType()   {return var_type;}

    void SetVarType(const QString& t);
    void SetVarName(const QString& name, QString var_type, int idx, const QString &key);
    void ModifyVarName(const QString& name);
    void SetLuaStr(const QString& lua_str, const QString &type_text = "");
    void SetEnumValue(const QString& value_str);
    bool SetEvtParam(int event_type_id, int param_id);

    void SetFunction(FunctionClass* function_class);
    void SetParamAt(int idx, CommonValueClass *v);

    QString GetFunctionName();
    int GetFunctionParamsNum();
    CommonValueClass *GetFunctionParamAt(int id);
    FunctionClass* GetFunctionInfo();

    int GetEventParamId();
    virtual QString GetLuaValueString(QString var_prefix);

    virtual bool UpdateVarNameAndType(int var_id, const QString& name, const QString &type, bool *need_update = nullptr); //函数的返回值表示类型是否出错，need_update表示数据是否有更新
    virtual bool IsUsingVar(const QString& vname);

    const static QString custom_name_prefix;
    const static int custom_name_prefix_len;

private:
    QString name;        //变量名
    FunctionClass* func; //函数：函数名、参数信息
    QString lua_str;     //自定义lua代码
    QString var_type;
    VALUE_TYPE value_type;

    QVector<CommonValueClass*> params; //函数参数
    void clearFuncParams();
    QString getFunctionText();

    bool checkLuaStrAndVarType(QString &str, const QString &var_t);
    bool checkVarType(const QString& type); //检查VT_VAR变量的变量类型是否为type

    int g_var_id;
};

class QJsonObject;
class StructValueClass : public CommonValueClass
{
public:
    StructValueClass();
    StructValueClass(StructValueClass* obj);
    ~StructValueClass();
    void operator=(const StructValueClass& obj);

    virtual QString GetLuaValueString(QString var_prefix);
    virtual QString GetText();
    inline virtual VALUE_TYPE GetValueType() {return VT_TABLE;}
    inline virtual QString GetVarType() {return var_type;}
    void SetVarType(QString name);

    virtual bool UpdateVarNameAndType(int var_id, const QString& name, const QString &type, bool *need_update = nullptr); //函数的返回值表示类型是否出错，need_update表示数据是否有更新
    virtual bool IsUsingVar(const QString& vname);

    CommonValueClass *GetValueByKey(QString name);
    void UpdateMember_Copy(QString key_name, CommonValueClass* value);
    void UpdateMember_Move(QString key_name, CommonValueClass* value);
    QStringList GetAllKeys();
    bool InitWithType(QString var_type);

private:
    void clearMembers();

    QMap<QString, CommonValueClass*> members;
    QString var_type;
};

#endif // VALUECLASS_H
