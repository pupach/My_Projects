#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctime>
#include <sys/time.h>

#include "../../my_lib/work_with_file.h"

// TODO: reinclude defence

CODE_ERRORS Graphiz_Dump(List *list);
CODE_ERRORS Write_Graphiz_Dump_to_File(FILE *stream_write, List *list);
