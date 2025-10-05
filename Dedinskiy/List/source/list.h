#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "../../my_lib/work_with_file.h"

static const int POISON_VAL_FOR_LIST = 99;

static const int BEG_CAPACITY_LIST = 10;

struct Elem_list
{
    int val = POISON_VAL_FOR_LIST;
    int next = -1;
    int prev = -1;
};


struct List
{
    Elem_list *arr = nullptr;
    int capacity = -1;
};

CODE_ERRORS List_Dump(List *list, int str, const char *func);

CODE_ERRORS Realloc_List(List *list, int new_capacity=-1);

CODE_ERRORS List_init(List *list, int beg_capacity=BEG_CAPACITY_LIST);

CODE_ERRORS List_add_Elem(List *list, int val_Elem, int ind_to_ins = 0);

CODE_ERRORS List_remove_Elem(List *list, int ind_to_ins);

int find_Elem_in_list(List *list, int ind_to_find);

