#include <stdlib.h>
#include <stdio.h>

#include <sqlite3.h>

#define CREATE_LOG_TABLE_SQL "CREATE TABLE IF NOT EXISTS snmp_log (host TEXT, timestamp TEXT, oid TEXT, value TEXT)"

#define INSERT_INTO_LOG_SQL "INSERT INTO snmp_log (host, timestamp, oid, value) VALUES (?, ?, ?, ?)"

typedef struct LogContext
{
    char *filename;
    sqlite3 *conn;
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
    
    create_log_table(context);
    
    return context;
}

void sqlite_log(LogContext *context, char *host, char *timestamp, char *oid, char *value)
{
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(context->conn, INSERT_INTO_LOG_SQL, sizeof(INSERT_INTO_LOG_SQL), &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "%s", sqlite3_errmsg(context->conn));
        abort();
    }
    
    sqlite3_bind_text(stmt, 1, host, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, timestamp, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, oid, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, value, -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        fprintf(stderr, "%s", sqlite3_errmsg(context->conn));
        abort();
    }
    
    sqlite3_finalize(stmt);
}

void destroy_sqlite_context(LogContext *context)
{
    sqlite3_close(context->conn);
    free(context);
}
