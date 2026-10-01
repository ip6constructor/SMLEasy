from __future__ import annotations

from typing import Any

from aiohttp import ClientError, ClientSession


class SmartmeterApiError(Exception):
    """Smartmeter API error."""


class SmartmeterApi:
    def __init__(self, session: ClientSession, host: str, port: int, use_ssl: bool) -> None:
        scheme = "https" if use_ssl else "http"
        self._base = f"{scheme}://{host}:{port}"
        self._session = session

    async def async_get_status(self) -> dict[str, Any]:
        url = f"{self._base}/api/status"
        try:
            async with self._session.get(url, timeout=15) as resp:
                resp.raise_for_status()
                data = await resp.json()
        except (ClientError, TimeoutError, ValueError) as err:
            raise SmartmeterApiError(str(err)) from err

        if not isinstance(data, dict):
            raise SmartmeterApiError("Invalid status payload")
        return data

    async def async_start_read(self) -> None:
        await self._async_post("/api/start")

    async def async_stop_read(self) -> None:
        await self._async_post("/api/stop")

    async def async_reset_counters(self) -> None:
        await self._async_post("/api/reset_counters")

    async def _async_post(self, path: str) -> None:
        url = f"{self._base}{path}"
        try:
            async with self._session.post(url, timeout=15) as resp:
                resp.raise_for_status()
        except (ClientError, TimeoutError) as err:
            raise SmartmeterApiError(str(err)) from err
