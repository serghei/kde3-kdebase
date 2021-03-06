/*
 * Copyright (C) 2012 Serghei Amelian <serghei.amelian@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef _POLKIT_AGENT_H_
#define _POLKIT_AGENT_H_

#include <kdedmodule.h>


struct _PolkitAgentListener;


class PolkitAgent : public KDEDModule {
public:
    PolkitAgent(const QCString &obj);
    ~PolkitAgent();

private:
    void initialize();
    void cleanup();

private:
    _PolkitAgentListener *listener;
};


#endif
