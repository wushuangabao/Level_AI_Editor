#ifndef ENUMDEFINE_H
#define ENUMDEFINE_H

#include <QMessageBox>

__attribute__((unused)) static void info(QString info)
{
    QMessageBox::information(nullptr, "提示", info);
}

enum NODE_TYPE
{
    INVALID = -1,

    /////////////////////////////////
    /// 事件
    /////////////////////////////////

    EVENT, // 由一个（事件）类型枚举、一个condition和一个动作类型的节点组成
    ETYPE, // 类型枚举，含一个整数（values[0]）

    /////////////////////////////////
    /// 条件
    /////////////////////////////////

    CONDITION, // 有and、or两种状态。子节点必须为条件节点，至少要有一个
    COMPARE, // 有且仅有2个子节点（左值和右值），有=、<、>等状态。父节点必须为condition节点

    /////////////////////////////////
    /// 动作
    /////////////////////////////////

    SET_VAR, // 创建临时变量，由变量名和一个value组合而成

    FUNCTION, // 有返回值的自定义函数，由一个类型枚举、一堆参数组合而成
    ACTION, // 无返回值的自定义函数，由一个类型枚举、一堆参数组合而成（目前废弃）

    // 流程（控制）节点
    SEQUENCE, // 序列，依次执行一堆动作节点
    CHOICE, // 选择，由一个condition节点和两个sequence节点组合而成
    LOOP, // 循环，依次执行一个序列中的动作节点，重复一定次数
    END, // 跳出当前序列或循环

    // 开启、关闭事件
    OPEN_EVENT,
    CLOSE_EVENT,

    /////////////////////////////////
    MAX
};

enum CONDITION_OP
{
    INVALID_CONDITION = -1,
    AND,
    OR,
    EQUAL_TO,
    GREATER_THAN,
    LESS_THAN,
    EQUAL_GREATER,
    EQUAL_LESS
};

__attribute__((unused)) static QString getNodeTypeStr(NODE_TYPE v)
{
    switch (v) {
    case EVENT:
        return "事件";
    case ETYPE:
        return "事件类型";
    case CONDITION:
        return "条件（组合）";
    case COMPARE:
        return "条件（比较）";
    case SET_VAR:
        return "SET_VAR";
    case SEQUENCE:
        return "SEQUENCE";
    case CHOICE:
        return "IF";
    case LOOP:
        return "LOOP";
    case END:
        return "BREAK";
    case FUNCTION:
        return "CALL";
    case ACTION:
        return "CALL";
    case OPEN_EVENT:
        return "OPEN_EVENT";
    case CLOSE_EVENT:
        return "CLOSE_EVENT";
    default:
        return "ERROR";
    }
}

__attribute__((unused)) static NODE_TYPE getNodeTypeEnum(QString str)
{
    if(str == "SET_VAR")
        return SET_VAR;
    else if(str == "CALL")
        return FUNCTION;
    else if(str == "IF")
        return CHOICE;
    else if(str == "LOOP")
        return LOOP;
    else if(str == "BREAK")
        return END;
    else if(str == "OPEN_EVENT")
        return OPEN_EVENT;
    else if(str == "CLOSE_EVENT")
        return CLOSE_EVENT;
    else return INVALID;
}

__attribute__((unused)) static QString getConditionStr(CONDITION_OP v)
{
    QString s;

    switch (v) {
    case INVALID_CONDITION:
        s = "INVALID";
        break;
    case AND:
        s = "AND";
        break;
    case OR:
        s = "OR";
        break;
    case EQUAL_TO:
        s = "==";
        break;
    case GREATER_THAN:
        s = ">";
        break;
    case LESS_THAN:
        s = "<";
        break;
    case EQUAL_GREATER:
        s = ">=";
        break;
    case EQUAL_LESS:
        s = "<=";
        break;
    default:
        s = "ERROR";
        break;
    }

    return s;
}

__attribute__((unused)) static CONDITION_OP getConditionEnum(QString s)
{
    if(s == "AND")
        return AND;
    else if(s == "OR")
        return OR;
    else if(s == "==")
        return EQUAL_TO;
    else if(s == ">")
        return GREATER_THAN;
    else if(s == "<")
        return LESS_THAN;
    else if(s == ">=")
        return EQUAL_GREATER;
    else if(s == "<=")
        return EQUAL_LESS;
    else
        return INVALID_CONDITION;
}

#endif // ENUMDEFINE_H
