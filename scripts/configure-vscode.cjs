// 跟實際編譯用同一份 SDK 路徑，換電腦再跑一次
const fs = require('node:fs');
const path = require('node:path');
const root = path.resolve(__dirname, '..');
const database = path.join(root, '.build/amb82/compile_commands.json');
if (!fs.existsSync(database)) throw new Error('請先依 README 編譯到 .build/amb82');
const commands = JSON.parse(fs.readFileSync(database, 'utf8').replace(/^\uFEFF/, ''));
const entry = commands.find(row => row.file.endsWith('Day3_StudyBuddy_Verify.ino.cpp'));
if (!entry?.arguments) throw new Error('找不到主程式的編譯參數');
const args = entry.arguments;
const normalize = value => value.replace(/\\/g, '/');
const includes = [...new Set(args.filter(arg => arg.startsWith('-I')).map(arg =>
  normalize(path.resolve(entry.directory, arg.slice(2)))))]
  .filter(dir => fs.existsSync(dir));
const arduino = includes.map(dir => path.join(dir, 'Arduino.h')).find(file => fs.existsSync(file));
if (!arduino) throw new Error('找不到 Arduino.h，請確認 SDK 已安裝');
let compiler = args[0];
if (!fs.existsSync(compiler) && fs.existsSync(compiler + '.exe')) compiler += '.exe';
if (!fs.existsSync(compiler)) throw new Error('找不到 ARM 編譯器');
const folder = path.join(root, '.vscode');
fs.mkdirSync(folder, { recursive: true });
const config = {
  configurations: [{
    name: 'Ameba AMB82-MINI',
    compilerPath: normalize(compiler),
    compilerArgs: args.filter(arg => arg.startsWith('-m')),
    intelliSenseMode: 'windows-gcc-arm', cStandard: 'gnu11', cppStandard: 'gnu++11',
    includePath: ['${workspaceFolder}/Day3_StudyBuddy_Verify', ...includes],
    defines: args.filter(arg => arg.startsWith('-D') && !arg.includes('{build.')).map(arg => arg.slice(2)),
    forcedInclude: [normalize(arduino)]
  }], version: 4
};
fs.writeFileSync(path.join(folder, 'c_cpp_properties.json'), JSON.stringify(config, null, 2) + '\n');
const settingsFile = path.join(folder, 'settings.json');
const settings = fs.existsSync(settingsFile) ? JSON.parse(fs.readFileSync(settingsFile, 'utf8')) : {};
settings['files.associations'] = { ...settings['files.associations'], '*.ino': 'cpp' };
fs.writeFileSync(settingsFile, JSON.stringify(settings, null, 2) + '\n');
const headers = ['WiFi.h', 'StreamIO.h', 'VideoStream.h', 'RTSP.h', 'NNObjectDetection.h', 'VideoStreamOverlay.h', 'FreeRTOS.h', 'semphr.h', 'AudioStream.h'];
for (const header of headers) {
  if (!includes.some(dir => fs.existsSync(path.join(dir, header)))) throw new Error('無法找到 ' + header);
}
console.log('PASS: VS Code uses the ARM compiler and SDK include paths; all main SDK headers resolved.');
