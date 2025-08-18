<#
.SYNOPSIS
跨平台编译C代码为动态库（.dll/.so/.dylib）
.DESCRIPTION
使用方法: compile.ps1 <库名>
示例: compile.ps1 fmt
功能: 将libfmt.c编译为对应平台的动态库
输出文件:
    Windows: libfmt.dll
    Linux:   libfmt.so
    macOS:   libfmt.dylib
#>
param(
    [Parameter(Mandatory=$false)]
    [string]$name
)

function Show-Help {
    Write-Host "使用 Get-Help 命令查看更多"
    exit 0
}

# 检查参数
if ([string]::IsNullOrEmpty($name) -or $name -in "--help", "-h", "/?") {
    Show-Help
}

$sourceFile = "lib$name.c"
$objFile = "lib$name.o"

# 检查源文件
if (-not (Test-Path $sourceFile -PathType Leaf)) {
    Write-Error "错误: 源文件 $sourceFile 不存在"
    exit 1
}

try {
    if ($IsWindows) {
        # Windows平台
        clang -c $sourceFile -o $objFile -fPIC
        clang -shared $objFile -o "lib$name.dll" '-Wl,--out-implib=lib$name.lib'
    } elseif ($IsLinux) {
        # Linux平台
        clang -c $sourceFile -o $objFile -fPIC
        clang -shared $objFile -o "lib$name.so"
    } elseif ($IsMacOS) {
        # macOS平台
        clang -c $sourceFile -o $objFile -fPIC
        clang -shared $objFile -o "lib$name.dylib"
    } else {
        Write-Error "错误: 不支持的操作系统"
        exit 1
    }

    # 清理临时文件
    if (Test-Path $objFile) {
        Remove-Item $objFile -Force
    }

    Write-Host "成功: 动态库已生成"
    if ($IsWindows) {
        Write-Host "输出: lib$name.dll, lib$name.lib"
    } else {
        Write-Host "输出: $(if ($IsLinux) { "lib$name.so" } else { "lib$name.dylib" })"
    }
} catch {
    Write-Error "编译失败: $_"
    exit 1
}
