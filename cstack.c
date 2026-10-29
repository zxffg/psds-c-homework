#include "cstack.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STACKS 20

// в стеке будет хранится узел (ссылка на предыдущий узел, size данных в байтах,
// сами данные)
struct node
{
    struct node* previous;
    unsigned int size;
    char data[];
};
typedef struct node s_node;

// структура slot: храним статус ячейки MAX_STACKS, ссылку на верхний узел и
// кол-во элементов в стеке
typedef struct
{
    int usage;          // если используется: 1, елси нет то 0
    s_node* top;        // указатель на верхний узел
    unsigned int count; // кол-во элементов в стеке
} slot;
static slot slots[MAX_STACKS];

// проверяем заняты ячейки или нет
hstack_t stack_new(void)
{
    for (int i = 0; i < MAX_STACKS; ++i)
    {
        if (slots[i].usage == 0)
        {
            // сбрасываем состояние для нового стека
            slots[i].usage = 1;
            slots[i].top = NULL;
            slots[i].count = 0;
            return i;
        }
    }
    return -1;
}

void stack_free(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack))
    {
        return;
    }

    s_node* node = slots[hstack].top;
    while (node != NULL)
    {
        s_node* last_node = node->previous; // запоминаем последний элемент
        free(node);
        node = last_node;
    }

    // сбрасываем состояние ячейки
    slots[hstack].top = NULL;
    slots[hstack].count = 0;
    slots[hstack].usage = 0;
}

void stack_free_all(void)
{
    for (int i = 0; i < MAX_STACKS; ++i)
    {
        stack_free(i);
    }
}

// проверяем что стек с таким номером уже есть
int stack_valid_handler(const hstack_t hstack)
{
    if (hstack < 0 || hstack >= MAX_STACKS)
    {
        return 0;
    }

    return slots[hstack].usage;
}

// смотрим размер стека
unsigned int stack_size(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack))
    {
        return 0;
    }
    return slots[hstack].count;
}

// получаем статус: создан стек или нет
static int push_internal(
    const hstack_t hstack, const void* data_in, const unsigned int size
)
{
    if (!stack_valid_handler(hstack) || data_in == NULL || size == 0)
    {
        return 0;
    }

    s_node* node = malloc(
        sizeof(s_node) + size
    ); // выделяем память под стек (размер узла + size)
    if (node == NULL)
    { // обязательная проверка, что память выделилась
        return 0;
    }

    node->size = size;                 // записываем размер данных в узел
    memcpy(node->data, data_in, size); // копируем данные
    node->previous =
        slots[hstack]
            .top; // пишем в узел ссылку на предыдущий узел (верхушка стека)
    slots[hstack].top = node; // новая верхушка
    slots[hstack].count += 1; // увеличиваем стек

    return 1;
}

void // добавляем элемент в стек
stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    push_internal(hstack, data_in, size);
}

unsigned int
stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    if (!stack_valid_handler(hstack) || data_out == NULL || size == 0)
    {
        return 0;
    }

    s_node* node =
        slots[hstack].top; // достаем элемент, затем проверяем, что все окей
    if (node == NULL || size < node->size /* маленький буфер */)
    {
        return 0;
    }

    unsigned int copied = node->size;     // запоминаем размер
    memcpy(data_out, node->data, copied); // копируем данные
    slots[hstack].top = node->previous;   // переназначаем верхуку
    slots[hstack].count -= 1;             // уменьшаем число стеков
    free(node);
    return copied;
}

unsigned stack_top_size(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack) || slots[hstack].top == NULL)
    {
        return 0;
    }

    return slots[hstack].top->size;
}

hstack_t stack_get_all(void)
{
    hstack_t result = stack_new(); // создаем новый стек
    if (result < 0)
    {
        return -1;
    }

    for (int i = 0; i < MAX_STACKS; ++i)
    {
        if (i != result && slots[i].usage == 1)
        { // если такой стек уже есть и это не мы
            if (push_internal(result, &i, sizeof(hstack_t)) == 0)
            {                       // кладем i в стек result
                stack_free(result); // не получилось, то откат
                return -1;
            }
        }
    }
    return result;
}
