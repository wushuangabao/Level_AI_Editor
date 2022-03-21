#include <QDebug>
#include "../ItemModels/enumdefine.h"
#include "../ItemModels/functioninfo.h"
#include "../Values/enuminfo.h"
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

    if(str != "")
    {
        this->lua_str = str;
        value_type = VT_STR;
    }

//    qDebug() << "Create value named \'" << name <<"\' at" << (uintptr_t)this << endl;
}

BaseValueClass::~BaseValueClass()
{
//    qDebug() << "Delete value at" << (uintptr_t)this << endl;
    if(this != nullptr)
        clearFuncParams();
}

void BaseValueClass::ClearData()
{
    value_type = VT_STR;
    func = nullptr;
    name = "未定义值";
    lua_str = "nil";
    var_type = "";
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
    if(value_type == VT_VAR || value_type == VT_PARAM)
        return name;
    else if(value_type == VT_STR || value_type == VT_ENUM)
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
    var_type = ""; //表示无法确定的值类型
}

void BaseValueClass::SetEnumValue(const QString &value_str)
{
    value_type = VT_ENUM;
    lua_str = value_str;

    // 检查一下变量类型是否和枚举值匹配
    bool ok = true;
    if(EnumInfo::GetInstance()->GetAllTypes().contains(var_type) == false)
    {
        info("变量类型错误！不存在" + var_type + "这个枚举类型");
        ok = false;
    }
    else if(EnumInfo::GetInstance()->GetEnumsOfType(var_type).indexOf(lua_str) == -1)
    {
        info("变量类型错误！" + var_type + "不存在" + lua_str + "这个值");
        ok = false;
    }
    // 强行设置 var_type (todo)
    if(!ok)
    {

    }
}

void BaseValueClass::SetEvtParam(const QString &lua_str, const QString &ui_str, QString var_type)
{
    value_type = VT_PARAM;
    name = ui_str;
    this->lua_str = lua_str;
    this->var_type = var_type;
}

void BaseValueClass::SetFunction(FunctionClass *function_class)
{
    value_type = VT_FUNC;
    func = function_class;
    var_type = func->GetReturnTypeAt(0);

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
    MY_ASSERT(params.size() > idx);

    qDebug() << "set value(" << (uintptr_t)this << ")'s param(" << (uintptr_t)(params[idx]) <<") = " << (uintptr_t)v << endl;

    *(params[idx]) = *v;
}

QString BaseValueClass::GetFunctionName()
{
    MY_ASSERT(func != nullptr);

    return func->GetNameLua();
}

int BaseValueClass::GetFunctionParamsNum()
{
    MY_ASSERT(func != nullptr);

    return func->GetParamNum();
}

BaseValueClass *BaseValueClass::GetFunctionParamAt(int id)
{
    int n = params.size();
    MY_ASSERT(n == func->GetParamNum());
    MY_ASSERT(id >= 0);
    MY_ASSERT(n > id);

    return params[id];
}

FunctionClass *BaseValueClass::GetFunctionInfo()
{
    return func;
}

QString BaseValueClass::GetEventParamInLua()
{
    if(value_type != VT_PARAM)
        return "";
    return lua_str;
}

bool BaseValueClass::UpdateVarNameAndType(int var_id, const QString &name, const QString &type)
{
    if(value_type == VT_VAR)
    {
        if(g_var_id == var_id)
        {
            this->name = name;
            if(var_type != "" && var_type != type)
                return false;
        }
    }
    else if(value_type == VT_FUNC && params.size() > 0)
    {
        int n = params.size();
        bool ok = true;
        for(int i = 0; i < n; i++)
        {
            if(!params[i]->UpdateVarNameAndType(var_id, name, type))
                ok = false;
        }
        if(!ok)
            info("函数" + GetText() + "的参数类型可能有错误");
        if(var_type != "" && var_type != type)
            return false;
    }
    else if(value_type == VT_STR)
    {
        if(lua_str.contains(name))
        {
            info("注意修改自定义值：" + lua_str);
        }
        if(var_type != "" && var_type != type)
            return false;
    }
    return true;
}

bool BaseValueClass::IsUsingVar(const QString &vname)
{
    switch (value_type) {
    case VT_VAR:
        if(name == vname)
        {
//            info("注意检查：" + GetText());
            return true;
        }
        break;
    case VT_STR:
        if(lua_str.contains(vname))
            info("注意检查：" + lua_str);
        break;
    case VT_FUNC:
        for(int i = 0; i < GetFunctionParamsNum(); i++)
        {
            if(GetFunctionParamAt(i)->IsUsingVar(vname))
            {
//                info("注意检查：" + GetText());
                return true;
            }
        }
        break;
    default:
        break;
    }

    return false;
}

void BaseValueClass::clearFuncParams()
{
    int n = params.size();
    if(n <= 0)
        return;

    for(int i = 0; i < n; i++)
    {
        delete params[i];
        params[i] = nullptr;
    }
    params.clear();
}

QString BaseValueClass::getFunctionText()
{
    MY_ASSERT(func != nullptr);

    int func_param_num = func->GetParamNum();
    int actually_p_num = params.size();
    MY_ASSERT(func_param_num <= actually_p_num);

    QString text = "";
    int pos = 0;
    int text_num = func->GetTextNum();

    if(func->param_is_before_text)
    {
        MY_ASSERT(func_param_num > 0);
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
