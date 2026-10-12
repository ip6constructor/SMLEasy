from __future__ import annotations

from typing import Any

import voluptuous as vol
from homeassistant import config_entries
from homeassistant.const import CONF_HOST, CONF_PORT
from homeassistant.helpers import selector
from homeassistant.helpers.aiohttp_client import async_get_clientsession

from .api import SmartmeterApi, SmartmeterApiError
from .const import CONF_SCAN_INTERVAL, CONF_USE_SSL, DEFAULT_PORT, DEFAULT_SCAN_INTERVAL, DOMAIN, PUSH_VALUES


def _schema(host: str = "", port: int = DEFAULT_PORT, use_ssl: bool = False, scan_interval: int = DEFAULT_SCAN_INTERVAL) -> vol.Schema:
    return vol.Schema(
        {
            vol.Required(CONF_HOST, default=host): str,
            vol.Required(CONF_PORT, default=port): int,
            vol.Optional(CONF_USE_SSL, default=use_ssl): bool,
            vol.Optional(CONF_SCAN_INTERVAL, default=scan_interval): int,
        }
    )


class SmartmeterConfigFlow(config_entries.ConfigFlow, domain=DOMAIN):
    VERSION = 1

    async def async_step_user(self, user_input: dict[str, Any] | None = None):
        errors: dict[str, str] = {}

        if user_input is not None:
            host = user_input[CONF_HOST].strip()
            port = int(user_input[CONF_PORT])
            use_ssl = bool(user_input.get(CONF_USE_SSL, False))
            scan_interval = max(5, int(user_input.get(CONF_SCAN_INTERVAL, DEFAULT_SCAN_INTERVAL)))

            api = SmartmeterApi(async_get_clientsession(self.hass), host, port, use_ssl)

            try:
                status = await api.async_get_status()
            except SmartmeterApiError:
                errors["base"] = "cannot_connect"
            else:
                unique = str(status.get("serial_bcd") or host)
                await self.async_set_unique_id(unique)
                self._abort_if_unique_id_configured()
                return self.async_create_entry(
                    title=f"SMLEasy {status.get('serial_bcd') or host}",
                    data={
                        CONF_HOST: host,
                        CONF_PORT: port,
                        CONF_USE_SSL: use_ssl,
                        CONF_SCAN_INTERVAL: scan_interval,
                    },
                )

        return self.async_show_form(step_id="user", data_schema=_schema(), errors=errors)

    @staticmethod
    def async_get_options_flow(config_entry: config_entries.ConfigEntry):
        return SmartmeterOptionsFlow(config_entry)


class SmartmeterOptionsFlow(config_entries.OptionsFlow):
    def __init__(self, config_entry: config_entries.ConfigEntry) -> None:
        # Recent HA versions expose config_entry as a read-only property.
        try:
            self.config_entry = config_entry
        except Exception:
            self._entry = config_entry

    @property
    def _config_entry(self) -> config_entries.ConfigEntry:
        return getattr(self, "_entry", None) or self.config_entry

    async def async_step_init(self, user_input: dict[str, Any] | None = None):
        if user_input is not None:
            data = {k: v for k, v in user_input.items() if v not in (None, "")}
            data[CONF_SCAN_INTERVAL] = max(5, int(user_input[CONF_SCAN_INTERVAL]))
            return self.async_create_entry(data=data)

        current = self._config_entry.options.get(
            CONF_SCAN_INTERVAL,
            self._config_entry.data.get(CONF_SCAN_INTERVAL, DEFAULT_SCAN_INTERVAL),
        )
        fields: dict[Any, Any] = {vol.Required(CONF_SCAN_INTERVAL, default=current): int}
        for name in PUSH_VALUES:
            key = f"push_{name}"
            existing = self._config_entry.options.get(key)
            marker = vol.Optional(key, description={"suggested_value": existing})
            fields[marker] = selector.EntitySelector(selector.EntitySelectorConfig(domain="sensor"))
        schema = vol.Schema(fields)
        return self.async_show_form(step_id="init", data_schema=schema)
