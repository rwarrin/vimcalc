#ifndef CALC_EXPRESSION_H

enum calc_node_type
{
    CalcNode_UnaryMinus = 0,

    CalcNode_Add,
    CalcNode_Subtract,
    CalcNode_Multiply,
    CalcNode_Divide,
    CalcNode_Modulus,

    CalcNode_BitwiseAnd,
    CalcNode_BitwiseOr,
    CalcNode_BitwiseXor,
    CalcNode_BitwiseNot,
    CalcNode_BitwiseLeftShift,
    CalcNode_BitwiseRightShift,

    CalcNode_Constant,
    CalcNode_Variable,
};
struct calc_node
{
    union
    {
        r64 R64Value;
        struct variable_table_node *Variable;
    };
    struct calc_node *Left;
    struct calc_node *Right;
    calc_node_type Type;
};

static void FreeNode(struct calc_node *Node);

#define CALC_EXPRESSION_H
#endif
