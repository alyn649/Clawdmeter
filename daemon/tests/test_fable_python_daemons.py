"""Tests for the opt-in weekly Fable row in the macOS and Windows daemons.

The `fable` config option defaults to off: the daemon must not touch the usage
endpoint and the payload must be byte-for-byte what it was before the feature.
When on, the `weekly_scoped` Fable row from `/api/oauth/usage` lands in the
payload as `f` / `fr` / `fn`.
"""

import asyncio
import time
from datetime import datetime, timezone
from unittest.mock import AsyncMock, MagicMock, patch

import pytest

import daemon.claude_usage_daemon as macos
import daemon.claude_usage_daemon_windows as windows

DAEMONS = pytest.mark.parametrize("d", [macos, windows], ids=["macos", "windows"])

HEADERS = {
    "anthropic-ratelimit-unified-5h-utilization": "0.07",
    "anthropic-ratelimit-unified-5h-reset": str(int(time.time()) + 3600),
    "anthropic-ratelimit-unified-7d-utilization": "0.06",
    "anthropic-ratelimit-unified-7d-reset": str(int(time.time()) + 86400),
    "anthropic-ratelimit-unified-5h-status": "allowed",
}


def _usage_body(display_name="Fable", percent=4.0, hours=48):
    resets = datetime.fromtimestamp(time.time() + hours * 3600, tz=timezone.utc)
    return {
        "five_hour": {"utilization": 7.0},
        "limits": [
            {"kind": "session", "group": "session", "percent": 7.0, "resets_at": None},
            {"kind": "weekly_scoped", "group": "weekly", "percent": percent,
             "resets_at": resets.isoformat().replace("+00:00", "Z"),
             "scope": {"model": {"display_name": display_name}}},
        ],
    }


def _client(body):
    post_resp = MagicMock(status_code=200, headers=HEADERS)
    get_resp = MagicMock(status_code=200)
    get_resp.json.return_value = body
    http = MagicMock()
    http.post = AsyncMock(return_value=post_resp)
    http.get = AsyncMock(return_value=get_resp)
    ctx = MagicMock()
    ctx.__aenter__ = AsyncMock(return_value=http)
    ctx.__aexit__ = AsyncMock(return_value=False)
    return http, ctx


def _poll(d, tmp_path, config_text, body):
    cfg = tmp_path / "config"
    cfg.write_text(config_text)
    http, ctx = _client(body)
    with patch.object(d, "CONFIG_FILE", cfg), \
         patch.object(d.httpx, "AsyncClient", return_value=ctx):
        payload = asyncio.run(d.poll_api("token"))
    return http, payload


@DAEMONS
def test_fable_defaults_off_and_skips_endpoint(d, tmp_path):
    http, payload = _poll(d, tmp_path, "chime = off\n", _usage_body())
    http.get.assert_not_called()
    assert not {"f", "fr", "fn"} & payload.keys()
    assert payload["s"] == 7 and payload["w"] == 6


@DAEMONS
def test_fable_on_adds_weekly_row(d, tmp_path):
    http, payload = _poll(d, tmp_path, "fable = on\n", _usage_body(percent=4.4, hours=48))
    http.get.assert_called_once()
    assert payload["f"] == 4
    assert payload["fn"] == "Fable"
    assert abs(payload["fr"] - 48 * 60) <= 1


@DAEMONS
def test_fable_on_without_row_leaves_payload_unchanged(d, tmp_path):
    _http, payload = _poll(d, tmp_path, "fable = on\n", _usage_body(display_name="Sonnet"))
    assert not {"f", "fr", "fn"} & payload.keys()
