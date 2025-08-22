<#
.SYNOPSIS
编译C/C++代码为LLVM IR

.DESCRIPTION
使用方法: path/to/compile.ps1 <库名>
示例: path/to/compile.ps1 fmt
功能: 将指定目录下的源文件编译为对应平台的动态库
假设源文件结构: <库名>/lib<库名>.cpp (如 fmt/libfmt.cpp)
输出文件:
    <库名>/lib<库名>.ll
#>
param(
    [Parameter(Mandatory=$false)]
    [string]$name
)

function Show-Help {
    Write-Host "使用 Get-Help path/to/compile.ps1 查看"
    exit 0
}

# 检查参数
if ([string]::IsNullOrEmpty($name) -or $name -in "--help", "-h", "/?") {
    Show-Help
}

# 提取纯库名（处理可能的路径分隔符）
$libName = $name -split '[\\/]' | Select-Object -Last 1
if ([string]::IsNullOrEmpty($libName)) {
    Write-Error "错误: 无效的库名"
    exit 1
}

# 源文件和输出路径设置
$sourceDir = $name  # 源文件目录（支持相对路径）
$sourceFile = Join-Path $sourceDir "lib$libName.cpp"
$IRFile = Join-Path $sourceDir "lib$libName.ll"

# 检查源文件是否存在
if (-not (Test-Path $sourceFile -PathType Leaf)) {
    Write-Error "错误: 源文件不存在 - $sourceFile"
    exit 1
}

# 确保输出目录存在
if (-not (Test-Path $sourceDir -PathType Container)) {
    New-Item -ItemType Directory -Path $sourceDir | Out-Null
}

try {
    clang++ -emit-llvm -S "$sourceFile" -o "$IRFile"
    if ($LASTEXITCODE -ne 0) {
        throw "编译器返回错误（退出码: $LASTEXITCODE）"
    }
    if (Test-Path "$sourceDir/lib$libName.hpp.pch" -PathType Leaf) {
        Remove-Item "$sourceDir/lib$libName.hpp.pch" -ErrorAction -SilentlyContinue
    }
    Write-Host "编译成功: $IRFile"
    exit 0
} catch {
    Write-Error "编译失败: $_"
    exit 1
}
