# Almanacco firmware

**Almanacco** è un calendario da parete su carta elettronica (e-ink) per la famiglia, basato su ESP32.

Questa è la versione **v0.1.2 "Hello world"**: serve solo a verificare al banco che scheda, driver board, display e cablaggio funzionino. All'accensione il firmware:

1. accende la driver board del display (pin PWR);
2. scrive "Hello, world!" con un bordo su tutto il pannello;
3. spegne la driver board;
4. mette l'ESP32 in deep sleep. L'immagine resta visibile anche senza alimentazione.

**Novità della v0.1.2**: la [modalità pulizia schermo](#modalità-pulizia-schermo), che riporta il display tutto a bianco prima di lasciarlo inutilizzato a lungo.

**Novità della v0.1.1**: corretta la diagnostica seriale. Se il display non risponde (timeout del pin BUSY), il firmware stampa "Refresh NON riuscito: timeout BUSY (durata N ms)" invece di "Refresh completo: N ms". Le due righe usano la stessa condizione del messaggio di ERRORE, quindi non possono più contraddirsi.

WiFi, batteria, orario, calendario e meteo arriveranno nelle versioni successive. La specifica di questa versione è in [docs/specs/almanacco-spec-fw-v0.1-hello-world.md](docs/specs/almanacco-spec-fw-v0.1-hello-world.md).

## Hardware

- **Scheda**: WEMOS LOLIN D32 (ESP32-WROOM-32, 4 MB di flash), alimentata **solo da USB**.
- **Batteria LiPo: NON collegata** in questa fase.
- **Display**: Waveshare 7.5" e-Paper V2, 800×480, bianco/nero (controller GDEW075T7), con **e-Paper Driver HAT** (pin PWR, cavo a 9 poli).

### Collegamenti

| Segnale driver board | Funzione | Pin LOLIN D32 |
| --- | --- | --- |
| VCC | Alimentazione 3,3 V | 3V |
| GND | Massa | GND |
| DIN | SPI MOSI | GPIO 14 |
| CLK | SPI SCK | GPIO 13 |
| CS | Chip select (attivo basso) | GPIO 15 |
| DC | Dati/comando | GPIO 27 |
| RST | Reset (attivo basso) | GPIO 26 |
| BUSY | Display occupato | GPIO 25 |
| PWR | Accensione della driver board (attivo alto) | GPIO 33 |

- VCC va sul pin **3V**, mai su 5V/USB.
- MISO non si usa. GPIO 12 resta libero: è un pin di strapping e influisce sull'avvio.
- Tutti i pin e le costanti hardware sono in [include/board_pins.h](include/board_pins.h).

## Toolchain

Per compilare serve [PlatformIO](https://platformio.org/): l'estensione per VS Code oppure la riga di comando (`brew install platformio` su macOS).

Versioni fissate in [platformio.ini](platformio.ini):

| Componente | Versione | Motivo |
| --- | --- | --- |
| Piattaforma `platformio/espressif32` | 8.0.0 | È l'ultima versione stabile (non pre-release). È la prima versione ufficiale PlatformIO con il core Arduino-ESP32 3.x (3.3.12, basato su ESP-IDF 5.5). Le serie 6.x e 7.x restano sul core 2.0.17, che non riceve più aggiornamenti. GxEPD2 1.6.x supporta il core 3.x, e conviene partire da lì in vista di WiFi e NTP nelle versioni successive. |
| GxEPD2 | 1.6.9 | Ultima versione. Contiene il driver `GxEPD2_750_T7` per il pannello V2. |
| Adafruit GFX Library | 1.13.0 | Ultima versione. Fornisce testo e font usati da GxEPD2. |

La 8.0.0 è uscita da meno di un mese (19 ottobre 2026), quindi è ancora poco collaudata. Se desse problemi, il ripiego è `platformio/espressif32@7.1.3`, con il core Arduino 2.0.17. Basta cambiare la riga `platform` in `platformio.ini`. Il codice della v0.1.1 è stato compilato con entrambe le piattaforme, senza errori e senza warning (`-Wall -Wextra`). La v0.1.2 è stata compilata, senza errori e senza warning, solo con la 8.0.0.

**Porta seriale non visibile?** La D32 comunica con il computer tramite un convertitore USB-seriale. Leggi la sigla sul chip vicino al connettore USB: se è un CH340, installa il driver WCH. Le versioni recenti di macOS di solito lo includono già. Prova anche un altro cavo, perché alcuni cavi trasportano solo la ricarica e non i dati.

## Compilare, caricare, monitor seriale

```bash
pio run                  # compila
pio run -t upload        # compila e carica sulla scheda
pio device monitor       # apre il monitor seriale a 115200 baud (Ctrl+C per uscire)
```

La prima compilazione scarica il compilatore e il core per ESP32: alcuni GB e diversi minuti. Le successive durano pochi secondi.

## Procedura di prova al banco

1. **A scheda scollegata**, controlla ogni collegamento con la tabella qui sopra, uno per uno. Soprattutto: VCC sul pin 3V, GND su GND, PWR su GPIO 33.
2. Controlla che il cavo piatto del pannello sia inserito a fondo nel connettore della driver board, con il blocchetto chiuso. Il vetro è sottile: appoggia il pannello su una superficie piana, senza piegare il cavo.
3. Batteria **scollegata**.
4. Collega la D32 al computer con un cavo USB dati.
5. `pio run -t upload`, poi `pio device monitor`.
6. Premi RESET sulla D32 e guarda la seriale. Deve mostrare, in ordine:
   ```
   Almanacco firmware v0.1.2 hello-world
   Chip: ESP32-D0WD..., revisione ...
   Flash: 4 MB
   MAC: xx:xx:xx:xx:xx:xx
   Causa dell'avvio: accensione o tasto RESET (codice 1)
   PWR: driver board accesa
   ...                      (diagnostica di GxEPD2: tempi del pin BUSY)
   Refresh completo: ~4000 ms
   PWR: driver board spenta
   Deep sleep: premi RESET per ripetere
   ```
   Sull'ESP32 il tasto RESET riavvia il chip come un'accensione, quindi la causa dell'avvio è la stessa nei due casi.
7. Guarda il display. Il testo deve essere **dritto** in verticale e il **bordo completo** su tutti e quattro i lati. Il refresh deve durare qualche secondo (riferimento Waveshare: circa 4 s).
8. Stacca l'USB: l'immagine deve **restare**. Ricollegalo (o premi RESET): il ciclo si ripete e il display si ridisegna.

### Se qualcosa non va

| Sintomo | Cause probabili |
| --- | --- |
| Il computer non vede la porta | Cavo solo di ricarica; driver USB-seriale mancante |
| Seriale muta | Velocità del monitor diversa da 115200; avvio prima che il monitor sia aperto (premi RESET) |
| Timeout del display (BUSY) | PWR non collegato o non alto; BUSY non su GPIO 25; cavo piatto non inserito bene. Se i collegamenti sono giusti, prova `EPD_RESET_PULSE_MS = 10` o un `EPD_PWR_SETTLE_MS` più lungo in `board_pins.h` |
| Immagine confusa o solo in parte | Driver sbagliato nel codice (pannello V2 = GDEW075T7); DIN/CLK invertiti |
| Testo capovolto | Cambia `EPD_ROTATION` in `board_pins.h` (1 ↔ 3) |
| Nulla sul display, nessun errore | VCC non sul pin 3V; GND mancante; PWR rimasto basso |

## Modalità pulizia schermo

Un display e-ink conserva l'ultima immagine anche senza alimentazione. Se il pannello deve restare inutilizzato a lungo (in un cassetto, tra una prova e l'altra), è meglio lasciarlo **tutto bianco**: un'immagine tenuta ferma per mesi può lasciare una traccia leggera.

La modalità pulizia è un ambiente PlatformIO a parte, `lolin_d32_clear`. Fa la stessa sequenza del firmware normale (seriale, accensione della driver board, inizializzazione, refresh completo, spegnimento, deep sleep), ma il refresh disegna uno schermo tutto bianco, senza bordo e senza testo.

```bash
pio run -e lolin_d32_clear -t upload   # compila e carica la modalità pulizia
pio device monitor                     # facoltativo: per leggere la seriale
```

La seriale mostra, dopo le righe sulla scheda:

```
Modalita' pulizia schermo: il display verra' riportato a bianco
PWR: driver board accesa
...
Refresh completo: ~4000 ms
PWR: driver board spenta
Deep sleep: premi RESET per ripetere
```

Se il display non risponde compare invece "Refresh NON riuscito: timeout BUSY (durata N ms)", seguito dalla stessa lista di controlli del firmware normale.

Quando il display è bianco, **stacca l'USB e riponi il pannello**.

> **Attenzione: non ricaricare subito il firmware normale.** Dopo il caricamento la scheda si riavvia da sola e il firmware normale ridisegnerebbe "Hello, world!", annullando la pulizia. Ricarica `pio run -t upload` (ambiente `lolin_d32`, il predefinito) solo quando riprendi a usare il display.

`pio run` senza `-e` compila solo l'ambiente predefinito `lolin_d32`. Per compilarli entrambi: `pio run -e lolin_d32 -e lolin_d32_clear`.

## Licenza

Copyright (C) 2026 Flavio Gambardella.

Questo firmware è software libero, distribuito sotto la **GNU General Public License, versione 3 o (a tua scelta) qualsiasi versione successiva** (GPL-3.0-or-later). Il testo integrale della GPL v3 è nel file [LICENSE](LICENSE).

Il motivo è che il firmware usa GxEPD2, distribuita sotto GPL-3.0. Adafruit GFX (BSD) e il core Arduino-ESP32 (LGPL-2.1 e Apache-2.0) sono compatibili.

La licenza copre il codice, **non** i nomi "Almanacco" e "La Bottega Gambardella" né il logo.

Le fonti consultate per scrivere il codice sono elencate in [PROVENANCE.md](PROVENANCE.md).
