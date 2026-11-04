// 用本機假小屋確認瀏覽器真的能走 HTTP，並截圖各種畫面
const fs = require('node:fs');
const path = require('node:path');
const http = require('node:http');
const assert = require('node:assert/strict');
const { spawn } = require('node:child_process');
const root = path.resolve(__dirname, '..');
const profile = path.join(root, '.build', 'network-browser-' + Date.now());
const page = fs.readFileSync(path.join(root, 'web/pet_test.html'), 'utf8');
const embedded = fs.readFileSync(path.join(root, 'Day3_StudyBuddy_Verify/PetWebPage.h'), 'utf8');
assert.ok(embedded.includes(page.replace(/\r\n/g, '\n')), 'embedded page is up to date');
let polls = 0, sequence = 1, revision = 3, food = 3, feeds = 2, feedRequests = 0, activity = 'STUDYING', work = 720, rest = 0;
let planPhase = 0, planRevision = 0, planFocus = 1500, planRest = 300, planGoal = 3600, planRequests = [];
const careLine = () => `CARE,123,${revision},1,${food},${feeds},900,180,300,0,1\n`;
const planLine = () => `PLAN,${planRevision},${planPhase},${planFocus},${planRest},${planGoal},0,0,600,900,3600,180,1200,1800,14400\n`;
const server = http.createServer((req, res) => {
  const ok = req.method === 'POST' && req.headers['x-studybuddy'] === '1';
  if (req.url === '/' && req.method === 'GET') { res.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8' }); res.end(page); return; }
  if (req.url.startsWith('/api/poll?after=') && ok) {
    polls++; sequence++;
    const body = `NET,${sequence},0,${sequence * 1000}\n` +
      'CSV,10,10,10,90,5,52,7,65,0,0,0,0,-1,0,50,-1,1,1,0,1000,30,30,40,8,80,0,0,0,0,0,0,2,1,1,0\n' +
      `STATE,10,${activity},1200,600,300,A,1,0,-,0,0,${activity === 'PHONE' ? 1 : 0},0,1,1,0,${activity === 'PHONE' ? 'phone-5-8-0' : 'book-3-42-0'}\n` +
      `PET,${sequence * 1000},HEALTHY,${activity === 'AWAY' ? 80 : 100},${work},${rest},1,2,1,1,0,1500,300,600,0,1\n` + careLine() + planLine();
    res.writeHead(200, { 'Content-Type': 'text/plain', 'Cache-Control': 'no-store' }); res.end(body); return;
  }
  if (req.url.startsWith('/api/plan?') && ok) {
    planRequests.push(req.url);
    const q = new URL('http://x' + req.url).searchParams;
    const f = +q.get('focus'), r = +q.get('rest'), g = +q.get('goal');
    const accepted = f >= 900 && f <= 3600 && r >= 180 && r <= 1200 && g >= 1800 && g <= 14400 && g >= f;
    if (accepted) { planPhase = 2; planRevision++; planFocus = f; planRest = r; planGoal = g; }
    res.writeHead(accepted ? 200 : 422, { 'Content-Type': 'text/plain' }); res.end(planLine()); return;
  }
  if (req.url.startsWith('/api/feed?boot=123&revision=') && ok) {
    feedRequests++;
    const accepted = Number(req.url.split('revision=')[1]) === revision && food > 0;
    if (accepted) { food--; feeds++; revision++; }
    res.writeHead(accepted ? 200 : 409, { 'Content-Type': 'text/plain', 'Cache-Control': 'no-store' }); res.end(careLine()); return;
  }
  res.writeHead(404); res.end();
});
const delay = ms => new Promise(resolve => setTimeout(resolve, ms));
let browser, ws;
const deadline = setTimeout(() => { console.error('Browser test timed out'); process.exitCode = 1; cleanup(); }, 90000);
function cleanup() { clearTimeout(deadline); if (ws) ws.close(); if (browser) browser.kill(); server.close(); server.closeAllConnections(); }
async function main() {
  await new Promise(resolve => server.listen(0, '127.0.0.1', resolve));
  browser = spawn('C:/Program Files/Google/Chrome/Application/chrome.exe', ['--headless=new', '--no-first-run', '--no-default-browser-check', '--disable-gpu', '--remote-debugging-port=0', '--user-data-dir=' + profile, 'about:blank'], { windowsHide: true, stdio: 'ignore' });
  const portFile = path.join(profile, 'DevToolsActivePort');
  for (let i = 0; i < 150 && !fs.existsSync(portFile); i++) await delay(100);
  const port = fs.readFileSync(portFile, 'utf8').split('\n')[0];
  const targets = await (await fetch('http://127.0.0.1:' + port + '/json')).json();
  ws = new WebSocket(targets.find(t => t.type === 'page').webSocketDebuggerUrl);
  await new Promise((resolve, reject) => { ws.onopen = resolve; ws.onerror = reject; });
  let id = 0; const pending = new Map(), errors = [];
  ws.onmessage = e => { const m = JSON.parse(e.data); if (m.method === 'Runtime.exceptionThrown') errors.push(m.params); if (m.id) { const p = pending.get(m.id); pending.delete(m.id); m.error ? p.reject(m.error) : p.resolve(m.result); } };
  const call = (method, params = {}) => new Promise((resolve, reject) => { const n = ++id; pending.set(n, { resolve, reject }); ws.send(JSON.stringify({ id: n, method, params })); });
  const evaluate = async expression => { const r = await call('Runtime.evaluate', { expression, awaitPromise: true, returnByValue: true, userGesture: true }); if (r.exceptionDetails) throw new Error(JSON.stringify(r.exceptionDetails)); return r.result.value; };
  const shot = async name => { const s = await call('Page.captureScreenshot', {}); fs.writeFileSync(path.join(root, `.build/${name}.png`), Buffer.from(s.data, 'base64')); };
  const tick = async () => { sequence++; await evaluate('pollNetwork()'); await delay(350); };
  await call('Runtime.enable'); await call('Page.enable');
  for (const [name, width, height] of [['desktop', 1360, 860], ['mobile', 390, 844]]) {
    planPhase = 0; activity = 'STUDYING'; work = 720; rest = 0;
    await call('Emulation.setDeviceMetricsOverride', { width, height, deviceScaleFactor: 1, mobile: name === 'mobile' });
    await call('Page.navigate', { url: 'http://127.0.0.1:' + server.address().port + '/' });
    await delay(900);
    assert.equal(await evaluate(`mode()`), 'NOPLAN', 'board without plan waits for settings');
    assert.equal(await evaluate(`document.getElementById('feedButton').disabled`), true);
    assert.ok(await evaluate('document.documentElement.scrollWidth <= document.documentElement.clientWidth'), 'no sideways scroll');
    await shot('web-noplan-' + name);
    // 超出範圍不送
    planRequests = [];
    await evaluate(`(()=>{const i=document.getElementById('focusInput');i.value='70';i.dispatchEvent(new Event('input',{bubbles:true}));})()`);
    assert.equal(await evaluate(`document.getElementById('startButton').disabled`), true);
    assert.match(await evaluate(`document.getElementById('focusErr').textContent`), /15～60/);
    await evaluate(`(()=>{const i=document.getElementById('focusInput');i.value='30';i.dispatchEvent(new Event('input',{bubbles:true}));})()`);
    await evaluate(`document.getElementById('startButton').click()`); await delay(400);
    assert.equal(planRequests.length, 1); assert.match(planRequests[0], /focus=1800&rest=300&goal=3600/);
    await tick();
    assert.equal(await evaluate(`document.getElementById('activityTitle').textContent`), '戴上眼鏡一起讀書');
    // 左邊固定：右邊捲動時左邊不動
    if (name === 'desktop') {
      const before = await evaluate(`document.getElementById('life').getBoundingClientRect().top`);
      await evaluate('window.scrollTo(0, document.body.scrollHeight)'); await delay(150);
      assert.ok(Math.abs(await evaluate(`document.getElementById('life').getBoundingClientRect().top`) - before) <= 1, 'life area stays fixed');
      assert.ok(await evaluate('window.scrollY') > 0, 'right panel scrolls');
      await evaluate('window.scrollTo(0,0)');
    }
    for (const mode of ['STUDYING', 'COMPUTER', 'PHONE', 'PRESENT']) {
      activity = mode; await tick(); await delay(500);
      assert.ok(await evaluate('document.documentElement.scrollWidth <= document.documentElement.clientWidth'));
      if (mode === 'PHONE') assert.equal(await evaluate(`document.getElementById('rewardStatus').textContent`), '獎勵已暫停');
      await shot(`web-${mode.toLowerCase()}-${name}`);
    }
    // 互動
    await evaluate(`document.getElementById('palmButton').click()`); await delay(200);
    assert.ok(await evaluate('petUntil>Date.now()'));
    await evaluate(`document.getElementById('toyButton').click()`);
    await evaluate(`document.querySelector('[data-toy=ball]').click()`); await delay(700);
    assert.equal(await evaluate('toy&&toy.kind'), 'ball'); await shot('web-toy-' + name);
    const oldFood = food, oldRequests = feedRequests;
    await evaluate('feed()'); await evaluate('feed()');
    assert.equal(food, oldFood - 1); assert.equal(feedRequests, oldRequests + 1);
    await delay(900); await shot('web-eat-' + name);
    // 休息：全畫面小雞
    activity = 'AWAY'; work = 1500; rest = 40; await tick();
    assert.equal(await evaluate(`document.getElementById('restOverlay').hidden`), false);
    await delay(3800); await shot('web-rest-' + name);
    await evaluate(`document.getElementById('restClose').click()`); assert.equal(await evaluate(`document.getElementById('restOverlay').hidden`), true);
    await shot('web-sleep-' + name);
    assert.ok(await evaluate(`document.querySelectorAll('#eventList li').length`) >= 4, 'events are logged');
    await evaluate('disconnect()'); const before = polls; await delay(1200);
    assert.equal(polls, before, 'disconnect stops requests');
  }
  assert.equal(errors.length, 0, JSON.stringify(errors));
  console.log('PASS: plan form ranges, fixed life area, study/computer/phone/rest scenes, toys, petting, feeding guard, event log, desktop/mobile');
}
main().catch(e => { console.error(e); process.exitCode = 1; }).finally(cleanup);
