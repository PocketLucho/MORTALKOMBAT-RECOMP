# MORTAL KOMBAT RECOMP - recompilador / recompiler (Windows)
# Recompila a C++ el programa TMS34010 de cada juego desde TUS ROMs (roms\) y lo compila en
# recomp\<juego>.dll. La primera vez descarga Zig (~75 MB, en tools\zig): un compilador de
# C++ autocontenido, sin instalar nada. / Downloads Zig once; nothing is installed.
param([string[]]$Games = @('mk', 'mk2', 'mk3', 'umk3'))
$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath $PSScriptRoot
function Say($es, $en) { Write-Host ""; Write-Host "== $es" -ForegroundColor Red; Write-Host "   $en" }
# Start-Process -ArgumentList does not quote arguments: paths with spaces would split.
function Q([string[]]$a) { $a | ForEach-Object { if ($_ -match '[\s"]') { '"' + ($_ -replace '"', '\"') + '"' } else { $_ } } }
function Fail($es, $en) { Write-Host ""; Write-Host "ERROR: $es" -ForegroundColor Red; Write-Host "       $en" -ForegroundColor Red; Remove-Item -Recurse -Force tools\gen -ErrorAction SilentlyContinue; exit 1 }
Say "MORTAL KOMBAT RECOMP: recompilacion estatica" "MORTAL KOMBAT RECOMP: static recompilation"
$mk = "tools\recomp\mkrecomp.exe"; $sdk = (Resolve-Path "tools\recomp\sdk").Path
if (-not (Test-Path $mk)) { Fail "Falta $mk." "$mk is missing." }
$zigver = '0.13.0'
$zigdir = "tools\zig\zig-windows-x86_64-$zigver"
$zig = Join-Path $zigdir 'zig.exe'
if (-not (Test-Path $zig)) {
  Say "Descargando Zig $zigver (~75 MB, solo esta vez)..." "Downloading Zig $zigver (~75 MB, only this once)..."
  New-Item -ItemType Directory -Force tools\zig | Out-Null
  [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
  try { Invoke-WebRequest -UseBasicParsing "https://ziglang.org/download/$zigver/zig-windows-x86_64-$zigver.zip" -OutFile tools\zig\zig.zip }
  catch { Fail "No se pudo descargar Zig (hay conexion?)." "Could not download Zig (are you online?)." }
  Expand-Archive -Force tools\zig\zig.zip tools\zig
  Remove-Item tools\zig\zig.zip
}
$zig = (Resolve-Path $zig).Path
New-Item -ItemType Directory -Force recomp | Out-Null
$built = 0
foreach ($g in $Games) {
  Remove-Item -Recurse -Force tools\gen -ErrorAction SilentlyContinue
  New-Item -ItemType Directory -Force tools\gen | Out-Null
  $margs = @('.', $g, 'tools\gen'); if (Test-Path "recomp\$g.miss") { $margs += "recomp\$g.miss" }
  # Start-Process: Windows PowerShell 5.1 turns a native program's stderr into errors (and with
  # 'Stop' aborts), so the output goes to files and only the exit code decides.
  $p = Start-Process -FilePath $mk -ArgumentList (Q $margs) -NoNewWindow -Wait -PassThru `
         -RedirectStandardOutput tools\gen\out.txt -RedirectStandardError tools\gen\log.txt
  $log = @(Get-Content tools\gen\out.txt, tools\gen\log.txt -ErrorAction SilentlyContinue)
  if ($p.ExitCode -ne 0) { if ($log.Count) { Write-Host "   $($log[-1])" }; continue }
  $log | ForEach-Object { Write-Host "   $_" }
  Say "${g}: compilando..." "${g}: compiling..."
  $files = Get-Content "tools\gen\rc_$g.files"
  $flags = @('c++', '-std=c++17', '-O2', '-fno-strict-aliasing', '-w', '-target', 'x86_64-windows-gnu', "-I$sdk")
  $jobs = @()
  foreach ($f in $files) {
    $pr = Start-Process -FilePath $zig -ArgumentList (Q ($flags + @('-c', "tools\gen\$f", '-o', "tools\gen\$f.o"))) -NoNewWindow -PassThru
    $null = $pr.Handle   # keep the handle, otherwise ExitCode is empty after the process ends
    $jobs += $pr
    while (@($jobs | Where-Object { -not $_.HasExited }).Count -ge [Environment]::ProcessorCount) { Start-Sleep -Milliseconds 200 }
  }
  $jobs | ForEach-Object { $_.WaitForExit() }
  if (@($jobs | Where-Object { $_.ExitCode -ne 0 }).Count -gt 0) { Fail "${g}: error de compilacion." "${g}: compilation error." }
  $l = Start-Process -FilePath $zig -ArgumentList (Q ($flags + @('-shared', '-o', "recomp\$g.dll") + ($files | ForEach-Object { "tools\gen\$_.o" }))) -NoNewWindow -Wait -PassThru
  if ($l.ExitCode -ne 0) { Fail "${g}: error al enlazar." "${g}: link error." }
  Remove-Item -Force "recomp\$g.lib", "recomp\$g.pdb" -ErrorAction SilentlyContinue
  Write-Host "   -> recomp\$g.dll"; $built++
}
Remove-Item -Recurse -Force tools\gen -ErrorAction SilentlyContinue   # generated C++ comes from the game: not kept
if ($built -eq 0) { Fail "No se encontro ningun romset en roms\." "No romset found in roms\." }
Say "Listo. Los juegos recompilados salen con REC en el menu." "Done. Recompiled games show REC in the menu."
Write-Host "   Juega un rato y vuelve a ejecutarlo: se anade el codigo que falto (recomp\*.miss)."
Write-Host "   Play for a while and run it again: the code that was missed (recomp\*.miss) is added."
