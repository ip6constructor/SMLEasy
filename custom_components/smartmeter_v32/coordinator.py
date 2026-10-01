from __future__ import annotations

from datetime import timedelta
import logging
from typing import Any

from homeassistant.core import HomeAssistant
from homeassistant.exceptions import ConfigEntryAuthFailed
from homeassistant.helpers.update_coordinator import DataUpdateCoordinator, UpdateFailed

from .api import SmartmeterApi, SmartmeterApiError
from .const import DOMAIN

LOGGER = logging.getLogger(__name__)


class SmartmeterCoordinator(DataUpdateCoordinator[dict[str, Any]]):
    def __init__(
        self,
        hass: HomeAssistant,
        api: SmartmeterApi,
        update_interval_seconds: int,
    ) -> None:
        super().__init__(
            hass,
            logger=LOGGER,
            name=DOMAIN,
            update_interval=timedelta(seconds=update_interval_seconds),
        )
        self.api = api

    async def _async_update_data(self) -> dict[str, Any]:
        try:
            return await self.api.async_get_status()
        except SmartmeterApiError as err:
            msg = str(err).lower()
            if "401" in msg or "403" in msg:
                raise ConfigEntryAuthFailed from err
            raise UpdateFailed(str(err)) from err
