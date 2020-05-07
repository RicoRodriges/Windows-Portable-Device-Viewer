New-Item -Path "${env:BUILD_HOME}" -ItemType "directory" | out-null
Set-Location -Path "${env:BUILD_HOME}"


cmake --version


$generate_cmd = "cmake -DBUILD_EXAMPLE=ON"
switch (${env:RUNTIME_LINKAGE}) {
  "static" {
    $generate_cmd = "${generate_cmd} -DBUILD_SHARED_LIBS=OFF"
  }
  default {
    $generate_cmd = "${generate_cmd} -DBUILD_SHARED_LIBS=ON"
  }
}
if (${env:COVERAGE_BUILD} -eq "True") {
  $generate_cmd = "${generate_cmd} -DBUILD_TESTING=ON"
}
$generate_cmd = "${generate_cmd} -G ""${env:CMAKE_GENERATOR}"""
if (${env:CMAKE_GENERATOR_PLATFORM}) {
  $generate_cmd = "${generate_cmd} -A ""${env:CMAKE_GENERATOR_PLATFORM}"""
}
$generate_cmd = "${generate_cmd} ""${env:APPVEYOR_BUILD_FOLDER}"""


Write-Host "CMake project generation command: ${generate_cmd}"
Invoke-Expression "${generate_cmd}"
if (${LastExitCode} -ne 0) {
  throw "Generation of project failed with exit code ${LastExitCode}"
}


$build_cmd = "cmake --build . --config ""${env:CONFIGURATION}"" -- /maxcpucount /verbosity:normal /logger:""C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"""
<#
if (${env:COVERITY_SCAN_BUILD} -eq "True") {
  $build_cmd = "cov-build.exe --dir cov-int ${build_cmd}"
}
#>
Write-Host "CMake project build command: ${build_cmd}"
Invoke-Expression "${build_cmd}"
if (${LastExitCode} -ne 0) {
  throw "Build failed with exit code ${LastExitCode}"
}

<#
if (${env:COVERITY_SCAN_BUILD} -eq "True") {
  # Compress results.
  Write-Host "Compressing Coverity Scan results..."
  7z.exe a -tzip "${env:BUILD_HOME}\${env:APPVEYOR_PROJECT_NAME}.zip" "${env:BUILD_HOME}\cov-int" -aoa -y | out-null
  if (${LastExitCode} -ne 0) {
    throw "Failed to zip Coverity Scan results with exit code ${LastExitCode}"
  }
  # Upload results to Coverity server.
  $coverity_build_version = ${env:APPVEYOR_REPO_COMMIT}.Substring(0, 7)
  Write-Host "Uploading Coverity Scan results (version: ${coverity_build_version})..."
  curl.exe `
    --connect-timeout "${env:CURL_CONNECT_TIMEOUT}" `
    --max-time "${env:CURL_MAX_TIME}" `
    --retry "${env:CURL_RETRY}" `
    --retry-delay "${env:CURL_RETRY_DELAY}" `
    --show-error --silent --location `
    --form token="${env:COVERITY_SCAN_TOKEN}" `
    --form email="${env:COVERITY_SCAN_NOTIFICATION_EMAIL}" `
    --form file=@"${env:BUILD_HOME}\${env:APPVEYOR_PROJECT_NAME}.zip" `
    --form version="${coverity_build_version}" `
    --form description="Build submitted via AppVeyor CI" `
    "https://scan.coverity.com/builds?project=${env:APPVEYOR_REPO_NAME}"
  if (${LastExitCode} -ne 0) {
    throw "Failed to upload Coverity Scan results with exit code ${LastExitCode}"
  }
}
#>

Set-Location -Path "${env:APPVEYOR_BUILD_FOLDER}"