// 網頁打包進韌體，修改 HTML 後跑一次
const fs = require('node:fs');
const path = require('node:path');
const root = path.resolve(__dirname, '..');
const html = fs.readFileSync(path.join(root, 'web/pet_test.html'), 'utf8').replace(/\r\n/g, '\n');
if (html.includes(')PETPAGE"')) throw new Error('Raw string delimiter collision');
const output = '// 由 scripts/embed-web.cjs 產生，請修改 web/pet_test.html\n#pragma once\nstatic const char PET_WEB_PAGE[] = R"PETPAGE(' + html + ')PETPAGE";\n';
const target = path.join(root, 'Day3_StudyBuddy_Verify/PetWebPage.h');
if (process.argv.includes('--check')) {
  if (!fs.existsSync(target) || fs.readFileSync(target, 'utf8') !== output) throw new Error('Embedded web page is out of date');
  console.log('PASS: embedded web page matches source');
} else {
  fs.writeFileSync(target, output);
  console.log('Embedded web page: ' + Buffer.byteLength(html) + ' bytes');
}
