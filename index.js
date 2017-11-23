'use strict';
const assert = require('assert');
const cluster = require('cluster');
const binding = require('bindings')('unlisten.node');

if (cluster.isWorker)
    assert.strictEqual(cluster.schedulingPolicy, cluster.SCHED_NONE);

function pause(server){
    binding.pause(server._handle.fd);
}

function resume(server){
    binding.resume(server._handle.fd);
}

module.exports = {pause, resume};
