/******************************************************************************
 * event_channel.h
 *
 * Event channels between domains.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Copyright (c) 2003-2004, K A Fraser.
 */

#include <xen.h>

#ifndef __XEN_PUBLIC_EVENT_CHANNEL_H__
#define __XEN_PUBLIC_EVENT_CHANNEL_H__

/* Xen 4.3: Moved them here - easier to compare. */
/* ` enum event_channel_op { // EVTCHNOP_* => struct evtchn_* */
#define EVTCHNOP_bind_interdomain 0
#define EVTCHNOP_bind_virq        1
#define EVTCHNOP_bind_pirq        2
#define EVTCHNOP_close            3
#define EVTCHNOP_send             4
#define EVTCHNOP_status           5
#define EVTCHNOP_alloc_unbound    6
#define EVTCHNOP_bind_ipi         7
#define EVTCHNOP_bind_vcpu        8
#define EVTCHNOP_unmask           9
#define EVTCHNOP_reset           10
/* ` } */

typedef uint32_t evtchn_port_t;

/*
 * EVTCHNOP_alloc_unbound: Allocate a port in domain <dom> and mark as
 * accepting interdomain bindings from domain <remote_dom>. A fresh port
 * is allocated in <dom> and returned as <port>.
 * NOTES:
 *  1. If the caller is unprivileged then <dom> must be DOMID_SELF.
 *  2. <rdom> may be DOMID_SELF, allowing loopback connections.
 */
typedef struct evtchn_alloc_unbound {
    /* IN parameters */
    domid_t dom, remote_dom;
    /* OUT parameters */
    evtchn_port_t port;
} evtchn_alloc_unbound_t;

/*
 * EVTCHNOP_bind_interdomain: Construct an interdomain event channel between
 * the calling domain and <remote_dom>. <remote_dom,remote_port> must identify
 * a port that is unbound and marked as accepting bindings from the calling
 * domain. A fresh port is allocated in the calling domain and returned as
 * <local_port>.
 * NOTES:
 *  2. <remote_dom> may be DOMID_SELF, allowing loopback connections.
 */
typedef struct evtchn_bind_interdomain {
    /* IN parameters. */
    domid_t remote_dom;
    evtchn_port_t remote_port;
    /* OUT parameters. */
    evtchn_port_t local_port;
} evtchn_bind_interdomain_t;

/*
 * EVTCHNOP_bind_virq: Bind a local event channel to VIRQ <irq> on specified
 * vcpu.
 * NOTES:
 *  1. A virtual IRQ may be bound to at most one event channel per vcpu.
 *  2. The allocated event channel is bound to the specified vcpu. The binding
 *     may not be changed.
 */
typedef struct evtchn_bind_virq {
    /* IN parameters. */
    uint32_t virq;
    uint32_t vcpu;
    /* OUT parameters. */
    evtchn_port_t port;
} evtchn_bind_virq_t;

/*
 * EVTCHNOP_bind_pirq: Bind a local event channel to PIRQ <irq>.
 * NOTES:
 *  1. A physical IRQ may be bound to at most one event channel per domain.
 *  2. Only a sufficiently-privileged domain may bind to a physical IRQ.
 */
typedef struct evtchn_bind_pirq {
    /* IN parameters. */
    uint32_t pirq;
#define BIND_PIRQ__WILL_SHARE 1
    uint32_t flags; /* BIND_PIRQ__* */
    /* OUT parameters. */
    evtchn_port_t port;
} evtchn_bind_pirq_t;

/*
 * EVTCHNOP_bind_ipi: Bind a local event channel to receive events.
 * NOTES:
 *  1. The allocated event channel is bound to the specified vcpu. The binding
 *     may not be changed.
 */
typedef struct evtchn_bind_ipi {
    uint32_t vcpu;
    /* OUT parameters. */
    evtchn_port_t port;
} evtchn_bind_ipi_t;

/*
 * EVTCHNOP_close: Close a local event channel <port>. If the channel is
 * interdomain then the remote end is placed in the unbound state
 * (EVTCHNSTAT_unbound), awaiting a new connection.
 */
typedef struct evtchn_close {
    /* IN parameters. */
    evtchn_port_t port;
} evtchn_close_t;

/*
 * EVTCHNOP_send: Send an event to the remote end of the channel whose local
 * endpoint is <port>.
 */
typedef struct evtchn_send {
    /* IN parameters. */
    evtchn_port_t port;
} evtchn_send_t;

/*
 * EVTCHNOP_status: Get the current status of the communication channel which
 * has an endpoint at <dom, port>.
 * NOTES:
 *  1. <dom> may be specified as DOMID_SELF.
 *  2. Only a sufficiently-privileged domain may obtain the status of an event
 *     channel for which <dom> is not DOMID_SELF.
 */
typedef struct evtchn_status {
    /* IN parameters */
    domid_t  dom;
    evtchn_port_t port;
    /* OUT parameters */
#define EVTCHNSTAT_closed       0  /* Channel is not in use.                 */
#define EVTCHNSTAT_unbound      1  /* Channel is waiting interdom connection.*/
#define EVTCHNSTAT_interdomain  2  /* Channel is connected to remote domain. */
#define EVTCHNSTAT_pirq         3  /* Channel is bound to a phys IRQ line.   */
#define EVTCHNSTAT_virq         4  /* Channel is bound to a virtual IRQ line */
#define EVTCHNSTAT_ipi          5  /* Channel is bound to a virtual IPI line */
    uint32_t status;
    uint32_t vcpu;                 /* VCPU to which this channel is bound.   */
    union {
        struct {
            domid_t dom;
        } unbound; /* EVTCHNSTAT_unbound */
        struct {
            domid_t dom;
            evtchn_port_t port;
        } interdomain; /* EVTCHNSTAT_interdomain */
        uint32_t pirq;      /* EVTCHNSTAT_pirq        */
        uint32_t virq;      /* EVTCHNSTAT_virq        */
    } u;
} evtchn_status_t;

/*
 * EVTCHNOP_bind_vcpu: Specify which vcpu a channel should notify when an
 * event is pending.
 * NOTES:
 *  1. IPI- and VIRQ-bound channels always notify the vcpu that initialised
 *     the binding. This binding cannot be changed.
 *  2. All other channels notify vcpu0 by default. This default is set when
 *     the channel is allocated (a port that is freed and subsequently reused
 *     has its binding reset to vcpu0).
 */
typedef struct evtchn_bind_vcpu {
    /* IN parameters. */
    evtchn_port_t port;
    uint32_t vcpu;
} evtchn_bind_vcpu_t;

/*
 * EVTCHNOP_unmask: Unmask the specified local event-channel port and deliver
 * a notification to the appropriate VCPU if an event is pending.
 */
typedef struct evtchn_unmask {
    /* IN parameters. */
    evtchn_port_t port;
} evtchn_unmask_t;

/*
 * EVTCHNOP_reset: Close all event channels associated with specified domain.
 * NOTES:
 *  1. <dom> may be specified as DOMID_SELF.
 *  2. Only a sufficiently-privileged domain may specify other than DOMID_SELF.
 */
typedef struct evtchn_reset {
    /* IN parameters. */
    domid_t dom;
} evtchn_reset_t;

typedef struct evtchn_op {
    uint32_t cmd; /* EVTCHNOP_* */
    union {
        evtchn_alloc_unbound_t    alloc_unbound;
        evtchn_bind_interdomain_t bind_interdomain;
        evtchn_bind_virq_t        bind_virq;
        evtchn_bind_pirq_t        bind_pirq;
        evtchn_bind_ipi_t         bind_ipi;
        evtchn_close_t            close;
        evtchn_send_t             send;
        evtchn_status_t           status;
        evtchn_bind_vcpu_t        bind_vcpu;
        evtchn_unmask_t           unmask;
    } u;
} evtchn_op_t;

#endif /* __XEN_PUBLIC_EVENT_CHANNEL_H__ */

/*
 * Local variables:
 * mode: C
 * c-set-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
