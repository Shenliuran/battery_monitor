#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <libudev.h>
#include <sys/select.h>
#include "../include/handler.h"
#include "../include/utils.h"

// -------------------------- 配置区域 --------------------------
#define AC_PATH "/sys/class/power_supply/AC/online"
#define BATTERY_PATH "/sys/class/power_supply/BAT0/capacity"
#define STATUS_PATH "/sys/class/power_supply/BAT0/status"
#define LOW_BATTERY_THRESHOLD 20
#define LOW_BATTERY_STEP 5
#define APP_NAME "battery-notifier"  // 通知的应用标识（自定义）
// ----------------------------------------------------------------

BatteryState battery_state = {
    -1, // prev_ac_online
    -1, // prev_battery_percent
    {0},// prev_battery_status[32]
    -1  // last_low_battery_notify
};

int main() {
    // 注册信号处理
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // 参数：应用名称，会关联到通知的归属
    if (!notify_init(APP_NAME)) {
        fprintf(stderr, "Failed to initialize libnotify\n");
        return EXIT_FAILURE;
    }

    // 初始化 udev
    struct udev* udev = udev_new();
    if (!udev) {
        fprintf(stderr, "Failed to create udev context\n");
        return EXIT_FAILURE;
    }

    // 创建监控器并过滤 power_supply 子系统
    struct udev_monitor* mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "power_supply", NULL);
    udev_monitor_enable_receiving(mon);

    // 获取文件描述符用于 poll
    int fd = udev_monitor_get_fd(mon);

    printf("[*] Battery monitor started\n");

    // 主循环
    while (running) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        // 等待事件（超时 1 秒，以便检查 running 标志）
        struct timeval tv = {.tv_sec = 1, .tv_usec = 0};
        int ret = select(fd + 1, &fds, NULL, NULL, &tv);

        if (ret > 0 && FD_ISSET(fd, &fds)) {
            struct udev_device* dev = udev_monitor_receive_device(mon);
            if (dev) {
                const char* action = udev_device_get_action(dev);
                if (action && strcmp(action, "change") == 0) {
                    // 读取最新状态
                    char* ac_str = read_sysfs_file(AC_PATH);
                    char* capacity_str = read_sysfs_file(BATTERY_PATH);
                    char* status_str = read_sysfs_file(STATUS_PATH);

                    if (ac_str && capacity_str && status_str) {
                        int current_ac = atoi(ac_str);
                        int current_percent = atoi(capacity_str);

                        // 初始化状态
                        if (battery_state.prev_battery_percent == -1) {
                            battery_state.prev_battery_percent = current_percent;
                            battery_state.last_low_battery_notify = current_percent;
                            printf("[*] Current battery: %d%%\n", current_percent);
                        }

                        if (handle_ac_change(current_ac) & 
                            handle_battery_status_change(status_str) &
                            handle_low_battery(current_percent)) {
                            fprintf(stderr, "Failed to create battery event handle.\n");
                            notify_uninit();  // 初始化失败也要清理
                            return EXIT_FAILURE;
                        }

                        // 更新电量
                        battery_state.prev_battery_percent = current_percent;
                    }

                    // 释放资源
                    free(ac_str);
                    free(capacity_str);
                    free(status_str);
                }
                udev_device_unref(dev);
            }
        }
    }

    // 清理资源
    udev_monitor_unref(mon);
    udev_unref(udev);
    notify_uninit();               // 反初始化 libnotify 库
    printf("\n[*] Monitor stopped\n");

    return EXIT_SUCCESS;
}

