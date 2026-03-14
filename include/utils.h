#ifndef UTILS_H
#define UTILS_H
#include <unistd.h>
#include <libudev.h>
#include <sys/select.h>
#include <signal.h>
#include <libnotify/notify.h>

static volatile sig_atomic_t running = 1;

void handle_signal(int sig);

char* read_sysfs_file(const char* path);

int send_notification(const char * title, const char * content, const char * icon_path, gint timeout, NotifyUrgency urgency);

#endif // UTILS_H
