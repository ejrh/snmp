#ifndef SQLITE_H
#define SQLITE_H

typedef struct LogContext LogContext;

LogContext *create_sqlite_context(char *filename);
void sqlite_log(LogContext *context, char *host, char *timestamp, char *oid, char *value);
void destroy_sqlite_context(LogContext *context);

#endif
