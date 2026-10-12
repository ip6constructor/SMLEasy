DOMAIN = "smleasy"
PLATFORMS = ["sensor", "button"]

CONF_SCAN_INTERVAL = "scan_interval"
CONF_USE_SSL = "use_ssl"

DEFAULT_PORT = 80
DEFAULT_SCAN_INTERVAL = 30

DATA_COORDINATOR = "coordinator"
DATA_API = "api"

DIAGNOSTICS_REDACT = {
	"password",
	"username",
	"host",
}

# Values Home Assistant pushes to the device (short name -> label); entities are picked in the options.
PUSH_VALUES = {
    "pv_power": "PV-Leistung",
    "battery_power": "Batterie-Leistung",
    "battery_soc": "Batterie-Ladestand",
    "wallbox_power": "Wallbox-Leistung",
    "house_power": "Hausverbrauch",
}
PUSH_REFRESH_SECONDS = 60
