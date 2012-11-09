#include "../src/sqlite-logging.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>


static void test_create()
{
    LogContext *context = create_sqlite_context("test.sqlite");
    sqlite_log(context, "localhost", "2012-11-09 17:39:56", "1.3.6.1.2.1.1.3.0", "42");
    destroy_sqlite_context(context);
}

int main()
{
    test_create();
    return 0;
}
