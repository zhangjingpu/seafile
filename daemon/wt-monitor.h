#ifndef SEAF_WT_MONITOR_H
#define SEAF_WT_MONITOR_H

#include <pthread.h>
#include <glib.h>

enum {
    WT_EVENT_CREATE_OR_UPDATE = 0,
    WT_EVENT_DELETE,
    WT_EVENT_RENAME,
    WT_EVENT_ADD_RECURSIVE,    /* need to scan down from the path */
    WT_EVENT_OVERFLOW,
};

typedef struct WTEvent {
    int ev_type;
    char *path;
    char *new_path;             /* only used by rename event */
} WTEvent;

void wt_event_free (WTEvent *event);

typedef struct WTStatus {
    char        repo_id[37];
    gint        last_check;
    gint        last_changed;

    pthread_mutex_t q_lock;
    GQueue *event_q;
} WTStatus;

typedef struct SeafWTMonitorPriv SeafWTMonitorPriv;

struct _SeafileSession;

typedef struct SeafWTMonitor {
    struct _SeafileSession      *seaf;
    SeafWTMonitorPriv   *priv;
} SeafWTMonitor;

SeafWTMonitor *
seaf_wt_monitor_new (struct _SeafileSession *seaf);

int
seaf_wt_monitor_start (SeafWTMonitor *monitor);

int
seaf_wt_monitor_watch_repo (SeafWTMonitor *monitor, const char *repo_id);

int
seaf_wt_monitor_unwatch_repo (SeafWTMonitor *monitor, const char *repo_id);

int
seaf_wt_monitor_refresh_repo (SeafWTMonitor *monitor, const char *repo_id);

WTStatus *
seaf_wt_monitor_get_worktree_status (SeafWTMonitor *monitor,
                                     const char *repo_id);

#endif
