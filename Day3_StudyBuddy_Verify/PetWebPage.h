// 由 scripts/embed-web.cjs 產生，請修改 web/pet_test.html
#pragma once
static const char PET_WEB_PAGE[] = R"PETPAGE(<!doctype html>
<html lang="zh-Hant">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>讀書小夥伴 寵物測試</title>
<style>
  :root {
    /* 底色：暖米白；主色：深青綠（按鈕）；點綴：琥珀黃（成長條、小雞） */
    --bg: #fbf7ef; --card: #ffffff; --text: #2d2a26; --muted: #7a7368;
    --line: #ece4d6; --accent: #2f6f5e; --accent-hover: #255a4c; --warm: #f2b53a;
    --soft: #f6f1e7;
    --st-studying: #e3f3ec; --st-computer: #ece8f8; --st-present: #f8f2e4;
    --st-away: #e9ecf1; --st-unsure: #e4eef9;
  }
  * { box-sizing: border-box; }
  body { margin: 0; background: var(--bg); color: var(--text);
         font-family: "Noto Sans TC", "Microsoft JhengHei", sans-serif; }
  main { max-width: 760px; margin: 0 auto; padding: 28px 16px 48px; }
  h1 { font-size: 22px; margin: 0 0 4px; letter-spacing: .5px; }
  .sub { color: var(--muted); font-size: 14px; margin: 0 0 20px; }
  .card { background: var(--card); border: 1px solid var(--line); border-radius: 16px;
          padding: 18px; margin-bottom: 16px; box-shadow: 0 1px 2px rgba(60, 45, 20, .05); }
  .pet-card { text-align: center; transition: background 1.2s ease; }
  /* 像素小雞：16×16 的 SVG 放大，保持方塊邊緣 */
  .pet { display: inline-block; width: 128px; height: 128px; margin-top: 4px; transition: transform .8s ease, opacity .8s; }
  .pet svg { width: 100%; height: 100%; display: block; shape-rendering: crispEdges; }
  .pet.studying { animation: bob 3s steps(4) infinite; }
  .pet.computer { animation: type 1.2s steps(2) infinite; }
  @keyframes type { 50% { transform: translateX(4px); } }
  .pet.away { opacity: .75; transform: translateY(6px); }
  .pet.tired { animation: bob 5s steps(3) infinite; opacity: .8; }
  .pet.weak { transform: rotate(12deg); opacity: .6; }
  .pet.dead { filter: grayscale(1); transform: rotate(90deg); opacity: .5; }
  .pet.welcome { animation: bob .5s steps(4) 3; }
  .pet.paused { animation: none; }
  @keyframes bob { 50% { transform: translateY(-8px); } }
  .ground { width: 140px; height: 6px; margin: 0 auto; border-radius: 3px; background: rgba(60, 45, 20, .1); }
  .badge { font-size: 26px; height: 32px; }
  .stage { color: var(--muted); font-size: 14px; }
  .state { display: inline-block; font-size: 18px; font-weight: 700; margin: 10px 0 6px; padding: 4px 14px;
           border-radius: 999px; background: rgba(255, 255, 255, .75); }
  .msg { font-size: 15px; color: #4d473f; min-height: 22px; }
  .stats { display: grid; grid-template-columns: repeat(2, 1fr); gap: 10px; margin-top: 14px; }
  .stat { background: rgba(255, 255, 255, .7); border: 1px solid var(--line); border-radius: 12px; padding: 10px; }
  .stat b { display: block; font-size: 20px; color: var(--accent); font-variant-numeric: tabular-nums; }
  .stat span { color: var(--muted); font-size: 13px; }
  .bar { height: 10px; background: rgba(60, 45, 20, .08); border-radius: 5px; overflow: hidden; margin-top: 12px; }
  .bar > div { height: 100%; width: 0; background: var(--warm); transition: width .6s; }
  h2 { font-size: 16px; margin: 0 0 10px; color: var(--accent); }
  .row { display: flex; flex-wrap: wrap; gap: 8px; align-items: center; }
  button { font: inherit; font-size: 14px; padding: 8px 14px; border-radius: 10px;
           border: 1px solid var(--line); background: var(--card); color: var(--text); cursor: pointer; }
  button:hover { border-color: var(--accent); }
  button:disabled { opacity: .5; cursor: default; }
  button.primary { background: var(--accent); color: #fff; border-color: var(--accent); }
  button.primary:hover { background: var(--accent-hover); }
  #conn { font-size: 14px; color: var(--muted); }
  pre { background: #23262b; color: #d7e7df; font-size: 12px; padding: 10px; border-radius: 10px;
        height: 160px; overflow: auto; margin: 0; white-space: pre-wrap; word-break: break-all; }
  .note { color: var(--muted); font-size: 13px; margin: 8px 0 0; }
  .chips { display: flex; flex-wrap: wrap; gap: 8px; }
  .chip { border-radius: 999px; padding: 6px 12px; font-size: 14px; background: var(--soft); color: var(--muted); }
  .chip.on { background: #dff2ea; color: #1f5c4a; font-weight: 700; }
  .chip.weak { background: #fdf0d2; color: #7a5410; }
  .hint { font-size: 14px; margin: 10px 0 0; padding: 8px 12px; border-radius: 10px; background: var(--soft); }
  select, input { font: inherit; padding: 7px; border: 1px solid var(--line); border-radius: 8px; background: var(--card); color: var(--text); max-width: 100%; }
  .setup { font-size: 14px; line-height: 1.8; padding-left: 22px; }
  #phoneNotice { color: #7a5410; min-height: 22px; font-size: 14px; margin-top: 8px; }
  #micNotice { color: var(--muted); min-height: 20px; font-size: 13px; }
  #micNotice.noisy { color: #8a3b12; font-weight: 700; }
  @media (max-width: 480px) { .stats { grid-template-columns: 1fr; } .pet { width: 104px; height: 104px; } }
</style>
</head>
<body>
<main>
  <h1>讀書小夥伴：寵物測試頁</h1>
  <p class="sub">自動陪你讀書與休息。到時以小雞叫聲提醒；拍兩下手，小雞會叫一聲回應。</p>

  <section class="card pet-card" id="petCard">
    <div class="badge" id="badge"></div>
    <div class="pet" id="pet" role="img" aria-label="像素小雞"></div>
    <div class="ground"></div>
    <div class="stage" id="stage">蛋</div>
    <div class="state" id="stateText">等待資料</div>
    <div class="msg" id="msg">連接開發板後，自動判斷桌前活動。</div>
    <div id="phoneNotice"></div>
    <div id="micNotice"></div>
    <div class="stats">
      <div class="stat"><b id="deskTime">0:00</b><span>累計在桌前</span></div>
      <div class="stat"><b id="modeTime">0:00／0:00</b><span>其中 讀書／電腦</span></div>
      <div class="stat"><b id="awayCount">0 次</b><span>離席次數</span></div>
      <div class="stat"><b id="lastAway">—</b><span>上次離席多久</span></div>
    </div>
    <div class="bar"><div id="growBar"></div></div>
    <div class="stage" id="growText"></div>
    <div class="stage" id="lastUpdate"></div>
    <p class="hint" id="petLifeText">等待第二週韌體資料</p>
    <div class="stats">
      <div class="stat"><b id="petEnergy">—</b><span>小雞活力</span></div>
      <div class="stat"><b id="petWork">—</b><span>本輪在桌前／提醒門檻</span></div>
      <div class="stat"><b id="petRest">—</b><span>有效離席／休息門檻</span></div>
      <div class="stat"><b id="petCycles">—</b><span>本代學習＋休息輪數</span></div>
    </div>
    <p class="note" id="petSettings">遊戲活力不是健康評分。看不到人只代表離席推測。</p>
  </section>

  <section class="card">
    <h2>Wi-Fi 連接開發板</h2>
    <div class="row" style="margin-bottom:10px">
      <label for="boardAddress">Ameba IP</label>
      <input id="boardAddress" placeholder="例如 192.168.1.100" inputmode="url" aria-label="Ameba IP 位址">
      <button id="btnOpenBoard">開啟板子網頁</button>
    </div>
    <div class="row">
      <button class="primary" id="btnConnect">開始 Wi-Fi 連線</button>
      <button id="btnDisconnect">中斷</button>
      <button id="btnSound">叫聲：開啟</button>
      <span id="conn">未連線</span>
    </div>
    <p class="note">電腦／手機與 Ameba 連到同一個 Wi-Fi，開啟 http://板子IP/ 後按「開始 Wi-Fi 連線」。板子仍需供電，不需要 USB 資料線。IP 可從路由器裝置清單、VLC 畫面或首次燒錄後的除錯輸出查看。</p>
    <p class="note" id="soundInfo">連線時啟用音效，聲音由電腦播放。保持網頁開啟；斷線時暫停養成。</p>
    <p class="note">板上麥克風只量音量，不錄音也不辨識說話內容。連拍兩下手會觸發小雞回應；周圍持續吵雜時顯示提示，不影響計時。</p>
  </section>

  <section class="card">
    <h2>自動判斷</h2>
    <p class="note" id="modeText">等待自動辨識資料</p>
    <p class="note">書本候選搭配附近的人與桌面，可補足部分低分結果。筆電，或同一人附近的鍵盤＋滑鼠，是電腦線索；與書本同時成立時只顯示「在桌前」。手機會排除較強的遙控器／滑鼠重疊候選，並提示是否靠近人。所有結果都需持續出現，手機靠近人不等於正在使用。</p>
  </section>

  <section class="card">
    <h2>擺放檢查（即時）</h2>
    <div class="row" style="margin-bottom:12px">
      <label for="checkTarget">正在校正</label>
      <select id="checkTarget"><option value="person">人在桌前</option><option value="book">書本（請放入測試區）</option><option value="phone">手機（請放入測試區）</option></select>
      <button id="btnExport">匯出辨識 CSV</button>
    </div>
    <div class="chips">
      <span class="chip" id="chipPerson">人 —</span>
      <span class="chip" id="chipBook">書 —</span>
      <span class="chip" id="chipPhone">手機 —</span>
      <span class="chip" id="chipLaptop">筆電 —</span>
      <span class="chip" id="chipTable">桌子 —</span>
      <span class="chip" id="chipKeyboard">鍵盤 —</span>
      <span class="chip" id="chipMouse">滑鼠 —</span>
      <span class="chip" id="chipRemote">遙控器 —</span>
    </div>
    <p class="hint" id="placeHint">連上開發板後，坐到平常讀書的位置，依提示調整鏡頭。</p>
    <p class="note" id="inferenceInfo">等待推論資料</p>
    <p class="note" id="recentInfo">最近 30 秒：等待資料</p>
    <p class="note" id="relationInfo">等待關聯判斷資料</p>
    <p class="note">每個標籤顯示原始最高分與通過門檻的比例；低分或桌面區外候選也保留最高分。分數不是準確率。CSV 保留最近 30 分鐘，含無目標時段，方便比較誤判。</p>
  </section>

  <section class="card">
    <h2>固定擺放規格（先依此實測）</h2>
    <ol class="setup">
      <li>單人使用，鏡頭固定在桌前正中央、約眼睛高度，距離臉部約 60～80 公分；以水平線向下俯 30° 起始，允許在 25～35° 內調整。</li>
      <li>VLC 畫面需同時看見肩膀、上半身及桌面，不必刻意面向鏡頭。若無法同時入鏡，先調整支架位置；角度是起始限制，並非保證最佳值。</li>
      <li>書本、鍵盤、滑鼠的中心放在畫面青色框內（左右 10～90%、上下 38～98%）。手機可在青色框內，或靠近畫面中人的身體範圍；人與桌子使用全畫面辨識。</li>
      <li>書本先用閱讀架與桌面成約 30～45°，讓封面或攤開的頁面清楚入鏡，寬度至少約占畫面 1/4。手機正面或背面朝鏡頭，長邊至少約占畫面高度 1/5，避免只露側邊或被手遮住。</li>
      <li>固定檯燈位置，避免螢幕反光、逆光與晃動。用膠帶標記支架、書架及手機測試位置；平放、手持與遮擋另外測試。</li>
    </ol>
    <p class="note">固定規格用於縮小測試範圍；網頁無法測量實際角度或保證辨識成功。桌子只是輔助線索，不要求每次都辨識到。現有模型沒有筆、紙或手部類別。</p>
  </section>

  <section class="card">
    <h2>小雞事件紀錄</h2>
    <button id="btnEvents">匯出事件 CSV</button>
    <pre id="petEvents" aria-live="polite"></pre>
    <p class="note">保留本頁最近 300 筆事件；重新整理會清空。板子重開機也會重設養成進度。</p>
  </section>

  <section class="card">
    <h2>網路原始資料</h2>
    <pre id="log"></pre>
  </section>
</main>

<script>

// 像素小雞：每列 16 格，K 輪廓、Y 身體、L 亮部、O 嘴腳、P 腮紅、W 蛋殼、S 陰影、H 反光
const PALETTE = { K: "#3b2a1a", Y: "#ffd447", L: "#fff1a6", O: "#f28c28", P: "#ff9aa2",
                  W: "#fdf3dc", S: "#e8d6ad", H: "#ffffff", Z: "#6f95c9" };
// 睡覺時右上角的像素 Z（列, 起始欄, 圖樣）
const SLEEP_Z = [[0, 12, "ZZZZ"], [1, 14, "Z"], [2, 13, "Z"], [3, 12, "ZZZZ"]];
const SPRITES = {
  egg: [
    "................", "......KKKK......", ".....KWWWWK.....", "....KWHWWWWK....",
    "...KWHWWWWWWK...", "...KWHWWWWWWK...", "..KWWWWWWWWWWK..", "..KWWWWWSWWWWK..",
    "..KWWWWWWWWWWK..", "..KWWSWWWWWWSK..", "..KWWWWWWWWWSK..", "...KWWWWWWWSK...",
    "...KSWWWWWWSSK..", "....KSSSSSSK....", ".....KKKKKK.....", "................",
  ],
  baby: [
    "................", "......KKKK......", ".....KYYYYK.....", "....KYYYYYYK....",
    "....KYKYYKYK....", "....KPYOOYPK....", "....KYYYYYYK....", "...KYYYYYYYYK...",
    "..KYYYYYYYYYYK..", "..KWKWWKKWWKWK..", "..KWWWWWWWWWWK..", "..KWSWWWWWWSWK..",
    "...KWWWWWWWWK...", "....KKKKKKKK....", "................", "................",
  ],
  adult: [
    "................", "......KK........", ".....KYYKKK.....", "....KYYYYYYK....",
    "...KYYYYYYYYK...", "...KYYKYYKYYK...", "...KYPYOOYPYK...", "...KYYYOOYYYK...",
    "..KYYYYYYYYYYK..", ".KYYLLYYYYYYYYK.", ".KYYLLLYYYYYYYK.", ".KYYYLLYYYYYYYK.",
    "..KYYYYYYYYYYK..", "...KKKKKKKKKK...", ".....KO..OK.....", ".....OO..OO.....",
  ],
};
// 睡覺時把眼睛那一列換成閉眼的橫線
const SLEEP_EYES = { baby: [4, "....KKKYYKKK...."], adult: [5, "...KYKKYYKKYK..."] };

function pixelSvg(name, sleeping) {
  const rows = SPRITES[name].slice();
  if (sleeping && SLEEP_EYES[name]) rows[SLEEP_EYES[name][0]] = SLEEP_EYES[name][1];
  if (sleeping) SLEEP_Z.forEach(([y, x, s]) => {
    rows[y] = rows[y].slice(0, x) + [...s].map((c, i) => rows[y][x + i] === "." ? c : rows[y][x + i]).join("") + rows[y].slice(x + s.length);
  });
  let rects = "";
  rows.forEach((row, y) => [...row].forEach((c, x) => {
    if (PALETTE[c]) rects += `<rect x="${x}" y="${y}" width="1" height="1" fill="${PALETTE[c]}"/>`;
  }));
  return `<svg viewBox="0 0 16 16" xmlns="http://www.w3.org/2000/svg">${rects}</svg>`;
}

// 先用短時間測成長
const STAGES = [
  { min: 0,   pet: "egg",   name: "蛋" },
  { min: 60,  pet: "baby",  name: "幼年" },
  { min: 300, pet: "adult", name: "成年" },
];

const STATES = {
  STUDYING: { text: "讀書中", msg: "小夥伴在旁邊陪你讀書。", cls: "studying", bg: "--st-studying" },
  COMPUTER: { text: "用電腦中", msg: "小夥伴在旁邊陪你打字。", cls: "computer", bg: "--st-computer" },
  PRESENT:  { text: "在桌前", msg: "小夥伴在旁邊陪你。", cls: "", bg: "--st-present" },
  AWAY:     { text: "未偵測到使用者", msg: "小夥伴趴著睡覺，等你回來。💤", cls: "away", bg: "--st-away" },
  UNSURE:   { text: "無法確認", msg: "畫面不太清楚，這段時間不列入累計。", cls: "", bg: "--st-unsure" },
};

const $ = id => document.getElementById(id);
const blank = () => ({ state: null, deskS: 0, studyS: 0, computerS: 0, awayCount: 0, lastAwayS: 0,
                       phone: false, phoneNear: false, mic: -1, micPeak: -1, noisy: false });
let current = blank();

function fmt(sec) {
  const h = Math.floor(sec / 3600), m = Math.floor(sec % 3600 / 60), s = sec % 60;
  return (h ? h + ":" + String(m).padStart(2, "0") : m) + ":" + String(s).padStart(2, "0");
}

// 把板子狀態畫在頁面上
function render() {
  const st = STATES[current.state];
  let stageIdx = 0;
  STAGES.forEach((s, i) => { if (current.deskS >= s.min) stageIdx = i; });
  const stage = STAGES[stageIdx], next = STAGES[stageIdx + 1];

  $("pet").innerHTML = pixelSvg(stage.pet, current.state === "AWAY");
  $("pet").className = "pet " + (st ? st.cls : "");
  $("stage").textContent = stage.name;
  $("deskTime").textContent = fmt(current.deskS);
  $("modeTime").textContent = fmt(current.studyS) + "／" + fmt(current.computerS);
  $("awayCount").textContent = current.awayCount + " 次";
  $("lastAway").textContent = current.awayCount ? fmt(current.lastAwayS) : "—";

  if (st) {
    $("stateText").textContent = st.text;
    $("msg").textContent = st.msg;
    $("petCard").style.background = `var(${st.bg})`;
  }
  $("badge").textContent = current.state === "UNSURE" ? "❓" : "";
  $("phoneNotice").textContent = !current.phone ? '' : current.phoneNear
    ? '📱 手機持續出現在人附近；位置關聯不代表正在使用，不影響讀書計時。'
    : '📱 看到手機；尚未判定是否使用，不影響讀書計時。';
  $("micNotice").textContent = current.mic < 0 ? '麥克風：無資料' : current.noisy
    ? `🔊 周圍有點吵（音量 ${current.mic}），小雞摀住耳朵；不影響計時。`
    : `🎙 麥克風：安靜（音量 ${current.mic}，最高 ${current.micPeak}）· 拍兩下手跟小雞打招呼`;
  $("micNotice").className = current.noisy ? 'noisy' : '';

  if (next) {
    const pct = (current.deskS - stage.min) / (next.min - stage.min) * 100;
    $("growBar").style.width = Math.min(100, pct) + "%";
    $("growText").textContent = `再陪伴 ${fmt(next.min - current.deskS)} 成長為「${next.name}」`;
  } else {
    $("growBar").style.width = "100%";
    $("growText").textContent = "已成長到最終階段";
  }
  $("lastUpdate").textContent = "最後更新 " + new Date().toLocaleTimeString();
  renderLife();
}

let life = null, lifeAt = 0, welcomeUntil = 0, soundEnabled = true, audioContext = null;
let eventSegment = 0, lastEventMs = -1;
let networkEnabled = false, networkTask = null, networkAbort = null, networkEpoch = 0;
let lastSequence = -1, lastBoardMs = -1, eventCursor = -1, suppressSound = true;
let replayingEvents = false, clapQuietUntil = 0;
const petEvents = [], eventKeys = new Set();
const PET_PHASES = ['HEALTHY', 'TIRED', 'WEAK', 'DEAD'];
const EVENT_NAMES = { WELCOME: '回座歡迎', LEAVE: '離席睡覺', REMIND: '休息提醒',
  WEAK: '活力不足', DIED: '小雞死亡', RESTED: '完成休息', REBORN: '重新孵蛋',
  CLAP: '拍手互動', NOISY: '環境變吵', QUIET: '恢復安靜' };

// 先經過點擊，瀏覽器才允許出聲
async function enableAudio() {
  if (!soundEnabled) return;
  try {
    const Audio = globalThis.AudioContext || globalThis.webkitAudioContext;
    if (!Audio) { $('soundInfo').textContent = '目前無法播放叫聲，仍會顯示休息提醒。'; return; }
    if (!audioContext) audioContext = new Audio();
    await audioContext.resume();
    $('soundInfo').textContent = '到休息時間提醒一次，繼續久坐到虛弱再提醒一次；拍手只跳躍，不出聲。';
  } catch (e) { $('soundInfo').textContent = '音效未啟用，可按叫聲按鈕重試；文字提醒仍有效。'; }
}

// 只在兩個休息提醒階段出聲
function chirp(kind = 'remind') {
  if (!soundEnabled || !audioContext || audioContext.state !== 'running') return;
  const count = kind === 'weak' ? 2 : 3;
  try {
    // 播完先等一下，避免麥克風聽到自己就跳
    clapQuietUntil = Date.now() + 5000;
    for (let i = 0; i < count; i++) {
      const start = audioContext.currentTime + i * .22;
      const osc = audioContext.createOscillator(), gain = audioContext.createGain();
      osc.type = 'sine';
      osc.frequency.setValueAtTime(2200, start);
      osc.frequency.exponentialRampToValueAtTime(3600, start + .06);
      osc.frequency.exponentialRampToValueAtTime(1900, start + .14);
      gain.gain.setValueAtTime(.001, start);
      gain.gain.exponentialRampToValueAtTime(.10, start + .02);
      gain.gain.exponentialRampToValueAtTime(.001, start + .15);
      osc.connect(gain); gain.connect(audioContext.destination);
      osc.onended = () => { osc.disconnect(); gain.disconnect(); };
      osc.start(start); osc.stop(start + .16);
    }
  } catch (e) { $('soundInfo').textContent = '叫聲播放失敗，請查看文字提醒。'; }
}
$('btnSound').onclick = async () => {
  soundEnabled = !soundEnabled;
  $('btnSound').textContent = soundEnabled ? '叫聲：開啟' : '叫聲：靜音';
  if (soundEnabled) await enableAudio();
};

function renderLife() {
  if (!life) return;
  const stale = Date.now() - lifeAt > 3000;
  const paused = life.paused || stale;
  const dead = life.phase === 'DEAD', sleeping = current.state === 'AWAY';
  const name = dead || !life.started ? 'egg' : life.cycles >= 3 ? 'adult' : 'baby';
  $('pet').innerHTML = pixelSvg(name, sleeping && !dead);
  const activity = current.state === 'STUDYING' ? 'studying' : current.state === 'COMPUTER' ? 'computer' : '';
  $('pet').className = 'pet ' + (paused ? 'paused' : dead ? 'dead' : sleeping ? 'away' :
    life.phase === 'WEAK' ? 'weak' : life.phase === 'TIRED' ? 'tired' :
    Date.now() < welcomeUntil ? 'welcome' : activity);
  $('stage').textContent = `第 ${life.generation} 代 · ${dead ? '死亡，等待休息後重生' : name === 'egg' ? '蛋' : name === 'baby' ? '幼年' : '成年'}`;
  $('badge').textContent = paused ? '⏸' : dead ? '💀' : sleeping ? '💤' :
    life.phase !== 'HEALTHY' ? '🥱' : Date.now() < welcomeUntil ? '🐣' : current.noisy ? '🙉' :
    current.state === 'STUDYING' ? '📖' : current.state === 'COMPUTER' ? '💻' : '';
  $('petLifeText').textContent = paused ? '養成暫停：等待穩定辨識與連線，不扣活力也不計休息。' :
    dead ? '小雞已死亡。完成有效離席休息後會變回蛋，回座後重新孵化。' :
    sleeping ? (life.rest >= life.restGoal ? '休息完成，回座後再一起開始。' : '小雞陪你休息；連續離席達門檻才算完成。') :
    life.phase === 'WEAK' ? '小雞很虛弱了，離開桌前休息一下吧。' :
    life.phase === 'TIRED' ? '這輪已到休息時間，小雞也累了。' : '小雞正在陪伴你；完成學習與休息才能成長。';
  $('petEnergy').textContent = life.energy + ' / 100';
  $('petWork').textContent = fmt(life.work) + ' / ' + fmt(life.remindGoal);
  $('petRest').textContent = fmt(life.rest) + ' / ' + fmt(life.restGoal);
  $('petCycles').textContent = life.cycles + ' 輪';
  $('growBar').style.width = Math.min(100, life.cycles / 3 * 100) + '%';
  $('growText').textContent = dead ? '完成休息後重新養成，桌前總紀錄保留。' :
    life.cycles >= 3 ? '已成年，仍需要定時休息。' : `再完成 ${3 - life.cycles} 輪學習＋休息成為成年小雞`;
  $('petSettings').textContent = `${life.demo ? '展示設定' : '一般設定'}：${fmt(life.remindGoal)} 提醒、休息 ${fmt(life.restGoal)}；超時在座 ${fmt(life.decay)} 活力耗盡。這是遊戲設定，不是健康診斷。`;
}

function handleLife(f) {
  if (f.length !== 16 || !PET_PHASES.includes(f[2])) return;
  const values = f.slice(1).filter((_, i) => i !== 1).map(Number);
  if (values.some(n => !Number.isSafeInteger(n) || n < 0)) return;
  const n = i => Number(f[i]);
  if (n(3) > 100 || !n(11) || !n(12) || !n(13) || !n(8) || n(10) > 1 || n(14) > 1 || n(15) > 1) return;
  const next = { uptime: n(1), phase: f[2], energy: n(3), work: n(4), rest: n(5),
    cycles: n(6), rests: n(7), generation: n(8), reminders: n(9), paused: !!n(10),
    remindGoal: n(11), restGoal: n(12), decay: n(13), demo: !!n(14), started: !!n(15) };
  // 重連只同步狀態，不補播舊提醒
  if (life && next.uptime >= life.uptime && next.generation === life.generation &&
      !next.paused && networkEnabled && !suppressSound && ['PRESENT', 'STUDYING', 'COMPUTER'].includes(current.state)) {
    if (next.phase === 'WEAK' && life.phase !== 'WEAK') chirp('weak');
    else if (next.phase === 'TIRED' && next.reminders > life.reminders) chirp('remind');
  }
  life = next; lifeAt = Date.now();
  suppressSound = false;
  renderLife();
}

function handlePetEvent(f) {
  if (f.length !== 4 || !EVENT_NAMES[f[3]]) return;
  const ms = Number(f[1]), id = Number(f[2]);
  if (!Number.isSafeInteger(ms) || ms < 0 || !Number.isSafeInteger(id) || id < 1) return;
  if (ms < lastEventMs) { eventSegment++; eventKeys.clear(); }
  lastEventMs = ms;
  const key = `${eventSegment}:${id}`;
  if (eventKeys.has(key)) return;
  eventKeys.add(key);
  petEvents.push({ key, at: Date.now(), ms, id, code: f[3] });
  while (petEvents.length > 300) eventKeys.delete(petEvents.shift().key);
  if (f[3] === 'WELCOME' || f[3] === 'REBORN') welcomeUntil = Date.now() + 2000;
  if (f[3] === 'CLAP' && networkEnabled && !suppressSound && !replayingEvents &&
      lastBoardMs >= ms && lastBoardMs - ms <= 2000 && Date.now() >= clapQuietUntil &&
      life && life.started && !life.paused && life.phase !== 'DEAD' && Date.now() - lifeAt <= 3000 &&
      ['PRESENT', 'STUDYING', 'COMPUTER'].includes(current.state)) {
    welcomeUntil = Date.now() + 2000;
    clapQuietUntil = Date.now() + 5000;
    if (life) renderLife();
  }
  $('petEvents').textContent = petEvents.slice(-30).map(e => `${new Date(e.at).toLocaleTimeString()}　${EVENT_NAMES[e.code]}`).join('\n');
  $('petEvents').scrollTop = $('petEvents').scrollHeight;
}
$('btnEvents').onclick = () => {
  const csv = 'received_ms,board_ms,event_id,event\r\n' + petEvents.map(e => `${e.at},${e.ms},${e.id},${e.code}`).join('\r\n');
  const url = URL.createObjectURL(new Blob(['\ufeff' + csv], { type: 'text/csv;charset=utf-8' }));
  const a = document.createElement('a'); a.href = url; a.download = 'chick-events.csv'; a.click();
  setTimeout(() => URL.revokeObjectURL(url), 1000);
};

let lastCsv = null, lastCsvAt = 0, lastStateAt = 0;
const csvHistory = [];
// 看看物品是太小、太低分，還是放錯位置
function renderPlacement(f, record = true) {
  if (record) {
    lastCsv = f;
    lastCsvAt = Date.now();
    csvHistory.push({ at: lastCsvAt, fields: [...f] });
    while (csvHistory.length && (csvHistory.length > 1800 || lastCsvAt - csvHistory[0].at > 1800000)) csvHistory.shift();
  }
  const n = i => parseInt(f[i], 10) || 0;
  const samples = n(2), modern = f.length >= 24, related = f.length >= 36;
  const chip = (id, label, hits, max, count, stable) => {
    const el = $(id), ratio = count ? hits / count : 0;
    el.textContent = `${label} ${count ? `${max} 分 · ${Math.round(ratio * 100)}%` : '無新結果'}${stable ? ' · 持續出現' : ''}`;
    el.className = "chip" + (ratio >= 0.6 ? " on" : hits ? " weak" : "");
  };
  chip("chipPerson", "人", n(3), n(4), samples, false);
  chip("chipBook", "書", n(5), n(6), samples, modern && n(17));
  chip("chipPhone", "手機", n(7), n(8), samples, modern && n(18));
  chip("chipLaptop", "筆電", n(9), n(10), samples, modern && n(19));
  chip("chipTable", "桌子", n(24), n(25), related ? samples : 0, false);
  chip("chipKeyboard", "鍵盤", n(26), n(27), related ? samples : 0, false);
  chip("chipMouse", "滑鼠", n(28), n(29), related ? samples : 0, false);
  chip("chipRemote", "遙控器", n(30), n(31), related ? samples : 0, false);
  $("relationInfo").textContent = related
    ? `本區間關聯補足：書 ${n(32)} 次、手機 ${n(33)} 次；鍵盤＋滑鼠電腦線索：${n(35) ? '持續成立' : '未成立'}。補足仍需原本有書／手機候選。`
    : '目前韌體沒有關聯欄位，請燒錄新版後使用自動關聯判斷。';
  const person = samples ? n(3) / samples : 0;
  const target = $("checkTarget").value;
  let hint;
  if (modern && (n(15) < 0 || n(15) > 2000)) hint = "物件辨識資料尚未到達或已逾時，請檢查開發板；這不代表物品已移走。";
  else if (!samples) hint = "這個區間沒有新物件推論結果，請先確認推論更新率。";
  else if (target !== "person") {
    const isBook = target === "book", label = isBook ? "書本" : "手機";
    const hits = n(isBook ? 5 : 7), max = n(isBook ? 6 : 8);
    const threshold = modern ? n(isBook ? 21 : 22) : 30;
    if (!max) hint = `未回傳${label}候選：確認完整露出並靠近鏡頭，再調整角度與反光；沒有候選時，其他類別無法補出${label}。`;
    else if (!hits) hint = max < threshold
      ? `${label}有候選（${max} 分），但未達 ${threshold} 分一般門檻${related ? '，也未通過位置、關聯或衝突檢查' : ''}。先改善大小、角度與光線。`
      : `${label}有候選但未通過範圍檢查${related ? '，或與更強的其他類別重疊' : ''}；書放青色框，手機放框內或人附近。`;
    else hint = `${label}本區間命中 ${hits}/${samples} 次。固定不動觀察 30 秒，再移走觀察是否誤判；持續出現狀態容許最多約 3 秒漏判。`;
  }
  else if (person < 0.3) hint = "幾乎看不到人：讓肩膀與上半身完整入鏡，確認光線與鏡頭位置。";
  else if (person < 0.8) hint = "人時有時無：鏡頭靠近一點、固定好，避免背對窗戶（逆光）。";
  else hint = "人的取景穩定。請再切換到「書本」或「手機」分別校正桌面，不必刻意看鏡頭。";
  $("placeHint").textContent = hint;
  const seconds = modern ? Math.max(1, n(20)) / 1000 : 1;
  $("inferenceInfo").textContent = modern
    ? `新推論：物件 ${(samples / seconds).toFixed(1)} 次/秒 · 物件資料距今 ${n(15) < 0 ? '尚無資料' : n(15) + ' ms'}`
    : '舊版韌體：取樣可能重複，請燒錄新版後比較推論命中率。';
  const recent = csvHistory.filter(row => lastCsvAt - row.at < 30000 && row.fields.length >= 24);
  const total = recent.reduce((sum, row) => sum + Number(row.fields[2]), 0);
  const ratio = i => total ? Math.round(recent.reduce((sum, row) => sum + Number(row.fields[i]), 0) / total * 100) + '%' : '—';
  $("recentInfo").textContent = `最近 30 秒（${recent.length} 筆，${total} 次新推論）：書 ${ratio(5)}、手機 ${ratio(7)}、人 ${ratio(3)}。這是命中比例，需配合物品實際是否在場解讀。`;
}

$("checkTarget").onchange = () => { if (lastCsv) renderPlacement(lastCsv, false); };
// 把這次測試存成 CSV
$("btnExport").onclick = () => {
  if (!csvHistory.length) { $("recentInfo").textContent = '目前沒有辨識紀錄可匯出。'; return; }
  const header = 'kind,t_s,samples,person_hits,person_max,book_hits,book_max,phone_hits,phone_max,laptop_hits,laptop_max,face_hits,face_max,face_pitch,face_samples,object_age_ms,face_age_ms,book_stable,phone_stable,laptop_stable,interval_ms,book_threshold,phone_threshold,laptop_threshold,table_hits,table_max,keyboard_hits,keyboard_max,mouse_hits,mouse_max,remote_hits,remote_max,book_assist_hits,phone_assist_hits,phone_near_person,computer_context';
  const csv = header + '\r\n' + csvHistory.map(row => [...row.fields.slice(0, 36), ...Array(Math.max(0, 36 - row.fields.length)).fill('')].join(',')).join('\r\n');
  const url = URL.createObjectURL(new Blob(['\ufeff' + csv], { type: 'text/csv;charset=utf-8' }));
  const a = document.createElement('a'); a.href = url; a.download = 'study-buddy-' + Date.now() + '.csv'; a.click();
  setTimeout(() => URL.revokeObjectURL(url), 1000);
};

// 太久沒資料就別當成即時結果
setInterval(() => {
  pollNetwork();
  if (life) renderLife();
  if (lastCsvAt && Date.now() - lastCsvAt > 3000) {
    $("placeHint").textContent = '超過 3 秒未收到新的 CSV，以上分數是舊資料，請確認 Wi-Fi 連線。';
    $("inferenceInfo").textContent = '資料已停止更新';
    $("relationInfo").textContent = '關聯資料已逾時';
    ['chipPerson', 'chipBook', 'chipPhone', 'chipLaptop', 'chipTable', 'chipKeyboard', 'chipMouse', 'chipRemote'].forEach(id => { $(id).className = 'chip'; });
  }
  if (lastStateAt && Date.now() - lastStateAt > 3000) {
    $("stateText").textContent = '狀態資料已逾時';
    $("msg").textContent = '等待新的開發板狀態，累計時間保留最後一次回報。';
    $("phoneNotice").textContent = ''; $("micNotice").textContent = ''; $("badge").textContent = '❓'; $("pet").className = 'pet';
  }
}, 1000);

function log(line) {
  const el = $("log");
  el.textContent += line + "\n";
  if (el.textContent.length > 20000) el.textContent = el.textContent.slice(-15000);
  el.scrollTop = el.scrollHeight;
}


// 一行一行讀板子送來的資料
function handleLine(line) {
  line = line.trim();
  if (!line) return;
  log(line);
  const f = line.split(",");
  if (f[1] === "t_s") return;
  if (f[0] === 'PET') { handleLife(f); return; }
  if (f[0] === 'EVENT') { handlePetEvent(f); return; }
  if (f[0] === "CSV" && f.length >= 11) { renderPlacement(f); return; }
  if (f[0] !== "STATE" || f.length < 9) return;
  if (!STATES[f[2]]) return;
  lastStateAt = Date.now();
  const n = i => parseInt(f[i], 10) || 0;
  if (f[6] !== 'A') {
    current = { ...blank(), state: 'UNSURE', deskS: current.deskS, studyS: current.studyS,
      computerS: current.computerS, awayCount: current.awayCount, lastAwayS: current.lastAwayS };
    render();
    $("modeText").textContent = '舊韌體回報非自動狀態，請燒錄新版；本頁不採用這筆活動判斷。';
    return;
  }
  const hasMic = f.length >= 17 && Number(f[14]) >= 0;
  current = { state: f[2], deskS: n(3), studyS: n(4), computerS: n(5),
              awayCount: n(7), lastAwayS: n(8), phone: f[12] === "1", phoneNear: f[13] === "1",
              mic: hasMic ? n(14) : -1, micPeak: hasMic ? n(15) : -1, noisy: hasMic && f[16] === "1" };
  $("modeText").textContent = f.length >= 14 ? '自動判斷：物件＋位置關聯' : '舊韌體自動判斷：尚無位置關聯，請燒錄新版';
  render();
}

// 本機 HTML 只帶你到板子頁面，資料保持同來源
function openBoard() {
  try {
    const raw = $('boardAddress').value.trim();
    if (!raw) throw new Error('請先填入 Ameba 的 IP');
    const url = new URL(raw.includes('://') ? raw : 'http://' + raw);
    if (url.protocol !== 'http:' || url.username || url.password || url.search || url.hash || url.pathname !== '/')
      throw new Error('請填 http://板子IP/，不要加入其他路徑');
    location.assign(url.origin + '/');
  } catch (e) { $('conn').textContent = '網址無法開啟：' + e.message; }
}
$('btnOpenBoard').onclick = openBoard;
if (location.protocol === 'http:') $('boardAddress').value = location.host;

// 每秒取一次快照，這個請求同時是心跳
async function pollNetwork() {
  if (!networkEnabled || networkTask) return;
  const epoch = networkEpoch;
  const controller = new AbortController();
  networkAbort = controller;
  const timeout = setTimeout(() => controller.abort(), 2000);
  const task = (async () => {
    try {
      const response = await fetch('/api/poll?after=' + eventCursor, {
        method: 'POST', headers: { 'X-StudyBuddy': '1' }, cache: 'no-store', signal: controller.signal
      });
      if (!response.ok) throw new Error('HTTP ' + response.status);
      const body = await response.text();
      if (!networkEnabled || epoch !== networkEpoch) return;
      const lines = body.trim().split(/\r?\n/), meta = lines.shift().split(',');
      const [seq, cursor, ms] = meta.slice(1).map(Number);
      if (meta[0] !== 'NET' || meta.length !== 4 || [seq, cursor, ms].some(n => !Number.isSafeInteger(n) || n < 0))
        throw new Error('不是新版 Ameba 的資料');
      const reboot = lastBoardMs >= 0 && (ms < lastBoardMs || seq < lastSequence);
      if (reboot) {
        life = null; suppressSound = true; lastSequence = -1;
        csvHistory.length = 0; eventSegment++; eventKeys.clear(); lastEventMs = -1;
      }
      const fresh = seq !== lastSequence;
      if (seq > 0 && fresh && !['CSV,', 'STATE,', 'PET,'].every(prefix => lines.some(line => line.startsWith(prefix))))
        throw new Error('資料尚未完整');
      replayingEvents = suppressSound;
      lastBoardMs = ms;
      for (const line of lines) {
        if (line.startsWith('EVENT,')) handleLine(line);
        else if (line === 'NOTICE,EVENT_GAP') log('部分舊事件已超出板端暫存，這次只補最近事件。');
        else if (seq > 0 && fresh) handleLine(line);
      }
      lastSequence = seq; lastBoardMs = ms; eventCursor = cursor;
      replayingEvents = false;
      $('conn').textContent = seq ? 'Wi-Fi 已連線 · ' + location.host : 'Wi-Fi 已連線，等待板端辨識啟動';
    } catch (e) {
      if (networkEnabled && epoch === networkEpoch) {
        suppressSound = true;
        $('conn').textContent = 'Wi-Fi 連線中斷，正在重試；請確認電源、網路與 IP。';
        if (life) { life.paused = true; renderLife(); }
      }
    } finally {
      clearTimeout(timeout);
      if (networkAbort === controller) networkAbort = null;
    }
  })();
  networkTask = task;
  try { await task; } finally { if (networkTask === task) networkTask = null; }
}

async function connect() {
  if (networkEnabled || networkTask) return;
  if (location.protocol !== 'http:') {
    $('conn').textContent = '請填入 IP 並按「開啟板子網頁」，再開始連線。';
    return;
  }
  enableAudio();
  networkEnabled = true; networkEpoch++;
  lastSequence = lastBoardMs = eventCursor = -1; suppressSound = true;
  $('btnConnect').disabled = true;
  csvHistory.length = 0; lastCsv = null; lastCsvAt = lastStateAt = 0;
  life = null; lifeAt = 0; welcomeUntil = 0; eventSegment++; lastEventMs = -1; eventKeys.clear();
  $('petLifeText').textContent = '等待 Ameba 的 Wi-Fi 養成資料';
  ['petEnergy', 'petWork', 'petRest', 'petCycles'].forEach(id => { $(id).textContent = '—'; });
  ['chipPerson', 'chipBook', 'chipPhone', 'chipLaptop', 'chipTable', 'chipKeyboard', 'chipMouse', 'chipRemote'].forEach(id => { $(id).textContent = '等待新資料'; $(id).className = 'chip'; });
  $('placeHint').textContent = '等待新的擺放檢查資料';
  $('inferenceInfo').textContent = '等待推論資料';
  $('recentInfo').textContent = '最近 30 秒：等待資料';
  $('relationInfo').textContent = '等待關聯判斷資料';
  $('modeText').textContent = '等待自動辨識資料';
  $('stateText').textContent = '等待新的開發板狀態';
  $('phoneNotice').textContent = ''; $('micNotice').textContent = '';
  $('conn').textContent = '正在連接 Ameba…';
  await pollNetwork();
}

async function disconnect() {
  networkEnabled = false; networkEpoch++;
  if (networkAbort) networkAbort.abort();
  if (networkTask) await networkTask;
  if (life) { life.paused = true; renderLife(); }
  $('btnConnect').disabled = false;
  $('conn').textContent = 'Wi-Fi 已中斷，養成將在心跳逾時後暫停';
}

$("btnConnect").onclick = connect;
$("btnDisconnect").onclick = disconnect;

render();
</script>
</body>
</html>
)PETPAGE";
