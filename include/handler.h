#ifndef HANDLER_H
#define HANDLER_H

#define AC_PATH "/sys/class/power_supply/AC/online"
#define BATTERY_PATH "/sys/class/power_supply/BAT0/capacity"
#define STATUS_PATH "/sys/class/power_supply/BAT0/status"
#define BATTERY_CHARGING_ICON "/usr/share/icons/Adwaita/symbolic/legacy/battery-good-charging-symbolic.svg"
#define BATTERY_DISCHARING_ICON "/usr/share/icons/Adwaita/symbolic/devices/battery-symbolic.svg"
#define LOW_BATTERY_WARN_ICON "/usr/share/icons/Adwaita/symbolic/status/battery-level-20-symbolic.svg"
#define CHARGING_COMPLETE_ICON "/usr/share/icons/Adwaita/symbolic/status/battery-level-80-charging-symbolic.svg"
#define BATTERY_CAUTION_ICON "/usr/share/icons/Adwaita/symbolic/status/battery-caution-symbolic.svg"
#define LOW_BATTERY_THRESHOLD 20
#define LOW_BATTERY_STEP 5

// 全局状态变量
typedef struct {
    int prev_ac_online;
    int prev_battery_percent;
    char prev_battery_status[32];
    int last_low_battery_notify;
} BatteryState;

extern BatteryState battery_state;

int handle_ac_change(int current_ac);

// 处理电池状态变化
int handle_battery_status_change(const char* current_status);

// 处理低电量提醒
int handle_low_battery(int current_percent);

#endif // HANDLER_H
