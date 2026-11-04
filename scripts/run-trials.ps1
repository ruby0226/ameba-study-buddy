# 第三週測試紀錄：每次重新擺放後按 Enter，讀板子 N 秒，自動記錄預期與實際結果
# 用法：
#   powershell -ExecutionPolicy Bypass -File scripts\run-trials.ps1 -Board 172.20.10.2 -Condition 正常 -Expect STUDYING
#   -Expect 可用 STUDYING、COMPUTER、PHONE、PRESENT、AWAY；前面加 ! 表示不應出現，例如 "!STUDYING,!PHONE"
param(
    [Parameter(Mandatory = $true)][string]$Board,
    [Parameter(Mandatory = $true)][string]$Condition,
    [Parameter(Mandatory = $true)][string]$Expect,
    [int]$Count = 10,
    [int]$Seconds = 10,
    [string]$Setup = '',
    [string]$Out = ''
)
$ErrorActionPreference = 'Stop'
# PowerShell 5.1 在參數預設值裡拿不到 PSScriptRoot，要在這裡才設定
if (-not $Out) { $Out = Join-Path (Split-Path -Parent $MyInvocation.MyCommand.Path) '..\第三週_測試紀錄.csv' }
$names = @{ STUDYING = '讀書'; COMPUTER = '電腦'; PHONE = '手機'; PRESENT = '在桌前'; AWAY = '離席'; UNSURE = '無法確認' }
$rules = @($Expect.Split(',') | ForEach-Object { $_.Trim() } | Where-Object { $_ })
foreach ($r in $rules) { if (-not $names.ContainsKey($r.TrimStart('!'))) { throw "不認得的狀態：$r" } }
$expectText = ($rules | ForEach-Object {
    if ($_.StartsWith('!')) { "$Seconds 秒內不出現$($names[$_.TrimStart('!')])" } else { "$Seconds 秒內顯示$($names[$_])" }
}) -join '，'

function Poll {
    $res = Invoke-WebRequest -UseBasicParsing -Method Post -Headers @{ 'X-StudyBuddy' = '1' } -TimeoutSec 3 -Uri "http://$Board/api/poll?after=-1"
    $text = [Text.Encoding]::UTF8.GetString($res.RawContentStream.ToArray())
    $state = $null; $book = 0; $phone = 0; $person = 0
    foreach ($line in $text -split "`n") {
        $f = $line.Trim().Split(',')
        if ($f[0] -eq 'STATE' -and $f.Count -gt 2) { $state = $f[2] }
        if ($f[0] -eq 'CSV' -and $f.Count -gt 8) { $person = [int]$f[4]; $book = [int]$f[6]; $phone = [int]$f[8] }
    }
    return @{ state = $state; book = $book; phone = $phone; person = $person }
}

$header = -not (Test-Path $Out)
$existing = if ($header) { 0 } else { @(Import-Csv $Out -Encoding UTF8 | Where-Object { $_.'條件' -eq $Condition }).Count }
Write-Host "條件：$Condition　預期：$expectText　共 $Count 次（已有 $existing 筆）" -ForegroundColor Cyan
if ($Setup) { Write-Host "擺放：$Setup" }
try { $null = Poll } catch { throw "連不到板子 http://$Board/ ，電腦和板子要在同一個網路" }

for ($i = 1; $i -le $Count; $i++) {
    $id = '{0}-{1:D2}' -f $Condition, ($existing + $i)
    $key = Read-Host "`n[$id] 重新擺好後按 Enter 開始（s 跳過、q 結束）"
    if ($key -eq 'q') { break }
    if ($key -eq 's') { continue }
    $watch = [Diagnostics.Stopwatch]::StartNew()
    $seen = New-Object System.Collections.ArrayList
    $first = @{}; $bookMax = 0; $phoneMax = 0; $failed = 0
    while ($watch.Elapsed.TotalSeconds -lt $Seconds) {
        try {
            $p = Poll
            if ($p.state) {
                if ($seen.Count -eq 0 -or $seen[$seen.Count - 1] -ne $p.state) { [void]$seen.Add($p.state) }
                if (-not $first.ContainsKey($p.state)) { $first[$p.state] = [Math]::Round($watch.Elapsed.TotalSeconds, 1) }
            }
            $bookMax = [Math]::Max($bookMax, $p.book); $phoneMax = [Math]::Max($phoneMax, $p.phone)
        } catch { $failed++ }
        Start-Sleep -Milliseconds 500
    }
    $ok = $true
    foreach ($r in $rules) {
        $s = $r.TrimStart('!')
        if ($r.StartsWith('!')) { if ($first.ContainsKey($s)) { $ok = $false } } else { if (-not $first.ContainsKey($s)) { $ok = $false } }
    }
    $path = ($seen | ForEach-Object { $names[$_] }) -join '→'
    $timing = ($rules | Where-Object { -not $_.StartsWith('!') -and $first.ContainsKey($_) } | ForEach-Object { "$($first[$_]) 秒出現$($names[$_])" }) -join '，'
    $actual = if ($timing) { "$timing（狀態：$path）" } else { "狀態：$path" }
    if ($failed) { $actual += "，連線失敗 $failed 次" }
    $verdict = if ($ok) { '是' } else { '否' }
    Write-Host "  實際：$actual　符合預期：$verdict" -ForegroundColor $(if ($ok) { 'Green' } else { 'Yellow' })
    $note = Read-Host '  備註（可空白，例如角度、光線、失敗原因）'
    $note = (@("書本最高 $bookMax 分", "手機最高 $phoneMax 分", $Setup, $note) | Where-Object { $_ }) -join '，'
    [pscustomobject][ordered]@{
        '條件' = $Condition; '測試編號' = $id; '預期行為' = $expectText; '實際行為' = $actual
        '是否符合預期' = $verdict; '備註' = $note; '時間' = (Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
    } | Export-Csv -Path $Out -Append -NoTypeInformation -Encoding UTF8
}
if (-not (Test-Path $Out)) { Write-Host "`n沒有記錄任何一次"; exit }
$rows = @(Import-Csv $Out -Encoding UTF8 | Where-Object { $_.'條件' -eq $Condition })
$pass = @($rows | Where-Object { $_.'是否符合預期' -eq '是' }).Count
Write-Host "`n$Condition：$pass / $($rows.Count) 符合預期，紀錄在 $Out" -ForegroundColor Cyan
