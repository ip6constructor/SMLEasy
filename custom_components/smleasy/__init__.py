from __future__ import annotations

from datetime import timedelta
import logging

from homeassistant.config_entries import ConfigEntry
from homeassistant.core import Event, HomeAssistant, callback
from homeassistant.helpers.event import async_track_state_change_event, async_track_time_interval
from homeassistant.helpers.aiohttp_client import async_get_clientsession

from .api import SmartmeterApi
from .const import (
    CONF_SCAN_INTERVAL,
    CONF_USE_SSL,
    DATA_API,
    DATA_COORDINATOR,
    DEFAULT_SCAN_INTERVAL,
    DOMAIN,
    PLATFORMS,
    PUSH_REFRESH_SECONDS,
    PUSH_VALUES,
)
from .coordinator import SmartmeterCoordinator

LOGGER = logging.getLogger(__name__)


async def async_setup_entry(hass: HomeAssistant, entry: ConfigEntry) -> bool:
    session = async_get_clientsession(hass)

    api = SmartmeterApi(
        session=session,
        host=entry.data["host"],
        port=entry.data["port"],
        use_ssl=entry.data.get(CONF_USE_SSL, False),
    )

    coordinator = SmartmeterCoordinator(
        hass=hass,
        api=api,
        update_interval_seconds=entry.options.get(
            CONF_SCAN_INTERVAL,
            entry.data.get(CONF_SCAN_INTERVAL, DEFAULT_SCAN_INTERVAL),
        ),
    )

    await coordinator.async_config_entry_first_refresh()

    hass.data.setdefault(DOMAIN, {})[entry.entry_id] = {
        DATA_API: api,
        DATA_COORDINATOR: coordinator,
    }

    await hass.config_entries.async_forward_entry_setups(entry, PLATFORMS)
    _setup_push(hass, entry, coordinator)
    entry.async_on_unload(entry.add_update_listener(_async_reload))
    return True


async def _async_reload(hass: HomeAssistant, entry: ConfigEntry) -> None:
    await hass.config_entries.async_reload(entry.entry_id)


def _setup_push(hass: HomeAssistant, entry: ConfigEntry, coordinator: SmartmeterCoordinator) -> None:
    """Publishes the chosen entities to the device topics (MQTT) on change and periodically."""
    mapping = {
        name: entity
        for name in PUSH_VALUES
        if (entity := entry.options.get(f"push_{name}"))
    }
    if not mapping or "mqtt" not in hass.config.components:
        if mapping:
            LOGGER.warning("MQTT integration is not set up; values cannot be sent to the SMLEasy device")
        return

    from homeassistant.components import mqtt

    async def publish(name: str, entity_id: str) -> None:
        base = (coordinator.data or {}).get("ha_input_base")
        state = hass.states.get(entity_id)
        if not base or state is None:
            return
        try:
            float(state.state)
        except ValueError:
            return
        await mqtt.async_publish(hass, f"{base}{name}", state.state, qos=0, retain=True)

    by_entity = {entity: name for name, entity in mapping.items()}

    @callback
    def changed(event: Event) -> None:
        name = by_entity.get(event.data["entity_id"])
        if name:
            hass.async_create_task(publish(name, event.data["entity_id"]))

    async def refresh_all(_now=None) -> None:
        for name, entity in mapping.items():
            await publish(name, entity)

    entry.async_on_unload(async_track_state_change_event(hass, list(by_entity), changed))
    entry.async_on_unload(async_track_time_interval(hass, refresh_all, timedelta(seconds=PUSH_REFRESH_SECONDS)))
    entry.async_on_unload(coordinator.async_add_listener(lambda: hass.async_create_task(refresh_all())))


async def async_unload_entry(hass: HomeAssistant, entry: ConfigEntry) -> bool:
    unload_ok = await hass.config_entries.async_unload_platforms(entry, PLATFORMS)
    if unload_ok:
        hass.data[DOMAIN].pop(entry.entry_id)
    return unload_ok
