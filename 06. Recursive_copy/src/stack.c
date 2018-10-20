#include <stdlib.h>
#include "stack.h"

Stack *stackCreate()
{
    Stack *stack = (Stack *)malloc(sizeof(Stack));
    stack->top = NULL;

    return stack;
}

void stackDestroy(Stack *stack)
{
    while (!stackIsEmpty(stack))
    {
        stackPop(stack);
    }

    free(stack);
}

void stackPush(Stack *stack, void *data)
{
    StackElement *element = (StackElement *)malloc(sizeof(StackElement));
    if (!element)
    {
        return;
    }
    element->data = data;

    if (stackIsEmpty(stack))
    {
        stack->top = element;
        element->next = NULL;
    }
    else
    {
        element->next = stack->top;
        stack->top = element;
    }
}

void *stackPop(Stack *stack)
{
    if (stackIsEmpty(stack))
    {
        return NULL;
    }

    StackElement *element = stack->top;
    stack->top = element->next;
    void *data = element->data;
    free(element);

    return data;
}

void *stackPeek(Stack *stack)
{
    if (stackIsEmpty(stack))
    {
        return NULL;
    }

    return stack->top->data;
}

char stackIsEmpty(Stack *stack)
{
    if (stack->top == NULL)
    {
        return 1;
    }

    return 0;
}