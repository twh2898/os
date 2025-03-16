#include "ebus.h"

#include "libc/proc.h"
#include "libc/stdio.h"

static int handle_event(ebus_t * bus, ebus_event_t * event);

int ebus_create(ebus_t * bus, size_t event_queue_size) {
    if (!bus) {
        return -1;
    }

    if (arr_create(&bus->handlers, 4, sizeof(ebus_handler_t))) {
        return -1;
    }

    if (cb_create(&bus->queue, event_queue_size, sizeof(ebus_event_t))) {
        return -1;
    }

    bus->next_handler_id = 1;
    bus->enabled         = 1;
    return 0;
}

void ebus_free(ebus_t * bus) {
    if (!bus) {
        return;
    }

    // TODO free elements
    arr_free(&bus->handlers);
    cb_free(&bus->queue);
}

int ebus_queue_size(ebus_t * bus) {
    return cb_len(&bus->queue);
}

int ebus_register_handler(ebus_t * bus, ebus_handler_t * handler) {
    if (!bus || !handler) {
        return -1;
    }

    if (!bus->enabled) {
        return 0;
    }

    if (handler->pid < 1) {
        handler->pid = getpid();
    }
    handler->id = bus->next_handler_id++;

    if (arr_insert(&bus->handlers, arr_size(&bus->handlers), handler)) {
        return -1;
    }

    return handler->id;
}

void ebus_unregister_handler(ebus_t * bus, int handler_id) {
    for (size_t i = 0; i < arr_size(&bus->handlers); i++) {
        ebus_handler_t * handler = arr_at(&bus->handlers, i);

        if (handler->id == handler_id) {
            arr_remove(&bus->handlers, i, 0);
            return;
        }
    }
}

void ebus_push(ebus_t * bus, ebus_event_t * event) {
    if (!bus || !event) {
        return;
    }

    if (!bus->enabled) {
        return;
    }

    if (cb_len(&bus->queue) == cb_buff_size(&bus->queue)) {
        cb_pop(&bus->queue, 0);
    }

    cb_push(&bus->queue, event);
}

int ebus_cycle(ebus_t * bus) {
    if (!bus) {
        return -1;
    }

    while (cb_len(&bus->queue) > 0) {
        ebus_event_t event;
        if (cb_pop(&bus->queue, &event)) {
            return -1;
        }

        if (handle_event(bus, &event)) {
            return -1;
        }
    }

    return 0;
}

static int handle_event(ebus_t * bus, ebus_event_t * event) {
    for (size_t i = 0; i < arr_size(&bus->handlers); i++) {
        ebus_handler_t * handler = arr_at(&bus->handlers, i);
        if (!handler) {
            return -1;
        }

        if (handler->event_id == event->event_id) {
            handler->callback_fn(event);
        }
    }

    return 0;
}
