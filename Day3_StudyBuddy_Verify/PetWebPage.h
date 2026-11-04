// 由 scripts/embed-web.cjs 產生，請修改 web/pet_test.html
#pragma once
static const char PET_WEB_PAGE[] = R"PETPAGE(<!doctype html>
<html lang="zh-Hant">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>啾啾的小屋</title>
<style>
:root{--bg:#f4eedd;--panel:#fffaf0;--ink:#3a3026;--soft:#7a6a58;--line:#3a3026;--accent:#e8a72e;--green:#6f9a55;--blue:#4f7f9c;--orange:#d9772e;--red:#c0584a;--shadow:#d8cdb4;--chip:#f1e6c8}
*{box-sizing:border-box}
html,body{margin:0;background:var(--bg);color:var(--ink);font-family:"Microsoft JhengHei","PingFang TC","Noto Sans TC",system-ui,sans-serif;font-size:15px;line-height:1.5}
button,input,select{font:inherit;color:inherit}
.app{display:grid;grid-template-columns:minmax(0,1fr) minmax(0,1fr);min-height:100vh}
.life{position:sticky;top:0;height:100vh;display:flex;flex-direction:column;border-right:4px solid var(--line);background:#e9dcc0;overflow:hidden}
.lifebar{display:flex;align-items:center;gap:10px;padding:10px 14px;border-bottom:4px solid var(--line);background:var(--panel)}
.lifebar h1{margin:0;font-size:18px;letter-spacing:2px;flex:1}
.dot{width:12px;height:12px;background:#b9ab93;box-shadow:0 0 0 2px var(--line)}
.dot.online{background:var(--green)}
.chip{font-size:12px;padding:2px 8px;background:var(--chip);border:2px solid var(--line);white-space:nowrap}
.stagewrap{position:relative;flex:1;min-height:0}
#stage{position:absolute;inset:0;width:100%;height:100%;image-rendering:pixelated;cursor:default}
#stage.palm{cursor:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='32' height='32' shape-rendering='crispEdges'%3E%3Cpath d='M10 4h4v12h2V2h4v14h2V4h4v16h-2v4h-2v4H12v-4h-2v-4H8v-4H6v-6h4z' fill='%23f6d2a8' stroke='%233a3026' stroke-width='2'/%3E%3C/svg%3E") 16 16,pointer}
.lifefoot{padding:10px 14px;border-top:4px solid var(--line);background:var(--panel);display:grid;gap:6px}
.lifefoot strong{font-size:17px}
.lifefoot p{margin:0;font-size:13px;color:var(--soft)}
.bar{height:12px;border:2px solid var(--line);background:#efe5cf;position:relative}
.bar>i{position:absolute;inset:0 auto 0 0;width:0;background:var(--green);transition:width .4s steps(8)}
.bar.blue>i{background:var(--blue)}.bar.orange>i{background:var(--orange)}.bar.gold>i{background:var(--accent)}
.row{display:flex;align-items:center;gap:8px;flex-wrap:wrap}
.panel{padding:18px clamp(14px,3vw,32px) 40px;display:grid;gap:20px;align-content:start;min-width:0}
.block{background:var(--panel);border:4px solid var(--line);box-shadow:6px 6px 0 var(--shadow);padding:16px;min-width:0}
.block>h2{margin:0 0 12px;font-size:17px;display:flex;align-items:center;gap:8px;letter-spacing:1px}
.block>h2 small{font-weight:normal;color:var(--soft);font-size:12px;margin-left:auto}
.icon{width:20px;height:20px;display:inline-block;flex:none}
.icon svg{width:100%;height:100%;display:block}
.actions{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:10px}
.btn{display:flex;align-items:center;justify-content:center;gap:8px;padding:10px 8px;background:#fff3d6;border:3px solid var(--line);box-shadow:3px 3px 0 var(--line);cursor:pointer;min-height:48px;text-align:center}
.btn:hover:not(:disabled){background:#ffe7ad}
.btn:active:not(:disabled){transform:translate(3px,3px);box-shadow:none}
.btn:disabled{opacity:.45;cursor:not-allowed}
.btn.on{background:#ffd46b}
.btn.primary{background:#bfe0a4}.btn.danger{background:#f3c2b7}
.btn:focus-visible,input:focus-visible,select:focus-visible{outline:3px dashed var(--blue);outline-offset:2px}
.sub{font-size:13px;color:var(--soft);margin:8px 0 0}
.toys{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:8px;margin-top:10px}
.toys .btn{min-height:40px;font-size:14px;padding:6px}
.stats{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:10px}
.stat{border:3px solid var(--line);padding:10px;background:#fffdf7;min-width:0}
.stat span{display:block;font-size:12px;color:var(--soft)}
.stat b{font-size:22px;font-variant-numeric:tabular-nums;letter-spacing:1px}
.stat .bar{margin-top:6px}
.wide{grid-column:1/-1}
.today{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:8px;margin-top:10px}
.today div{border:2px solid var(--line);padding:6px;text-align:center;background:#fffdf7}
.today span{display:block;font-size:12px;color:var(--soft)}
.today b{font-variant-numeric:tabular-nums}
.stages{display:flex;gap:6px;margin-top:10px}
.stages div{flex:1;border:2px solid var(--line);padding:4px 2px;text-align:center;font-size:11px;background:#fffdf7;opacity:.45}
.stages div.reached{opacity:1}.stages div.current{background:#ffe7ad}
.stages canvas{width:32px;height:32px;image-rendering:pixelated;display:block;margin:0 auto}
form{display:grid;gap:12px}
fieldset{border:3px solid var(--line);margin:0;padding:10px 12px;min-width:0}
legend{padding:0 6px;font-weight:bold}
label.field{display:grid;gap:4px}
label.field>span{font-size:14px}
label.field small{color:var(--soft);font-size:12px}
input[type=number],input[type=time],select{width:100%;padding:8px;border:3px solid var(--line);background:#fff;border-radius:0}
input[aria-invalid=true]{border-color:var(--red);background:#fff0ec}
.err{color:var(--red);font-size:12px;min-height:1em}
.grid2{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:10px}
.radio{display:flex;gap:14px;flex-wrap:wrap}
.radio label{display:flex;align-items:center;gap:6px}
.planstate{border:3px dashed var(--line);padding:8px 10px;background:#fffdf7;font-size:14px}
#eventList{list-style:none;margin:10px 0 0;padding:0;max-height:340px;overflow:auto;border:3px solid var(--line);background:#fffdf7}
#eventList li{display:grid;grid-template-columns:auto 1fr;gap:10px;padding:6px 10px;border-bottom:2px dashed var(--shadow);font-size:14px}
#eventList li:last-child{border-bottom:0}
#eventList time{color:var(--soft);font-variant-numeric:tabular-nums}
#eventList .empty{display:block;color:var(--soft)}
.toast{position:fixed;left:50%;bottom:20px;transform:translateX(-50%);background:var(--ink);color:#fffaf0;padding:10px 16px;border:3px solid #fffaf0;box-shadow:4px 4px 0 rgba(0,0,0,.3);font-size:14px;opacity:0;pointer-events:none;transition:opacity .2s;z-index:30;max-width:calc(100vw - 32px)}
.toast.visible{opacity:1}
.rest{position:fixed;inset:0;z-index:20;background:#25304a;display:flex;flex-direction:column}
.rest[hidden]{display:none}
#restStage{flex:1;width:100%;min-height:0;image-rendering:pixelated}
.restinfo{position:absolute;left:0;right:0;top:24px;text-align:center;color:#fdf3d0;pointer-events:none;padding:0 16px}
.restinfo strong{display:block;font-size:clamp(20px,4vw,34px);letter-spacing:3px}
.restinfo span{font-size:clamp(14px,2.4vw,18px)}
.restclose{position:absolute;right:16px;bottom:16px}
.demo{font-size:12px;color:var(--orange)}
.hidden{display:none!important}
@media (max-width:820px){
 .app{grid-template-columns:1fr}
 .life{height:48vh;border-right:0;border-bottom:4px solid var(--line);z-index:5}
 .lifefoot{padding:6px 12px;gap:4px}
 .lifefoot p{display:none}
 .actions{grid-template-columns:repeat(3,minmax(0,1fr))}
 .btn{font-size:14px;padding:8px 4px}
}
@media (max-width:420px){.grid2,.stats{grid-template-columns:1fr}.today{grid-template-columns:repeat(2,minmax(0,1fr))}}
@media (prefers-reduced-motion:reduce){.bar>i{transition:none}}
</style>
</head>
<body>
<div class="app">
 <section class="life" id="life" aria-label="小雞生活區">
  <div class="lifebar"><span class="dot" id="connectionDot"></span><h1>啾啾的小屋</h1><span class="chip" id="modeChip">準備中</span></div>
  <div class="stagewrap"><canvas id="stage" role="img" aria-label="像素小雞的房間"></canvas></div>
  <div class="lifefoot" role="status" aria-live="polite">
   <div class="row"><strong id="activityTitle">等你設定計畫</strong><span class="chip" id="rewardStatus">獎勵等待中</span></div>
   <p id="activityDescription">在右邊設定目的、時間、專注間隔與休息時間，就能開始。</p>
   <p id="trackLine"></p>
   <div class="row" style="flex-wrap:nowrap"><span style="font-size:12px;white-space:nowrap">活力</span><div class="bar" style="flex:1"><i id="energyFill"></i></div><span id="energyLabel" style="font-size:12px;min-width:3em;text-align:right">--</span></div>
  </div>
 </section>

 <main class="panel" id="panel">
  <section class="block" aria-labelledby="playTitle">
   <h2 id="playTitle"><span class="icon" data-icon="palm"></span>和啾啾互動<small id="growthName">一顆小小的期待</small></h2>
   <div class="actions">
    <button class="btn" id="palmButton" type="button" aria-pressed="false"><span class="icon" data-icon="palm"></span>摸摸</button>
    <button class="btn" id="feedButton" type="button"><span class="icon" data-icon="seed"></span><span id="feedLabel">餵食</span></button>
    <button class="btn" id="toyButton" type="button" aria-expanded="false"><span class="icon" data-icon="ball"></span>玩具</button>
   </div>
   <div class="toys hidden" id="toys">
    <button class="btn" type="button" data-toy="ball"><span class="icon" data-icon="ball"></span>小球</button>
    <button class="btn" type="button" data-toy="feather"><span class="icon" data-icon="feather"></span>羽毛</button>
    <button class="btn" type="button" data-toy="blocks"><span class="icon" data-icon="blocks"></span>積木</button>
   </div>
   <p class="sub" id="feedHint">開始計畫後，認真學習就能收集穀穀。</p>
   <div class="row" style="margin-top:8px"><span class="icon" data-icon="seed"></span><b><span id="foodCount">0</span> 份穀穀</b><div class="bar gold" style="flex:1;min-width:120px" id="rewardProgress" role="progressbar" aria-label="下一份穀穀" aria-valuemin="0" aria-valuemax="100" aria-valuenow="0"><i id="rewardFill"></i></div></div>
   <p class="sub" id="rewardHint">讀書或電腦學習才會累積。</p>
   <div class="stages" id="growthStages"></div>
  </section>

  <section class="block" aria-labelledby="recordTitle">
   <h2 id="recordTitle"><span class="icon" data-icon="clock"></span>紀錄與時間<small class="demo" id="demoNote" hidden>展示版：時間已縮短</small></h2>
   <div class="planstate" id="planSummary">還沒有學習計畫。</div>
   <div class="stats" style="margin-top:10px">
    <div class="stat wide"><span>總目標進度（有效學習）</span><b id="goalTime">00:00 / 00:00</b><div class="bar gold"><i id="goalFill"></i></div></div>
    <div class="stat"><span id="sessionLabel">這一輪專注</span><b id="sessionTime">00:00</b><div class="bar" id="sessionBar"><i id="sessionFill"></i></div><span id="sessionFoot">設定後開始計時</span></div>
    <div class="stat"><span>休息</span><b id="restTime">00:00</b><div class="bar blue"><i id="restFill"></i></div><span id="restFoot">離開桌前就開始休息</span></div>
   </div>
   <div class="today" aria-label="開機以來的統計">
    <div><span>在桌前</span><b id="statDesk">00:00</b></div>
    <div><span>讀書</span><b id="statStudy">00:00</b></div>
    <div><span>電腦</span><b id="statComputer">00:00</b></div>
    <div><span>手機出現</span><b id="statPhone">00:00</b></div>
    <div><span>完成休息</span><b id="statRests">0</b></div>
    <div><span>休息提醒</span><b id="statReminders">0</b></div>
   </div>
  </section>

  <section class="block" aria-labelledby="setTitle">
   <h2 id="setTitle"><span class="icon" data-icon="gear"></span>設定與事件紀錄</h2>
   <form id="planForm" novalidate>
    <fieldset>
     <legend>學習計畫（必填）</legend>
     <div class="grid2">
      <label class="field"><span>目的</span><select id="purpose"><option value="0">讀書</option><option value="1">電腦作業</option><option value="2">讀書＋電腦</option></select></label>
      <label class="field"><span>總目標時間（<span class="unit">分鐘</span>）</span><input id="goalInput" type="number" inputmode="numeric" required><small id="goalRange"></small><span class="err" id="goalErr"></span></label>
      <label class="field"><span>專注間隔（<span class="unit">分鐘</span>）</span><input id="focusInput" type="number" inputmode="numeric" required><small id="focusRange"></small><span class="err" id="focusErr"></span></label>
      <label class="field"><span>休息時間（<span class="unit">分鐘</span>）</span><input id="restInput" type="number" inputmode="numeric" required><small id="restRange"></small><span class="err" id="restErr"></span></label>
     </div>
     <div style="margin-top:10px"><span style="font-size:14px">開始</span>
      <div class="radio"><label><input type="radio" name="startMode" value="now" checked>立即開始</label><label><input type="radio" name="startMode" value="at">指定時間</label><input id="startAt" type="time" style="width:auto" disabled></div>
      <span class="err" id="startErr"></span>
     </div>
    </fieldset>
    <div class="actions" style="grid-template-columns:repeat(2,minmax(0,1fr))">
     <button class="btn primary" id="startButton" type="submit"><span class="icon" data-icon="play"></span>開始計畫</button>
     <button class="btn danger" id="stopButton" type="button"><span class="icon" data-icon="stop"></span>結束計畫</button>
    </div>
    <p class="sub" id="formHint">等待小屋回應…</p>
   </form>
   <div class="row" style="margin-top:14px">
    <button class="btn" id="soundButton" type="button" aria-pressed="true"><span class="icon" data-icon="sound"></span><span id="soundLabel">叫聲：開</span></button>
    <button class="btn" id="exportButton" type="button"><span class="icon" data-icon="download"></span>匯出紀錄</button>
    <button class="btn" id="connectButton" type="button"><span class="icon" data-icon="play"></span><span id="connectLabel">重新連線</span></button>
   </div>
   <div class="row hidden" id="addressBox" style="margin-top:10px"><input id="homeAddress" placeholder="小屋位址，例如 192.168.1.88" style="flex:1;padding:8px;border:3px solid var(--line)"><button class="btn" id="openHome" type="button">前往</button></div>
   <h3 style="margin:16px 0 0;font-size:15px">每次事件</h3>
   <ol id="eventList"><li class="empty">還沒有事件。</li></ol>
  </section>
 </main>
</div>

<div class="rest" id="restOverlay" hidden>
 <canvas id="restStage" aria-hidden="true"></canvas>
 <div class="restinfo"><strong>休息中</strong><span id="restOverlayText">離開桌前走一走</span></div>
 <button class="btn restclose" id="restClose" type="button">縮小畫面</button>
</div>
<div class="toast" id="toast" role="status"></div>

<script>
'use strict';
const $=id=>document.getElementById(id);
// ── 像素圖示：8×8，每個字代表一格顏色 ──
const IC={'#':'#3a3026','y':'#f7cf4a','o':'#e8772e','g':'#6f9a55','b':'#4f7f9c','r':'#c0584a','w':'#ffffff','s':'#f6d2a8','p':'#e7c08a'};
const ICONS={
 palm:['.s.s.s..','.s.s.s..','.s.s.s.s','.sssss.s','.ssssss.','.sssss..','..ssss..','..ssss..'],
 seed:['........','...yy...','..yyyy..','.yyoyyy.','.yyyyyy.','..yyyy..','...yy...','........'],
 ball:['..rrrr..','.rrwrrr.','rrwrrrrr','rrrrrrrr','wwwwwwww','rrrrrrrr','.rrrrrr.','..rrrr..'],
 feather:['......bb','.....bbb','....bbb.','...bbb..','..bbb...','.bbb....','.#......','#.......'],
 blocks:['........','.rr.....','.rr.bb..','....bb..','.yyyy...','.yyyy...','gggggg..','gggggg..'],
 clock:['..####..','.#wwww#.','#ww#www#','#ww#www#','#ww###w#','#wwwwww#','.#wwww#.','..####..'],
 gear:['...##...','.#.##.#.','..####..','###ww###','###ww###','..####..','.#.##.#.','...##...'],
 play:['.#......','.##.....','.###....','.####...','.####...','.###....','.##.....','.#......'],
 stop:['........','.######.','.######.','.######.','.######.','.######.','.######.','........'],
 sound:['...#....','..##..#.','####.#..','####..#.','####.#..','..##..#.','...#....','........'],
 download:['...##...','...##...','...##...','.######.','..####..','...##...','#......#','########']
};
function icon(name){const m=ICONS[name];if(!m)return '';let r='';m.forEach((row,y)=>[...row].forEach((c,x)=>{if(IC[c])r+=`<rect x="${x}" y="${y}" width="1" height="1" fill="${IC[c]}"/>`;}));return `<svg viewBox="0 0 8 8" shape-rendering="crispEdges" aria-hidden="true">${r}</svg>`;}
document.querySelectorAll('[data-icon]').forEach(e=>{e.innerHTML=icon(e.dataset.icon);});

// ── 板子資料 ──
const STATES={UNKNOWN:'正在找你的位置',UNSURE:'正在找你的位置',PRESENT:'在桌前陪伴',STUDYING:'一起認真讀書',COMPUTER:'電腦學習中',PHONE:'手機出現 · 獎勵暫停',AWAY:'休息一下，等等見'};
const ACTIVITIES={
 STUDYING:['戴上眼鏡一起讀書','啾啾戴好眼鏡翻書，讀書時間正在累積穀穀。','穀穀累積中'],
 COMPUTER:['電腦學習時間','啾啾在電腦前打字，陪你查資料、寫作業。','穀穀累積中'],
 PHONE:['手機出現，穀穀先等等','學習獎勵暫停、進度保留，在座與休息提醒仍繼續。','獎勵已暫停'],
 PRESENT:['先坐好，準備一下','找到書本或電腦後，才會開始累積學習獎勵。','獎勵等待中'],
 AWAY:['離開桌前，休息一下','啾啾也去睡一下，休息好了再一起努力。','休息中'],
 NOPLAN:['等你設定計畫','在右邊設定目的、時間、專注間隔與休息時間，就能開始。','獎勵等待中'],
 WAITING:['計畫快開始了','時間到了啾啾就會醒來陪你。','等待開始'],
 READY:['等小屋回應','正在連到 Ameba，請確認在同一個 Wi-Fi。','獎勵等待中'],
 PAUSED:['暫停一下，等你回來','暫時看不清楚或連線中斷，學習進度先保留。','獎勵已暫停'],
 DEAD:['先好好休息，再迎接新生命','離開桌前完成一次休息，小屋會迎來新的蛋。','獎勵已暫停']
};
const EVENTS={WELCOME:'你回到桌前，啾啾也準備好了。',LEAVE:'離開桌前，伸個懶腰。',REMIND:'專注時間到，帶啾啾休息一下吧。',WEAK:'啾啾有點累了，這次真的要休息囉。',DIED:'啾啾累倒了。完成休息後，會迎來新的蛋。',RESTED:'好好休息過，重新充滿活力。',REBORN:'一顆新的蛋，等你一起慢慢養大。',PLAN_SET:'小屋收到新的學習計畫。',PLAN_START:'計畫開始，啾啾陪你專心。',GOAL:'達成這次的總目標時間！',NOISY:'周圍有點熱鬧。',QUIET:'環境又安靜下來了。'};
const PURPOSES=['讀書','電腦作業','讀書＋電腦'];
const GROWTH=[['小小的蛋',0],['小絨球',1],['小探險家',3],['陪讀夥伴',8],['小屋守護者',16]];
const TRACK_NAMES={book:'書本',phone:'手機',laptop:'筆電',keyboard:'鍵盤',mouse:'滑鼠'};
let current={state:'UNKNOWN',desk:0,study:0,computer:0,tracks:[]},life=null,care=null,plan=null,lastSnapshotAt=0;
let networkEnabled=false,networkTask=null,networkAbort=null,epoch=0,lastSequence=-1,lastBoardMs=-1,eventCursor=-1,suppressSound=true;
let soundEnabled=true,audio=null,feedBusy=false,feedAbort=null,planBusy=false,toastUntil=0,celebrateUntil=0,eatUntil=0,petUntil=0;
let memories=[],eventIds=new Set(),stage=-1,phoneSeconds=0,palmMode=false,toy=null,restDismissed=false,formTouched=false;
const reduced=matchMedia('(prefers-reduced-motion: reduce)');
function fmt(s){s=Math.max(0,Math.floor(s||0));const h=Math.floor(s/3600),m=Math.floor(s%3600/60),x=s%60;return (h?h+':'+String(m).padStart(2,'0'):String(m).padStart(2,'0'))+':'+String(x).padStart(2,'0');}
function fresh(){return networkEnabled&&life&&Date.now()-lastSnapshotAt<=3000&&!life.paused;}
function connected(){return networkEnabled&&life&&Date.now()-lastSnapshotAt<=3000;}
function toast(text){$('toast').textContent=text;toastUntil=Date.now()+3500;$('toast').classList.add('visible');}
function remember(text){
 memories.unshift({at:new Date(),text});memories=memories.slice(0,300);
 const list=$('eventList');list.replaceChildren(...memories.map(m=>{const li=document.createElement('li'),time=document.createElement('time'),span=document.createElement('span');time.textContent=m.at.toLocaleTimeString('zh-TW',{hour12:false});span.textContent=m.text;li.append(time,span);return li;}));
}
function integers(values){return values.every(v=>/^\d+$/.test(v)&&Number.isSafeInteger(Number(v)));}
function parseLife(line){
 const f=line.split(',');if(f.length!==16||f[0]!=='PET'||!['EGG','HEALTHY','TIRED','WEAK','DEAD'].includes(f[2])||!integers(f.slice(1).filter((_,i)=>i!==1)))return null;
 const n=i=>Number(f[i]);if(n(3)>100||!n(8)||n(10)>1||n(14)>1||n(15)>1)return null;
 return {uptime:n(1),phase:f[2],energy:n(3),work:n(4),rest:n(5),cycles:n(6),rests:n(7),generation:n(8),reminders:n(9),paused:!!n(10),remindGoal:n(11),restGoal:n(12),decay:n(13),demo:!!n(14),started:!!n(15)};
}
function parseTracks(text){if(!text||text==='-')return [];return text.split('|').map(p=>p.split('-')).filter(p=>p.length===4&&Object.hasOwn(TRACK_NAMES,p[0])&&integers(p.slice(1))).map(p=>({name:TRACK_NAMES[p[0]],id:Number(p[1]),seconds:Number(p[2]),lost:p[3]==='1'})).slice(0,4);}
function trackText(tracks){return tracks.length?'持續追蹤：'+tracks.map(t=>`${t.name} #${t.id} · ${t.lost?'暫時看不到，等它回來':fmt(t.seconds)}`).join('　'):'';}
function parseState(line){const f=line.split(',');if(f.length<14||f[0]!=='STATE'||!Object.hasOwn(STATES,f[2])||f[6]!=='A'||!integers([f[1],f[3],f[4],f[5],f[12],f[13]])||Number(f[12])>1||Number(f[13])>1)return null;return {state:f[2],desk:Number(f[3]),study:Number(f[4]),computer:Number(f[5]),phone:f[12]==='1',noisy:f[16]==='1',tracks:parseTracks(f[17])};}
function parseCare(line){const f=line.trim().split(',');if(f.length!==11||f[0]!=='CARE'||!integers(f.slice(1)))return null;const n=i=>Number(f[i]);if(n(4)>99||!n(3)||!n(8)||n(7)>n(8)||n(10)>1)return null;return {boot:n(1),revision:n(2),generation:n(3),food:n(4),feeds:n(5),learned:n(6),next:n(7),goal:n(8),lastFeed:n(9),available:!!n(10)};}
function parsePlan(line){
 const f=line.trim().split(',');if(f.length!==15||f[0]!=='PLAN'||!integers(f.slice(1)))return null;const n=i=>Number(f[i]);
 if(n(2)>3||n(6)>2||!n(9)||n(9)>n(10)||!n(11)||n(11)>n(12)||!n(13)||n(13)>n(14))return null;
 return {revision:n(1),phase:['NONE','WAITING','RUNNING','DONE'][n(2)],focus:n(3),rest:n(4),goal:n(5),purpose:n(6),startsIn:n(7),learned:n(8),focusMin:n(9),focusMax:n(10),restMin:n(11),restMax:n(12),goalMin:n(13),goalMax:n(14)};
}
function planActive(){return plan&&(plan.phase==='RUNNING'||plan.phase==='DONE');}
async function enableAudio(){if(!soundEnabled)return;try{audio ||= new (window.AudioContext||window.webkitAudioContext)();await audio.resume();}catch{}}
function chirp(kind){if(!soundEnabled||!audio||audio.state!=='running')return;for(let i=0;i<(kind==='weak'?2:3);i++){const start=audio.currentTime+i*.23,osc=audio.createOscillator(),gain=audio.createGain();osc.type='square';osc.frequency.setValueAtTime(1800,start);osc.frequency.exponentialRampToValueAtTime(2900,start+.07);osc.frequency.exponentialRampToValueAtTime(1500,start+.15);gain.gain.setValueAtTime(.0001,start);gain.gain.exponentialRampToValueAtTime(.05,start+.015);gain.gain.exponentialRampToValueAtTime(.0001,start+.18);osc.connect(gain);gain.connect(audio.destination);osc.start(start);osc.stop(start+.2);osc.onended=()=>{osc.disconnect();gain.disconnect();};}}
function receiveCare(next,quiet=false){
 // 以小屋的存量為準，晚到的回應別蓋回去
 if(care&&next.boot===care.boot&&next.revision<care.revision)return;
 if(!quiet&&care&&next.boot===care.boot&&next.generation===care.generation){
  if(next.feeds>care.feeds){eatUntil=Date.now()+5500;toast('穀穀收到！啾啾的成長 +1。');remember('餵啾啾吃了穀穀，成長 +1。');}
  else if(next.food>care.food){celebrateUntil=Date.now()+2200;toast('認真學習的收穫：穀穀 +1。');remember('認真學習，收穫一份穀穀。');}
 }
 care=next;
}
function receivePlan(next,quiet=false){
 if(!next)return;if(plan&&next.revision<plan.revision&&next.phase===plan.phase)return;
 const first=!plan;plan=next;if(first||!formTouched)fillForm();
}
function applySnapshot(text){
 // 同一筆資料不延長有效時間，重連也不補叫
 const lines=text.trim().split(/\r?\n/),net=lines[0].split(',');if(net.length!==4||net[0]!=='NET'||!integers(net.slice(1)))throw Error('Incomplete home');
 const seq=Number(net[1]),board=Number(net[3]),nextCare=parseCare(lines.find(l=>l.startsWith('CARE,'))||''),nextPlan=parsePlan(lines.find(l=>l.startsWith('PLAN,'))||'');
 const reboot=board<lastBoardMs||seq<lastSequence||(care&&nextCare&&care.boot!==nextCare.boot);
 const quiet=suppressSound||reboot||Date.now()-lastSnapshotAt>3000;
 if(reboot){life=null;care=null;plan=null;lastSequence=-1;eventIds.clear();eatUntil=0;celebrateUntil=0;remember('小屋重新開機，請重新設定學習計畫。');}
 if(seq>0&&seq!==lastSequence){
  const next=parseLife(lines.find(l=>l.startsWith('PET,'))||''),state=parseState(lines.find(l=>l.startsWith('STATE,'))||'');
  if(!next||!state)throw Error('Incomplete companion');
  if(life&&!quiet&&next.generation===life.generation&&!next.paused&&['PRESENT','STUDYING','COMPUTER','PHONE'].includes(state.state)){
   if(next.phase==='WEAK'&&life.phase!=='WEAK')chirp('weak');
   else if(next.phase==='TIRED'&&next.reminders>life.reminders)chirp('remind');
  }
  current=state;life=next;lastSnapshotAt=Date.now();lastSequence=seq;suppressSound=false;
 }
 if(nextCare)receiveCare(nextCare,quiet);
 receivePlan(nextPlan,quiet);
 if(!quiet)for(const line of lines){const f=line.split(',');if(f[0]!=='EVENT'||f.length!==4||!integers(f.slice(1,3))||eventIds.has(f[2]))continue;eventIds.add(f[2]);if(eventIds.size>300)eventIds.delete(eventIds.values().next().value);const label=EVENTS[f[3]];if(label)remember(label);}
 eventCursor=Number(net[2]);lastBoardMs=board;render();
}
function pollNetwork(){
 if(!networkEnabled)return Promise.resolve();if(networkTask)return networkTask;
 const token=epoch;networkTask=(async()=>{const controller=new AbortController();networkAbort=controller;const timeout=setTimeout(()=>controller.abort(),2200);try{const res=await fetch('/api/poll?after='+eventCursor,{method:'POST',headers:{'X-StudyBuddy':'1'},cache:'no-store',signal:controller.signal});if(!res.ok)throw Error('Unavailable home');const body=await res.text();if(token===epoch&&networkEnabled)applySnapshot(body);}catch{if(token===epoch&&networkEnabled){suppressSound=true;if(life)life.paused=true;render();}}finally{clearTimeout(timeout);if(networkAbort===controller)networkAbort=null;}})().finally(()=>{networkTask=null;});return networkTask;
}
async function connect(){
 if(location.protocol!=='http:'&&location.protocol!=='https:'){$('addressBox').classList.remove('hidden');toast('請輸入小屋的 Wi-Fi 位址。');return;}
 if(networkEnabled)return;networkEnabled=true;++epoch;lastSequence=-1;lastBoardMs=-1;eventCursor=-1;suppressSound=true;life=null;care=null;eatUntil=0;render();await pollNetwork();
}
async function disconnect(){networkEnabled=false;++epoch;networkAbort?.abort();feedAbort?.abort();suppressSound=true;eatUntil=0;feedBusy=false;if(life)life.paused=true;render();await networkTask;}
async function post(url){const controller=new AbortController(),timeout=setTimeout(()=>controller.abort(),2500);try{return await fetch(url,{method:'POST',headers:{'X-StudyBuddy':'1'},cache:'no-store',signal:controller.signal});}finally{clearTimeout(timeout);}}
async function feed(){
 // 只送一次；沒收到回覆就查存量，不再扣一次
 if(feedBusy||Date.now()<eatUntil||!fresh()||!planActive()||!care?.available||!care.food||!life.started||life.phase==='DEAD'||care.generation!==life.generation)return;
 feedBusy=true;const token=epoch,boot=care.boot,revision=care.revision;render();
 try{const res=await post(`/api/feed?boot=${boot}&revision=${revision}`);const next=parseCare(await res.text());if(token!==epoch||!networkEnabled)return;if(!next||next.boot!==boot||(!res.ok&&res.status!==409))throw Error('Unconfirmed feed');receiveCare(next);if(!res.ok)toast('啾啾先緩一下，等小屋更新後再餵。');}
 catch{if(token===epoch&&networkEnabled)toast('正在確認餵食結果，先別急著再按。');}
 finally{if(token===epoch){feedBusy=false;render();await pollNetwork();}}
}

// ── 計畫表單：一定要填，而且要在小屋給的範圍內 ──
function demoUnit(){return plan&&plan.focusMax<=300;}
function unitScale(){return demoUnit()?1:60;}
function fillForm(){
 if(!plan)return;const k=unitScale(),u=demoUnit()?'秒':'分鐘';
 document.querySelectorAll('.unit').forEach(e=>e.textContent=u);
 for(const [id,key] of [['focus','focus'],['rest','rest'],['goal','goal']]){
  const input=$(id+'Input'),min=Math.ceil(plan[key+'Min']/k),max=Math.floor(plan[key+'Max']/k);
  input.min=min;input.max=max;input.step=1;$(id+'Range').textContent=`可設定 ${min}～${max} ${u}`;
  const preset=plan.phase!=='NONE'?plan[key]/k:demoUnit()?{focus:60,rest:10,goal:180}[key]:{focus:25,rest:5,goal:60}[key];
  input.value=String(Math.min(max,Math.max(min,Math.round(preset))));
 }
 $('purpose').value=String(plan.purpose||0);validateForm();
}
function readForm(){
 const k=unitScale(),num=id=>{const v=$(id).value.trim();return /^\d+$/.test(v)?Number(v)*k:NaN;};
 const f=num('focusInput'),r=num('restInput'),g=num('goalInput'),errors={};
 if(!plan)return {errors:{form:'等待小屋回應'}};
 const check=(key,v,name)=>{if(!Number.isInteger(v))errors[key]=`請填寫${name}`;else if(v<plan[key+'Min']||v>plan[key+'Max'])errors[key]=`${name}要在 ${Math.ceil(plan[key+'Min']/k)}～${Math.floor(plan[key+'Max']/k)} 之間`;};
 check('focus',f,'專注間隔');check('rest',r,'休息時間');check('goal',g,'總目標時間');
 if(!errors.goal&&!errors.focus&&g<f)errors.goal='總目標不能比一次專注還短';
 let delay=0;const mode=document.querySelector('input[name=startMode]:checked').value;
 if(mode==='at'){const v=$('startAt').value;if(!/^\d\d:\d\d$/.test(v))errors.start='請選擇開始時間';else{const [h,m]=v.split(':').map(Number),now=new Date(),at=new Date(now);at.setHours(h,m,0,0);if(at<=now)at.setDate(at.getDate()+1);delay=Math.round((at-now)/1000);if(delay>12*3600)errors.start='開始時間要在 12 小時內';}}
 return {focus:f,rest:r,goal:g,purpose:Number($('purpose').value),delay,errors};
}
function validateForm(){
 const v=readForm();
 for(const key of ['focus','rest','goal']){$(key+'Err').textContent=v.errors[key]||'';$(key+'Input').setAttribute('aria-invalid',String(!!v.errors[key]));}
 $('startErr').textContent=v.errors.start||'';
 const ok=!Object.keys(v.errors).length;$('startButton').disabled=!ok||planBusy||!connected();
 $('formHint').textContent=!connected()?'等待小屋回應…':!ok?'請先把必填欄位設定在範圍內。':plan&&plan.phase!=='NONE'?'重新開始會套用新設定，這一輪計時從頭開始。':'專注間隔與休息時間一定要設定，才能開始。';
 return v;
}
async function startPlan(e){
 e?.preventDefault();await enableAudio();const v=validateForm();if(Object.keys(v.errors).length||planBusy||!connected())return;
 planBusy=true;validateForm();
 try{const res=await post(`/api/plan?focus=${v.focus}&rest=${v.rest}&goal=${v.goal}&purpose=${v.purpose}&delay=${v.delay}`);const next=parsePlan(await res.text());if(!res.ok||!next)throw Error(res.status===422?'range':'net');formTouched=false;receivePlan(next);remember(`設定計畫：${PURPOSES[v.purpose]}，專注 ${fmt(v.focus)}、休息 ${fmt(v.rest)}、總目標 ${fmt(v.goal)}${v.delay?'，'+fmt(v.delay)+' 後開始':''}。`);toast(v.delay?'計畫已排好，時間到會自動開始。':'計畫開始！');restDismissed=false;}
 catch(err){toast(err.message==='range'?'小屋不接受這個設定，請確認範圍。':'沒有送到小屋，請再試一次。');}
 finally{planBusy=false;validateForm();render();await pollNetwork();}
}
async function stopPlan(){
 if(!plan||plan.phase==='NONE'||planBusy||!connected())return;if(!confirm('要結束目前的學習計畫嗎？'))return;
 planBusy=true;try{const res=await post('/api/plan/stop');const next=parsePlan(await res.text());if(next)receivePlan(next);remember('結束了學習計畫。');}catch{toast('沒有送到小屋，請再試一次。');}finally{planBusy=false;validateForm();render();}
}

// ── 互動：都只是畫面，不影響板子判斷 ──
function canPlay(){return fresh()&&planActive()&&life.started&&life.phase!=='DEAD'&&current.state!=='AWAY'&&current.state!=='PHONE';}
function petChick(){if(!canPlay()){toast(current.state==='PHONE'?'先把手機放下，再來摸摸啾啾。':'啾啾在等你，一起好好學習，也好好休息。');return;}if(Date.now()<petUntil)return;petUntil=Date.now()+1800;remember('摸了摸啾啾。');}
function playToy(kind){if(!canPlay()){toast('計畫進行中、人在桌前時才能玩玩具。');return;}if(toy&&Date.now()<toy.until)return;toy={kind,start:Date.now(),until:Date.now()+6000};remember({ball:'和啾啾玩小球。',feather:'拿羽毛逗啾啾。',blocks:'陪啾啾堆積木。'}[kind]);$('toys').classList.add('hidden');$('toyButton').setAttribute('aria-expanded','false');}
function growth(){if(!life?.started||life.phase==='DEAD')return 0;const score=(care?.generation===life.generation?care.feeds:0)+life.cycles*2;return score>=16?4:score>=8?3:score>=3?2:1;}
function mode(){
 if(!networkEnabled||!life)return 'READY';if(!connected())return 'PAUSED';if(life.phase==='DEAD')return 'DEAD';
 if(!plan||plan.phase==='NONE')return 'NOPLAN';if(plan.phase==='WAITING')return 'WAITING';if(life.paused&&current.state!=='AWAY')return 'PAUSED';
 return Object.hasOwn(ACTIVITIES,current.state)?current.state:'PAUSED';
}
function resting(){return connected()&&planActive()&&current.state==='AWAY'&&life&&life.work>0&&!life.paused;}
function setBar(id,ratio){$(id).style.width=Math.max(0,Math.min(100,ratio*100))+'%';}
function render(){
 const m=mode(),a=ACTIVITIES[m],online=fresh(),alive=life?.started&&life.phase!=='DEAD';
 $('connectionDot').classList.toggle('online',connected());$('modeChip').textContent=m==='READY'?'連線中':m==='PAUSED'?'暫停':m==='NOPLAN'?'未設定':STATES[current.state]||'陪伴中';
 $('activityTitle').textContent=a[0];$('activityDescription').textContent=a[1];$('rewardStatus').textContent=a[2];
 $('trackLine').textContent=connected()&&current.tracks?trackText(current.tracks):'';
 $('energyLabel').textContent=life?life.energy+'%':'--';setBar('energyFill',(life?.energy||0)/100);
 $('connectLabel').textContent=connected()?'已連線':'重新連線';$('connectButton').disabled=connected();
 // 互動區
 $('feedButton').disabled=feedBusy||Date.now()<eatUntil||!online||!planActive()||!care?.available||!care.food||!alive||care.generation!==life.generation;
 $('feedLabel').textContent=feedBusy?'送出中…':Date.now()<eatUntil?'享用中…':'餵食';
 $('feedHint').textContent=!connected()?'等小屋回應後才能互動。':!planActive()?'開始計畫後，認真學習就能收集穀穀。':!care?'糧倉還沒準備好。':!alive?'等小雞醒來，再一起吃穀穀。':!care.available?'回到桌前才可以餵食。':!care.food?'讀書或電腦學習，收集第一份穀穀。':'每次餵食用 1 份穀穀，成長 +1。';
 $('foodCount').textContent=care?.food??0;const progress=care?Math.max(0,Math.min(100,Math.floor((care.goal-care.next)/care.goal*100))):0;$('rewardFill').style.width=progress+'%';$('rewardProgress').setAttribute('aria-valuenow',String(progress));
 $('rewardHint').textContent=!care?'讀書或電腦學習才會累積。':online&&current.state==='PHONE'?`手機出現，暫停在剩餘 ${fmt(care.next)}。`:online&&['STUDYING','COMPUTER'].includes(current.state)&&alive?`再學習 ${fmt(care.next)}，收穫下一份。`:`進度保留，還差 ${fmt(care.next)} 有效學習。`;
 $('palmButton').classList.toggle('on',palmMode);$('palmButton').setAttribute('aria-pressed',String(palmMode));$('stage').classList.toggle('palm',palmMode);
 const nextStage=growth();if(nextStage!==stage){const grew=stage>0&&nextStage>stage;stage=nextStage;drawStages();if(grew&&online){celebrateUntil=Date.now()+2200;remember('啾啾長成了'+GROWTH[stage][0]+'。');}}
 $('growthName').textContent=life?`${GROWTH[stage][0]} · 第 ${life.generation} 代`:'一顆小小的期待';
 // 紀錄區
 $('demoNote').hidden=!life?.demo;
 $('planSummary').textContent=!plan?'等待小屋回應…':plan.phase==='NONE'?'還沒有學習計畫，請在下方設定。':`${PURPOSES[plan.purpose]} · 專注 ${fmt(plan.focus)} · 休息 ${fmt(plan.rest)} · `+(plan.phase==='WAITING'?`${fmt(plan.startsIn)} 後開始`:plan.phase==='DONE'?'已達成總目標，繼續陪伴中':'進行中');
 const goalS=plan&&plan.phase!=='NONE'?plan.goal:0,learned=plan&&plan.phase!=='NONE'?Math.min(plan.learned,plan.goal):0;$('goalTime').textContent=`${fmt(learned)} / ${fmt(goalS)}`;setBar('goalFill',goalS?learned/goalS:0);
 const work=life?.work||0,focusGoal=life?.remindGoal||1,restS=life?.rest||0,restGoal=life?.restGoal||1;
 $('sessionTime').textContent=fmt(work);setBar('sessionFill',work/focusGoal);$('sessionBar').className='bar'+(work>=focusGoal?' orange':'');
 $('sessionFoot').textContent=!planActive()?'計畫開始後才計時':work>=focusGoal?'專注時間到了，起身休息吧':`再 ${fmt(focusGoal-work)} 提醒休息`;
 $('restTime').textContent=fmt(restS);setBar('restFill',restS/restGoal);$('restFoot').textContent=resting()?`再休息 ${fmt(restGoal-restS)}`:`休息 ${fmt(restGoal)} 就能恢復活力`;
 $('statDesk').textContent=fmt(current.desk);$('statStudy').textContent=fmt(current.study);$('statComputer').textContent=fmt(current.computer);$('statPhone').textContent=fmt(phoneSeconds);$('statRests').textContent=life?.rests??0;$('statReminders').textContent=life?.reminders??0;
 if(!formTouched)validateForm();else validateForm();
 $('stopButton').disabled=!plan||plan.phase==='NONE'||planBusy||!connected();
 // 休息時整個畫面交給小雞
 const showRest=resting()&&!restDismissed;if($('restOverlay').hidden===showRest){$('restOverlay').hidden=!showRest;if(showRest)restScene.start=performance.now();}
 if(!resting())restDismissed=false;
 $('restOverlayText').textContent=life?`${fmt(restS)} / ${fmt(restGoal)}　離開桌前走一走，回來時啾啾會醒來`:'';
 $('toast').classList.toggle('visible',Date.now()<toastUntil);
}

// ── 像素畫：小雞 16×16，一格一個顏色 ──
const PAL={Y:'#f7cf4a',H:'#fde68a',D:'#d99a25',O:'#e8772e',E:'#2d2018',C:'#f29a86',K:'#d99a25',W:'#ffffff',G:'#3a3026',L:'#cde9f5',R:'#c0584a',N:'#6f9a55',A:'#ffd23f'};
const GRAY={Y:'#c9c3b6',H:'#dcd7cc',D:'#a59e90',O:'#a59e90',E:'#5b544b',C:'#c9c3b6',K:'#a59e90'};
const BODY=[
 '................',
 '......KK........',
 '.....YYYY.......',
 '....YYHYYY......',
 '...YYHYYYYY.....',
 '...YYYYYYYOO....',
 '...YYYYYCYY.....',
 '..YYYYYYYYYY....',
 '.YYDYYYYYYYYY...',
 '.YDDYYYYYYYYY...',
 '.YYDYYYYYYYYY...',
 '..YYYYYYYYYY....',
 '...YYYYYYYY.....',
 '................',
 '................',
 '................'];
const EGG=['....WWWW....','...WWWWWW...','..WWWWWWWW..','..WWHWWWWW..','.WWHWWWWWWW.','.WWWWWWWWWW.','.WWWWWWWWHW.','.WWWWWWWWWW.','..WWWWWWWW..','..WWWWWWWW..','...WWWWWW...','....WWWW....'];
// 座標取整數，像素邊緣才不會糊出格線
function px(c,x,y,s,col){c.fillStyle=col;c.fillRect(Math.round(x),Math.round(y),Math.ceil(s),Math.ceil(s));}
function sprite(c,rows,x,y,s,pal,flip){const w=rows[0].length;rows.forEach((row,j)=>{for(let i=0;i<w;i++){const k=row[i];if(k!=='.'&&pal[k])px(c,x+(flip?w-1-i:i)*s,y+j*s,s,pal[k]);}});}
// o: face 方向、eye 眼睛、feet 腳、wing 翅膀、peck 低頭、glasses、stage、gray
function drawChick(c,x,y,s,o={}){
 const pal=o.gray?{...PAL,...GRAY}:PAL,flip=o.face<0,X=i=>x+(flip?15-i:i)*s,P=(i,j,k)=>px(c,X(i),y+j*s,s,pal[k]||k);
 const head=o.peck?2:0,headX=o.peck?1:0;
 BODY.forEach((row,j)=>{for(let i=0;i<16;i++){const k=row[i];if(k==='.')continue;const isHead=j<=6;const jj=isHead?j+head:j,ii=isHead?i+headX:i;if(ii<16)P(ii,jj,k==='K'&&o.stage>=2?'N':k);}});
 const ey=4+head,ex=8+headX;
 if(o.eye==='closed')P(ex-1,ey,'E'),P(ex,ey,'E'),P(ex+1,ey,'E');
 else if(o.eye==='happy')P(ex-1,ey,'E'),P(ex,ey-1,'E'),P(ex+1,ey,'E');
 else if(o.eye==='half')P(ex,ey,'E'),P(ex+1,ey,'E');
 else if(o.eye==='x')P(ex-1,ey-1,'E'),P(ex+1,ey-1,'E'),P(ex,ey,'E'),P(ex-1,ey+1,'E'),P(ex+1,ey+1,'E');
 else P(ex,ey,'E'),P(ex,ey-1,'E');
 if(o.wing){P(2,8,'Y');P(1,9,'Y');P(2,9,'Y');P(1,7,'D');P(2,7,'D');P(0,8,'D');}
 const f=o.feet??0;
 if(f===0){P(5,13,'O');P(4,14,'O');P(5,14,'O');P(9,13,'O');P(8,14,'O');P(9,14,'O');}
 else if(f===1){P(6,13,'O');P(5,14,'O');P(6,14,'O');P(8,13,'O');P(8,14,'O');P(9,14,'O');}
 // 眼鏡：貼著眼睛的小框，往後一條鏡腳
 if(o.glasses){for(const [i,j] of [[ex-1,ey-2],[ex,ey-2],[ex+1,ey-2],[ex-1,ey+1],[ex,ey+1],[ex+1,ey+1],[ex-2,ey-1],[ex-2,ey],[ex+2,ey-1],[ex+2,ey],[ex-3,ey-1],[ex-4,ey-1]])P(i,j,'G');for(const [i,j] of [[ex-1,ey-1],[ex-1,ey],[ex+1,ey-1],[ex+1,ey]])P(i,j,'L');}
 if(o.stage>=3)for(let i=3;i<=10;i++)P(i,7,'R');
 if(o.stage>=4){P(5,0,'A');P(7,0,'A');P(9,0,'A');for(let i=5;i<=9;i++)P(i,1,'A');}
 if(o.halo)for(let i=4;i<=10;i++)P(i,-1,'A');
}
function heart(c,x,y,s){const H=['.R.R.','RRRRR','RRRRR','.RRR.','..R..'];sprite(c,H,x,y,s,PAL);}
function zee(c,x,y,s,col='#3a3026'){const Z=['GGG','..G','.G.','G..','GGG'];sprite(c,Z,x,y,s,{G:col});}
function bubble(c,x,y,s,kind){
 c.fillStyle='#fffaf0';c.fillRect(x,y,9*s,8*s);c.fillStyle='#3a3026';c.fillRect(x,y,9*s,s);c.fillRect(x,y+7*s,9*s,s);c.fillRect(x,y,s,8*s);c.fillRect(x+8*s,y,s,8*s);c.fillRect(x+2*s,y+8*s,s,s);
 const ix=x+2*s,iy=y+2*s;
 if(kind==='heart')heart(c,ix,iy,s);else if(kind==='z')zee(c,ix+s,iy-s/2,s);else if(kind==='q')sprite(c,['GGG.','...G','.GG.','....','.G..'],ix+s,iy-s/2,s,{G:'#3a3026'});
 else if(kind==='pause')sprite(c,['G.G','G.G','G.G','G.G'],ix+s,iy,s,{G:'#3a3026'});else if(kind==='moon')sprite(c,['.GG.','G...','G...','.GG.'],ix,iy,s,{G:'#4f7f9c'});
 else if(kind==='book')sprite(c,['RRRRR','RWRWR','RWRWR','RRRRR'],ix,iy,s,PAL);
 else if(kind==='sweat')sprite(c,['..B..','.BBB.','BBBBB','.BBB.'],ix,iy,s,{B:'#7fb7d6'});else if(kind==='star')sprite(c,['..A..','AAAAA','.AAA.','A...A'],ix,iy,s,PAL);
}
// 房間：左邊書桌、中間地毯、右邊床和飯碗
function room(c,W,H,s,tint){
 const floor=Math.round(H*.6);
 c.fillStyle=tint==='read'?'#e3ecd2':tint==='type'?'#dbe7ec':'#efe4c9';c.fillRect(0,0,W,floor);
 c.fillStyle='rgba(58,48,38,.06)';for(let x=0;x<W;x+=8*s)c.fillRect(x,0,s,floor);
 c.fillStyle='#c9a978';c.fillRect(0,floor-3*s,W,3*s);c.fillStyle='#d8b887';c.fillRect(0,floor,W,H-floor);
 c.fillStyle='#c4a272';for(let y=floor+6*s;y<H;y+=8*s)c.fillRect(0,y,W,s);
 // 窗
 const wx=Math.round(W*.38),ww=Math.round(W*.26),wy=Math.max(4*s,floor-38*s),wh=Math.min(24*s,floor-wy-8*s);
 if(wh>8*s){c.fillStyle='#9b7650';c.fillRect(wx-2*s,wy-2*s,ww+4*s,wh+4*s);c.fillStyle='#bfe3ea';c.fillRect(wx,wy,ww,wh);c.fillStyle='#ffffff';c.fillRect(wx+3*s,wy+4*s,8*s,2*s);c.fillRect(wx+ww-14*s,wy+9*s,9*s,2*s);c.fillStyle='#9db97a';c.fillRect(wx,wy+wh-5*s,ww,5*s);c.fillStyle='#9b7650';c.fillRect(wx+ww/2-s/2,wy,s,wh);c.fillRect(wx,wy+wh/2,ww,s);}
 // 書架
 const sx=Math.round(W*.72);c.fillStyle='#9b7650';c.fillRect(sx,floor-30*s,W*.22,2*s);[['#6f9a55',3,9],['#c0584a',3,7],['#4f7f9c',4,10],['#e8a72e',3,8]].forEach(([col,w,h],i)=>{c.fillStyle=col;c.fillRect(sx+2*s+i*5*s,floor-30*s-h*s,w*s,h*s);});
 // 書桌與檯燈
 const dx=Math.round(W*.04);c.fillStyle='#a8825a';c.fillRect(dx,floor-14*s,30*s,3*s);c.fillRect(dx+2*s,floor-11*s,2*s,14*s);c.fillRect(dx+26*s,floor-11*s,2*s,14*s);c.fillStyle='#6f9a55';c.fillRect(dx+20*s,floor-22*s,7*s,3*s);c.fillStyle='#3a3026';c.fillRect(dx+23*s,floor-19*s,s,5*s);
 // 床
 const bx=Math.round(W*.70),by=floor+2*s;c.fillStyle='#9b7650';c.fillRect(bx,by-6*s,2*s,14*s);c.fillRect(bx,by+6*s,W*.27,2*s);c.fillStyle='#fffaf0';c.fillRect(bx+2*s,by,7*s,5*s);c.fillStyle='#8fb37a';c.fillRect(bx+9*s,by,W*.27-11*s,6*s);c.fillStyle='#7aa165';for(let i=bx+11*s;i<bx+W*.27-2*s;i+=4*s)c.fillRect(i,by,s,6*s);
 // 地毯與飯碗
 c.fillStyle='#e9d6a6';c.fillRect(W*.26,H-18*s,W*.42,7*s);c.fillStyle='#d8c08a';c.fillRect(W*.26+2*s,H-16*s,W*.42-4*s,3*s);
 const gx=Math.round(W*.62),gy=H-12*s;c.fillStyle='#6b8c9e';c.fillRect(gx,gy,10*s,3*s);c.fillRect(gx+s,gy+3*s,8*s,s);
 if(care?.food){c.fillStyle='#f7cf4a';for(let i=0;i<Math.min(care.food,4);i++)c.fillRect(gx+(2+i*2)*s,gy-s,s,s);}
 return {floor,bowl:{x:gx,y:gy},bed:{x:bx+10*s,y:by-4*s},desk:{x:dx,y:floor}};
}
function book(c,x,y,s,flip){
 c.fillStyle='#c0584a';c.fillRect(x,y,16*s,9*s);c.fillStyle='#fffaf0';c.fillRect(x+s,y+s,7*s,7*s);c.fillRect(x+8*s,y+s,7*s,7*s);
 c.fillStyle='#b9ab93';for(let j=2;j<7;j+=2){c.fillRect(x+2*s,y+j*s,5*s,s);c.fillRect(x+9*s,y+j*s,5*s,s);}
 if(flip){c.fillStyle='#fdf3d0';c.fillRect(x+8*s,y-2*s,5*s,9*s);c.fillStyle='#3a3026';c.fillRect(x+8*s,y-2*s,s,9*s);}
}
function laptop(c,x,y,s,t){
 c.fillStyle='#3a3026';c.fillRect(x,y,18*s,12*s);c.fillStyle='#bfe3ea';c.fillRect(x+s,y+s,16*s,10*s);
 c.fillStyle='#4f7f9c';const lines=Math.floor(t/300)%6;for(let i=0;i<=lines&&i<5;i++)c.fillRect(x+2*s,y+(2+i*2)*s,(5+((i*7)%9))*s,s);
 if(Math.floor(t/400)%2)c.fillRect(x+2*s+((5+((Math.min(lines,4)*7)%9))*s),y+(2+Math.min(lines,4)*2)*s,s,s);
 c.fillStyle='#8a8278';c.fillRect(x-2*s,y+12*s,22*s,3*s);c.fillStyle='#6b645b';for(let i=0;i<9;i++)c.fillRect(x+(i*2)*s,y+13*s,s,s);
}
function toyDraw(c,W,H,s,floor,t,cx){
 if(!toy)return null;const e=(Date.now()-toy.start)/1000;if(e>6){toy=null;return null;}
 if(toy.kind==='ball'){const bx=W*.2+((Math.sin(e*1.3)+1)/2)*W*.55,by=H-14*s-Math.abs(Math.sin(e*5))*14*s;sprite(c,['.RRR.','RRWRR','RRRRR','WWWWW','.RRR.'],bx,by,s*1.5|0||1,PAL);return {target:bx-8*s};}
 if(toy.kind==='feather'){const fx=cx+10*s+Math.sin(e*3)*6*s,fy=H-44*s+Math.cos(e*4)*4*s;sprite(c,['....BB','...BBB','..BBB.','.BBB..','G.....'],fx,fy,s,{B:'#4f7f9c',G:'#3a3026'});return {hop:true};}
 const n=Math.min(5,Math.floor(e*1.2)+1),fall=e>5;['R','N','A','O','R'].slice(0,n).forEach((k,i)=>{const bx=cx+22*s+(fall?i*5*s:((i%2)*2*s)),by=fall?H-16*s:H-16*s-i*5*s;c.fillStyle=PAL[k];c.fillRect(bx,by,6*s,5*s);c.fillStyle='rgba(0,0,0,.15)';c.fillRect(bx,by+4*s,6*s,s);});
 return {build:true};
}
// 手機模式：左半邊整個變成一支手機，小雞在螢幕裡滑影片
function phoneScene(c,W,H,s,t){
 c.fillStyle='#e7d9bd';c.fillRect(0,0,W,H);
 const pw=Math.min(W*.74,(H*.92)*.52),ph=Math.min(H*.92,pw/.52),px0=(W-pw)/2,py0=(H-ph)/2;
 c.fillStyle='#3a3026';c.fillRect(px0,py0,pw,ph);c.fillStyle='#2b2520';c.fillRect(px0+pw*.38,py0+2*s,pw*.24,2*s);
 const sx=px0+3*s,sy=py0+6*s,sw=pw-6*s,sh=ph-12*s;c.fillStyle='#fdf6e3';c.fillRect(sx,sy,sw,sh);
 c.save();c.beginPath();c.rect(sx,sy,sw,sh);c.clip();
 const scroll=(t/40)%(18*s);const colors=['#f3c2b7','#bfe0a4','#bfe3ea','#ffe7ad'];
 for(let i=-1;i<8;i++){const cy=sy+8*s+i*18*s-scroll;c.fillStyle=colors[(i+8+Math.floor(t/720))%4];c.fillRect(sx+3*s,cy,sw-6*s,14*s);c.fillStyle='rgba(58,48,38,.25)';c.fillRect(sx+5*s,cy+10*s,sw*.4,s);heart(c,sx+sw-10*s,cy+3*s,s);}
 c.fillStyle='#d9772e';c.fillRect(sx,sy,sw,7*s);c.fillStyle='#fffaf0';c.fillRect(sx+3*s,sy+2*s,s,3*s);c.fillRect(sx+5*s,sy+2*s,s,3*s);c.fillRect(sx+8*s,sy+3*s,Math.min(sw-12*s,26*s),s);
 const cs=Math.max(2,Math.round(sw/42)),cx=sx+sw/2-8*cs,cy=sy+sh-17*cs;c.fillStyle='rgba(253,246,227,.85)';c.fillRect(sx,cy-2*cs,sw,19*cs);
 drawChick(c,cx,cy,cs,{eye:Math.floor(t/1500)%4===0?'happy':'open',wing:Math.floor(t/350)%2,feet:2,stage});
 c.restore();c.fillStyle='#5b544b';c.fillRect(px0+pw*.4,py0+ph-4*s,pw*.2,s);
}
const scene={w:0,h:0,s:2,x:0,tx:0,face:1,next:0,last:0,chickBox:null};
function stageFrame(ts){
 const cv=$('stage'),c=cv.getContext&&cv.getContext('2d');if(!c){requestAnimationFrame(stageFrame);return;}
 const dpr=window.devicePixelRatio||1,cw=cv.clientWidth,ch=cv.clientHeight;if(!cw||!ch){requestAnimationFrame(stageFrame);return;}
 if(cv.width!==Math.round(cw*dpr)||cv.height!==Math.round(ch*dpr)){cv.width=Math.round(cw*dpr);cv.height=Math.round(ch*dpr);}
 c.imageSmoothingEnabled=false;const W=cv.width,H=cv.height,s=Math.max(2,Math.floor(Math.min(W/150,H/120)));scene.s=s;
 const t=reduced.matches?0:ts,m=mode(),now=Date.now(),dt=Math.min((ts-scene.last)/1000||0,.05);scene.last=ts;
 if(m==='PHONE'){phoneScene(c,W,H,s,t);scene.chickBox=null;requestAnimationFrame(stageFrame);return;}
 const tint=m==='STUDYING'?'read':m==='COMPUTER'?'type':'';const r=room(c,W,H,s,tint);
 const cs=s*2,baseY=H-12*s-16*cs+2*cs;let cx=scene.x,face=scene.face,o={stage,face},bub=null;
 const walk=(target,speed)=>{const d=target-scene.x;if(Math.abs(d)>2){scene.x+=Math.sign(d)*Math.min(Math.abs(d),speed*dt*s);scene.face=Math.sign(d);o.feet=Math.floor(ts/180)%2;return false;}return true;};
 const toyInfo=toyDraw(c,W,H,s,r.floor,t,scene.x);
 if(m==='READY'||m==='NOPLAN'||m==='WAITING'||(life&&!life.started&&m!=='PAUSED'&&m!=='AWAY')){
  const ex=W/2-6*cs,ey=H-14*s-12*cs,wob=reduced.matches?0:(Math.floor(ts/400)%8===0?s:0);
  // 先畫一圈深色當外框，淺色地毯上也看得清楚
  for(const [ox,oy] of [[-1,0],[1,0],[0,-1],[0,1]])sprite(c,EGG,ex+wob+ox*s,ey+oy*s,cs,{W:'#8a7a62',H:'#8a7a62'});sprite(c,EGG,ex+wob,ey,cs,PAL);if(m==='WAITING')bubble(c,ex+10*cs,ey-6*cs,s,'z');else bubble(c,ex+10*cs,ey-6*cs,s,m==='READY'?'q':'star');scene.chickBox={x:ex,y:ey,w:12*cs,h:12*cs};requestAnimationFrame(stageFrame);return;
 }
 if(m==='DEAD'){cx=W/2-8*cs;o={...o,gray:true,eye:'x',halo:true,feet:2};c.save();drawChick(c,cx,baseY+4*cs,cs,o);c.restore();bubble(c,cx+15*cs,baseY-4*cs,s,'moon');}
 else if(m==='AWAY'){cx=r.bed.x;o={...o,eye:'closed',feet:2,face:-1};drawChick(c,cx,r.bed.y-10*cs+(Math.floor(ts/900)%2?s:0),cs,o);zee(c,cx+14*cs,r.bed.y-14*cs-((ts/60)%(10*s)),s);}
 else if(m==='STUDYING'){const at=W*.30;if(walk(at,40)){scene.face=1;o={...o,glasses:true,eye:Math.floor(ts/2600)%5===0?'closed':'open',feet:2};book(c,scene.x+13*cs,baseY+10*cs,s,Math.floor(ts/1300)%3===0);}else o.glasses=true;drawChick(c,scene.x,baseY+(o.feet===2&&Math.floor(ts/1300)%2?s:0),cs,{...o,face:scene.face});bub='book';}
 else if(m==='COMPUTER'){const at=W*.24;if(walk(at,40)){scene.face=1;o={...o,feet:2,wing:Math.floor(ts/160)%2,eye:'open'};laptop(c,scene.x+15*cs,baseY+5*cs,s,ts);}drawChick(c,scene.x,baseY,cs,{...o,face:scene.face});}
 else if(Date.now()<eatUntil){if(walk(r.bowl.x-14*cs,70)){scene.face=1;o={...o,peck:Math.floor(ts/260)%2,feet:0};}drawChick(c,scene.x,baseY,cs,{...o,face:scene.face});bub=null;}
 else{
  if(m==='PAUSED')bub='q';else if(life?.phase==='WEAK'||life?.phase==='TIRED'){bub=life.phase==='WEAK'?'sweat':'moon';o.eye='half';}
  if(m==='PRESENT'||m==='PAUSED'){
   if(toyInfo?.target!==undefined)walk(toyInfo.target,60);else if(!reduced.matches&&m==='PRESENT'){if(now>scene.next){scene.tx=W*(.15+Math.random()*.5);scene.next=now+5000+Math.random()*5000;}walk(scene.tx,life?.phase==='WEAK'?10:25);}
  }
  let y=baseY;if(toyInfo?.hop||now<celebrateUntil)y-=Math.abs(Math.sin(ts/150))*5*s,o.feet=2;
  if(now<petUntil)o.eye='happy',bub='heart';drawChick(c,scene.x,y,cs,{...o,face:scene.face});
 }
 if(m!=='DEAD'&&m!=='AWAY'){scene.chickBox={x:scene.x,y:baseY,w:16*cs,h:16*cs};if(now<petUntil&&!reduced.matches)for(let i=0;i<3;i++)heart(c,scene.x+(3+i*5)*cs,baseY-((ts/12+i*40)%(20*s)),s);if(bub)bubble(c,scene.x+(scene.face>0?14:-8)*cs,baseY-6*cs,s,bub);if(now<celebrateUntil)for(let i=0;i<4;i++)bubble(c,scene.x+(i*6-4)*cs,baseY-10*cs-(i%2)*4*s,s,'star');}
 requestAnimationFrame(stageFrame);
}
if(!scene.x)scene.x=200;
// 休息全畫面：大大的小雞從旁邊走到中間，再睡著
const restScene={start:0};
function restFrame(ts){
 const ov=$('restOverlay');if(!ov.hidden){const cv=$('restStage'),c=cv.getContext&&cv.getContext('2d');if(c&&cv.clientWidth){
  const dpr=window.devicePixelRatio||1;cv.width=Math.round(cv.clientWidth*dpr);cv.height=Math.round(cv.clientHeight*dpr);c.imageSmoothingEnabled=false;
  const W=cv.width,H=cv.height,s=Math.max(4,Math.floor(Math.min(W,H)*.5/16)),e=reduced.matches?99:(ts-restScene.start)/1000;
  c.fillStyle='#25304a';c.fillRect(0,0,W,H);c.fillStyle='#fdf3d0';for(let i=0;i<40;i++){const x=(i*97)%W,y=(i*53)%(H*.6);if((Math.floor(ts/600)+i)%5)c.fillRect(x,y,Math.max(2,s/4),Math.max(2,s/4));}
  sprite(c,['..MMM.','.MM...','MM....','MM....','MM....','.MM...','..MMM.'],W*.8,H*.1,Math.max(3,Math.round(s/2.5)),{M:'#fdf3d0'});
  c.fillStyle='#3b4766';c.fillRect(0,H*.78,W,H*.22);
  const end=W/2-8*s,startX=-16*s,p=Math.min(1,e/3.2),x=startX+(end-startX)*p,y=H*.78-13*s;
  if(p<1)drawChick(c,x,y,s,{face:1,feet:Math.floor(ts/200)%2,stage});
  else{drawChick(c,x,y+(Math.floor(ts/1100)%2?Math.max(1,s/4):0),s,{face:1,eye:'closed',feet:2,stage});for(let i=0;i<3;i++)zee(c,x+(13+i*3)*s,y-((ts/50+i*s*4)%(s*12))+2*s,Math.max(2,s/2),'#fdf3d0');}
 }}
 requestAnimationFrame(restFrame);
}
function drawStages(){const box=$('growthStages');box.replaceChildren(...GROWTH.map((g,i)=>{const d=document.createElement('div'),cv=document.createElement('canvas');cv.width=32;cv.height=32;const c=cv.getContext&&cv.getContext('2d');if(c){if(i===0)sprite(c,EGG,4,4,2,PAL);else drawChick(c,0,2,2,{stage:i});}d.className=(i<=stage?'reached ':'')+(i===stage?'current':'');d.append(cv,document.createTextNode(g[0]));return d;}));}
function exportCsv(){
 const rows=[['時間','事件']].concat([...memories].reverse().map(m=>[m.at.toLocaleString('zh-TW',{hour12:false}),m.text]));
 const csv='\ufeff'+rows.map(r=>r.map(v=>'"'+String(v).replace(/"/g,'""')+'"').join(',')).join('\r\n');
 const a=document.createElement('a');a.href=URL.createObjectURL(new Blob([csv],{type:'text/csv'}));a.download='啾啾紀錄.csv';a.click();setTimeout(()=>URL.revokeObjectURL(a.href),1000);
}
// 點在小雞身上才算摸摸
function stageClick(ev){
 if(!palmMode)return;const cv=$('stage'),r=cv.getBoundingClientRect(),k=cv.width/(r.width||1),x=(ev.clientX-r.left)*k,y=(ev.clientY-r.top)*k,b=scene.chickBox;
 if(b&&x>=b.x-8&&x<=b.x+b.w+8&&y>=b.y-8&&y<=b.y+b.h+8)petChick();
}
$('stage').addEventListener('pointerdown',stageClick);
$('palmButton').onclick=()=>{palmMode=!palmMode;if(palmMode){toast('手掌模式：點一下左邊的啾啾摸摸牠。');petChick();}render();};
$('feedButton').onclick=feed;
$('toyButton').onclick=()=>{const open=$('toys').classList.toggle('hidden');$('toyButton').setAttribute('aria-expanded',String(!open));};
document.querySelectorAll('[data-toy]').forEach(b=>b.onclick=()=>playToy(b.dataset.toy));
$('planForm').addEventListener('submit',startPlan);$('stopButton').onclick=stopPlan;
$('planForm').addEventListener('input',()=>{formTouched=true;validateForm();});
document.querySelectorAll('input[name=startMode]').forEach(r=>r.onchange=()=>{$('startAt').disabled=r.value!=='at'||!r.checked;validateForm();});
$('soundButton').onclick=async()=>{soundEnabled=!soundEnabled;$('soundLabel').textContent=soundEnabled?'叫聲：開':'叫聲：關';$('soundButton').setAttribute('aria-pressed',String(soundEnabled));if(soundEnabled)await enableAudio();};
$('exportButton').onclick=exportCsv;$('connectButton').onclick=connect;$('restClose').onclick=()=>{restDismissed=true;render();};
$('openHome').onclick=()=>{try{const value=$('homeAddress').value.trim(),url=new URL(value.includes('://')?value:'http://'+value);if(url.protocol!=='http:'||!url.hostname||url.username||url.password||url.search||url.hash||url.pathname!=='/')throw Error();location.assign(url.href);}catch{toast('請輸入小屋的 Wi-Fi 位址，例如 192.168.1.88。');}};
document.addEventListener('pointerdown',()=>enableAudio(),{once:true});
document.addEventListener('visibilitychange',()=>{if(!document.hidden&&networkEnabled){suppressSound=true;pollNetwork();}});
setInterval(()=>{if(networkEnabled)pollNetwork();if(fresh()&&current.state==='PHONE')phoneSeconds++;render();},1000);
drawStages();render();requestAnimationFrame(stageFrame);requestAnimationFrame(restFrame);connect();
</script>
</body>
</html>
)PETPAGE";
