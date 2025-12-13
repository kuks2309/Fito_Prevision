$p = [Environment]::GetEnvironmentVariable('Path', 'User')
$p = $p.Replace('Python313', 'Python311')
[Environment]::SetEnvironmentVariable('Path', $p, 'User')
Write-Host "PATH updated: Python313 -> Python311"
