# unlisten

```js
const unlisten = require('unlisten');
```

Node.js module: Temporarily stop accepting connections on a TCP or HTTP(S) server.

## Introduction

Once you get a `net.Server`, `http.Server` or `https.Server` in Node.js to
start listening on a port, they will automatically accept every connection that
comes their way. Node.js doesn't let you stop accepting temporarily without
tearing down the whole server (which involves closing the listening socket).

Why would you want to stop accepting?

If you're using `cluster.SCHED_NONE` instead of the default `cluster.SCHED_RR`
as Node.js' [scheduling policy](https://nodejs.org/api/cluster.html#cluster_cluster_schedulingpolicy),
your workers share the listening socket. When a new connection comes, all
workers that are currently blocked in `epoll_wait` wake up and race to accept
it; if none is in `epoll_wait`, the next one to call `epoll_wait` gets it. This
naturally excludes workers that are very busy. Still, other workers can bite more
than they can chew, especially given that the way the kernel seems to work, the
same worker tends to win the race every time.

So if the workers have some kind of safeguard against overload, such as the
[toobusy-js](https://www.npmjs.com/package/toobusy-js) module, they can do
better than just denying service: using the `unlisten` module, a worker can
temporarily stop participating in the accept race and let other, less busy
workers help out.

## Usage

To make an instance of `net.Server`, `http.Server` or `https.Server` stop
accepting connections:

```js
unlisten.pause(server);
```

To resume accepting:

```js
unlisten.resume(server);
```

The calls are idempotent (that means, calling `pause` or `resume` the second
time in a row on the same server does nothing).

This is only for listening servers! Don't try it on sockets that represent
established connections rather than servers, or on servers that haven't started
listening or have been closed.

## Limitations

* This module only works (and compiles) on Linux.
* In Node.js cluster workers, it requires `cluster.SCHED_NONE`.
* It fiddles with the `libuv` internals, and might stop working with future
  versions of Node.js. PRs to keep it compatible are welcome.
* The server instance must have already started listening.
