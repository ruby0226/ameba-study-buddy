const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');
const html = fs.readFileSync(require('node:path').join(__dirname,'../web/pet_test.html'),'utf8');
assert.doesNotMatch(html,/navigator\.serial|data-mode|data-sim|csvHistory|chipBook|checkTarget|拍手|拍兩下/);
assert.doesNotMatch(html,/\p{Extended_Pictographic}/u, 'All artwork is pixel SVG, not emoji');
const elements = new Map();
function node(){return {textContent:'',innerHTML:'',style:{setProperty(){}},dataset:{},classList:{add(){},toggle(){}},setAttribute(){},append(){},replaceChildren(){},showModal(){this.open=true;},close(){this.open=false;},getBoundingClientRect(){return {left:0,top:200,width:960,height:420};},offsetWidth:78,offsetHeight:94,clientWidth:1100,clientHeight:1300};}
function element(id){assert.ok(html.includes(`id="${id}"`),id);if(!elements.has(id))elements.set(id,node());return elements.get(id);}
let now=10000, fetchImpl=async()=>{throw Error('offline');}, chirps=0;
class Audio {constructor(){this.state='running';this.currentTime=0;this.destination={};}async resume(){} createOscillator(){return {frequency:{setValueAtTime(){},exponentialRampToValueAtTime(){}},connect(){},disconnect(){},start(){chirps++;},stop(){}};}createGain(){return {gain:{setValueAtTime(){},exponentialRampToValueAtTime(){}},connect(){},disconnect(){}};}}
const context=vm.createContext({document:{getElementById:element,querySelectorAll:()=>[],createElement:node,addEventListener(){}},window:{AudioContext:Audio,addEventListener(){}},matchMedia:()=>({matches:false}),requestAnimationFrame(){},Date:class extends Date {static now(){return now;}},console,URL,AbortController,setTimeout:()=>0,clearTimeout(){},setInterval(){},fetch:(...a)=>fetchImpl(...a),location:{protocol:'http:',assign(v){this.assigned=v;}}});
const run=code=>vm.runInContext(code,context);
run(html.match(/<script>([\s\S]*?)<\/script>/)[1]);
const life=(phase='HEALTHY',reminders=0,paused=0)=>`PET,1000,${phase},90,120,0,0,0,1,${reminders},${paused},1500,300,600,0,1`;
const care=(revision=2,food=2,feeds=0,boot=123)=>`CARE,${boot},${revision},1,${food},${feeds},600,300,300,0,1`;
const snapshot=(seq=1,p=life(),c=care(),event='')=>`NET,${seq},0,${seq*1000}\nSTATE,10,STUDYING,120,60,20,A,0,0,-,0,0,0,0\n${p}\n${c}\n${event}`;
const send=s=>context.applySnapshot(s);
async function main(){
 assert.equal(element('feedButton').disabled,true);
 assert.equal(context.parseLife('PET,broken'),null);
 assert.equal(context.parseCare('CARE,1,1,1,100,0,0,1,300,0,1'),null);
 assert.equal(context.parseCare('CARE,1,1,1,,0,0,1,300,0,1'),null);
 assert.equal(context.parseState('STATE,0,MANUAL,0,0,0,A,0,0,-,0,0,0,0'),null);
 assert.equal(context.parseState('STATE,0,UNSURE,0,0,0,A,0,0,-,0,0,0,0').state,'UNSURE');
 fetchImpl=async()=>({ok:true,text:async()=>snapshot()});
 await context.connect();assert.equal(run('fresh()'),true);assert.equal(element('foodCount').textContent,2);
 const received=run('lastSnapshotAt');now+=1000;send(snapshot());assert.equal(run('lastSnapshotAt'),received);
 now+=2200;send(snapshot());assert.equal(run('fresh()'),false);assert.equal(element('feedButton').disabled,true);
 send(snapshot(2));assert.equal(run('fresh()'),true);
 send(snapshot(3,life('TIRED',1)));assert.equal(chirps,3);
 send(snapshot(4,life('TIRED',1)));assert.equal(chirps,3);
 send(snapshot(5,life('WEAK',1)));assert.equal(chirps,5);
 send(snapshot(6,life('WEAK',1),care(),'EVENT,1000,1,CLAP'));assert.equal(run('memories.length'),0);
 send(snapshot(7,life(),care(3,3)));assert.equal(run('memories.length'),1);
 const beforePet=run('care.revision');element('petButton').onclick();assert.equal(run('care.revision'),beforePet,'petting is cosmetic');
 let feedCalls=0;
 fetchImpl=async(url,opts)=>{assert.equal(opts.method,'POST');assert.equal(opts.headers['X-StudyBuddy'],'1');if(url.startsWith('/api/feed')){feedCalls++;assert.match(url,/boot=123&revision=3$/);return {ok:true,text:async()=>care(4,2,1)};}return {ok:true,text:async()=>snapshot(8,life(),care(4,2,1))};};
 await context.feed();await context.feed();assert.equal(feedCalls,1);assert.equal(run('care.feeds'),1);assert.equal(run('care.food'),2);assert.ok(run('eatUntil')>now);
 context.receiveCare(context.parseCare(care(3,3)));assert.equal(run('care.revision'),4,'out-of-order poll must not undo feed');
 now+=7000;send(snapshot(9,life(),care(5,2,3)));assert.equal(run('growth()'),2);
 now+=7000;send(snapshot(10,life(),care(6,0,3)));await context.feed();assert.equal(feedCalls,1,'no food cannot send feed');
 send(snapshot(11,life(),care(7,1,3)));let attempted=0;
 fetchImpl=async url=>{if(url.startsWith('/api/feed')){attempted++;throw Error('response lost');}return {ok:true,text:async()=>snapshot(12,life(),care(8,0,4))};};
 await context.feed();assert.equal(attempted,1);assert.equal(run('care.food'),0,'lost response reconciles by polling');
 now+=7000;send(snapshot(1,life('WEAK',1),care(0,0,0,456)));assert.equal(run('care.boot'),456);assert.equal(chirps,5,'reboot must not replay reminders');
 await context.disconnect();assert.equal(run('fresh()'),false);assert.equal(element('feedButton').disabled,true);
 fetchImpl=async()=>({ok:true,text:async()=>snapshot(2,life('TIRED',1),care(0,0,0,456))});await context.connect();assert.equal(chirps,5,'reconnect is quiet');
 run('soundEnabled=false');send(snapshot(3,life('WEAK',1),care(0,0,0,456)));assert.equal(chirps,5);
 element('homeAddress').value='javascript:alert(1)';element('openHome').onclick();assert.equal(context.location.assigned,undefined);
 element('homeAddress').value='192.168.1.88';element('openHome').onclick();assert.equal(context.location.assigned,'http://192.168.1.88/');
 await context.disconnect();context.location.protocol='file:';await context.connect();assert.equal(element('settings').open,true);assert.equal(run('networkEnabled'),false);
 console.log('PASS: pixel UI, telemetry validation/freshness, reminders, rewards, feed reconciliation, reboot and reconnect');
}
main().catch(e=>{console.error(e);process.exitCode=1;});
