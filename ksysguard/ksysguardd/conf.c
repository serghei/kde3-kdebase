/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2001 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Command.h"

#include "conf.h"

CONTAINER LogFileList = 0;
CONTAINER SensorList = 0;
char *RegisterDomain = 0;

void LogFileList_cleanup(void *ptr);
void freeConfigFile(void);

void LogFileList_cleanup(void *ptr)
{
    if(ptr)
    {
        if(((ConfigLogFile *)ptr)->name)
            free(((ConfigLogFile *)ptr)->name);

        free(ptr);
    }
}

void freeConfigFile(void)
{
    destr_ctnr(LogFileList, LogFileList_cleanup);
    destr_ctnr(SensorList, free);
}

void parseConfigFile(const char *filename)
{
    FILE *config;
    char line[2048];
    char *begin, *token, *tmp;
    ConfigLogFile *confLog;

    LogFileList = new_ctnr();
    SensorList = new_ctnr();

    if((config = fopen(filename, "r")) == NULL)
    {
        log_error("can't open config file '%s'", filename);

        /**
          If we can't open a config file we have to add the
          available sensors manually
         */
        push_ctnr(SensorList, strdup("ProcessList"));
        push_ctnr(SensorList, strdup("Memory"));
        push_ctnr(SensorList, strdup("Stat"));
        push_ctnr(SensorList, strdup("NetDev"));
        push_ctnr(SensorList, strdup("NetStat"));
        push_ctnr(SensorList, strdup("Apm"));
        push_ctnr(SensorList, strdup("Acpi"));
        push_ctnr(SensorList, strdup("CpuInfo"));
        push_ctnr(SensorList, strdup("LoadAvg"));
        push_ctnr(SensorList, strdup("LmSensors"));
        push_ctnr(SensorList, strdup("DiskStat"));
        push_ctnr(SensorList, strdup("LogFile"));
        push_ctnr(SensorList, strdup("DellLaptop"));

        return;
    }

    while(fgets(line, sizeof(line), config) != NULL)
    {
        if((line[0] == '#') || (strlen(line) == 0))
            continue;

        if(strchr(line, '#'))
            *(strchr(line, '#')) = '\0';

        if(line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';

        if(!strncmp(line, "RegisterDomain", 14) && (begin = strchr(line, '=')))
            RegisterDomain = strdup(begin + 1);

        if(!strncmp(line, "LogFiles", 8) && (begin = strchr(line, '=')))
        {
            begin++;

            for(token = strtok(begin, ","); token; token = strtok(NULL, ","))
            {
                if((confLog = (ConfigLogFile *)malloc(sizeof(ConfigLogFile))) == NULL)
                {
                    log_error("malloc() no free memory avail");
                    continue;
                }
                confLog->name = strdup(token);
                tmp = strchr(confLog->name, ':');
                *tmp = '\0';
                confLog->path = tmp;
                confLog->path++;

                push_ctnr(LogFileList, confLog);
            }
        }

        if(!strncmp(line, "Sensors", 7) && (begin = strchr(line, '=')))
        {
            begin++;

            for(token = strtok(begin, ","); token; token = strtok(NULL, ","))
                push_ctnr(SensorList, strdup(token));
        }
    }

    fclose(config);
}

int sensorAvailable(const char *sensor)
{
    char *name;

    for(name = first_ctnr(SensorList); name; name = next_ctnr(SensorList))
    {
        if(!strcmp(name, sensor))
            return 1;
    }

    return 0;
}
