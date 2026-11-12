# Almanacco firmware: specifica v0.1 "Hello world"

*Versione 0.1 · 12 novembre 2026 · Autore della specifica: Flavio Gambardella con Claude · Licenza del firmware: GPL-3.0-or-later*

Questa specifica descrive **solo il comportamento** del primo firmware di Almanacco. Va implementata in una sessione di Claude Code nuova, seguendo le regole del §0.

---

## 0. Regole clean room per chi implementa

1. **Non cercare, aprire, clonare né citare** il progetto `kristiantm/eink-family-calendar-esp32`, il suo tutorial su Instructables o altri progetti di "e-ink family calendar". Se un risultato di ricerca porta lì, non aprirlo.
2. **Fonti ammesse**: questa specifica; documentazione ed esempi della libreria GxEPD2 (GPL-3.0, compatibile con la licenza del progetto); documentazione di Adafruit GFX; wiki Waveshare di "7.5inch e-Paper HAT" ed "e-Paper Driver HAT"; documentazione di Espressif, Arduino-ESP32 e PlatformIO; pagina prodotto e schema della LOLIN D32.
3. Ogni fonte consultata va annotata in `PROVENANCE.md` (§6).
4. Se manca un dettaglio, **chiedere a Flavio**, non dedurlo da progetti simili.
5. Flavio è un programmatore, ma alle prime armi con l'elettronica: commenti nel codice **in italiano**, spiegando il perché di ogni passaggio legato all'hardware (pin PWR, reset, BUSY, hibernate).

## 1. Obiettivo

Verificare al banco, sulla breadboard, che **scheda, driver board, display e cablaggio funzionino**: il firmware accende la driver board, scrive "Hello, world!" sul display, la spegne e mette l'ESP32 in deep sleep. L'immagine deve restare visibile anche senza alimentazione.

**Fuori dallo scopo** di questa versione: WiFi, lettura della batteria, NTP, calendario, meteo, risveglio a tempo, lettere accentate (UTF-8), refresh parziale, livelli di grigio. Ognuno avrà la sua specifica.

## 2. Hardware

- **Scheda**: WEMOS LOLIN D32 (ESP32-WROOM-32, 4 MB flash), alimentata **solo da USB**.
- **Batteria LiPo: NON collegata** in questa fase (la polarità del connettore JST va verificata prima, a parte).
- **Display**: Waveshare 7.5" e-Paper V2, 800×480, bianco/nero (controller del pannello: GDEW075T7), con **e-Paper Driver HAT** dotata di pin **PWR** e cavo a 9 poli.
- **Collegamenti** con jumper Dupont sulla breadboard:

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
| PWR | Accensione della driver board (**attivo alto**) | GPIO 33 |

Note:
- **MISO non si usa**: il display riceve soltanto. Non usare GPIO 12 (è un pin di strapping dell'ESP32 e influisce sull'avvio).
- **PWR** va portato alto prima di inizializzare il display: senza, la driver board non è alimentata anche se VCC è collegato.
- VCC va al pin **3V**, mai a 5V/USB.
- Tutti i pin vanno definiti in **un solo file** (`include/board_pins.h`), così un cambio di cablaggio tocca una riga sola.

## 3. Toolchain

- **PlatformIO** (VS Code o riga di comando), `framework = arduino`, `board = lolin_d32`, `monitor_speed = 115200`.
- Piattaforma ESP32: scegliere la versione stabile più recente adatta a GxEPD2 e **fissarla con il numero esatto** in `platformio.ini` (niente versioni flottanti). Annotare la scelta e il motivo in `README.md`.
- Librerie: **GxEPD2** e **Adafruit GFX Library**, entrambe con **versione esatta**.
- Se il computer non vede la porta seriale della D32, può servire il driver del convertitore USB-seriale della scheda (da verificare sulla scheda quale chip monta).

## 4. Comportamento

All'accensione o al reset il firmware esegue questa sequenza, **una volta sola**:

1. **Seriale**: apre la seriale a 115200 baud, attende al massimo 2 secondi che il monitor si colleghi, poi stampa:
   - `Almanacco firmware v0.1 hello-world`;
   - modello e revisione del chip, dimensione della flash, MAC;
   - causa dell'avvio (accensione, reset, risveglio).
2. **Accensione della driver board**: GPIO 33 come uscita, livello **alto**, poi una breve attesa di stabilizzazione (valore in una costante, con il motivo in commento). Stampa `PWR: driver board accesa`.
3. **Inizializzazione**:
   - SPI con i pin del §2 (SCK 13, MOSI 14, MISO non usato, CS 15);
   - display con la classe bianco/nero di GxEPD2 e il driver per il pannello GDEW075T7 800×480 (CS 15, DC 27, RST 26, BUSY 25);
   - diagnostica seriale della libreria **attiva**;
   - durata dell'impulso di reset in una costante: partire dal valore breve consigliato da GxEPD2 per le driver board Waveshare e documentare in commento come cambiarlo se il display non risponde.
4. **Orientamento**: **verticale** (480×800). La rotazione è una costante in `board_pins.h` o in un file di configurazione; Flavio verifica sul banco quale valore dà il testo dritto con il connettore della driver board nel verso scelto per la cornice.
5. **Disegno**, con un **refresh completo**:
   - sfondo bianco;
   - bordo rettangolare nero di 2 px a 4 px dai margini, per verificare che tutta l'area attiva sia pilotata, angoli compresi;
   - titolo grande `Hello, world!`;
   - sotto, in un font più piccolo: `Almanacco - firmware v0.1`, `Display 800x480 B/N`, `PWR su GPIO 33`;
   - solo caratteri ASCII.
6. **Tempi**: misura e stampa in seriale la durata del refresh in millisecondi. Riferimento Waveshare: refresh completo di circa 4 s.
7. **Spegnimento del display**: mette il pannello in ibernazione con la funzione della libreria, poi porta **GPIO 33 basso**. Stampa `PWR: driver board spenta`. Il pannello non va mai lasciato alimentato senza refresh in corso: Waveshare avverte che può danneggiarsi.
8. **Deep sleep senza risveglio**:
   - prima di dormire **blocca GPIO 33 a livello basso** con il meccanismo di hold dei pin RTC dell'ESP32, così il pin non resta flottante durante il sonno;
   - stampa `Deep sleep: premi RESET per ripetere`;
   - entra in deep sleep senza sorgenti di risveglio. Solo il tasto RESET o una nuova alimentazione fanno ripartire il ciclo.
   - Al riavvio, il firmware deve **rilasciare l'hold** di GPIO 33 prima di riusarlo al passo 2.

### Gestione degli errori

- Se il display resta occupato oltre il timeout della libreria, stampare in seriale un messaggio chiaro con una lista di controllo: PWR alto?, BUSY collegato a GPIO 25?, VCC a 3V?, cavo a 9 poli inserito bene sulla driver board?
- In caso di errore il firmware esegue comunque i passi 7 e 8 (driver board spenta, deep sleep), per non lasciare il pannello alimentato.

## 5. Struttura del codice

```
almanacco-firmware/
├── LICENSE                 testo integrale GPL-3.0, invariato, da gnu.org
├── README.md               in italiano (§6)
├── PROVENANCE.md           registro clean room (§6)
├── platformio.ini
├── .gitignore              almeno .pio/ e file locali dell'editor
├── include/
│   └── board_pins.h        pin, rotazione e costanti hardware
└── src/
    ├── main.cpp            sequenza del §4
    ├── display_power.h/.cpp   accensione, spegnimento e hold del pin PWR
    └── (altri moduli solo se servono davvero)
```

- Ogni file sorgente comincia con:
  ```
  // SPDX-License-Identifier: GPL-3.0-or-later
  // Copyright (C) 2026 Flavio Gambardella
  ```
- La gestione del pin PWR sta in un modulo a sé, perché verrà riusata da tutte le versioni successive.
- Niente codice morto, niente funzioni fuori scopo.

## 6. Repository e licenza

- Repo **pubblico** su GitHub; nome suggerito `almanacco-firmware`.
- **Licenza: GPL-3.0-or-later.**
  - Il file `LICENSE` contiene il testo integrale della GNU GPL versione 3, senza modifiche. È lo stesso file sia per "GPL-3.0-only" sia per "GPL-3.0-or-later": la variante "or later" si dichiara nelle intestazioni SPDX dei file e nel README.
  - Su GitHub, alla creazione del repo, si può scegliere il modello "GNU General Public License v3.0": fornisce lo stesso testo.
  - Motivo: GxEPD2 è distribuita sotto GPL-3.0, quindi il firmware che la usa va distribuito sotto GPL-3.0; Adafruit GFX (BSD) e il core Arduino ESP32 (LGPL-2.1 e Apache-2.0) sono compatibili.
- **README.md** in italiano con:
  - cos'è Almanacco (una riga) e cosa fa questa versione;
  - hardware e tabella dei collegamenti;
  - come compilare, caricare e aprire il monitor seriale;
  - procedura di prova (§8);
  - licenza: GPL-3.0-or-later;
  - nota: la licenza copre il codice, non i nomi "Almanacco" e "La Bottega Gambardella" né il logo.
- **PROVENANCE.md**: dichiara che il codice è scritto senza consultare il codice di altri progetti di calendari e-ink, ed elenca le fonti usate (documentazione e esempi, con link e licenza). Si aggiorna a ogni versione.
- Primo commit: `LICENSE`, `README.md`, `PROVENANCE.md`, `.gitignore`. Secondo commit: il codice.

## 7. Criteri di accettazione

**Verificabili da chi implementa:**
- `pio run` compila senza errori e senza warning nel codice del progetto.
- Pin, rotazione e costanti stanno tutti in `include/board_pins.h`.
- Intestazioni SPDX presenti in ogni file sorgente; `LICENSE`, `README.md` e `PROVENANCE.md` presenti.

**Verificabili da Flavio al banco:**
1. Il monitor seriale mostra, in ordine: intestazione, dati del chip, causa dell'avvio, PWR accesa, durata del refresh, PWR spenta, deep sleep.
2. Il display mostra il testo **dritto** in verticale e il **bordo completo** su tutti e quattro i lati.
3. La durata del refresh è dell'ordine di qualche secondo (riferimento: circa 4 s).
4. Staccando il cavo USB, l'immagine **resta** sul display.
5. Premendo RESET (o ricollegando l'USB) il ciclo si ripete e il display si ridisegna.

## 8. Procedura di prova passo passo (per Flavio)

1. **A scheda scollegata**, controllare ogni collegamento con la tabella del §2, uno per uno. Soprattutto: VCC sul pin 3V, GND su GND, PWR su GPIO 33.
2. Controllare che il cavo piatto del pannello sia inserito a fondo nel connettore della driver board, con il blocchetto chiuso. Il vetro del pannello è sottile: appoggiarlo su una superficie piana, senza piegare il cavo.
3. Batteria **scollegata**.
4. Collegare la D32 al computer con un cavo USB che trasporta dati (alcuni cavi caricano soltanto).
5. `pio run -t upload`, poi `pio device monitor`.
6. Premere RESET e confrontare la seriale con il criterio 1 del §7.
7. Guardare il display (criteri 2 e 3), poi staccare l'USB (criterio 4) e ricollegarlo (criterio 5).

**Se qualcosa non va:**

| Sintomo | Cause probabili |
| --- | --- |
| Il computer non vede la porta | Cavo solo di ricarica; driver USB-seriale mancante |
| Seriale muta | Velocità del monitor diversa da 115200; avvio prima che il monitor sia aperto (premere RESET) |
| Timeout del display (BUSY) | PWR non collegato o non alto; BUSY non su GPIO 25; cavo piatto non inserito bene |
| Immagine confusa o solo in parte | Driver sbagliato nel codice (pannello V2 = GDEW075T7); DIN/CLK invertiti |
| Testo capovolto | Cambiare la costante di rotazione (altro valore verticale) |
| Nulla sul display, nessun errore | VCC non sul pin 3V; GND mancante; PWR rimasto basso |

## 9. Specifiche successive (indicative)

v0.2 lettura della batteria sul pin 35 · v0.3 WiFi e pagina di configurazione · v0.4 NTP e risveglio orario · v0.5 lettura degli eventi · v0.6 meteo · v0.7 schermata della giornata in italiano UTF-8, "Aggiornato alle HH:MM", "Ricaricami".
