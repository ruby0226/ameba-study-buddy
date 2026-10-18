// 用本機伺服器確認瀏覽器真的能走 HTTP
const fs = require('node:fs');
const path = require('node:path');
const http = require('node:http');
const assert = require('node:assert/strict');
const { spawn } = require('node:child_process');
const root = path.resolve(__dirname, '..');
const profile = path.join(root, '.build', 'network-browser-' + Date.now());
const page = fs.readFileSync(path.join(root, 'web/pet_test.html'), 'utf8');
const embedded = fs.readFileSync(path.join(root, 'Day3_StudyBuddy_Verify/PetWebPage.h'), 'utf8');
assert.ok(embedded.includes(page.replace(/\r\n/g, '\n')));
let polls = 0, sequence = 1, revision = 3, food = 3, feeds = 2, feedRequests = 0;
const careLine = () => `CARE,123,${revision},1,${food},${feeds},900,180,300,0,1\n`;
const server = http.createServer((req, res) => {
  if (req.url === '/' && req.method === 'GET') {
    res.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8' }); res.end(page); return;
  }
  if (req.url.startsWith('/api/poll?after=') && req.method === 'POST' && req.headers['x-studybuddy'] === '1') {
    polls++;
    const body = `NET,${sequence},0,${sequence * 1000}\n` +
      'CSV,10,10,10,90,5,52,7,65,0,0,0,0,-1,0,50,-1,1,1,0,1000,30,30,40,8,80,0,0,0,0,0,0,2,1,1,0\n' +
      'STATE,10,STUDYING,120,60,20,A,0,0,-,0,0,1,1\n' +
      `PET,${sequence * 1000},HEALTHY,100,720,0,0,0,1,0,0,1500,300,600,0,1\n` + careLine();
    res.writeHead(200, { 'Content-Type': 'text/plain', 'Cache-Control': 'no-store' }); res.end(body); return;
  }
  if (req.url.startsWith('/api/feed?boot=123&revision=') && req.method === 'POST' && req.headers['x-studybuddy'] === '1') {
    feedRequests++;
    const accepted = Number(req.url.split('revision=')[1]) === revision && food > 0;
    if (accepted) { food--; feeds++; revision++; }
    res.writeHead(accepted ? 200 : 409, { 'Content-Type': 'text/plain', 'Cache-Control': 'no-store' }); res.end(careLine()); return;
  }
  res.writeHead(404); res.end();
});
const delay = ms => new Promise(resolve => setTimeout(resolve, ms));
let browser, ws;
const deadline = setTimeout(() => { console.error('Browser test timed out'); process.exitCode = 1; cleanup(); }, 60000);
function cleanup() {
  clearTimeout(deadline);
  if (ws) ws.close();
  if (browser) browser.kill();
  server.close(); server.closeAllConnections();
}
async function main() {
  await new Promise(resolve => server.listen(0, '127.0.0.1', resolve));
  browser = spawn('C:/Program Files/Google/Chrome/Application/chrome.exe', [
    '--headless=new', '--no-first-run', '--no-default-browser-check', '--disable-gpu',
    '--remote-debugging-port=0', '--user-data-dir=' + profile, 'about:blank'
  ], { windowsHide: true, stdio: 'ignore' });
  const portFile = path.join(profile, 'DevToolsActivePort');
  for (let i = 0; i < 150 && !fs.existsSync(portFile); i++) await delay(100);
  const port = fs.readFileSync(portFile, 'utf8').split('\n')[0];
  const targets = await (await fetch('http://127.0.0.1:' + port + '/json')).json();
  ws = new WebSocket(targets.find(t => t.type === 'page').webSocketDebuggerUrl);
  await new Promise((resolve, reject) => { ws.onopen = resolve; ws.onerror = reject; });
  let id = 0;
  const pending = new Map(), errors = [];
  ws.onmessage = e => {
    const m = JSON.parse(e.data);
    if (m.method === 'Runtime.exceptionThrown') errors.push(m.params);
    if (m.id) {
      const p = pending.get(m.id); pending.delete(m.id);
      m.error ? p.reject(m.error) : p.resolve(m.result);
    }
  };
  const call = (method, params = {}) => new Promise((resolve, reject) => {
    const n = ++id; pending.set(n, { resolve, reject }); ws.send(JSON.stringify({ id: n, method, params }));
  });
  const evaluate = async expression => {
    const result = await call('Runtime.evaluate', { expression, awaitPromise: true, returnByValue: true, userGesture: true });
    if (result.exceptionDetails) throw new Error(JSON.stringify(result.exceptionDetails));
    return result.result.value;
  };
  await call('Runtime.enable'); await call('Page.enable');
  for (const [name, width, height] of [['desktop', 1360, 1200], ['mobile', 390, 844]]) {
    await call('Emulation.setDeviceMetricsOverride', { width, height, deviceScaleFactor: 1, mobile: name === 'mobile' });
    await call('Page.navigate', { url: 'http://127.0.0.1:' + server.address().port + '/' });
    await delay(500);
    assert.equal(await evaluate(`document.getElementById('feedButton').disabled`), true);
    await evaluate(`document.getElementById('connectButton').click()`); await delay(200);
    assert.match(await evaluate(`document.getElementById('connectionLabel').textContent`), /正在陪伴/);
    assert.equal(await evaluate(`document.getElementById('activityLabel').textContent`), '一起認真讀書');
    const received = await evaluate('lastSnapshotAt');
    await evaluate('pollNetwork()');
    assert.equal(await evaluate('lastSnapshotAt'), received, 'repeat snapshot ignored');
    sequence++;
    await evaluate('pollNetwork()');
    assert.ok(await evaluate('lastSnapshotAt') >= received);
    assert.ok(await evaluate('document.documentElement.scrollWidth <= document.documentElement.clientWidth'));
    const beforeMove = await evaluate('motion.x');
    await evaluate('motion.next = 0;'); await delay(600);
    assert.notEqual(await evaluate('motion.x'), beforeMove, 'chick moves around the page');
    await evaluate(`document.getElementById('settingsButton').click()`);
    assert.equal(await evaluate(`document.getElementById('settings').open`), true);
    await call('Input.dispatchKeyEvent', {type:'keyDown', key:'Escape', code:'Escape', windowsVirtualKeyCode:27});
    await call('Input.dispatchKeyEvent', {type:'keyUp', key:'Escape', code:'Escape', windowsVirtualKeyCode:27});
    await delay(100);assert.equal(await evaluate(`document.getElementById('settings').open`), false);
    const oldFood = food, oldFeeds = feeds, oldRequests = feedRequests;
    await evaluate('feed()'); await evaluate('feed()');
    assert.equal(food, oldFood - 1);assert.equal(feeds, oldFeeds + 1);assert.equal(feedRequests, oldRequests + 1);
    assert.equal(await evaluate(`document.getElementById('foodCount').textContent`), String(food));
    assert.equal(await evaluate(`document.getElementById('feedButton').disabled`), true);
    sequence++;await evaluate('pollNetwork()');
    await evaluate('eatUntil = 0; celebrateUntil = 0; toastUntil = 0; motion.ready = false; render();');await delay(50);
    await evaluate('target(roomPoint(.49,.86)); motion.x=motion.tx; motion.y=motion.ty; motion.next=Date.now()+10000;');await delay(50);
    const shot = await call('Page.captureScreenshot', { captureBeyondViewport: true });
    fs.writeFileSync(path.join(root, '.build/chick-home-' + name + '.png'), Buffer.from(shot.data, 'base64'));
    await call('Emulation.setEmulatedMedia', {features:[{name:'prefers-reduced-motion',value:'reduce'}]});
    await evaluate('eatUntil = 0;');await delay(50);const still = await evaluate('motion.x');await delay(100);assert.equal(await evaluate('motion.x'), still);
    await call('Emulation.setEmulatedMedia', {features:[]});
    await evaluate('disconnect()');
    const before = polls;
    await delay(1100);
    assert.equal(polls, before, 'disconnect stops heartbeat requests');
  }
  assert.equal(errors.length, 0, JSON.stringify(errors));
  const report = { passed: true, polls, feedRequests, checks: ['same-origin HTTP', 'POST heartbeat', 'snapshot dedup', 'feed mutation and double-click guard', 'walking', 'reduced motion', 'keyboard dialog', 'disconnect', 'desktop/mobile layout'], scope: 'mock HTTP server, not Ameba hardware' };
  fs.writeFileSync(path.join(root, '.build/network-browser-result.json'), JSON.stringify(report, null, 2));
  console.log('PASS: browser HTTP, feeding, walking, reduced motion, dialog, disconnect and desktop/mobile layout');
}
main().catch(e => { console.error(e); process.exitCode = 1; }).finally(cleanup);
