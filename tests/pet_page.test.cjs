const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');
const html = fs.readFileSync(require('node:path').join(__dirname,'../web/pet_test.html'),'utf8');
assert.doesNotMatch(html,/navigator\.serial|拍手|拍兩下/);
assert.doesNotMatch(html,/\p{Extended_Pictographic}/u, 'All artwork is pixel art, not emoji');
assert.doesNotMatch(html,/<img|<link|\.png|\.jpg|src=["']https?:|url\(["']?https?:/, 'No outside images or network assets');
const elements = new Map();
function node(){return {textContent:'',innerHTML:'',value:'',hidden:false,disabled:false,style:{setProperty(){}},dataset:{},className:'',
 classList:{set:new Set(),add(c){this.set.add(c);},remove(c){this.set.delete(c);},toggle(c,on){const v=on===undefined?!this.set.has(c):!!on;v?this.set.add(c):this.set.delete(c);return v;},contains(c){return this.set.has(c);}},
 attrs:{},setAttribute(k,v){this.attrs[k]=v;},append(){},replaceChildren(...k){this.children=k;},addEventListener(t,f){this['on'+t]=f;},getBoundingClientRect(){return {left:0,top:0,width:100,height:100};},clientWidth:0,clientHeight:0};}
function element(id){assert.ok(html.includes(`id="${id}"`),id);if(!elements.has(id))elements.set(id,node());return elements.get(id);}
element('restOverlay').hidden=true;
let now=10000, fetchImpl=async()=>{throw Error('offline');}, chirps=0, requests=[];
class Audio {constructor(){this.state='running';this.currentTime=0;this.destination={};}async resume(){} createOscillator(){return {frequency:{setValueAtTime(){},exponentialRampToValueAtTime(){}},connect(){},disconnect(){},start(){chirps++;},stop(){}};}createGain(){return {gain:{setValueAtTime(){},exponentialRampToValueAtTime(){}},connect(){},disconnect(){}};}}
const startMode={value:'now',checked:true};
const context=vm.createContext({document:{getElementById:element,querySelectorAll:s=>s==='input[name=startMode]'?[startMode]:[],querySelector:()=>startMode,createElement:node,createTextNode:t=>({textContent:t}),addEventListener(){}},
 window:{AudioContext:Audio,addEventListener(){},devicePixelRatio:1},matchMedia:()=>({matches:false}),requestAnimationFrame(){},performance:{now:()=>now},
 Date:class extends Date {constructor(...a){super(...(a.length?a:[now]));} static now(){return now;}},console,URL,AbortController,setTimeout:()=>0,clearTimeout(){},setInterval(){},confirm:()=>true,
 fetch:(...a)=>{requests.push(a[0]);return fetchImpl(...a);},location:{protocol:'http:',assign(v){this.assigned=v;}}});
const run=code=>vm.runInContext(code,context);
run(html.match(/<script>([\s\S]*?)<\/script>/)[1]);
const life=(phase='HEALTHY',reminders=0,paused=0,work=120,rest=0)=>`PET,1000,${phase},90,${work},${rest},0,0,1,${reminders},${paused},1500,300,600,0,1`;
const care=(revision=2,food=2,feeds=0,boot=123)=>`CARE,${boot},${revision},1,${food},${feeds},600,300,300,0,1`;
const planLine=(phase=0,rev=0)=>`PLAN,${rev},${phase},1500,300,3600,0,0,0,900,3600,180,1200,1800,14400`;
let planNow=planLine();
const snapshot=(seq=1,p=life(),c=care(),event='',activity='STUDYING')=>`NET,${seq},0,${seq*1000}\nSTATE,10,${activity},120,60,20,A,0,0,-,0,0,${activity==='PHONE'?1:0},0,1,1,0,book-3-12-0\n${p}\n${c}\n${planNow}\n${event}`;
const send=s=>context.applySnapshot(s);
async function main(){
 // 解析：壞資料一律丟掉
 assert.equal(context.parseLife('PET,broken'),null);
 assert.equal(context.parseCare('CARE,1,1,1,100,0,0,1,300,0,1'),null);
 assert.equal(context.parseState('STATE,0,MANUAL,0,0,0,A,0,0,-,0,0,0,0'),null);
 assert.equal(context.parsePlan('PLAN,1,4,1,1,1,0,0,0,1,2,1,2,1,2'),null,'unknown phase');
 assert.equal(context.parsePlan('PLAN,1,0,1,1,1,0,0,0,5,2,1,2,1,2'),null,'min above max');
 assert.equal(context.parsePlan(planLine()).focusMax,3600);
 const tracked=context.parseState('STATE,0,STUDYING,0,0,0,A,0,0,-,0,0,0,0,10,20,0,book-3-12-0|phone-5-4-1|cat-1-1-0');
 assert.equal(tracked.tracks.length,2);assert.match(context.trackText(tracked.tracks),/書本 #3 · 00:12.*手機 #5 · 暫時看不到/);
 // 連上但沒有計畫：不能餵、一定要先設定
 fetchImpl=async()=>({ok:true,text:async()=>snapshot()});
 assert.equal(run("networkEnabled"),true,"page connects by itself");await run("networkTask");await context.pollNetwork();
 assert.equal(run('mode()'),'NOPLAN');assert.equal(element('activityTitle').textContent,'等你設定計畫');
 assert.equal(element('focusInput').value,'25');assert.equal(element('restInput').value,'5');assert.equal(element('focusRange').textContent,'可設定 15～60 分鐘');
 assert.equal(element('startButton').disabled,false);
 // 超出範圍：擋下，不送出
 element('restInput').value='1';run('formTouched=true');run('validateForm()');
 assert.equal(element('startButton').disabled,true);assert.match(element('restErr').textContent,/3～20/);
 element('restInput').value='';run('validateForm()');assert.match(element('restErr').textContent,/請填寫休息時間/);
 element('goalInput').value='40';element('focusInput').value='50';element('restInput').value='5';run('validateForm()');assert.match(element('goalErr').textContent,/不能比一次專注還短/);
 requests=[];await context.startPlan();assert.equal(requests.filter(u=>u.startsWith('/api/plan')).length,0,'invalid plan never sent');
 // 合法設定：送出秒數
 element('goalInput').value='60';element('focusInput').value='30';element('restInput').value='10';element('purpose').value='1';
 fetchImpl=async(url)=>{if(url.startsWith('/api/plan?')){planNow=planLine(2,1);return {ok:true,status:200,text:async()=>planNow};}return {ok:true,text:async()=>snapshot(2)};};
 requests=[];await context.startPlan();
 assert.ok(requests.includes('/api/plan?focus=1800&rest=600&goal=3600&purpose=1&delay=0'),requests.join());
 assert.equal(run('plan.phase'),'RUNNING');assert.ok(run('memories.some(m=>m.text.startsWith("設定計畫"))'));
 // 三種活動
 send(snapshot(3));assert.equal(element('activityTitle').textContent,'戴上眼鏡一起讀書');assert.match(element('trackLine').textContent,/書本 #3/);
 send(snapshot(4,life(),care(),'', 'COMPUTER'));assert.equal(element('activityTitle').textContent,'電腦學習時間');
 send(snapshot(5,life(),care(),'', 'PHONE'));assert.equal(element('rewardStatus').textContent,'獎勵已暫停');assert.match(element('rewardHint').textContent,/暫停/);
 assert.equal(run('canPlay()'),false,'no toys while the phone is out');
 // 叫聲只在兩階段提醒
 send(snapshot(6,life('TIRED',1)));assert.equal(chirps,3);
 send(snapshot(7,life('TIRED',1)));assert.equal(chirps,3);
 send(snapshot(8,life('WEAK',1)));assert.equal(chirps,5);
 send(snapshot(9,life('WEAK',1),care(),'EVENT,1000,1,CLAP'));assert.ok(!run('memories.some(m=>/CLAP|拍/.test(m.text))'));
 send(snapshot(10,life(),care(),'EVENT,1000,2,GOAL'));assert.ok(run('memories.some(m=>m.text.includes("總目標"))'));
 // 互動：摸摸和玩具只改畫面
 send(snapshot(11,life(),care(3,3)));const rev=run('care.revision');
 run('petChick()');assert.ok(run('petUntil')>now);assert.equal(run('care.revision'),rev);
 run('playToy("ball")');assert.equal(run('toy.kind'),'ball');
 // 餵食連點只送一次
 let feedCalls=0;
 fetchImpl=async(url,opts)=>{assert.equal(opts.method,'POST');assert.equal(opts.headers['X-StudyBuddy'],'1');if(url.startsWith('/api/feed')){feedCalls++;assert.match(url,/boot=123&revision=3$/);return {ok:true,text:async()=>care(4,2,1)};}return {ok:true,text:async()=>snapshot(12,life(),care(4,2,1))};};
 await context.feed();await context.feed();assert.equal(feedCalls,1);assert.equal(run('care.feeds'),1);assert.ok(run('eatUntil')>now);
 context.receiveCare(context.parseCare(care(3,3)));assert.equal(run('care.revision'),4,'out-of-order poll must not undo feed');
 // 休息：整頁變成小雞休息畫面，可以縮小
 now+=6000;send(snapshot(13,life('HEALTHY',1,0,60,4),care(4,2,1),'', 'AWAY'));
 assert.equal(element('restOverlay').hidden,false);assert.match(element('restOverlayText').textContent,/00:04 \/ 05:00/);
 element('restClose').onclick();assert.equal(element('restOverlay').hidden,true);
 send(snapshot(14,life('HEALTHY',1,0,0,0),care(4,2,1),'', 'STUDYING'));assert.equal(run('restDismissed'),false,'next rest shows again');
 // 斷線：暫停、不補叫
 now+=3500;run('render()');assert.equal(run('fresh()'),false);assert.equal(element('feedButton').disabled,true);
 const before=chirps;send(snapshot(15,life('TIRED',2)));assert.equal(chirps,before,'reconnect does not replay old chirps');
 // 重開機：清掉計畫
 send(`NET,1,0,10\nSTATE,10,STUDYING,0,0,0,A,0,0,-,0,0,0,0\n${life()}\n${care(1,0,0,999)}\n${planLine()}`);assert.equal(run('plan.phase'),'NONE');
 await context.disconnect();assert.equal(run('networkEnabled'),false);
 console.log('PASS: plan required and range checked, distinct study/computer/phone scenes, rest overlay, toys and petting, feeding guard, chirps and reconnect');
}
main().catch(e=>{console.error(e);process.exitCode=1;});
