from __future__ import annotations

from dataclasses import dataclass
from datetime import datetime
from typing import Any

from homeassistant.components.sensor import SensorDeviceClass, SensorEntity, SensorEntityDescription, SensorStateClass
from homeassistant.config_entries import ConfigEntry
from homeassistant.const import (
    EntityCategory,
    PERCENTAGE,
    UnitOfElectricCurrent,
    UnitOfElectricPotential,
    UnitOfEnergy,
    UnitOfFrequency,
    UnitOfInformation,
    UnitOfPower,
)
from homeassistant.core import HomeAssistant
from homeassistant.helpers.device_registry import DeviceInfo
from homeassistant.helpers.entity_platform import AddEntitiesCallback
from homeassistant.helpers.update_coordinator import CoordinatorEntity

from .const import DATA_COORDINATOR, DOMAIN
from .coordinator import SmartmeterCoordinator


@dataclass(frozen=True, kw_only=True)
class SmartmeterSensorDescription(SensorEntityDescription):
    key_in_payload: str
    divisor: float | None = None
    precision: int | None = None
    as_bool_text: bool = False


DESCRIPTIONS: tuple[SmartmeterSensorDescription, ...] = (
    SmartmeterSensorDescription(
        key="fwd_active_wh",
        key_in_payload="fwd_active_wh",
        name="Import Energie",
        native_unit_of_measurement=UnitOfEnergy.KILO_WATT_HOUR,
        state_class=SensorStateClass.TOTAL_INCREASING,
        device_class=SensorDeviceClass.ENERGY,
        icon="mdi:transmission-tower-import",
        divisor=1000.0,
        precision=3,
    ),
    SmartmeterSensorDescription(
        key="rev_active_wh",
        key_in_payload="rev_active_wh",
        name="Export Energie",
        native_unit_of_measurement=UnitOfEnergy.KILO_WATT_HOUR,
        state_class=SensorStateClass.TOTAL_INCREASING,
        device_class=SensorDeviceClass.ENERGY,
        icon="mdi:transmission-tower-export",
        divisor=1000.0,
        precision=3,
    ),
    SmartmeterSensorDescription(
        key="import_react_varh",
        key_in_payload="import_react_varh",
        name="Import Blindenergie",
        native_unit_of_measurement="varh",
        state_class=SensorStateClass.TOTAL_INCREASING,
        icon="mdi:sine-wave",
    ),
    SmartmeterSensorDescription(
        key="export_react_varh",
        key_in_payload="export_react_varh",
        name="Export Blindenergie",
        native_unit_of_measurement="varh",
        state_class=SensorStateClass.TOTAL_INCREASING,
        icon="mdi:sine-wave",
    ),
    SmartmeterSensorDescription(
        key="power_plus",
        key_in_payload="fwd_w",
        name="Netzbezug Leistung",
        native_unit_of_measurement=UnitOfPower.WATT,
        state_class=SensorStateClass.MEASUREMENT,
        device_class=SensorDeviceClass.POWER,
        icon="mdi:flash",
    ),
    SmartmeterSensorDescription(
        key="power_minus",
        key_in_payload="rev_w",
        name="Netzeinspeisung Leistung",
        native_unit_of_measurement=UnitOfPower.WATT,
        state_class=SensorStateClass.MEASUREMENT,
        device_class=SensorDeviceClass.POWER,
        icon="mdi:flash-outline",
    ),
    SmartmeterSensorDescription(
        key="voltage_l1",
        key_in_payload="v_l1_mv",
        name="Spannung L1",
        native_unit_of_measurement=UnitOfElectricPotential.VOLT,
        state_class=SensorStateClass.MEASUREMENT,
        device_class=SensorDeviceClass.VOLTAGE,
        icon="mdi:sine-wave",
        divisor=1000.0,
        precision=3,
    ),
    SmartmeterSensorDescription(
        key="voltage_l2",
        key_in_payload="v_l2_mv",
        name="Spannung L2",
        native_unit_of_measurement=UnitOfElectricPotential.VOLT,
        state_class=SensorStateClass.MEASUREMENT,
        device_class=SensorDeviceClass.VOLTAGE,
        icon="mdi:sine-wave",
        divisor=1000.0,
        precision=3,
    ),
    SmartmeterSensorDescription(
        key="voltage_l3",
        key_in_payload="v_l3_mv",
        name="Spannung L3",
        native_unit_of_measurement=UnitOfElectricPotential.VOLT,
        state_class=SensorStateClass.MEASUREMENT,
        device_class=SensorDeviceClass.VOLTAGE,
        icon="mdi:sine-wave",
        divisor=1000.0,
        precision=3,
    ),
    SmartmeterSensorDescription(
        key="current_l1",
        key_in_payload="i_l1_ma",
        name="Strom L1",
        native_unit_of_measurement=UnitOfElectricCurrent.AMPERE,
        state_class=SensorStateClass.MEASUREMENT,
        device_class=SensorDeviceClass.CURRENT,
        icon="mdi:current-ac",
        divisor=1000.0,
        precision=3,
    ),
    SmartmeterSensorDescription(
        key="current_l2",
        key_in_payload="i_l2_ma",
        name="Strom L2",
        native_unit_of_measurement=UnitOfElectricCurrent.AMPERE,
        state_class=SensorStateClass.MEASUREMENT,
        device_class=SensorDeviceClass.CURRENT,
        icon="mdi:current-ac",
        divisor=1000.0,
        precision=3,
    ),
    SmartmeterSensorDescription(
        key="current_l3",
        key_in_payload="i_l3_ma",
        name="Strom L3",
        native_unit_of_measurement=UnitOfElectricCurrent.AMPERE,
        state_class=SensorStateClass.MEASUREMENT,
        device_class=SensorDeviceClass.CURRENT,
        icon="mdi:current-ac",
        divisor=1000.0,
        precision=3,
    ),
    SmartmeterSensorDescription(
        key="frequency",
        key_in_payload="freq_mhz",
        name="Frequenz",
        native_unit_of_measurement=UnitOfFrequency.HERTZ,
        state_class=SensorStateClass.MEASUREMENT,
        device_class=SensorDeviceClass.FREQUENCY,
        icon="mdi:sine-wave",
        divisor=1000.0,
        precision=3,
    ),
    SmartmeterSensorDescription(
        key="power_factor_l1",
        key_in_payload="pf_l1",
        name="Leistungsfaktor L1",
        native_unit_of_measurement=PERCENTAGE,
        state_class=SensorStateClass.MEASUREMENT,
        icon="mdi:percent",
        divisor=10.0,
        precision=1,
    ),
    SmartmeterSensorDescription(
        key="job_error",
        key_in_payload="job_error",
        name="Systemfehler",
        icon="mdi:alert-circle-outline",
    ),
    SmartmeterSensorDescription(
        key="system_state",
        key_in_payload="job_state",
        name="Systemstatus",
        icon="mdi:state-machine",
    ),
    SmartmeterSensorDescription(
        key="system_uptime",
        key_in_payload="uptime_s",
        name="System Uptime",
        native_unit_of_measurement="s",
        state_class=SensorStateClass.MEASUREMENT,
        device_class=SensorDeviceClass.DURATION,
        suggested_unit_of_measurement="h",
        icon="mdi:timer-outline",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="has_alarms",
        key_in_payload="has_alarms",
        name="Alarme Aktiv",
        as_bool_text=True,
        icon="mdi:alarm-light-outline",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="alarm_list",
        key_in_payload="alarm_list",
        name="Alarm Liste",
        icon="mdi:format-list-bulleted",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="populated",
        key_in_payload="populated",
        name="Messdaten Verfuegbar",
        as_bool_text=True,
        icon="mdi:database-check-outline",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="tx_bytes",
        key_in_payload="tx_bytes",
        name="TX Bytes",
        native_unit_of_measurement=UnitOfInformation.BYTES,
        device_class=SensorDeviceClass.DATA_SIZE,
        state_class=SensorStateClass.TOTAL_INCREASING,
        icon="mdi:upload-network-outline",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="rx_bytes",
        key_in_payload="rx_bytes",
        name="RX Bytes",
        native_unit_of_measurement=UnitOfInformation.BYTES,
        device_class=SensorDeviceClass.DATA_SIZE,
        state_class=SensorStateClass.TOTAL_INCREASING,
        icon="mdi:download-network-outline",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="tx_frames",
        key_in_payload="tx_frames",
        name="TX Frames",
        state_class=SensorStateClass.TOTAL_INCREASING,
        icon="mdi:send-outline",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="rx_frames",
        key_in_payload="rx_frames",
        name="RX Frames",
        state_class=SensorStateClass.TOTAL_INCREASING,
        icon="mdi:tray-arrow-down",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="crc_errors",
        key_in_payload="crc_errors",
        name="CRC Fehler",
        state_class=SensorStateClass.TOTAL_INCREASING,
        icon="mdi:close-octagon-outline",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="nak_count",
        key_in_payload="nak_count",
        name="NAK Anzahl",
        state_class=SensorStateClass.TOTAL_INCREASING,
        icon="mdi:cancel",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="ack_count",
        key_in_payload="ack_count",
        name="ACK Anzahl",
        state_class=SensorStateClass.TOTAL_INCREASING,
        icon="mdi:check-circle-outline",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="wakeup_count",
        key_in_payload="wakeup_count",
        name="Wakeup Anzahl",
        state_class=SensorStateClass.TOTAL_INCREASING,
        icon="mdi:alarm-check",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="flag_ident",
        key_in_payload="flag_ident",
        name="Session IDENT",
        as_bool_text=True,
        icon="mdi:identifier",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="flag_logon",
        key_in_payload="flag_logon",
        name="Session LOGON",
        as_bool_text=True,
        icon="mdi:login",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="flag_auth",
        key_in_payload="flag_auth",
        name="Session AUTH",
        as_bool_text=True,
        icon="mdi:key-outline",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="last_login_ok",
        key_in_payload="last_login_ok",
        name="Letzter Login OK",
        as_bool_text=True,
        icon="mdi:account-check-outline",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="meter_time",
        key_in_payload="meter_time",
        name="Zaehlerzeit",
        device_class=SensorDeviceClass.TIMESTAMP,
        icon="mdi:clock-outline",
    ),
    SmartmeterSensorDescription(
        key="ip",
        key_in_payload="ip",
        name="IP Aktiv",
        icon="mdi:ip-network-outline",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="ssid",
        key_in_payload="ssid",
        name="SSID Aktiv",
        icon="mdi:wifi",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="ip_sta",
        key_in_payload="ip_sta",
        name="IP STA",
        icon="mdi:ip",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="ip_ap",
        key_in_payload="ip_ap",
        name="IP AP",
        icon="mdi:access-point-network",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="ssid_sta",
        key_in_payload="ssid_sta",
        name="SSID STA",
        icon="mdi:wifi-arrow-up-down",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="ssid_ap",
        key_in_payload="ssid_ap",
        name="SSID AP",
        icon="mdi:wifi-settings",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="tx_pin",
        key_in_payload="tx_pin",
        name="UART TX Pin",
        icon="mdi:serial-port",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="rx_pin",
        key_in_payload="rx_pin",
        name="UART RX Pin",
        icon="mdi:serial-port",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="manufacturer",
        key_in_payload="manufacturer",
        name="Hersteller",
        icon="mdi:factory",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="model",
        key_in_payload="model",
        name="Modell",
        icon="mdi:chip",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="serial_bcd",
        key_in_payload="serial_bcd",
        name="Seriennummer",
        icon="mdi:identifier",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="utility_serial",
        key_in_payload="utility_serial",
        name="Utility Seriennummer",
        icon="mdi:card-account-details-outline",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="fw_version",
        key_in_payload="fw_version",
        name="Firmware Version",
        icon="mdi:chip",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="app_version",
        key_in_payload="app_version",
        name="App Version",
        icon="mdi:application-cog-outline",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
    SmartmeterSensorDescription(
        key="idf_version",
        key_in_payload="idf_version",
        name="ESP-IDF Version",
        icon="mdi:tools",
        entity_category=EntityCategory.DIAGNOSTIC,
    ),
)


async def async_setup_entry(hass: HomeAssistant, entry: ConfigEntry, async_add_entities: AddEntitiesCallback) -> None:
    coordinator: SmartmeterCoordinator = hass.data[DOMAIN][entry.entry_id][DATA_COORDINATOR]
    async_add_entities(SmartmeterSensor(coordinator, entry, description) for description in DESCRIPTIONS)


class SmartmeterSensor(CoordinatorEntity[SmartmeterCoordinator], SensorEntity):
    entity_description: SmartmeterSensorDescription

    def __init__(self, coordinator: SmartmeterCoordinator, entry: ConfigEntry, description: SmartmeterSensorDescription) -> None:
        super().__init__(coordinator)
        self.entity_description = description
        self._entry = entry
        self._attr_unique_id = f"{entry.entry_id}_{description.key}"
        self._attr_has_entity_name = False
        self._attr_name = f"SMLEasy {description.name}"

    @property
    def device_info(self) -> DeviceInfo:
        data = self.coordinator.data or {}
        serial = str(data.get("serial_bcd") or self._entry.entry_id)
        model = str(data.get("model") or "MT631/MS2020")
        manufacturer = str(data.get("manufacturer") or "Iskraemeco")
        return DeviceInfo(
            identifiers={(DOMAIN, serial)},
            name=f"SMLEasy {serial}",
            manufacturer=manufacturer,
            model=model,
            sw_version=str(data.get("fw_version") or "unknown"),
            configuration_url=f"http://{self._entry.data['host']}:{self._entry.data['port']}",
        )

    @property
    def native_value(self) -> Any:
        data = self.coordinator.data or {}
        raw = data.get(self.entity_description.key_in_payload)
        if raw is None:
            return None

        if self.entity_description.as_bool_text:
            return "ON" if bool(raw) else "OFF"

        if self.entity_description.device_class == SensorDeviceClass.TIMESTAMP:
            if not isinstance(raw, str) or not raw.strip():
                return None
            try:
                return datetime.fromisoformat(raw.replace("Z", "+00:00"))
            except ValueError:
                return None

        if self.entity_description.divisor is not None:
            value = float(raw) / self.entity_description.divisor
            if self.entity_description.precision is not None:
                return round(value, self.entity_description.precision)
            return value

        return raw
