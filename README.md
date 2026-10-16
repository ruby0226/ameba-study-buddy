# 讀書小夥伴：Ameba 影像辨識與小雞電子寵物

「讓 Ameba 看懂生活——影像辨識創意實作」第二週原型。AMB82-MINI 在板上辨識人、書本、手機及電腦相關物件，自動記錄桌前活動；同一個 Wi-Fi 的手機或電腦開啟 Ameba 提供的網頁，即可看到電子寵物、休息提醒及事件紀錄。

活動全自動判斷，沒有手動讀書模式或模擬辨識按鈕。網頁不透過 USB 收資料；USB 用於首次燒錄及供電。此專案沒有加入 OTA。

GitHub：[ruby0226/ameba-study-buddy](https://github.com/ruby0226/ameba-study-buddy)。第二週操作影片尚待錄製與上傳。

## 目前進度

2026-10-16：程式已加入時間一致判斷、物件位置關聯、Wi-Fi 網頁、休息養成、事件紀錄與麥克風互動。網頁自動測試及 AMB82-MINI 編譯已通過。先前 Wi-Fi 韌體曾成功燒錄；本次兩階段叫聲與設定檔整理版本仍需重新燒錄和實機驗收。

以下項目尚未完成：系統化的書本／手機辨識率、拍手誤觸率、長時間 Wi-Fi 穩定性及新版操作影片。自動測試使用模擬資料，不代表真實攝影機準確度。

## 硬體與軟體

| 項目 | 說明 |
| --- | --- |
| 開發板 | Realtek Ameba AMB82-MINI、攝影機、板上麥克風 |
| 電源與首次燒錄 | 可傳資料的 USB 線；日常可接充電器或行動電源 |
| 網路 | 板子與瀏覽器所在裝置可互通的區域網路 |
| 顯示 | Chrome／Edge；RTSP 畫面另外用 VLC 查看 |
| 開發環境 | Arduino IDE 2.x 或 Arduino CLI，Realtek AmebaPro2 SDK |
| 本機驗證 SDK | `4.1.1-build20260922`；FQBN：`realtek:AmebaPro2:Ameba_AMB82-MINI` |
| 開發用 Node.js | 建議 Node.js 22 以上，打包與網頁測試只用內建模組，不需 npm install |
| 額外硬體 | 不強制；RGB LED 預設未啟用，沒有蜂鳴器或喇叭輸出模組 |

板端模型為 SDK 內建 `DEFAULT_YOLOV7TINY`，沒有自行訓練模型。推論輸入設定為 576 × 320、10 FPS；此處是影像設定，不是量測出的模型 FPS。RTSP 使用 FHD H.264。

## 運作流程

```mermaid
flowchart TD
    A[攝影機影像] --> B[Ameba 執行 YOLOv7-tiny]
    B --> C[分數與位置關聯篩選]
    C --> D[跨時間累積辨識證據]
    D --> E{人在桌前且資料有效?}
    E -->|是| F[自動分類與桌前計時]
    E -->|確認離席| G[休息計時]
    E -->|資料過期或不確定| H[暫停養成]
    F --> I[板端寵物狀態與事件]
    G --> I
    H --> I
    J[板上麥克風] --> K[音量門檻與雙拍手候選]
    K --> I
    I --> L[HTTP 快照與事件 API]
    L --> M[瀏覽器寵物動畫、休息叫聲、CSV]
    M -->|每秒輪詢兼心跳| L
```

Ameba 負責推論、判斷、計時與事件；瀏覽器負責呈現、合成叫聲及匯出。影像不送到雲端辨識。RTSP 與網頁狀態是兩條不同資料路徑；瀏覽器頁面沒有內嵌即時攝影機影片。

## 相較原始物件偵測範例的延伸

以官方 `ObjectDetectionLoop`／`ObjectDetectionCallback` 的物件偵測、框選與 RTSP 功能為基準，新增下列應用層功能。課堂實際使用的原始範例版本仍需補註；若該版本已包含某項功能，不能把保留部分列為新增。

第一週提交 [`bcc7f0c`](https://github.com/ruby0226/ameba-study-buddy/commit/bcc7f0c4f0a38d435a810db30f16ab84891ae86a) 已有時間證據、位置關聯與基本活動計時。第二週新增重點是 Wi-Fi 直接提供網頁、板端休息養成、寵物事件紀錄、網頁心跳保護及麥克風互動；不能把第一週已完成功能稱為本週才新增。

| 延伸 | 本專案行為 | 主要檔案 |
| --- | --- | --- |
| 多次一致才成立 | 最近五個秒區間累積新推論，排除瞬間閃爍 | `DetectionEvidence.h` |
| 多類別位置關聯 | 書／手機弱候選輔助、相似物件衝突排除、鍵盤＋滑鼠電腦線索 | `SceneRelations.h` |
| 辨識結果帶動反應 | 自動讀書、電腦、在桌前、離席與無法確認狀態 | `.ino`、`web/pet_test.html` |
| 休息與寵物養成 | 分階段提醒、活力、休息恢復、死亡重生及成長 | `PetLife.h` |
| Wi-Fi 網頁整合 | 板端提供網頁、輪詢 API、心跳及短期事件補收 | `PetNetwork.h` |
| 計數與歷史 | 活動秒數、離席次數、寵物事件及 CSV 匯出 | `.ino`、網頁 |
| 異常處理 | 推論過期、人物不明或心跳中斷時暫停養成 | `.ino`、`PetLife.h` |
| 麥克風互動 | 雙拍手候選與持續音量事件，網頁跳躍／環境提示 | `PetSound.h` |

## 資料夾

```text
Day3_StudyBuddy_Verify/
  Day3_StudyBuddy_Verify.ino    主程式
  DetectionEvidence.h         跨時間辨識證據
  SceneRelations.h            物件位置與衝突判斷
  PetLife.h                   寵物與休息規則
  PetSound.h                  麥克風音量與雙拍手候選
  PetNetwork.h                HTTP 伺服器、心跳、事件暫存
  PetWebPage.h                已打包網頁，直接編譯可用
  WiFiSecrets.example.h       空白 Wi-Fi 設定範本
web/pet_test.html             網頁原始檔
scripts/embed-web.cjs         更新韌體內的網頁
tests/                       網頁、規則及瀏覽器測試
README.md                    安裝與使用說明
第二週_原型操作與測試.md       詳細操作規則
第二週_改進與繳交檢查.md       測試目標及繳交清單
第二週_實測紀錄.csv            尚待填入的實測表
```

`WiFiSecrets.h`、`.build/`、私人影片與作業 Word 留在本機，不上傳。程式倉庫不需要 Word 或影片才能運作。

## 安裝、設定與首次燒錄

### 1. 安裝開發板套件

在 Arduino IDE 的「偏好設定 → 額外的開發板管理員網址」加入官方 AmebaPro2 套件索引，再於開發板管理員安裝套件，選擇 `AmebaPro2 ARM (32-bits) Boards → AMB82-MINI`。官方 stable 索引：

```text
https://github.com/Ameba-AIoT/ameba-arduino-pro2/raw/main/Arduino_package/package_realtek_amebapro2_index.json
```

本專案驗證環境使用上述 `4.1.1-build20260922`，官方另提供 dev 套件索引；套件可用版本可能不同。若使用其他版本，請重新編譯和實測，勿把版本差異當成已驗證。安裝步驟與索引來源見 [官方 Getting Started](https://github.com/Ameba-AIoT/ameba-arduino-doc/blob/main/source/ameba_pro2/amb82-mini/Getting_Started/Getting%20Started%20with%20Ameba.rst)。

### 2. 填寫本機 Wi-Fi 設定

將 `Day3_StudyBuddy_Verify/WiFiSecrets.example.h` 複製成同資料夾的 `WiFiSecrets.h`，修改兩個字串：

```cpp
#pragma once
#define WIFI_SSID "你的Wi-Fi名稱"
#define WIFI_PASSWORD "你的Wi-Fi密碼"
```

不要修改範本中的預設字串，也不要提交 `WiFiSecrets.h`。設定檔不存在時出現 `WiFiSecrets.h: No such file or directory`，表示還沒完成這個步驟。本機原有設定已另外保留。

### 3. 開啟與編譯

開啟 `Day3_StudyBuddy_Verify/Day3_StudyBuddy_Verify.ino`，保留同資料夾所有標頭。`PetWebPage.h` 已放在專案裡，沒有改網頁時可直接按 Arduino IDE 的「驗證」。

若已安裝 Arduino CLI，也可在專案根目錄執行：

```text
arduino-cli compile --fqbn realtek:AmebaPro2:Ameba_AMB82-MINI --build-path .build/amb82 Day3_StudyBuddy_Verify
```

### 4. 燒錄

接上 USB 資料線，選擇實際出現的序列埠。若工具不能自動進入燒錄模式：按住 BOOT，按一下並放開 RST，最後放開 BOOT，再執行上傳。先關閉占用序列埠的監控視窗。

本專案已使用下列 CLI 方式燒錄過；`COM3` 只是例子，請換成自己的連接埠：

```text
arduino-cli upload --fqbn realtek:AmebaPro2:Ameba_AMB82-MINI --port COM3 --input-file .build/amb82/flash_ntz.bin Day3_StudyBuddy_Verify
```

確認輸出為 `upload success`。此 SDK 的燒錄失敗可能仍以成功程序代碼結束，因此不能只看 exit code。完成後單按 RST 啟動程式，這次不要按 BOOT。

### 5. 開啟網頁

序列埠監控設為 115200 baud，找到 `Pet web: http://<IP>/`。也可從路由器的裝置清單，或 VLC 畫面上的 `WEB:` 取得 IP。

手機／電腦連到可與板子互通的 Wi-Fi，在瀏覽器開啟 `http://<IP>/`，按「開始 Wi-Fi 連線」。例如板子分到 `192.168.1.100`，就開 `http://192.168.1.100/`；這個 IP 只是範例。

按下連線會嘗試啟用瀏覽器音效。若靜音或音效被阻擋，文字提醒仍有效。保持網頁在前景；背景分頁節流可能造成心跳逾時。日常使用可將板子改由充電器供電，IP 可能因重新連線而改變。

從本機開 `web/pet_test.html` 時，它會引導前往板子網址；不要用 GitHub Pages 代替板端網頁。HTTP API 與頁面採同來源，沒有跨來源存取設定，也沒有外網登入服務。

## 活動判斷與固定擺放

建議單人、固定鏡頭與光源，畫面包含肩膀、上半身與桌面。鏡頭距離臉約 60～80 cm，向下俯約 30°，以 25～35°作為起始調整範圍。書面可先用閱讀架維持與桌面 30～45°；書寬至少約占畫面 1/4，手機長邊至少約占畫面高度 1/5。這些是待實測的擺放建議，網頁不會自動量角度。

書、筆電、鍵盤、滑鼠的中心需在青框內：左右 10～90%、上下 38～98%。手機可在青框內或靠近人的身體。人與桌子的判斷使用全畫面。

| 訊號 | 規則 |
| --- | --- |
| 模型最低候選分數 | 25 分 |
| 一般應用門檻 | 人 50，書與手機 30，其餘關聯類別 40 |
| 書／手機 25～29 分 | 必須另通過同幀位置關聯；沒有候選就不推測存在 |
| 物品穩定 | 至少 3 次命中、跨 2 個秒區間、首末相隔至少 1 秒；窗口比例書 15%、手機 25%、電腦 40% |
| 短暫漏判 | 最後命中後最多保留 3 秒；推論超過 2 秒未更新則清除 |
| 在座／離席 | 穩定在座約 3 秒才成立；約 10 秒確認離席 |

只有書成立為 `STUDYING`；只有電腦線索成立為 `COMPUTER`；兩者都有或都沒有為 `PRESENT`。確認離席為 `AWAY`，資料不足或過期為 `UNSURE`。手機另行顯示「出現／靠近人」，不直接判定使用手機或分心。

模型使用 person、book、cell phone、laptop、diningtable、keyboard、mouse、remote。紙、筆不是本版獨立辨識類別；沒有手部、抬頭或低頭模型。更完整的關聯規則見 [辨識優化與固定擺放](辨識優化與固定擺放.md)，該文件保留早期驗證紀錄；目前連線方式以本 README 為準。

## 小雞養成與叫聲

| 行為 | 一般設定 `PET_DEMO=0` | 展示設定 `PET_DEMO=1` |
| --- | --- | --- |
| 首次休息提醒 | 在座 25 分鐘 | 在座 60 秒 |
| 持續久坐第二次提醒 | 在座約 30 分鐘，活力降至 50 | 在座約 75 秒 |
| 活力歸零 | 在座約 35 分鐘 | 在座約 90 秒 |
| 有效休息 | 確認離席後連續 5 分鐘 | 確認離席後連續 10 秒 |
| 成長 | 完成 3 輪達提醒門檻的學習＋休息 | 同左 |

這是遊戲參數，不是健康診斷或人體上限。在座、讀書、電腦情境都計入本輪時間，避免書本漏判造成不提醒。首次提醒是一組三聲短叫聲，第二次是一組兩聲短叫聲；每階段只播一次，死亡不額外叫。拍手只跳躍、不出聲。重連同步不補播舊提醒。

短暫離席不重設本輪時間。完成有效休息會恢復活力；只有本輪達到提醒門檻才獲得成長。死亡後完成休息會重生為蛋、代數加一，回座再孵化。原有桌前活動總時間不因死亡重設。

錄影要加速時，在主程式將 `#define PET_DEMO 0` 改為 `1`，重新編譯燒錄；網頁會顯示展示設定。這仍依實際辨識運作，不是模擬活動。

## 麥克風互動

`PetSound.h` 每約 20 ms 讀取音量；兩次尖峰相隔 150～900 ms 視為雙拍手候選。預設尖峰門檻 8000、持續音量門檻 3000，皆是振幅值，不能當分貝。最近五秒有三秒偏高才進入環境吵雜，降至一秒或以下才解除。

網頁只回應兩秒內的新拍手事件；必須仍在座、小雞存活、辨識及連線有效。叫聲或拍手動畫後五秒內不再回應拍手。這不是聲音分類模型，桌面敲擊、打字或喇叭可能造成誤觸，需實測校正。

## 連線、暫停與儲存限制

- 網頁每秒輪詢一次；Ameba 超過 3 秒未收到心跳，暫停寵物養成。
- 推論超過 2 秒未更新、人物狀態不明或養成迴圈停頓超過 1.5 秒，也暫停養成。
- 暫停不扣活力、不增加本輪在座時間、不當作休息，未完成的休息進度歸零。
- 活動總時間與寵物養成是不同計時器；前者仍可在辨識有效時繼續累計，後者依賴網頁心跳。
- 養成與計數在 RAM，板子斷電或 RST 會重設，尚無 SD 卡存檔。
- 板端保留最近 32 筆事件。網頁自動重試可補收短暫漏掉的事件；手動重新連線從目前開始。
- 網頁保留最近 300 筆寵物事件、顯示最後 30 筆，可匯出 CSV；重新整理會清空頁面紀錄。
- 校正資料最多保留最近 30 分鐘，重新連線會清空；需要留存請先匯出。
- 任一分頁送心跳都會維持養成，展示時建議只開一個主頁。

## 網路介面與資料格式

| 請求 | 行為 |
| --- | --- |
| `GET /` 或 `GET /pet_test.html` | 取得內嵌網頁 |
| `POST /api/poll?after=-1` | 首次同步；需 `X-StudyBuddy: 1` 標頭 |
| `POST /api/poll?after=<event_id>` | 取得快照與該事件編號之後仍在暫存的事件；同時更新心跳 |

回應是 UTF-8 純文字，每行一筆：

```text
NET,snapshot_sequence,last_event_id,board_ms
CSV,...
STATE,t_s,state,desk_s,study_s,computer_s,mode,away_count,last_away_s,head,headdown_s,remind,phone_present,phone_near_person,mic_level,mic_peak,noisy
PET,t_ms,phase,energy,work_s,rest_s,cycles,rests,generation,reminders,paused,remind_goal_s,rest_goal_s,decay_s,demo,started
EVENT,t_ms,event_id,event_name
```

CSV 為 36 欄；新版 STATE 為 17 欄，最後三欄是麥克風資訊。PET 為 16 欄。時間為開機後秒數或毫秒，非日曆時間。麥克風沒有資料時音量為 -1；人臉舊欄位保留停用值。事件種類為 WELCOME、LEAVE、REMIND、WEAK、DIED、RESTED、REBORN、CLAP、NOISY、QUIET。

網頁用 NET 序號去除重複快照，事件以編號去重。暫存溢出時回傳 `NOTICE,EVENT_GAP`。網頁 HTTP 請求逾 2 秒取消後重試；板端 Wi-Fi 斷線約每 10 秒嘗試重連，IP 改變需重新開網址。

## 修改網頁

只改 `web/pet_test.html` 不會更新板子內的頁面。修改後執行：

```text
node scripts/embed-web.cjs
node scripts/embed-web.cjs --check
```

再編譯、燒錄並重新整理瀏覽器。`PetWebPage.h` 是產生檔，請勿直接改它。

## 自動測試與實機驗收

在根目錄執行網頁測試：

```text
node tests/pet_page.test.cjs
node scripts/embed-web.cjs --check
```

使用 C++14 編譯器檢查規則；測試使用編譯期斷言，無需執行板端程式。先建立 `.build` 資料夾，再執行：

```text
g++ -std=c++14 -Wall -Wextra -pedantic -c tests/pet_life.test.cpp -o .build/pet_life.test.o
g++ -std=c++14 -Wall -Wextra -pedantic -c tests/scene_relations.test.cpp -o .build/scene_relations.test.o
```

可用 SDK 的 `arm-none-eabi-g++` 取代 g++。`tests/network_browser.test.cjs` 另以本機 HTTP 伺服器和無視窗 Chrome 驗證輪詢、斷線及手機／桌面版面：

```text
node tests/network_browser.test.cjs
```

瀏覽器測試目前使用 Windows Chrome 預設路徑 `C:/Program Files/Google/Chrome/Application/chrome.exe`；其他安裝位置需調整該測試的執行檔路徑。結果與截圖寫入 `.build/`。

必須另外完成攝影機、麥克風與實際 Wi-Fi 測試。條件、操作及通過標準見 [第二週改進與繳交檢查](第二週_改進與繳交檢查.md)，將實測寫入 [第二週實測紀錄](第二週_實測紀錄.csv)。目前表格未填結果，不代表測試已通過。

## 常見問題

| 問題 | 檢查方式 |
| --- | --- |
| 燒錄顯示 ping fail／upload fail | 確認 BOOT／RST 順序、實際 COM 埠、資料線及序列埠未被占用 |
| 編譯找不到 WiFiSecrets.h | 先複製設定範本並填入本機 Wi-Fi |
| 網頁開不起來 | 確認板子已啟動、實際 IP、同網路且無訪客裝置隔離；使用 http 而非 https |
| 開得到網頁但沒更新 | 按「開始 Wi-Fi 連線」，檢查板端推論有無更新；HTTP 成功不代表影像有效 |
| 沒有叫聲 | 點擊連線啟用音效、確認未靜音；首次同步或重連不會補播舊提醒 |
| 書／手機辨識忽有忽無 | 固定光線、角度、大小及青框位置，查看候選分數與命中比例，勿把分數當準確率 |
| 拍手不觸發／誤觸 | 查看音量與尖峰值，測試距離；離席、死亡、暫停或冷卻期間不回應 |
| 關頁面後停止養成 | 目前設計需要網頁心跳，屬於已知規則 |
| 網頁還是舊內容 | 重新產生 PetWebPage.h、編譯、燒錄及整理頁面 |

## 來源與自行修改

參考 [Realtek AmebaPro2 SDK](https://github.com/Ameba-AIoT/ameba-arduino-pro2)、[官方物件偵測說明](https://github.com/Ameba-AIoT/ameba-arduino-doc/blob/main/source/ameba_pro2/amb82-mini/Example_Guides/Neural%20Network/Object%20Detection.rst)、[HTTP 伺服器範例](https://ameba-doc-arduino-sdk.readthedocs-hosted.com/en/latest/ameba_pro2/amb82-mini/Example_Guides/WiFi/Simple%20Http%20Server%20to%20Control%20LED.html)與 [AudioStream 介面](https://ameba-doc-arduino-sdk.readthedocs-hosted.com/en/latest/ameba_pro2/amb82-mini/API_Documents/Multimedia/Class%20AudioStream.html)。模型、相機、RTSP 與底層驅動使用官方 SDK。

本專案自行加入時間證據、位置關聯、活動狀態、寵物計時、HTTP 資料協定、網頁互動與測試。開發過程使用 Codex 協助程式修改、測試及文件整理；實機驗證與結果判讀由組員負責。原始範例依第一週紀錄為 ObjectDetectionLoop／ObjectDetectionCallback，確切課堂版本及第二週實際分工仍需補註，不能把 AI 產生或模擬結果當成實測。

SDK 及第三方模型遵循其各自授權；本倉庫未另行替第三方內容指定授權。公開上傳不會包含私人 Wi-Fi 設定、已編譯韌體或未授權的人物影片。
