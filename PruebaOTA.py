from ota import OTAUpdater
from WIFI_CONFIG import SSID, PASSWORD

firmware_url = "https://raw.githubusercontent.com/LMario28/Marquesina/main/"

ota_updater = OTAUpdater(SSID, PASSWORD, firmware_url, "PruebaOTA.py")
ota_updater.download_and_install_update_if_available()

print("Uno")
