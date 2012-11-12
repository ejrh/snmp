#include <stdlib.h>
#include <stdio.h>

#include <sqlite3.h>

#define CREATE_LOG_TABLE_SQL "CREATE TABLE IF NOT EXISTS snmp_log (host TEXT, timestamp TEXT, oid TEXT, value TEXT)"

#define INSERT_INTO_LOG_SQL "INSERT INTO snmp_log (host, timestamp, oid, value) VALUES (?, ?, ?, ?)"

typedef struct LogContext
{
    char *filename;
    sqlite3 *conn;
    sqlite3_stmt *insert_stmt;
} LogContext;

static void create_log_table(LogContext *context)
{
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(context->conn, CREATE_LOG_TABLE_SQL, sizeof(CREATE_LOG_TABLE_SQL), &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "%s", sqlite3_errmsg(context->conn));
        abort();
    }
    
    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        fprintf(stderr, "%s", sqlite3_errmsg(context->conn));
        abort();
    }
    
    sqlite3_finalize(stmt);
}

LogContext *create_sqlite_context(char *filename)
{
    LogContext *context = malloc(sizeof(LogContext));
    context->filename = filename;
    if (sqlite3_open(filename, &context->conn) != SQLITE_OK)
    {
        fprintf(stderr, "%s", sqlite3_errmsg(context->conn));
        sqlite3_close(context->conn);
        free(context);
        return NULL;
    }
    context->insert_stmt = NULL;
    
    create_log_table(context);
    
    return context;
}

void sqlite_log(LogContext *context, char *host, char *timestamp, char *oid, char *value)
{
    if (context->insert_stmt == NULL && sqlite3_prepare_v2(context->conn, INSERT_INTO_LOG_SQL, sizeof(INSERT_INTO_LOG_SQL), &context->insert_stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "%s", sqlite3_errmsg(context->conn));
        abort();
    }
    
    sqlite3_bind_text(context->insert_stmt, 1, host, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(context->insert_stmt, 2, timestamp, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(context->insert_stmt, 3, oid, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(context->insert_stmt, 4, value, -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(context->insert_stmt) != SQLITE_DONE)
    {
        fprintf(stderr, "%s", sqlite3_errmsg(context->conn));
        abort();
    }
    
    if (sqlite3_reset(context->insert_stmt) != SQLITE_OK)
    {
        fprintf(stderr, "%s", sqlite3_errmsg(context->conn));
        abort();
    }
    
    if (sqlite3_clear_bindings(context->insert_stmt) != SQLITE_OK)
    {
        fprintf(stderr, "%s", sqlite3_errmsg(context->conn));
        abort();
    }
}

void destroy_sqlite_context(LogContext *context)
{
    if (context->insert_stmt)
        sqlite3_finalize(context->insert_stmt);

    sqlite3_close(context->conn);
    free(context);
}
