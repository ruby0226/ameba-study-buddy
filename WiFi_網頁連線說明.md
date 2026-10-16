# Ameba 直接提供寵物網頁

2026-10-15 更新。新版不透過 USB 傳送網頁資料；Ameba 在同一個區域網路提供 HTTP 網頁與狀態 API。

## 使用方法

1. 將 `WiFiSecrets.example.h` 複製為同資料夾的 `WiFiSecrets.h`，填入自己的 Wi-Fi，首次用傳輸線燒錄新版韌體。本機設定檔不會上傳 GitHub。這次新增的是網頁連線，沒有新增 OTA 網路燒錄。
2. Ameba 接電源，與電腦或手機連到同一個 Wi-Fi。日常使用可改接 USB 充電器或行動電源，仍需要供電。
3. 找到 Ameba 的 IP：路由器的連線裝置清單、VLC 畫面上的 `WEB: http://.../`，或首次燒錄後的序列埠除錯訊息 `Pet web: http://.../` 都可查看。
4. 瀏覽器開啟 `http://板子IP/`，按「開始 Wi-Fi 連線」。這次點擊同時啟用叫聲；不需選 COM 埠。
5. 若先開啟電腦上的 `web/pet_test.html`，填入 IP 並按「開啟板子網頁」，再開始連線。本機檔案不直接跨來源抓板子資料。

例如 IP 是 `192.168.1.100`，網址就是 `http://192.168.1.100/`。此位址只是示例，不是已確認的板子 IP。

網頁、樣式、像素小雞與合成音效都在韌體內，不需要雲端服務、SD 卡或額外安裝網頁伺服器。攝影機的 RTSP 影像仍用 VLC 查看，這次沒有改成瀏覽器影片串流。

## 運作方式

- `GET /`：板子送出內嵌寵物網頁。
- `POST /api/poll?after=事件編號`：網頁每秒取得最新 CSV／STATE／PET 快照及漏收事件；同時當作養成心跳。附帶 `X-StudyBuddy: 1` 標頭。
- `NET,快照編號,最後事件編號,板端毫秒`：讓網頁辨別新資料、重啟與事件進度。
- 同一份快照只處理一次，避免重複計入辨識歷史；連線仍通但模型不更新時，頁面仍會提示資料逾時。
- 板子保留最近 32 筆寵物事件。短暫掉線可補回；超出暫存會顯示缺口提示。手動重新連線從當下開始，不回播舊提醒。
- 請求超過 2 秒未完成，網頁取消並自動重試；手動中斷會停止後續請求。板子超過 3 秒沒收到心跳就暫停養成。
- Wi-Fi 掉線時，板端網路工作每隔約 10 秒嘗試重新連線。若路由器重新分配了 IP，需要改用新的網址；可在路由器設定 DHCP 保留位址。
- 網路工作與辨識主迴圈分開執行，共用快照受到 mutex 保護，傳送網頁時不持有辨識鎖。

同時開多個網頁時，任一頁仍在傳心跳就會維持養成；本原型建議只開一個主頁。聲音播放與背景分頁行為仍受瀏覽器限制。

## 修改與編譯

主程式資料夾新增 `PetNetwork.h` 及自動產生的 `PetWebPage.h`，兩者都要一併保留。

網頁的原始檔仍是 `web/pet_test.html`。修改後先執行：

```text
node scripts/embed-web.cjs
node scripts/embed-web.cjs --check
```

再編譯／燒錄 `.ino`。直接修改本機 HTML 不會自動更新板子裡的網頁；不要手改產生的 `PetWebPage.h`。

## 測試與限制

- 已通過 AMB82 MINI 韌體編譯。
- `tests/pet_page.test.cjs` 檢查網路重試、取消請求、快照去重、重啟同步、網址驗證及既有養成功能。
- `tests/network_browser.test.cjs` 使用本機模擬 HTTP 伺服器與 Chrome，驗證真的由 HTTP 載入網頁、POST 心跳、手機／桌面版面與停止連線；這不是板端 Wi-Fi 實測。
- 實機仍需確認 IP、網頁傳送、推論速度、掉線恢復與音效。燒錄失敗或尚未進入 BOOT 模式時，不能把編譯結果當成已更新板子。
- 本版是同網路 HTTP 原型，沒有登入或加密；不設定路由器外網轉發。學校／訪客 Wi-Fi 若開啟裝置隔離，兩台裝置即使同 SSID 也可能無法互連。

官方介面參考：[Ameba Simple HTTP Server](https://ameba-doc-arduino-sdk.readthedocs-hosted.com/en/latest/ameba_pro2/amb82-mini/Example_Guides/WiFi/Simple%20Http%20Server%20to%20Control%20LED.html)。實作使用本機 SDK 的 WiFiServer／WiFiClient 與 FreeRTOS。
