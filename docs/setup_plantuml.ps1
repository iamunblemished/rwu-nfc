# Setup script to download PlantUML for local diagram generation
# This avoids relying on external PlantUML servers

$plantumlUrl = "https://github.com/plantuml/plantuml/releases/download/v1.2024.8/plantuml-1.2024.8.jar"
$plantumlPath = "$PSScriptRoot\plantuml.jar"

Write-Host "Downloading PlantUML jar file..." -ForegroundColor Cyan

try {
    # Download PlantUML jar
    Invoke-WebRequest -Uri $plantumlUrl -OutFile $plantumlPath -UseBasicParsing
    Write-Host "PlantUML jar downloaded successfully to: $plantumlPath" -ForegroundColor Green
    
    # Verify Java is installed
    try {
        $javaVersion = java -version 2>&1 | Select-String "version"
        Write-Host "Java found: $javaVersion" -ForegroundColor Green
        Write-Host "`nPlantUML setup complete! You can now build the documentation with 'make html'" -ForegroundColor Green
    }
    catch {
        Write-Host "`nWARNING: Java is not installed or not in PATH." -ForegroundColor Yellow
        Write-Host "Please install Java Runtime Environment (JRE) to use PlantUML." -ForegroundColor Yellow
        Write-Host "Download from: https://www.java.com/download/" -ForegroundColor Yellow
    }
}
catch {
    Write-Host "Error downloading PlantUML: $_" -ForegroundColor Red
    Write-Host "`nAlternative: The documentation can use online PlantUML server instead." -ForegroundColor Yellow
}
