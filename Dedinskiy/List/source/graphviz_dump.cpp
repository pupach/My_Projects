#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctime>
#include <sys/time.h>

#include "list.h"
#include "graphviz_dump.h"



CODE_ERRORS Graphiz_Dump(List *list)
{
    char *filename = Create_Filename_by_Time();

    FILE *stream_write = fopen(filename, "w");   //обернуть в define
    if(stream_write == nullptr)     return PTR_NULL;

    HADLER_EROR(Write_Graphiz_Dump_to_File(stream_write, list));

    fclose(stream_write);

    HADLER_EROR(Create_Png_And_Show(filename));

    free(filename);

    return ALL_GOOD;
}

................................|..@..$...ede.........

  @  $
ede


CODE_ERRORS Write_Graphiz_Dump_to_File(FILE *stream_write, List *list)
{

    fprintf(stream_write, "digraph\n{\n");
    fprintf(stream_write, "rankdir = LR;\n\n");

    fprintf(stream_write, "node [shape = Mrecord, color  = \"navy\", style = \"filled\"];\n");

    for (int i = 0; i <= list->capacity; i++)
    {
        const char *color = "#fff3e0";

        Elem_list cur_elem= list->arr[i];

        fprintf(stream_write,
                "node%d[label = \"%d | data = %d | next = %d | prev = %d\", fillcolor = \"%s\"];\n",
                i, i, cur_elem.val, cur_elem.next, cur_elem.prev,
                color);
    }
        fprintf(stream_write, "node%d [style = \"dashed\", label = \"idx = %d\"];\n",
            list->capacity + 1, list->capacity + 1);

    fprintf(stream_write, "free  [label = \"free = %d\", ", list->arr[0].val);
    fprintf(stream_write, "fillcolor = \"#33ff66\"];\n");

    fprintf(stream_write, "\nedge [color = \"orange\"];\n\n");
    Elem_list cur_elem= list->arr[0];

    fprintf(stream_write, "node%d -> node%d;\n", 0, cur_elem.val);

    fprintf(stream_write, "\nedge [color = \"cornFlowerBlue\"];\n\n");
    for (int i = 0; i <= list->capacity; i++)
    {
        Elem_list cur_elem= list->arr[i];

        fprintf(stream_write, "node%d -> node%d;\n", i, cur_elem.next);
    }

    fprintf(stream_write, "\nedge [color = \"salmon\"];\n\n");
    for (int i = 0; i <= list->capacity; i++)
    {
        Elem_list cur_elem= list->arr[i];

        fprintf(stream_write, "node%d -> node%d;\n", i, cur_elem.prev);
    }
    fprintf(stream_write, "}\n");

    return ALL_GOOD;
}
