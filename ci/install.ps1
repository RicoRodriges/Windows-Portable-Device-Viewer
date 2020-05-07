# Original script:
# https://github.com/mabrarov/asio_samples/blob/develop/scripts/appveyor/install.ps1

switch (${env:CONFIGURATION}) {
  "Debug" {
    $env:CMAKE_BUILD_CONFIG = "DEBUG"
  }
  "Release" {
    $env:CMAKE_BUILD_CONFIG = "RELEASE"
  }
  default {
    throw "Unsupported build configuration: ${env:CONFIGURATION}"
  }
}


$cmake_generator_msvc_version_suffix = " ${env:MSVC_VERSION}" -replace "([\d]+)\.([\d]+)", '$1'
switch (${env:MSVC_VERSION}) {
  "14.2" {
    $cmake_generator_msvc_version_suffix = " 16 2019"
  }
  "14.1" {
    $cmake_generator_msvc_version_suffix = " 15 2017"
  }
  "14.0" {
    $cmake_generator_msvc_version_suffix = "${cmake_generator_msvc_version_suffix} 2015"
  }
  default {
    throw "Unsupported ${env:TOOLCHAIN} version for CMake generator: ${env:MSVC_VERSION}"
  }
}
$env:CMAKE_GENERATOR = "Visual Studio${cmake_generator_msvc_version_suffix}"


switch (${env:PLATFORM}) {
  "Win32" {
    $env:CMAKE_GENERATOR_PLATFORM = "Win32"
  }
  "x64" {
    $env:CMAKE_GENERATOR_PLATFORM = "x64"
  }
  default {
    throw "Unsupported platform for CMake generator: ${env:PLATFORM}"
  }
}


<#
$env:COVERITY_SCAN_BUILD = (${env:COVERITY_SCAN_CANDIDATE} -eq "1") `
  -and (${env:APPVEYOR_REPO_BRANCH} -eq "coverity_scan") `
  -and (${env:CONFIGURATION} -eq "Release") `
  -and (${env:PLATFORM} -eq "x64")
#>
$env:COVERAGE_BUILD = (${env:COVERAGE_BUILD_CANDIDATE} -eq "1") `
  -and (${env:CONFIGURATION} -eq "Debug") `
  -and (${env:PLATFORM} -eq "x64")
if ((${env:COVERAGE_BUILD} -eq "True") -and `
 -not (Test-Path -Path env:CODECOV_TOKEN) -or (${env:CODECOV_TOKEN} -eq "")) {
  throw "CODECOV_TOKEN environment variable is not defined but is required to upload coverage report to Codecov"
}
$env:CODECOV_FLAG = "windows_${env:OS_VERSION}__${env:PLATFORM}__${env:TOOLCHAIN}_${env:MSVC_VERSION}" -replace "[\.-]", "_"


Write-Host "APPVEYOR_BUILD_FOLDER     : ${env:APPVEYOR_BUILD_FOLDER}"
if (${env:COVERAGE_BUILD} -eq "True") {
  Write-Host "Installing OpenCppCoverage from Chocolatey package"
  appveyor-retry choco install -y --no-progress opencppcoverage
  if (${LastExitCode} -ne 0) {
    throw "Installation of OpenCppCoverage Chocolatey package failed with ${LastExitCode} exit code"
  }
  Write-Host "OpenCppCoverage installed"

  Write-Host "Installing Codecov from pip package"
  pip install --disable-pip-version-check --retries "${env:PIP_RETRY}" codecov=="${env:CODECOV_VERSION}"
  if (${LastExitCode} -ne 0) {
    throw "Installation of Codecov pip package failed with exit code ${LastExitCode}"
  }
  Write-Host "Codecov installed"
}


Set-Location -Path "${env:APPVEYOR_BUILD_FOLDER}"