#include <stdio.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/scrnsaver.h>

#include "xchat-plugin.h"

#define PNAME "AutoAway"
#define PDESC "Sets user away after some inactivity"
#define PVERSION "0.8"
#define AUTOAWAY_SETTINGS_FILE_NAME "autoaway.conf"
#define DEFAULT_TIMEOUT 300000
#define CHECK_INTERVAL 5000

static xchat_plugin *ph;
static char patchToPluginSettings[250];
static int timeout;

static XScreenSaverInfo *ss_info;
static Display *display;

int getPatchToSettingsFile(char *result, char *fileName) {

    strcpy(result, xchat_get_info(ph, "xchatdirfs"));
    strcat(result, "/");
    strcat(result, fileName);
    return 0;
}

int readIntegerFromFile(int *integer, char *file) {
	
    FILE *pFile;
    pFile = fopen (file, "r");
    if (pFile != NULL) {
        fscanf(pFile, "%d", integer);
        fclose (pFile);
        return 0;
    }
    return 1;
}

int writeIntegerToFile(int integer, char *file) {

    FILE *pFile;
    pFile = fopen (file, "w");
    if (pFile != NULL) {
        fprintf(pFile, "%i", integer);
        fclose (pFile);
        return 0;
    }
    return 1;
}

static int autoAwayCb(char *word[], char *word_eol[], void *userdata) {
	
    timeout = atoi(word[2]);
    timeout *= 60000;
    if (writeIntegerToFile(timeout, patchToPluginSettings)) {
        xchat_printf(ph, "Something went wrong. Check %s\n", patchToPluginSettings);
    } else {
        xchat_printf(ph, "Auto away set for %d minutes\n", timeout/60000);
    }
    return XCHAT_EAT_NONE;
}

int processUserActivity(int idleTime) {

    if (xchat_get_info(ph, "away") != NULL) {
        if (idleTime < CHECK_INTERVAL) {
            xchat_command(ph, "BACK");
        }
    } else {
        if (idleTime >= timeout) {
            xchat_command(ph, "AWAY");
        }
    }
    return 0;
}

static int checkUserActivityCb(void *userdata) {
    
    /*If timeout == 0 then don't change status */
    if (timeout == 0) return 1;
    XScreenSaverQueryInfo(display, DefaultRootWindow(display), ss_info);
    processUserActivity(ss_info->idle);
    return 1;
}

void xchat_plugin_get_info(char **name, char **desc, char **version, void **reserved) {
     	
    *name = PNAME;
    *desc = PDESC;
    *version = PVERSION;
}

int xchat_plugin_init(xchat_plugin *plugin_handle,char **plugin_name,char **plugin_desc,char **plugin_version,char *arg) {
     
    *plugin_name = PNAME;
    *plugin_desc = PDESC;
    *plugin_version = PVERSION;
	
    ph = plugin_handle;
    ss_info = XScreenSaverAllocInfo();
    display = XOpenDisplay(0);

    getPatchToSettingsFile(patchToPluginSettings, AUTOAWAY_SETTINGS_FILE_NAME);
    if (readIntegerFromFile(&timeout, patchToPluginSettings)) {
        timeout = DEFAULT_TIMEOUT;
    }

    xchat_hook_command(ph, "AUTOAWAY", XCHAT_PRI_NORM, autoAwayCb, "usage: /autoaway <timeInMinutes>", 0);
    xchat_hook_timer(ph, CHECK_INTERVAL, checkUserActivityCb, NULL);
    xchat_printf(ph, "AutoAway Plugin loaded successfully, set for %d minutes\n", timeout/60000);
    return 1;
}