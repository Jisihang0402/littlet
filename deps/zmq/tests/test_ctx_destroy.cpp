﻿/*
    Copyright (c) 2007-2013 Contributors as noted in the AUTHORS file

    This file is part of 0MQ.

    0MQ is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    0MQ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "testutil.hpp"

static void receiver (void *socket)
{
    char buffer[16];
    int rc = zmq_recv (socket, &buffer, sizeof (buffer), 0);
    assert(rc == -1);
}

void test_ctx_destroy()
{
    int rc;
    
    //  Set up our context and sockets
    void *ctx = zmq_ctx_new ();
    assert (ctx);
    
    void *socket = zmq_socket (ctx, ZMQ_PULL);
    assert (socket);

    // Close the socket
    rc = zmq_close (socket);
    assert (rc == 0);
    
    // Destroy the context
    rc = zmq_ctx_destroy (ctx);
    assert (rc == 0);
}

void test_ctx_shutdown()
{
    int rc;
    
    //  Set up our context and sockets
    void *ctx = zmq_ctx_new ();
    assert (ctx);
    
    void *socket = zmq_socket (ctx, ZMQ_PULL);
    assert (socket);

    // Spawn a thread to receive on socket
    void *receiver_thread = zmq_threadstart (&receiver, socket);

    // Wait for thread to start up and block
    msleep (SETTLE_TIME);

    // Shutdown context, if we used destroy here we would deadlock.
    rc = zmq_ctx_shutdown (ctx);
    assert (rc == 0);

    // Wait for thread to finish
    zmq_threadclose (receiver_thread);

    // Close the socket.
    rc = zmq_close (socket);
    assert (rc == 0);

    // Destory the context, will now not hang as we have closed the socket.
    rc = zmq_ctx_destroy (ctx);
    assert (rc == 0);
}

int main (void)
{
    setup_test_environment();

    test_ctx_destroy();
    test_ctx_shutdown();

    return 0;
}
