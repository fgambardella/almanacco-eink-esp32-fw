# Provenienza del codice (registro clean room)

Il firmware di Almanacco è scritto **senza consultare il codice di altri progetti di calendari e-ink**. Ogni versione parte da una specifica di comportamento scritta da Flavio Gambardella. Chi implementa usa solo le fonti ammesse dalla specifica (§0) e le annota qui sotto.

Questo file si aggiorna a ogni versione.

## v0.1 "Hello world" (12 novembre 2026)

**Implementazione**: Claude Code (modello Claude Opus 5.5, Anthropic), in una sessione nuova, sotto la supervisione di Flavio Gambardella. Il punto di partenza è la specifica [docs/specs/almanacco-spec-fw-v0.1-hello-world.md](docs/specs/almanacco-spec-fw-v0.1-hello-world.md). Nessun progetto di calendario e-ink è stato cercato, aperto o consultato. Le domande sui dettagli mancanti sono state poste a Flavio.

### Fonti consultate

| Fonte | Cosa è stato usato | Link | Licenza |
| --- | --- | --- | --- |
| Specifica Almanacco v0.1 | Comportamento, pin, struttura, criteri | `docs/specs/almanacco-spec-fw-v0.1-hello-world.md` | del progetto |
| GxEPD2 1.6.9: intestazioni e sorgenti della libreria (`GxEPD2_BW.h`, `GxEPD2_EPD.h/.cpp`, `epd/GxEPD2_750_T7.h/.cpp`) | Firma di `init()` (diagnostica, durata del reset, selezione SPI), `hibernate()`, `end()`, livello attivo e timeout del pin BUSY del driver GDEW075T7 | https://github.com/ZinggJM/GxEPD2 (copia installata da PlatformIO in `.pio/libdeps`) | GPL-3.0 |
| GxEPD2 1.6.9: esempio `GxEPD2_Example.ino` | Impulso di reset di 2 ms per le driver board Waveshare con circuito di reset "intelligente"; uso di `SPI.begin()` con pin rimappati sull'ESP32; impostazioni SPI a 4 MHz | https://github.com/ZinggJM/GxEPD2/tree/master/examples/GxEPD2_Example | GPL-3.0 |
| Adafruit GFX Library 1.13.0 | `getTextBounds()`, `drawRect()`, font FreeSans inclusi nella libreria | https://github.com/adafruit/Adafruit-GFX-Library | BSD-2-Clause (font: GNU FreeFont, GPL-3.0 con eccezione per i font) |
| Core Arduino-ESP32 3.3.12: `SPIClass::begin()` | Verifica che una seconda chiamata a `begin()` (fatta da GxEPD2) non reimposti i pin già scelti | https://github.com/espressif/arduino-esp32 (copia installata da PlatformIO) | LGPL-2.1 |
| API ESP-IDF 5.5 (intestazioni installate da PlatformIO: `driver/rtc_io.h`, `esp_sleep.h`, `esp_system.h`, `esp_mac.h`) | `esp_deep_sleep_start()`, `esp_sleep_disable_wakeup_source()`, `rtc_gpio_hold_en/dis()`, `esp_reset_reason()`, `esp_efuse_mac_get_default()` | https://docs.espressif.com/projects/esp-idf/en/stable/esp32/ | Apache-2.0 |
| Registro PlatformIO | Versioni disponibili di `platformio/espressif32`, `zinggjm/GxEPD2`, `adafruit/Adafruit GFX Library` (`pio pkg show`) | https://registry.platformio.org | n/d |
| Note di rilascio di platform-espressif32 | Versioni del core Arduino e di ESP-IDF incluse in 8.0.0, 7.x e 6.13.0, per scegliere la piattaforma | https://github.com/platformio/platform-espressif32/releases | Apache-2.0 |

Per le API di Espressif, Arduino-ESP32 e PlatformIO (opzioni di `platformio.ini` come `build_src_flags`) è servita anche la conoscenza generale del modello, che deriva dalla documentazione ufficiale di questi progetti. In questa sessione le pagine web di quella documentazione non sono state aperte. Le API sono state verificate sulle intestazioni installate e con una compilazione riuscita.

Il testo di `LICENSE` è la GNU GPL v3 integrale e invariata (https://www.gnu.org/licenses/gpl-3.0.txt). Era già presente nel commit iniziale del repository.
