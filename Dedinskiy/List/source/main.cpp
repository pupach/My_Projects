#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "graphviz_dump.h"

#define LIST_DUMP(list)\
    List_Dump(list, __LINE__, __PRETTY_FUNCTION__)

#define STDOUT stderr


int main()
{
    List list = {};

    HADLER_EROR(List_init(&list));

    HADLER_EROR(List_add_Elem(&list, 10));
    HADLER_EROR(List_add_Elem(&list, 23, 1));

    HADLER_EROR(List_add_Elem(&list, 34, 1));

    HADLER_EROR(Graphiz_Dump(&list));

    HADLER_EROR(LIST_DUMP(&list));

    free(list.arr);
}
