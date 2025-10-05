#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
/*
void *CALLOC_AND_CHECK(size_t size_elem, int amount)
{
    void *ptr = calloc(size_elem, amount);
    if(ptr != nullptr)
    {
        return ptr;
    }
    else
    {
        fprintf("");
    }
}*/


CODE_ERRORS List_init(List *list, int beg_capacity)
{
    assert(list != nullptr);

    list->capacity = beg_capacity;

    list->arr = (Elem_list *)calloc(sizeof(Elem_list), list->capacity + 1);
    if(list->arr == nullptr)        return PTR_NULL;

    list->arr[0].val = 1;
    list->arr[0].next = 0;
    list->arr[0].prev = 0;
    for(int i = 1; i < list->capacity + 1; i++)
    {
       list->arr[i].prev = i - 1;
       list->arr[i].next = i + 1;
    }
    return ALL_GOOD;
}

CODE_ERRORS Realloc_List(List *list, int new_capacity)
{
    assert(list != nullptr);

    if(new_capacity == -1)  new_capacity = list->capacity * 2;

    list->arr = (Elem_list *)realloc(list->arr, (list->capacity + 1)* sizeof(Elem_list));
    if(list->arr == nullptr)        return PTR_NULL;//TODO assign realloc value to variable first

    return ALL_GOOD;
}

CODE_ERRORS List_add_Elem_on_O1(List *list, int val_Elem, int ind_to_ins)
{
    assert(list != nullptr);

    if(list->arr[0].val == list->capacity)
    {
        Realloc_List(list);
    }
    Elem_list *free_elem = list->arr + list->arr[0].val;
    int next_free_Elem = free_elem->next;

    free_elem->val = val_Elem;
    free_elem->prev = list->arr[ind_to_ins].prev;
    free_elem->next = ind_to_ins;

    list->arr[ind_to_ins].prev = list->arr[0].val;
    list->arr[free_elem->prev].next = list->arr[0].val;

    list->arr[0].val = next_free_Elem;

    list->arr[next_free_Elem].prev = list->arr[0].prev;
    return ALL_GOOD;
}


CODE_ERRORS List_add_Elem(List *list, int val_Elem, int ind_to_ins)
{
    assert(list != nullptr);

    int ind_cur_Elem = find_Elem_in_list(list, ind_to_ins);
    if(ind_cur_Elem == -1)
    {
        return OUT_BORDER;
    }
    return List_add_Elem_on_O1(list, val_Elem, ind_cur_Elem);
}

CODE_ERRORS List_remove_Elem_on_O1(List *list, int ind_to_ins)
{
    assert(list != nullptr);

    list->arr[list->arr[ind_to_ins].prev].next = list->arr[ind_to_ins].next;
    list->arr[list->arr[ind_to_ins].next].prev = list->arr[ind_to_ins].prev;

    list->arr[next_free_Elem].prev = list->arr[0].prev;
    list->arr[next_free_Elem].next = list->arr[0].val;

    list->arr[list->arr[0].val].prev = ind_to_ins;

    list->arr[0].val = next_free_Elem;

    return ALL_GOOD;
}

CODE_ERRORS List_remove_Elem(List *list, int ind_to_ins)
{
    assert(list != nullptr);

    int ind_cur_Elem = find_Elem_in_list(list, ind_to_ins);

    if(ind_cur_Elem == -1)
    {
        return OUT_BORDER;
    }
    return List_remove_Elem_on_O1(list, ind_cur_Elem);
}

int find_Elem_in_list(List *list, int ind_to_find)
{
    assert(list != nullptr);

    if (ind_to_find >= list->capacity)
    {
        return -1;
    }
    int cur_ind = 0;
    int cur_Elem = -1;
    while(cur_Elem != ind_to_find)
    {
        cur_ind = list->arr[cur_ind].next;
        cur_Elem += 1;
    }
    return cur_ind;
}

CODE_ERRORS List_Dump(List *list, int str, const char *func)
{
    assert(list != nullptr);

    fprintf(stderr, "List_Dump called in func %s,str %d\n", func, str);

    fprintf(stderr, "_________________________________________________________\n");

    for(int i = 0; i < list->capacity; i++ )
    {
        fprintf(stderr, "   %d    |", i);
    }

    fprintf(stderr, "\n_________________________________________________________\n");
    int ind_cur_elem = 0;

    do
    {
        fprintf(stderr, "   %d    |", list->arr[ind_cur_elem]);

        ind_cur_elem++;
    }
    while(ind_cur_elem != (list->capacity - 1));

    fprintf(stderr, "\n_________________________________________________________\n");

    ind_cur_elem = 0;
    do
    {
        fprintf(stderr, "   %d    |", list->arr[(list->arr[ind_cur_elem].next)]);

        ind_cur_elem++;
    }
    while(ind_cur_elem != (list->capacity - 1));


    fprintf(stderr, "\n_________________________________________________________\n");

    ind_cur_elem = 0;
    do
    {
        fprintf(stderr, "   %d    |", list->arr[(list->arr[ind_cur_elem].prev)]);

        ind_cur_elem++;
    }
    while(ind_cur_elem != (list->capacity - 1));

    fprintf(stderr, "\n_________________________________________________________\n");

}


//что делать с len_arr_v2
