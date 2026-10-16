
// 用 Node 跑，檢查網頁有沒有壞掉
const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');
const path = require('node:path');
const root = path.join(__dirname, '..');
const html = fs.readFileSync(path.join(root, 'web/pet_test.html'), 'utf8');
const ino = fs.readFileSync(path.join(root, 'Day3_StudyBuddy_Verify/Day3_StudyBuddy_Verify.ino'), 'utf8');
const header = ino.match(/Serial.println\("(CSV,t_s,[^"]+)"\)/)[1].split(',');
assert.equal(header.length, 36);
assert.doesNotMatch(html, /data-mode|data-sim|sendMode|simSend/);
assert.doesNotMatch(html, /navigator\.serial|requestPort|readSerial/);
assert.doesNotMatch(ino, /ActivityMode|MODE_STUDY|modeChar/);
const elements = new Map();
const element = id => {
  assert.ok(html.includes(`id="${id}"`), `missing element: ${id}`);
  if (!elements.has(id)) elements.set(id, { textContent: '', className: '', style: {},
    value: id === 'checkTarget' ? 'person' : '', classList: { toggle() {} }, click() {} });
  return elements.get(id);
};
let now = 10000, exported, interval;
let fetchImpl = async () => { throw new Error('offline'); };
class TestURL extends URL {
  static createObjectURL(blob) { exported = blob; return 'blob:test'; }
  static revokeObjectURL() {}
}
let chirps = 0;
class FakeAudio {
  constructor() { this.state = 'suspended'; this.currentTime = 0; this.destination = {}; }
  async resume() { this.state = 'running'; }
  createOscillator() { return { frequency: { setValueAtTime() {}, exponentialRampToValueAtTime() {} },
    connect() {}, disconnect() {}, start() { chirps++; }, stop() {} }; }
  createGain() { return { gain: { setValueAtTime() {}, exponentialRampToValueAtTime() {} }, connect() {}, disconnect() {} }; }
}
const context = vm.createContext({
  document: { getElementById: element, querySelectorAll: () => [], createElement: () => ({ click() {} }) },
  navigator: {}, console, Blob, TextEncoder, TextDecoderStream, ReadableStream, WritableStream, AudioContext: FakeAudio,
  Date: class extends Date { static now() { return now; } },
  URL: TestURL, AbortController,
  location: { protocol: 'http:', host: '192.168.1.88', assign(value) { this.assigned = value; } },
  fetch: (...args) => fetchImpl(...args),
  setTimeout: () => 0, clearTimeout() {}, setInterval(fn) { interval = fn; return 0; },
});
vm.runInContext(html.match(/<script>([\s\S]*?)<\/script>/)[1], context);
const run = code => vm.runInContext(code, context);
const send = line => context.handleLine(line);
const row = overrides => {
  const data = { CSV: 'CSV', t_s: 10, samples: 10, person_hits: 10, person_max: 90,
    book_hits: 0, book_max: 0, phone_hits: 0, phone_max: 0, laptop_hits: 0, laptop_max: 0,
    face_hits: 3, face_max: 85, face_pitch: 50, face_samples: 4, object_age_ms: 50,
    face_age_ms: 80, book_stable: 0, phone_stable: 0, laptop_stable: 0, interval_ms: 1000,
    book_threshold: 30, phone_threshold: 30, laptop_threshold: 40,
    table_hits: 0, table_max: 0, keyboard_hits: 0, keyboard_max: 0,
    mouse_hits: 0, mouse_max: 0, remote_hits: 0, remote_max: 0,
    book_assist_hits: 0, phone_assist_hits: 0, phone_near_person: 0, computer_context: 0, ...overrides };
  return header.map(key => data[key]).join(',');
};

async function main() {
  send(header.join(','));
  assert.equal(run('csvHistory.length'), 0, 'header is not a sample');
  element('checkTarget').value = 'book';
  send(row({ book_max: 27 }));
  assert.match(element('chipBook').textContent, /27 分 · 0%/);
  assert.match(element('placeHint').textContent, /未達 30 分/);
  assert.doesNotMatch(element('inferenceInfo').textContent, /正臉/);
  send(row({ book_max: 44 }));
  assert.match(element('placeHint').textContent, /範圍檢查/);
  send(row({ book_max: 60, book_hits: 5, book_stable: 1 }));
  assert.match(element('chipBook').textContent, /持續出現/);
  assert.match(element('recentInfo').textContent, /書 17%/);
  send(row({ samples: 0, person_hits: 0, face_samples: 0, face_hits: 0 }));
  assert.match(element('chipBook').textContent, /無新結果/);
  assert.match(element('placeHint').textContent, /沒有新物件推論/);
  send(row({ object_age_ms: 2001 }));
  assert.match(element('placeHint').textContent, /逾時/);
  element('checkTarget').value = 'phone';
  send(row({ phone_max: 0 }));
  assert.match(element('placeHint').textContent, /未回傳手機候選/);
  send(row({ phone_max: 55, phone_hits: 7, phone_stable: 1 }));
  assert.match(element('chipPhone').textContent, /70%.*持續出現/);
  send('STATE,10,PRESENT,100,20,30,A,1,22,U,0,0,1');
  assert.match(element('phoneNotice').textContent, /手機/);
  assert.equal(element('deskTime').textContent, '1:40');
  send('STATE,10,STUDYING,100,20,30,A,1,22,D,1200,1,1,1');
  assert.equal(element('pet').className, 'pet studying', 'legacy head flags do not trigger reminders');
  assert.doesNotMatch(element('msg').textContent, /抬頭|低頭/);
  assert.match(element('modeText').textContent, /自動判斷：物件＋位置關聯/);
  assert.match(element('phoneNotice').textContent, /不代表正在使用/);
  send('STATE,10,STUDYING,999,999,999,S,1,22,-,0,0,1');
  assert.equal(element('stateText').textContent, '無法確認');
  assert.equal(element('deskTime').textContent, '1:40', 'manual legacy packet cannot overwrite counters');
  assert.match(element('modeText').textContent, /非自動狀態/);
  send('STATE,11,PRESENT,100,20,30,A,1,22,U,0,0');
  assert.equal(element('phoneNotice').textContent, '', 'old firmware has no phone flag');
  send('STATE,12,INVALID,100,20,30,A,1,22,U,0,0,1');
  assert.equal(element('stateText').textContent, '在桌前');
  send(row({ table_hits: 8, table_max: 70, keyboard_hits: 5, keyboard_max: 80,
    book_assist_hits: 2, phone_assist_hits: 3, computer_context: 1 }));
  assert.match(element('chipTable').textContent, /70 分 · 80%/);
  assert.match(element('chipKeyboard').textContent, /80 分 · 50%/);
  assert.match(element('relationInfo').textContent, /書 2 次、手機 3 次.*持續成立/);
  send('CSV,12,10,10,88,3,41,0,0,0,0,8,72,55');
  assert.match(element('inferenceInfo').textContent, /舊版韌體/);
  assert.match(element('relationInfo').textContent, /沒有關聯欄位/);
  element('btnExport').onclick();
  const lines = (await exported.text()).trim().split(/\r?\n/);
  assert.ok(lines.length > 2);
  for (const line of lines) assert.equal(line.split(',').length, 36, 'export columns align, including legacy rows');
  now += 3100; interval();
  assert.match(element('stateText').textContent, /逾時/);
  assert.match(element('placeHint').textContent, /舊資料/);
  assert.equal(element('phoneNotice').textContent, '');
  assert.match(element('relationInfo').textContent, /逾時/);
  now += 1800001; send(row({}));
  assert.equal(run('csvHistory.length'), 1, 'history expires after 30 minutes');

  let requested = 0, seq = 1, cursor = 0, boardMs = 1000, responseExtra = '';
  const payload = () => `NET,${seq},${cursor},${boardMs}\n` +
    row({ phone_hits: 5, phone_max: 65, phone_stable: 1 }) +
    '\nSTATE,15,PRESENT,120,20,30,A,1,22,-,0,0,1,0\nPET,1000,HEALTHY,100,1,0,0,0,1,0,0,60,10,30,1,1\n' + responseExtra;
  const fetchOK = async (url, options) => {
    requested++;
    assert.match(url, /^\/api\/poll\?after=-?\d+$/);
    assert.equal(options.method, 'POST');
    assert.equal(options.headers['X-StudyBuddy'], '1');
    return { ok: true, async text() { return payload(); } };
  };
  fetchImpl = fetchOK;
  await context.connect();
  await context.connect();
  assert.equal(requested, 1, 'double connect is ignored');
  assert.equal(element('deskTime').textContent, '2:00');
  assert.match(element('phoneNotice').textContent, /手機/);
  await context.pollNetwork();
  assert.equal(run('csvHistory.length'), 1, 'duplicate snapshot is not another inference sample');
  const lastAt = run('lastCsvAt');
  now += 4000; await context.pollNetwork();
  assert.equal(run('lastCsvAt'), lastAt, 'HTTP success cannot hide stopped inference');
  seq++; boardMs += 1000; await context.pollNetwork();
  assert.equal(run('csvHistory.length'), 2);

  send('PET,10000,HEALTHY,100,50,0,0,0,1,0,0,60,10,30,1,1');
  assert.match(element('petSettings').textContent, /展示設定/);
  assert.equal(element('petEnergy').textContent, '100 / 100');
  assert.equal(chirps, 0);
  send('PET,11000,TIRED,100,60,0,0,0,1,1,0,60,10,30,1,1');
  assert.equal(chirps, 3, 'one reminder produces three short chirps');
  send('PET,12000,TIRED,97,61,0,0,0,1,1,0,60,10,30,1,1');
  assert.equal(chirps, 3, 'repeated telemetry does not repeat sound');
  send('PET,13000,WEAK,50,75,0,0,0,1,1,0,60,10,30,1,1');
  assert.equal(chirps, 5, 'weak stage adds two chirps');
  send('PET,13500,WEAK,49,75,0,0,0,1,1,0,60,10,30,1,1');
  assert.equal(chirps, 5, 'weak stage only alerts once');
  assert.match(element('pet').className, /weak/);
  send('EVENT,14000,1,DIED'); send('EVENT,14000,1,DIED');
  assert.equal(run('petEvents.length'), 1, 'duplicate event ignored');
  send('PET,14000,DEAD,0,90,0,0,0,1,1,0,60,10,30,1,1');
  assert.match(element('stage').textContent, /死亡/);
  assert.match(element('pet').className, /dead/);
  send('PET,15000,DEAD,0,90,0,0,0,1,1,1,60,10,30,1,1');
  assert.match(element('petLifeText').textContent, /暫停/);
  send('PET,16000,HEALTHY,100,0,10,0,1,2,1,0,60,10,30,1,0');
  assert.match(element('stage').textContent, /第 2 代 · 蛋/);
  send('PET,17000,HEALTHY,100,0,0,3,4,2,1,0,60,10,30,1,1');
  assert.match(element('stage').textContent, /成年/);
  send('PET,18000,HEALTHY,999,0,0,3,4,2,1,0,60,10,30,1,1');
  assert.equal(run('life.energy'), 100, 'invalid energy ignored');
  await element('btnSound').onclick();
  send('PET,19000,TIRED,100,60,0,3,4,2,2,0,60,10,30,1,1');
  assert.equal(chirps, 5, 'mute suppresses audio');
  await element('btnSound').onclick();
  send('PET,20000,TIRED,100,60,0,3,4,2,3,1,60,10,30,1,1');
  assert.equal(chirps, 5, 'paused state suppresses sound');
  now += 3100; interval();
  if (run('networkTask')) await run('networkTask');
  assert.match(element('petLifeText').textContent, /暫停/);
  element('btnEvents').onclick();
  assert.match(await exported.text(), /board_ms,event_id,event.*\r\n.*14000,1,DIED/);
  run('life = null');
  send('PET,21000,TIRED,90,63,0,3,4,2,3,0,60,10,30,1,1');
  assert.equal(chirps, 5, 'first snapshot after reconnect never replays old reminders');
  await context.disconnect();
  assert.equal(run('networkEnabled'), false);
  assert.equal(element('btnConnect').disabled, false);
  assert.match(element('petLifeText').textContent, /暫停/);
  await context.connect();
  fetchImpl = async () => { throw new Error('WiFi down'); };
  await context.pollNetwork();
  assert.match(element('conn').textContent, /重試/);
  assert.equal(run('suppressSound'), true);
  fetchImpl = fetchOK; seq++; boardMs += 1000;
  await context.pollNetwork();
  assert.match(element('conn').textContent, /Wi-Fi 已連線/);
  seq = 1; boardMs = 1;
  await context.pollNetwork();
  assert.equal(run('csvHistory.length'), 1, 'board reboot starts a new inference history');
  assert.equal(run('life.reminders'), 0);

  // 拍手只跳不叫，也不重播舊互動
  const beforeClap = chirps;
  now += 6000; boardMs = 30000; seq++; cursor = 2;
  responseExtra = 'EVENT,30000,2,CLAP\n';
  await context.pollNetwork();
  assert.equal(chirps, beforeClap, 'fresh clap is silent');
  assert.equal(run('welcomeUntil'), now + 2000, 'fresh clap triggers animation');
  await context.pollNetwork();
  assert.equal(chirps, beforeClap, 'duplicate clap is silent');
  now += 1000;
  boardMs += 1000; seq++; cursor++;
  responseExtra = `EVENT,${boardMs},${cursor},CLAP\n`;
  await context.pollNetwork();
  assert.equal(chirps, beforeClap, 'clap during cooldown is silent');
  assert.equal(run('welcomeUntil'), now + 1000, 'cooldown does not restart animation');
  now += 6000; boardMs += 6000; seq++; cursor++;
  responseExtra = `EVENT,${boardMs - 3000},${cursor},CLAP\n`;
  await context.pollNetwork();
  assert.equal(chirps, beforeClap, 'delayed clap is not replayed');
  assert.ok(run('welcomeUntil') < now, 'delayed clap does not animate');
  fetchImpl = async () => { throw new Error('WiFi down'); };
  await context.pollNetwork();
  fetchImpl = fetchOK; boardMs += 1000; seq++; cursor++;
  responseExtra = `EVENT,${boardMs},${cursor},CLAP\n`;
  await context.pollNetwork();
  assert.equal(chirps, beforeClap, 'recovery snapshot does not replay clap after PET');
  assert.ok(run('welcomeUntil') < now, 'recovery does not replay animation');
  boardMs += 1000; seq++; cursor++;
  responseExtra = `EVENT,${boardMs},${cursor},CLAP\n`;
  await context.pollNetwork();
  assert.equal(chirps, beforeClap, 'next live clap stays silent');
  assert.equal(run('welcomeUntil'), now + 2000, 'next live clap animates after recovery');
  now += 6000;
  run('life.phase = "DEAD"; lastBoardMs = 50000');
  send('EVENT,50000,99,CLAP');
  assert.equal(chirps, beforeClap, 'dead chick does not answer clap');
  run('life.phase = "HEALTHY"; lifeAt = Date.now(); current.state = "UNSURE"');
  send('EVENT,50000,100,CLAP');
  assert.equal(chirps, beforeClap, 'uncertain presence does not answer clap');

  let releaseRequest;
  fetchImpl = (url, { signal }) => new Promise((resolve, reject) => {
    releaseRequest = resolve;
    signal.addEventListener('abort', () => reject(new Error('aborted')));
  });
  const pending = context.pollNetwork();
  await context.disconnect(); await pending;
  assert.equal(run('networkTask'), null, 'disconnect aborts pending poll');
  assert.equal(run('networkEnabled'), false);
  element('boardAddress').value = '192.168.1.99'; context.openBoard();
  assert.equal(context.location.assigned, 'http://192.168.1.99/');
  element('boardAddress').value = 'javascript:alert(1)'; context.openBoard();
  assert.match(element('conn').textContent, /網址無法開啟/);
  context.location.protocol = 'file:'; await context.connect();
  assert.equal(run('networkEnabled'), false, 'local file directs user to board-hosted page');
  console.log('PASS: Wi-Fi polling/retry/cancel/reboot, snapshot dedup, automatic-only state, pet life/audio/events, legacy compatibility, board URL validation');
}
main().catch(error => { console.error(error); process.exitCode = 1; });
