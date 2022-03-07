#include <QDebug>
#include "../ItemModels/enumdefine.h"
#include "../ItemModels/functioninfo.h"
#include "valueclass.h"

BaseValueClass::BaseValueClass()
{
    params = QVector<BaseValueClass*>();
    ClearData();
//    qDebug() << "Create value 1 at" << (uintptr_t)this << endl;
}

BaseValueClass::BaseValueClass(BaseValueClass *obj)
{
    name = obj->name;
    lua_str = obj->lua_str;
    value_type = obj->value_type;
    func = obj->func;
    var_type = obj->var_type;
    g_var_id = obj->g_var_id;

    int n = obj->params.size();
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
        {
            params.append(new BaseValueClass(obj->params[i]));
        }
    }

//    qDebug() << "Create value copy(" << (uintptr_t)obj << ") at" << (uintptr_t)this << endl;
}

BaseValueClass::BaseValueClass(QString str)
{
    params = QVector<BaseValueClass*>();
    ClearData();

    this->lua_str = str;
    value_type = VT_STR;

//    qDebug() << "Create value named \'" << name <<"\' at" << (uintptr_t)this << endl;
}

BaseValueClass::~BaseValueClass()
{
//    qDebug() << "Delete value at" << (uintptr_t)this << endl;
    clearFuncParams();
}

void BaseValueClass::ClearData()
{
    value_type = VT_STR;
    func = nullptr;
    name = "未定义值";
    lua_str = "0";
    var_type = "number";
    g_var_id = -1;
    clearFuncParams();
}

void BaseValueClass::operator=(const BaseValueClass &obj)
{
    if(this == &obj)
        return;

    name = obj.name;
    lua_str = obj.lua_str;
    value_type = obj.value_type;
    func = obj.func;
    var_type = obj.var_type;
    g_var_id = obj.g_var_id;

    clearFuncParams();
    int n = obj.params.size();
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
        {
            params.append(new BaseValueClass(obj.params[i]));
        }
    }
}

QString BaseValueClass::GetText()
{
    if(value_type == VT_VAR)
        return name;
    else if(value_type == VT_STR || value_type == VT_ENUM || value_type == VT_PARAM)
        return lua_str;
    else if(value_type == VT_FUNC)
        return getFunctionText();
    else
        return "ERROR";
}

VALUE_TYPE BaseValueClass::GetValueType()
{
    return value_type;
}

QString BaseValueClass::GetVarType()
{
    return var_type;
}

void BaseValueClass::SetVarType(const QString &t)
{
    var_type = t;
}

void BaseValueClass::SetVarName(const QString &text, QString type, int idx)
{
    value_type = VT_VAR;
    name = text;
    var_type = type;
    if(idx != -1)
        g_var_id = idx;
    else
        info("未设置变量id！");
}

void BaseValueClass::SetLuaStr(const QString &text)
{
    value_type = VT_STR;
    lua_str = text;
}

void BaseValueClass::SetEnumValue(const QString &value_str)
{
    value_type = VT_ENUM;
    lua_str = value_str;
}

void BaseValueClass::SetEvtParam(const QString &param_str)
{
    value_type = VT_PARAM;
    lua_str = param_str;
}

void BaseValueClass::SetFunction(FunctionClass *function_class)
{
    value_type = VT_FUNC;
    func = function_class;

    clearFuncParams();
    int n = func->GetParamNum();
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
        {
            params.append(new BaseValueClass("0"));
        }
    }
}

void BaseValueClass::SetParamAt(int idx, BaseValueClass *v)
{
    Q_ASSERT(params.size() > idx);

    qDebug() << "set value(" << (uintptr_t)this << ")'s param(" << (uintptr_t)(params[idx]) <<") = " << (uintptr_t)v << endl;

    *(params[idx]) = *v;
}

QString BaseValueClass::GetFunctionName()
{
    Q_ASSERT(func != nullptr);

    return func->GetName();
}

int BaseValueClass::GetFunctionParamsNum()
{
    Q_ASSERT(func != nullptr);

    return func->GetParamNum();
}

BaseValueClass *BaseValueClass::GetFunctionParamAt(int id)
{
    int n = params.size();
    Q_ASSERT(n == func->GetParamNum());
    Q_ASSERT(id >= 0);
    Q_ASSERT(n > id);

    return params[id];
}

FunctionClass *BaseValueClass::GetFunctionInfo()
{
    return func;
}

void BaseValueClass::UpdateVarName(int var_id, const QString &name)
{
    if(value_type == VT_VAR)
    {
        if(g_var_id == var_id)
            this->name = name;
    }
    else if(value_type == VT_FUNC && params.size() > 0)
    {
        int n = params.size();
        for(int i = 0; i < n; i++)
        {
            params[i]->UpdateVarName(var_id, name);
        }
    }
    else if(value_type == VT_STR)
    {
        if(lua_str.contains(name))
        {
            info("注意修改自定义值：" + lua_str);
        }
    }
}

void BaseValueClass::clearFuncParams()
{
    int n = params.size();
    if(n <= 0)
        return;

    qDeleteAll(params);
    params.clear();
}

QString BaseValueClass::getFunctionText()
{
    Q_ASSERT(func != nullptr);

    int func_param_num = func->GetParamNum();
    int actually_p_num = params.size();
    Q_ASSERT(func_param_num <= actually_p_num);

    QString text = "";
    int pos = 0;
    int text_num = func->GetTextNum();

    if(func->param_is_before_text)
    {
        Q_ASSERT(func_param_num > 0);
        text = "(" + params[0]->GetText() + ")";
        pos++;
    }

    for(int i = 0; i < text_num; i++)
    {
        text = text + func->GetTextAt(i);
        if(pos < func_param_num)
        {
            text = text + "(" + params[pos]->GetText() + ")";
            pos++;
        }
    }

    return text;
}
