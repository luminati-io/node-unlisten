#include <node.h>
#include <v8.h>
#include <uv.h>
#include <poll.h>

typedef void *QUEUE[2];
#define QUEUE_NEXT(q)       (*(QUEUE **) &((*(q))[0]))
#define QUEUE_PREV(q)       (*(QUEUE **) &((*(q))[1]))
#define QUEUE_PREV_NEXT(q)  (QUEUE_NEXT(QUEUE_PREV(q)))
#define QUEUE_NEXT_PREV(q)  (QUEUE_PREV(QUEUE_NEXT(q)))
#define QUEUE_EMPTY(q)                                                        \
  ((const QUEUE *) (q) == (const QUEUE *) QUEUE_NEXT(q))
#define QUEUE_INSERT_TAIL(h, q)                                               \
  do {                                                                        \
    QUEUE_NEXT(q) = (h);                                                      \
    QUEUE_PREV(q) = QUEUE_PREV(h);                                            \
    QUEUE_PREV_NEXT(q) = (q);                                                 \
    QUEUE_PREV(h) = (q);                                                      \
  }                                                                           \
  while (0)

static void set_events(uv_tcp_t* tcp, int events){
    uv_loop_t* loop = uv_default_loop();
    uv__io_t* w = &tcp->io_watcher;
    w->pevents = events;
    if (QUEUE_EMPTY(&w->watcher_queue))
        QUEUE_INSERT_TAIL(&loop->watcher_queue, &w->watcher_queue);
}

struct find_tcp_t {
    int fd;
    uv_tcp_t* found;
};

static void find_tcp_cb(uv_handle_t* handle, void* arg){
    int fd;
    find_tcp_t* state = (find_tcp_t*)arg;
    if (state->found || handle->type!=UV_TCP)
        return;
    if (uv_fileno(handle, &fd))
        return;
    if (fd==state->fd)
        state->found = (uv_tcp_t*)handle;
}

static uv_tcp_t* find_tcp(int fd){
    find_tcp_t state = {fd: fd, found: nullptr};
    uv_walk(uv_default_loop(), find_tcp_cb, &state);
    return state.found;
}

static void pause(const v8::FunctionCallbackInfo<v8::Value>& args){
    int fd = args[0]->Uint32Value();
    uv_tcp_t* tcp = find_tcp(fd);
    if (tcp)
        set_events(tcp, POLLOUT); // libuv freaks out if we set events to 0
}

static void resume(const v8::FunctionCallbackInfo<v8::Value>& args){
    int fd = args[0]->Uint32Value();
    uv_tcp_t* tcp = find_tcp(fd);
    if (tcp)
        set_events(tcp, POLLIN);
}

static void init(v8::Local<v8::Object> exports){
    NODE_SET_METHOD(exports, "pause", pause);
    NODE_SET_METHOD(exports, "resume", resume);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, init);
