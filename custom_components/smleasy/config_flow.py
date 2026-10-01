from __future__ import annotations

from typing import Any

import voluptuous as vol
from homeassistant import config_entries
from homeassistant.const import CONF_HOST, CONF_PORT
from homeassistant.helpers.aiohttp_client import async_get_clientsession

from .api import SmartmeterApi, SmartmeterApiError
from .const import CONF_SCAN_INTERVAL, CONF_USE_SSL, DEFAULT_PORT, DEFAULT_SCAN_INTERVAL, DOMAIN


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
        self.config_entry = config_entry

    async def async_step_init(self, user_input: dict[str, Any] | None = None):
        if user_input is not None:
            return self.async_create_entry(data={CONF_SCAN_INTERVAL: max(5, int(user_input[CONF_SCAN_INTERVAL]))})

        current = self.config_entry.options.get(
            CONF_SCAN_INTERVAL,
            self.config_entry.data.get(CONF_SCAN_INTERVAL, DEFAULT_SCAN_INTERVAL),
        )
        schema = vol.Schema({vol.Required(CONF_SCAN_INTERVAL, default=current): int})
        return self.async_show_form(step_id="init", data_schema=schema)
