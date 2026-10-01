from __future__ import annotations

from dataclasses import dataclass
from typing import Callable, Awaitable

from homeassistant.components.button import ButtonEntity, ButtonEntityDescription
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.device_registry import DeviceInfo
from homeassistant.helpers.entity_platform import AddEntitiesCallback
from homeassistant.helpers.update_coordinator import CoordinatorEntity

from .api import SmartmeterApi
from .const import DATA_API, DATA_COORDINATOR, DOMAIN
from .coordinator import SmartmeterCoordinator


@dataclass(frozen=True, kw_only=True)
class SmartmeterButtonDescription(ButtonEntityDescription):
    press_fn_name: str


DESCRIPTIONS: tuple[SmartmeterButtonDescription, ...] = (
    SmartmeterButtonDescription(
        key="start_read",
        name="Ablesung starten",
        press_fn_name="async_start_read",
    ),
    SmartmeterButtonDescription(
        key="stop_read",
        name="Ablesung stoppen",
        press_fn_name="async_stop_read",
    ),
    SmartmeterButtonDescription(
        key="reset_counters",
        name="Zähler zurücksetzen",
        press_fn_name="async_reset_counters",
    ),
)


async def async_setup_entry(hass: HomeAssistant, entry: ConfigEntry, async_add_entities: AddEntitiesCallback) -> None:
    data = hass.data[DOMAIN][entry.entry_id]
    coordinator: SmartmeterCoordinator = data[DATA_COORDINATOR]
    api: SmartmeterApi = data[DATA_API]

    async_add_entities(SmartmeterButton(coordinator, api, entry, description) for description in DESCRIPTIONS)


class SmartmeterButton(CoordinatorEntity[SmartmeterCoordinator], ButtonEntity):
    entity_description: SmartmeterButtonDescription

    def __init__(
        self,
        coordinator: SmartmeterCoordinator,
        api: SmartmeterApi,
        entry: ConfigEntry,
        description: SmartmeterButtonDescription,
    ) -> None:
        super().__init__(coordinator)
        self.entity_description = description
        self._api = api
        self._entry = entry
        self._attr_unique_id = f"{entry.entry_id}_{description.key}"
        self._attr_has_entity_name = True

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

    async def async_press(self) -> None:
        fn: Callable[[], Awaitable[None]] = getattr(self._api, self.entity_description.press_fn_name)
        await fn()
        await self.coordinator.async_request_refresh()
