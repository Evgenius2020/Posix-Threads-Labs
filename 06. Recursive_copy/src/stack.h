#ifndef STACK_VOID
#define STACK_VOID

typedef struct StackElement
{
    struct StackElement *next;
    void *data;
} StackElement;

typedef struct Stack
{
    StackElement *top;
} Stack;

// Returns empty stack.
Stack *stackCreate();
// Pop all elements of stack and 'free(stack)'.
void stackDestroy(Stack *stack);

// Creates new StackElement based in data, inserts it in stack.
void stackPush(Stack *stack, void *data);
// Returns top element's data and removing it from stack.
void *stackPop(Stack *stack);
// Returns top element's data.
void *stackPeek(Stack *stack);
// Returns 1 if (stack->top == NULL) if true and 0 if else.
char stackIsEmpty(Stack *stack);

#endif