/*******************************************************************************
   Copyright (C) 2014 MyanDB Software Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.
*******************************************************************************/

#ifndef PMDTCPLISTENER_H
#define PMDTCPLISTENER_H

#include "core.h"
#include "runnable.h"
#include "osSocket.h"

using namespace myan::utils;

class pmdTcpListener : public Runnable
{
    public:
        pmdTcpListener();
        ~pmdTcpListener();
        void run();
    protected:
    private:
        osSocket _svrsock;
};

#endif // PMDTCPLISTENER_H
