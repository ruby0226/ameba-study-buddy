# 啾啾的小屋｜Ameba 讀書小夥伴

「讓 Ameba 看懂生活」專題。AMB82-MINI 在板上用攝影機辨識桌前活動，再由板子透過 Wi-Fi 提供網頁，用像素小雞陪伴學習、提醒休息與發放學習獎勵。

## 主要功能

| 辨識情境 | 網頁與獎勵 |
| --- | --- |
| 讀書 | 綠色讀書場景，小雞翻書，累積穀穀 |
| 使用電腦 | 藍色電腦場景，小雞打字，累積穀穀 |
| 手機出現 | 橘色暫停提示，獎勵倒數暫停並保留進度 |
| 在桌前（無書、電腦、手機） | 一般場景，不累積獎勵 |
| 離席休息 | 小雞休息，完成休息後恢復活力，死亡後完成休息會重生 |

- Ameba 負責辨識、活動判斷、計時、獎勵和餵食驗證，網頁負責畫面與叫聲。
- 手機優先於書本，書本優先於電腦。手機出現只代表「先暫停獎勵」，不代表判定正在滑手機。
- 物件偵測到就開始追蹤並給 ID：分數 ≥50 立刻追，其餘 2 秒內再看到一次即追。
- **畫面追蹤**：YOLO 框到時記住那塊畫面的樣子（16×16 灰階樣板），之後 YOLO 漏看，就在上次位置附近 ±8 像素找最像的地方繼續跟。連續 3 秒都找不到，或從畫面左右上緣拿出 1 秒，才算消失。只靠畫面最多撐 5 秒，之後要 YOLO 再看到一次，避免框黏在背景上。RTSP 上 `track` 表示 YOLO 框到，`vis` 表示靠畫面追蹤，`lost` 表示暫時找不到。
- 畫面追蹤用另一個 640×360 JPEG 拍照通道，解成 160×90 灰階，在板子上比對。主程式 `USE_VISUAL_TRACK` 改成 0 可以關掉。

## 材料

| 材料 | 數量 | 用途 |
| --- | --- | --- |
| Realtek AMB82-MINI 開發板（含隨附攝影機） | 1 | 影像輸入與板上辨識 |
| USB 資料線 | 1 | 燒錄、序列埠、供電 |
| USB 充電器 | 選用 | 日常使用時供電，不需接電腦 |
| 電腦或手機（Chrome／Edge） | 1 | 開啟小屋網頁，同一個 Wi-Fi |
| 鏡頭固定架 | 1 | 固定角度，讓人與桌面同時入鏡 |
| 書、手機、筆電 | 各 1 | 辨識目標 |

## 接線

不需要額外接線，攝影機接在板子的相機排線座，USB 接電腦即可。程式保留 RGB LED 腳位（R=7、G=8、B=4），預設 `USE_RGB_LED 0` 不啟用。

## 軟體與設定

| 項目 | 設定 |
| --- | --- |
| 開發環境 | Arduino IDE 2，或 arduino-cli |
| SDK | Realtek AmebaPro2 Arduino 套件 `4.1.1-build20260922`，板型 **AMB82-MINI**（`realtek:AmebaPro2:Ameba_AMB82-MINI`） |
| 模型 | 官方內建 YOLOv7-tiny 物件偵測（`DEFAULT_YOLOV7TINY`），不自行訓練 |
| 模型輸入 | 576×320、10 fps、RGB（相機通道 3） |
| RTSP 串流 | 1920×1080、30 fps、H.264、2 Mbps（相機通道 0），VLC 開 `rtsp://<IP>:554` |
| 追蹤用拍照 | 640×360、10 fps、JPEG（相機通道 1），解成 160×90 灰階 |
| 模型門檻 | 信心 0.15、NMS 0.3 |
| 類別門檻 | 人 50、書 20、手機 25、筆電 60（攤開的書常被看成筆電，所以較高）。只看筆電判斷用電腦，鍵盤滑鼠不算。書在主角旁邊又在桌上、手機在主角旁邊時 15 也可；追蹤中延續 15 |
| 使用者 | 畫面裡有好幾個人時，只認框最大（最靠近鏡頭）的人，RTSP 上是白框，其他人是黃框、不算 |
| 範圍 | 全畫面，不另外限制桌面區 |
| 在座／離席 | 人穩定 3 秒算在座，消失 10 秒算離席，離席後要穩定看到人 3 秒才算回座 |

## 燒錄與操作

1. Arduino IDE 的開發板管理員安裝 AmebaPro2 套件，選擇 **AMB82-MINI** 與正確的序列埠。
2. 將 `Day3_StudyBuddy_Verify/WiFiSecrets.example.h` 複製為同資料夾的 `WiFiSecrets.h`，填入 Wi-Fi 名稱與密碼。這個檔案不會上傳。
3. 開啟 `Day3_StudyBuddy_Verify/Day3_StudyBuddy_Verify.ino`，編譯並上傳。若無法自動進入燒錄模式：按住 BOOT，按一下 RST，再放開 BOOT，然後上傳。
4. 上傳完成後只按一下 **RST** 啟動。
5. 序列埠監控設為 **115200**，找到 `Pet web: http://<IP>/`。
6. 電腦或手機連同一個 Wi-Fi，開啟該網址，按「開始陪伴」。在座時網頁要保持在前景，離席休息時可以關掉或鎖屏。

命令列方式：

```text
arduino-cli compile --fqbn realtek:AmebaPro2:Ameba_AMB82-MINI --build-path .build/amb82 Day3_StudyBuddy_Verify
arduino-cli upload --fqbn realtek:AmebaPro2:Ameba_AMB82-MINI --port COM3 --input-file .build/amb82/flash_ntz.bin Day3_StudyBuddy_Verify
```

## 網頁

板子直接提供網頁，開啟後會自動連線。畫面分成左右兩邊：

- **左邊：小雞生活區**，固定不動，右邊捲動時不會跟著跑。全部是像素畫：
  - 讀書：小雞戴眼鏡看書、翻頁；
  - 電腦：小雞在筆電前打字，螢幕文字一行行出現；
  - 手機：整個左半邊變成一支手機，小雞在螢幕裡滑影片，學習獎勵暫停；
  - 在桌前：小雞散步；吃飯時走到碗邊啄穀穀；被摸時冒愛心；
  - 休息：整個網頁變成夜空，大大的小雞從左邊走到中間睡覺，顯示休息倒數。可以按「縮小畫面」先關掉。
  - 疲倦、虛弱、死亡、還沒孵化的蛋各有不同樣子。
- **右邊：功能區**，可以捲動：
  1. **互動**：摸摸（手掌模式，點左邊的小雞）、餵食、玩具（小球、羽毛、積木）。摸摸和玩具只是畫面互動，不影響辨識和獎勵。
  2. **紀錄與時間**：計畫狀態、總目標進度、這一輪專注與休息倒數、開機以來的在桌前／讀書／電腦／手機時間、完成休息與提醒次數。
  3. **設定與事件紀錄**：學習計畫表單、叫聲開關、每次事件的時間紀錄，可以匯出成 CSV。

## 學習計畫（必填）

一定要先設定計畫，小雞才會開始計時。範圍由板子決定，超出範圍網頁不能送出，板子也會拒絕。

| 欄位 | 一般版 | 展示版（`PET_DEMO=1`） |
| --- | --- | --- |
| 目的 | 讀書、電腦作業、讀書＋電腦 | 同左 |
| 開始 | 立即，或 12 小時內的指定時間 | 同左 |
| 總目標時間 | 30 分鐘～4 小時，只算有效學習（讀書、電腦） | 60～1800 秒 |
| 專注間隔（到了提醒休息） | 15～60 分鐘 | 30～300 秒 |
| 休息時間（確認離席後要累積的時間） | 3～20 分鐘 | 10～120 秒 |

- 總目標不能比一次專注間隔還短。
- 提醒休息後，再撐專注間隔的四成（最少 15 秒）活力會歸零。
- 每份穀穀需要的有效學習時間：一般版 5 分鐘、展示版 15 秒。
- 達成總目標後，小屋仍會繼續陪伴和提醒休息。
- 重新設定計畫，這一輪的計時會從頭開始。板子重開機後需要重新設定。

板子介面（都要 POST 並帶 `X-StudyBuddy: 1`）：`/api/poll?after=<事件編號>`、`/api/feed?boot=…&revision=…`、`/api/plan?focus=秒&rest=秒&goal=秒&purpose=0~2&delay=秒`、`/api/plan/stop`。

在座時網頁關閉或斷線超過 3 秒、辨識結果超過 2 秒沒更新，會暫停養成。離席休息不需要網頁開著，看不清楚的片刻只暫停累積、不會把已休息的時間歸零。資料存在板子 RAM，重開機會重設。

## 測試

第三週測試用 `scripts/run-trials.ps1`。每次重新擺好後按 Enter，腳本讀板子 10 秒，記錄預期、實際狀態與是否符合，寫入 `第三週_測試紀錄.csv`：

```text
powershell -ExecutionPolicy Bypass -File scripts\run-trials.ps1 -Board <板子IP> -Condition 正常 -Expect STUDYING -Setup "書平放，鏡頭俯角約 30 度"
powershell -ExecutionPolicy Bypass -File scripts\run-trials.ps1 -Board <板子IP> -Condition 干擾 -Expect STUDYING -Setup "背光"
powershell -ExecutionPolicy Bypass -File scripts\run-trials.ps1 -Board <板子IP> -Condition 無目標 -Expect "!STUDYING,!PHONE"
```

規則與網頁的自動測試見 [技術說明](專案技術說明.md)。自動測試不代表攝影機實測結果。

## 檔案

| 檔案 | 內容 |
| --- | --- |
| `Day3_StudyBuddy_Verify/Day3_StudyBuddy_Verify.ino` | 主程式：相機、模型、狀態判斷、RTSP 標示 |
| `SceneRelations.h` | 各類物品的門檻、主角（最大的人）與互相衝突的判斷 |
| `ObjectTracks.h` | 物件追蹤：開始、延續與消失 |
| `VisualTracker.h`、`GrayCamera.h` | 畫面追蹤：灰階樣板比對，與 JPEG 拍照解碼 |
| `StudyActivity.h` | 讀書／電腦／手機的優先順序與獎勵規則 |
| `PetLife.h`、`PetCare.h`、`PetPlan.h` | 休息、活力、死亡重生、穀穀與餵食、學習計畫與範圍檢查 |
| `PetNetwork.h`、`PetWebPage.h` | 板子上的網頁伺服器與內嵌網頁 |
| `PetSound.h` | 麥克風環境音量 |
| `web/pet_test.html` | 網頁原始檔，修改後執行 `node scripts/embed-web.cjs` |
| `tests/` | 規則、網頁與瀏覽器測試 |

基於 [Realtek AmebaPro2 SDK](https://github.com/Ameba-AIoT/ameba-arduino-pro2) 與官方 YOLOv7-tiny。活動判斷、追蹤、Wi-Fi 小屋、休息養成、學習獎勵與測試工具為本組整合，並以生成式 AI 協助撰寫程式，組員負責理解、實測與結果確認。
