#!/bin/bash
# Regression test for the opt-in weekly Fable row in claude-usage-daemon.sh:
# read_fable_setting(), parse_fable_usage() and the "fr" aging in age_payload().
#
# `fable` defaults to off so existing setups never hit the usage endpoint; when
# on, the weekly_scoped Fable row from /api/oauth/usage becomes the payload
# fragment ,"f":..,"fr":..,"fn":"Fable", and heartbeats age "fr" like "wr".
set -u

DAEMON="$(dirname "$0")/../claude-usage-daemon.sh"

extract() { awk -v fn="$1" '$0 ~ "^"fn"\\(\\) \\{"{f=1} f{print} f&&/^\}/{exit}' "$DAEMON"; }
eval "$(extract read_fable_setting)"
eval "$(extract parse_fable_usage)"
eval "$(extract age_payload)"

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
CONFIG_FILE="$TMP/config"

fail=0
check() {  # check <label> <got> <want>
    if [ "$2" = "$3" ]; then echo "PASS: $1"; else echo "FAIL: $1: got '$2' want '$3'"; fail=1; fi
}

# --- config: off unless explicitly on ---
rm -f "$CONFIG_FILE"
check "fable: no config -> off"           "$(read_fable_setting)" "off"
printf 'chime = on\n' > "$CONFIG_FILE"
check "fable: unset -> off"               "$(read_fable_setting)" "off"
printf '  fable = ON # comment\r\n' > "$CONFIG_FILE"
check "fable: on, case/comment/CRLF"      "$(read_fable_setting)" "on"
printf 'fable = yes\n' > "$CONFIG_FILE"
check "fable: junk -> off"                "$(read_fable_setting)" "off"

# --- parsing the usage endpoint body ---
NOW=1790000000
RESET=$(python3 -c "from datetime import datetime, timezone; print(datetime.fromtimestamp($NOW + 48*3600, tz=timezone.utc).isoformat().replace('+00:00', 'Z'))")
BODY='{"five_hour":{"utilization":7},"limits":[
  {"kind":"session","group":"session","percent":7.0,"resets_at":null},
  {"kind":"weekly_scoped","group":"weekly","percent":4.4,"resets_at":"'"$RESET"'","scope":{"model":{"display_name":"Fable"}}}]}'
check "parse: Fable row -> fragment" \
    "$(printf '%s' "$BODY" | parse_fable_usage "$NOW")" ',"f":4,"fr":2880,"fn":"Fable"'
check "parse: no Fable row -> empty" \
    "$(printf '%s' "${BODY//Fable/Sonnet}" | parse_fable_usage "$NOW")" ''
check "parse: not JSON -> empty" \
    "$(printf 'Bad Gateway' | parse_fable_usage "$NOW")" ''
check "parse: no limits key -> empty" \
    "$(printf '{"five_hour":{}}' | parse_fable_usage "$NOW")" ''

# --- heartbeat ages the Fable countdown, leaves the rest alone ---
P='{"s":7,"sr":100,"w":6,"wr":200,"f":4,"fr":300,"fn":"Fable","ok":true}'
check "age: fr ticks down with sr/wr" \
    "$(age_payload "$P" 600)" '{"s":7,"sr":90,"w":6,"wr":190,"f":4,"fr":290,"fn":"Fable","ok":true}'
check "age: unknown fr (-1) untouched" \
    "$(age_payload '{"fr":-1}' 600)" '{"fr":-1}'

exit $fail
