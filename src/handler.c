#include "../include/handler.h"
#include "../include/utils.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libudev.h>
#include <sys/select.h>


// 处理 AC 适配器状态变化
int handle_ac_change(int current_ac) {
    if (battery_state.prev_ac_online == -1) {
        battery_state.prev_ac_online = current_ac;
        return EXIT_SUCCESS;
    }

    if (current_ac != battery_state.prev_ac_online) {
        battery_state.prev_ac_online = current_ac;
        char msg[128];

        if (current_ac) {
            snprintf(msg, sizeof(msg), "Power is connected! Current capacity: %d%%", battery_state.prev_battery_percent);
            return send_notification("🔌 Charging Started", msg, BATTERY_CHARGING_ICON, 5000, NOTIFY_URGENCY_LOW);
        } else {
            snprintf(msg, sizeof(msg), "Power is disconnected! Current capacity: %d%%", battery_state.prev_battery_percent);
            return send_notification("🔋 Charging Stopped", msg,BATTERY_DISCHARING_ICON, 5000, NOTIFY_URGENCY_LOW);
        }
    } else {
        return EXIT_SUCCESS;
    }
}

// 处理电池状态变化
int handle_battery_status_change(const char* current_status) {
    if (strlen(battery_state.prev_battery_status) == 0) {
        strncpy(battery_state.prev_battery_status, current_status, sizeof(battery_state.prev_battery_status) - 1);
        return EXIT_SUCCESS;
    }

    if (strcmp(current_status, battery_state.prev_battery_status) != 0) {
        strncpy(battery_state.prev_battery_status, current_status, sizeof(battery_state.prev_battery_status) - 1);
        char msg[128];

        if (strcmp(current_status, "Full") == 0) {
            return send_notification("✅ The battery is fully charged", "It is recommended to unplug the charger to extend battery life.", CHARGING_COMPLETE_ICON, 5000, NOTIFY_URGENCY_NORMAL);
        } else if (strcmp(current_status, "Charging") == 0) {
            snprintf(msg, sizeof(msg), "Current capacity: %d%%", battery_state.prev_battery_percent);
            return send_notification("⚡ Charging...", msg, BATTERY_CHARGING_ICON, 5000, NOTIFY_URGENCY_LOW);
        } else {
            return EXIT_SUCCESS;
        }
    } else {
        return EXIT_SUCCESS;
    }
}

// 处理低电量提醒
int handle_low_battery(int current_percent) {
    if (strcmp(battery_state.prev_battery_status, "Discharging") != 0) {
        return EXIT_SUCCESS;
    }

    if (current_percent <= LOW_BATTERY_THRESHOLD) {
        if (battery_state.last_low_battery_notify == -1) {
            battery_state.last_low_battery_notify = current_percent;
        }

        if (current_percent < battery_state.last_low_battery_notify &&
            (battery_state.last_low_battery_notify - current_percent) >= LOW_BATTERY_STEP) {
            battery_state.last_low_battery_notify = current_percent;
            char msg[128];
            const char* icon = (current_percent <= 10) ? BATTERY_CAUTION_ICON : LOW_BATTERY_WARN_ICON;

            snprintf(msg, sizeof(msg), "Battery level is low! %d%%", current_percent);
            return send_notification("☢️ Low Battery Warning", msg, icon, 5000, NOTIFY_URGENCY_CRITICAL);
        } else {
            return EXIT_SUCCESS;
        }
    } else {
        return EXIT_SUCCESS;
    }
}
