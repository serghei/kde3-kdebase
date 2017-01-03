/*
 * Copyright (C) 2017 Serghei Amelian <serghei.amelian@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <fcntl.h>
#include <unistd.h>

#include <qstringlist.h>

#include "cpumonitor.h"


struct CpuMonitor::Private {
    int cpuPrevTotal;
    int cpuPrevIdle;
    int cpuPrevUsage;
    int fdProcStat;
};


CpuMonitor::CpuMonitor()
{
    d = new Private;

    d->cpuPrevTotal = 0;
    d->cpuPrevIdle = 0;
    d->cpuPrevUsage = 0;

    d->fdProcStat = ::open("/proc/stat", O_RDONLY);

    // unable to open /proc/stat
    if(-1 == d->fdProcStat)
        return;

    timerEvent(nullptr);
    startTimer(1000);
}


CpuMonitor::~CpuMonitor()
{
    if(-1 != d->fdProcStat)
        ::close(d->fdProcStat);

    delete d;
}


int CpuMonitor::cpuUsage() const
{
    return d->cpuPrevUsage;
}


void CpuMonitor::timerEvent(QTimerEvent*)
{
    // rewind
    ::lseek(d->fdProcStat, 0L, SEEK_SET);

    // read /proc/stat
    char buf[128];
    ssize_t size = ::read(d->fdProcStat, buf, sizeof(buf) - 1);

    // to go further, we need at least few bytes,
    // for example 32 bytes
    if(32 > size)
        return;

    // ensure that the buffer is ending with '\0'
    buf[size] = '\0';

    // the buffer should begin with "cpu " word
    if(0 != ::strncmp(buf, "cpu ", 4))
        return;

    char *line = buf + 4;

    // search for end-of-line (we need only first line)
    char *eol = ::strchr(line, '\n');

    // hmm, we didn't found the end of line
    if(!eol)
        return;

    // we need only the first line, so block it here
    *eol = '\0';

    // https://www.kernel.org/doc/Documentation/filesystems/proc.txt
    //
    // The very first  "cpu" line aggregates the  numbers in all  of the other "cpuN"
    // lines.  These numbers identify the amount of time the CPU has spent performing
    // different kinds of work.  Time units are in USER_HZ (typically hundredths of a
    // second).  The meanings of the columns are as follows, from left to right:
    //
    // [0] user: normal processes executing in user mode
    // [1] nice: niced processes executing in user mode
    // [2] system: processes executing in kernel mode
    // [3] idle: twiddling thumbs
    // [4] iowait: waiting for I/O to complete
    // [5] irq: servicing interrupts
    // [6] softirq: servicing softirqs
    // [7] steal: involuntary wait
    // [8] guest: running a normal guest
    // [9] guest_nice: running a niced guest

    // split the line into list of values
    QStringList cpuValues = QStringList::split(' ', line);

    // should be at least 10 fields
    if(10 > cpuValues.count())
        return;

    // compute non-idle (user + nice + system + irq + softirq + steal)
    int cpuNonIdle =
            cpuValues[0].toInt() + // user
            cpuValues[1].toInt() + // nice
            cpuValues[2].toInt() + // system
            cpuValues[5].toInt() + // irq
            cpuValues[6].toInt() + // softirq
            cpuValues[7].toInt();  // steal

    // compute idle (idle + iowait)
    int cpuIdle =
            cpuValues[3].toInt() + // idle
            cpuValues[4].toInt();  // iowait

    // compute total (idle + non-idle)
    int cpuTotal = cpuNonIdle + cpuIdle;

    // compute deltas
    int cpuTotalDelta = cpuTotal - d->cpuPrevTotal;
    int cpuIdleDelta = cpuIdle - d->cpuPrevIdle;

    // save current values
    d->cpuPrevTotal = cpuTotal;
    d->cpuPrevIdle = cpuIdle;

    // compute percent of cpu usage
    int cpuUsage = 100 * (cpuTotalDelta - cpuIdleDelta) / cpuTotalDelta;

    if(cpuUsage != d->cpuPrevUsage)
    {
        d->cpuPrevUsage = cpuUsage;
        emit cpuUsageChanged();
    }
}


#include "cpumonitor.moc"
