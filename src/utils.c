#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libudev.h>
#include <sys/select.h>
#include <libnotify/notify.h>
#include "../include/utils.h"

#define APP_NAME "battery-notifier"  // 通知的应用标识（自定义）

volatile sig_atomic_t running = 1;

// 信号处理函数（优雅退出）
void handle_signal(int sig) {
    (void)sig;
    running = 0;
}
// 读取 sysfs 文件内容（返回字符串，需调用者 free）
char* read_sysfs_file(const char* path) {
    FILE* fp = fopen(path, "r");
    if (!fp) {
        perror("Failed to open sysfs file");
        return NULL;
    }

    char* buffer = malloc(64);
    if (!buffer) {
        perror("Failed to allocate memory");
        fclose(fp);
        return NULL;
    }

    if (fgets(buffer, 64, fp) == NULL) {
        free(buffer);
        buffer = NULL;
    } else {
        // 去除换行符
        buffer[strcspn(buffer, "\n")] = '\0';
    }

    fclose(fp);
    return buffer;
}

int send_notification(const char * title, const char * content, const char * icon_path, gint timeout, NotifyUrgency urgency) {
    // 1. 初始化 libnotify 库（必须第一步）
    // 参数：应用名称，会关联到通知的归属
    if (!notify_init(APP_NAME)) {
        fprintf(stderr, "Failed to initialize libnotify\n");
        return EXIT_FAILURE;
    }

    // 2. 创建通知对象
    // 参数：标题、内容、图标（NULL 表示无图标）
    NotifyNotification *notification = notify_notification_new(
        title,             // 通知标题（等效 notify-send 的第一个参数）
        content,           // 通知内容（第二个参数）
        icon_path          // 图标路径（如 "dialog-information" 系统图标，或绝对路径）
    );

    if (!notification) {
        fprintf(stderr, "Failed to create notification\n");
        notify_uninit();  // 初始化失败也要清理
        return EXIT_FAILURE;
    }

    // 可选：设置通知超时时间（单位：毫秒，-1 表示默认，0 表示永不消失）
    notify_notification_set_timeout(notification, timeout);  // 5秒后自动消失

    // 配置提示等级
    notify_notification_set_urgency(notification, urgency);

    // 3. 显示通知（核心步骤）
    GError *error = NULL;  // 用于捕获错误信息
    if (!notify_notification_show(notification, &error)) {
        fprintf(stderr, "Failed to show notification: %s\n", error->message);
        g_error_free(error);  // 释放错误对象
        g_object_unref(notification);  // 释放通知对象
        notify_uninit();
        return EXIT_FAILURE;
    }

    // 4. 清理资源（避免内存泄漏）
    g_object_unref(notification);  // 释放通知对象
    notify_uninit();               // 反初始化 libnotify 库

    printf("Notification sent successfully!\n");
    return EXIT_SUCCESS;

}
