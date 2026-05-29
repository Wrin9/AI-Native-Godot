#!/usr/bin/env node
/**
 * Godot MCP Bridge — Stdio → HTTP JSON-RPC
 * Craft Agent MCP stdio 请求转发到 Godot HTTP MCP Server。
 */
const http = require('http');
const MCP_URL = process.env.GODOT_MCP_URL || 'http://127.0.0.1:9877/';

function sendToHttp(jsonRequest) {
  return new Promise((resolve, reject) => {
    const body = JSON.stringify(jsonRequest);
    const url = new URL(MCP_URL);
    const req = http.request({
      hostname: url.hostname, port: parseInt(url.port) || 80, path: url.pathname || '/',
      method: 'POST',
      headers: { 'Content-Type': 'application/json', 'Content-Length': Buffer.byteLength(body) },
      timeout: 30000
    }, (res) => {
      const chunks = [];
      res.on('data', c => chunks.push(c));
      res.on('end', () => {
        const raw = Buffer.concat(chunks).toString();
        const idx = raw.indexOf('{');
        if (idx >= 0) {
          try { resolve(JSON.parse(raw.substring(idx))); }
          catch(e) { reject(e); }
        } else { reject(new Error('No JSON')); }
      });
    });
    req.on('error', reject);
    req.on('timeout', () => { req.destroy(); reject(new Error('Timeout')); });
    req.write(body);
    req.end();
  });
}

let busy = false;
let pending = [];
let buf = '';

function drain() {
  if (busy || pending.length === 0) return;
  busy = true;
  const line = pending.shift();
  (async () => {
    try {
      const req = JSON.parse(line);
      // MCP notifications (no id) - skip HTTP, no response needed
      if (req.id === undefined || req.id === null) {
      } else {
        const res = await sendToHttp(req);
        process.stdout.write(JSON.stringify(res) + '\n');
      }
    } catch(e) {
      process.stdout.write(JSON.stringify({jsonrpc:'2.0',id:null,error:{code:-32603,message:e.message}}) + '\n');
    }
    busy = false;
    drain();
  })();
}

process.stdin.setEncoding('utf8');
process.stdin.on('data', (chunk) => {
  buf += chunk;
  let nl;
  while ((nl = buf.indexOf('\n')) >= 0) {
    const line = buf.substring(0, nl).replace(/\r$/, '');
    buf = buf.substring(nl + 1);
    if (line) pending.push(line);
  }
  drain();
});
process.stdin.on('end', () => {
  const check = setInterval(() => {
    if (!busy && pending.length === 0) { clearInterval(check); process.exit(0); }
  }, 50);
});
