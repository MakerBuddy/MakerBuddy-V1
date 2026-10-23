# GitHub firmware feed

Clients use this JSON endpoint:

[https://raw.githubusercontent.com/MakerBuddy/MakerBuddy-Firmware-OTA/main/firmware.json](https://raw.githubusercontent.com/MakerBuddy/MakerBuddy-Firmware-OTA/main/firmware.json)

The corresponding GitHub `blob` page is a file viewer; it is not a JSON endpoint.
Keep the repository public so browser/mobile clients need no GitHub credentials.
The raw response currently has text/plain MIME, which `response.json()` handles,
and `Access-Control-Allow-Origin: *`, allowing hosted WebApp checks. Extensions
have a fixed raw GitHub host permission and Firefox/Safari CSP entry. Local-device
HTTP permissions remain optional and separate.

The verified feed reports version **1.9.1**, date **2026-05-24**, and this URL:
`https://github.com/MakerBuddy/MakerBuddy-Firmware-OTA/blob/main/MakerBuddy.ino.bin`.
That URL serves a GitHub page. Update checking succeeds, but clients cannot offer
OTA for it. Workspace firmware is 2.9, so the current feed is also an older release.
Do not relabel an old binary as a new version; publish matching tested metadata
and a matching binary as a separate release decision.

For the existing OTA policy, provide a direct **HTTP** `.bin` URL with a binary
matching `version`. To move binaries to GitHub HTTPS, use a direct raw or release
asset URL and first implement/test ESP32 TLS trust, redirects, and payload sizing.
The current clients deliberately guard against issuing OTA for unsupported URLs.
No public feed edit, binary upload, or device update was performed here.

GitHub caches raw files (observed max-age=300); publication can take several
minutes to appear even when a client bypasses its local cache. Reload extensions
after their manifest changes and rebuild mobile to apply version/build metadata.
Check-only validation never needs an OTA install.

## Historical server route repair

The GitHub migration removes this server fix from the update-check requirements.
The earlier diagnostic and configuration fragment below remain for reference.

### Repair the former firmware feed HTTPS route

On 30 September 2026, the HTTP feed returned valid JSON (firmware 2.3), while
`https://app.makerbuddy.cc/firmware.json` returned 301 with Location pointing back
to HTTP. A clean installed Chromium extension reproduced `ERR_TOO_MANY_REDIRECTS`
when its HTTP request was upgraded to HTTPS. Changing client permissions alone
cannot repair that server redirect loop.

The public feed must serve JSON directly at its HTTPS URL with status 200 and
`Access-Control-Allow-Origin: *`. The JSON's `url` should remain the existing
**HTTP** firmware binary URL: ESP32 OTA uses HTTP. Do not change local device
WebSockets or redirect firmware binary downloads to HTTPS as part of this fix.

For an Apache/LiteSpeed host, find the existing HTTPS-to-HTTP rule for this
subdomain and exclude `/firmware.json`. The accompanying
[configuration fragment](firmware-feed-https.htaccess.example) adds the exclusion
condition to that existing redirect rule. Merge it into the rule before the
RewriteRule; do not overwrite the existing site configuration. This fragment
requires the current firmware.json file to exist at the document root.

If the redirect is configured at the virtual-host, hosting-panel, CDN, or
Cloudflare rule level, make the same path exclusion at that level instead.
Confirm the certificate is valid and no HTTPS-to-HTTP redirect remains. The
hosted dashboard can retain its existing transport policy for local WebSockets.

After applying the server change:

```sh
curl -i --max-time 20 https://app.makerbuddy.cc/firmware.json
```

Expected: HTTP 200, application/json, a valid version and HTTP `.bin` URL, and
CORS allowing the dashboard. Reload each extension after updating its manifest,
then check for updates from a connected device. No OTA install is needed to
verify the check. The live feed remains 2.3; updating its published release/binary
is a separate release decision. No server settings or binary were published by
this repository change.
