LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := parabot

include $(XASH3D_CONFIG)

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a-hard)
LOCAL_MODULE_FILENAME = libparabot_hardfp
endif

LOCAL_CFLAGS += -Wno-write-strings

LOCAL_CPPFLAGS := $(LOCAL_CFLAGS) -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections \
			-fno-asynchronous-unwind-tables -fsigned-char

LOCAL_LDFLAGS += -Wl,--gc-sections

LOCAL_C_INCLUDES := \
		$(LOCAL_PATH)/. \
		$(LOCAL_PATH)/.. \
                $(LOCAL_PATH)/../bot \
                $(LOCAL_PATH)/../utils \
                $(LOCAL_PATH)/../common \
                $(LOCAL_PATH)/../dlls \
                $(LOCAL_PATH)/../engine \
                $(LOCAL_PATH)/../pm_shared \
                $(LOCAL_PATH)/../metamod

LOCAL_SRC_FILES := \
		../bot/bot.cpp \
		../bot/bot_client.cpp \
		animation.cpp \
		commands.cpp \
		dll.cpp \
		engine.cpp \
		h_export.cpp \
		linkfunc.cpp \
		startframe.cpp \
		util.cpp \
		../bot/marker.cpp \
		../bot/parabot.cpp \
		../bot/pb_action.cpp \
		../bot/pb_cell.cpp \
		../bot/pb_chat.cpp \
		../bot/pb_combat.cpp \
		../bot/pb_combatgoals.cpp \
		../bot/pb_configuration.cpp \
		../bot/pb_focus.cpp \
		../bot/pb_global.cpp \
		../bot/pb_goalfinder.cpp \
		../bot/pb_goals.cpp \
		../bot/pb_journey.cpp \
		../bot/pb_kills.cpp \
		../bot/pb_mapcells.cpp \
		../bot/pb_mapgraph.cpp \
		../bot/pb_mapimport.cpp \
		../bot/pb_navpoint.cpp \
		../bot/pb_needs.cpp \
		../bot/pb_observer.cpp \
		../bot/pb_path.cpp \
		../bot/pb_perception.cpp \
		../bot/pb_roaming.cpp \
		../bot/pb_sectors.cpp \
		../bot/pb_vistable.cpp \
		../bot/pb_weapon.cpp \
		../bot/pb_weaponhandling.cpp \
		../bot/pbt_dynarray.cpp \
		../bot/pbt_priorityqueue.cpp \
		../bot/sounds.cpp \
		../bot/utilityfuncs.cpp \
		../metamod/meta_api.cpp

include $(BUILD_SHARED_LIBRARY)
