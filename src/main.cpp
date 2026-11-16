// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Flavio Gambardella

// Almanacco firmware v0.1.2 "Hello world".
//
// Prova al banco di scheda, driver board, display e cablaggio: accende la
// driver board, scrive "Hello, world!", la spegne e mette l'ESP32 in deep
// sleep. Il ciclo riparte solo con il tasto RESET o una nuova alimentazione.
//
// Compilato con ALMANACCO_CLEAR_SCREEN (ambiente lolin_d32_clear) diventa la
// "modalità pulizia schermo": stessa sequenza, ma il display viene riportato
// tutto a bianco, da fare prima di lasciare il pannello inutilizzato a lungo.

#include <Arduino.h>
#include <SPI.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <esp_mac.h>
#include <esp_sleep.h>
#include <esp_system.h>

#include "board_pins.h"
#include "display_power.h"

namespace {

constexpr const char* kFirmwareBanner = "Almanacco firmware v0.1.2 hello-world";

// Display bianco/nero 800x480 con controller GDEW075T7 (Waveshare 7.5" V2).
// Il secondo parametro del template è l'altezza del buffer in RAM: con
// l'altezza intera l'immagine sta tutta in memoria (800x480 / 8 = 48 000
// byte) e viene inviata al pannello in un colpo solo.
GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT> display(
    GxEPD2_750_T7(PIN_EPD_CS, PIN_EPD_DC, PIN_EPD_RST, PIN_EPD_BUSY));

const char* resetReasonText(esp_reset_reason_t reason) {
  switch (reason) {
    case ESP_RST_POWERON:
      // Sull'ESP32 il tasto RESET agisce sul pin EN, che riavvia il chip
      // esattamente come un'accensione: per questo i due casi coincidono.
      return "accensione o tasto RESET";
    case ESP_RST_EXT:
      return "reset da pin esterno";
    case ESP_RST_SW:
      return "reset software";
    case ESP_RST_PANIC:
      return "reset dopo un errore del firmware (panic)";
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT:
      return "reset del watchdog";
    case ESP_RST_DEEPSLEEP:
      return "risveglio dal deep sleep";
    case ESP_RST_BROWNOUT:
      return "calo di tensione (brownout)";
    default:
      return "altra causa";
  }
}

void printBootInfo() {
  Serial.println(kFirmwareBanner);

  Serial.printf("Chip: %s, revisione %u\n", ESP.getChipModel(),
                static_cast<unsigned>(ESP.getChipRevision()));
  Serial.printf("Flash: %u MB\n",
                static_cast<unsigned>(ESP.getFlashChipSize() / (1024 * 1024)));

  uint8_t mac[6] = {};
  esp_efuse_mac_get_default(mac);
  Serial.printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1],
                mac[2], mac[3], mac[4], mac[5]);

  const esp_reset_reason_t reason = esp_reset_reason();
  Serial.printf("Causa dell'avvio: %s (codice %d)\n", resetReasonText(reason),
                static_cast<int>(reason));
}

void initDisplay() {
  // Il bus SPI va configurato con i nostri pin prima di inizializzare il
  // display: altrimenti la libreria userebbe i pin SPI predefiniti dell'ESP32
  // (18 e 23), che sulla nostra breadboard non sono collegati.
  SPI.begin(PIN_EPD_SCK, PIN_EPD_MISO, PIN_EPD_MOSI, PIN_EPD_CS);

  // Parametri di init:
  // - SERIAL_BAUD: attiva la diagnostica seriale di GxEPD2 (stampa i tempi
  //   di attesa del pin BUSY e gli eventuali "Busy Timeout!");
  // - true: primo avvio dopo un'accensione, quindi serve un refresh completo;
  // - EPD_RESET_PULSE_MS: durata dell'impulso su RST (vedi board_pins.h);
  // - false: RST pilotato normalmente (alto/basso), come vuole la driver HAT.
  display.init(SERIAL_BAUD, true, EPD_RESET_PULSE_MS, false, SPI,
               SPISettings(EPD_SPI_HZ, MSBFIRST, SPI_MODE0));
  display.setRotation(EPD_ROTATION);
}

#ifdef ALMANACCO_CLEAR_SCREEN

// Modalità pulizia schermo: tutto il pannello bianco, senza bordo né testo.
// Il refresh completo che segue pilota ogni pixel, quindi cancella anche
// eventuali tracce lasciate dalle immagini precedenti.
void drawBlank() {
  display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);
}

#else

// Scrive una riga di testo centrata orizzontalmente, con la linea di base
// (il "rigo" su cui poggiano le lettere) all'altezza baselineY.
void drawCentered(const char* text, int16_t baselineY) {
  int16_t x1 = 0;
  int16_t y1 = 0;
  uint16_t w = 0;
  uint16_t h = 0;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  const int16_t x = (display.width() - static_cast<int16_t>(w)) / 2 - x1;
  display.setCursor(x, baselineY);
  display.print(text);
}

void drawHelloWorld() {
  const int16_t w = display.width();
  const int16_t h = display.height();

  display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);

  // Bordo nero di 2 px a 4 px dai margini: se si vede intero su tutti e
  // quattro i lati, tutta l'area attiva del pannello viene pilotata.
  display.drawRect(4, 4, w - 8, h - 8, GxEPD_BLACK);
  display.drawRect(5, 5, w - 10, h - 10, GxEPD_BLACK);

  display.setTextColor(GxEPD_BLACK);

  display.setFont(&FreeSansBold24pt7b);
  drawCentered("Hello, world!", h / 2 - 20);

  display.setFont(&FreeSans12pt7b);
  drawCentered("Almanacco - firmware v0.1", h / 2 + 50);
  drawCentered("Display 800x480 B/N", h / 2 + 90);
  // Il numero del pin viene da board_pins.h, così resta giusto se cambia.
  char pwrLine[24];
  snprintf(pwrLine, sizeof(pwrLine), "PWR su GPIO %d", PIN_EPD_PWR);
  drawCentered(pwrLine, h / 2 + 130);
}

#endif  // ALMANACCO_CLEAR_SCREEN

// Il controller tiene BUSY basso mentre lavora. Se dopo un'operazione è
// ancora basso, la libreria ha smesso di aspettare per timeout.
bool displayStillBusy() {
  return digitalRead(PIN_EPD_BUSY) == LOW;
}

void printBusyChecklist() {
  Serial.println("ERRORE: il display non ha risposto (timeout del pin BUSY).");
  Serial.println("Controlla, a scheda scollegata:");
  Serial.printf("  - PWR alto? Il pin PWR della driver board e' su GPIO %d?\n", PIN_EPD_PWR);
  Serial.printf("  - BUSY collegato a GPIO %d?\n", PIN_EPD_BUSY);
  Serial.println("  - VCC sul pin 3V della D32 (non 5V/USB)?");
  Serial.println("  - Cavo a 9 poli inserito bene sulla driver board?");
  Serial.println("  - Cavo piatto del pannello inserito a fondo, blocchetto chiuso?");
}

}  // namespace

void setup() {
  // Se arriviamo da un deep sleep, PWR è ancora bloccato basso: lo
  // sblocchiamo subito, prima di qualsiasi altro uso del pin.
  display_power::releaseHold();

  // 1. Seriale.
  Serial.begin(SERIAL_BAUD);
  delay(SERIAL_WAIT_MS);
  Serial.println();
  printBootInfo();
#ifdef ALMANACCO_CLEAR_SCREEN
  Serial.println("Modalita' pulizia schermo: il display verra' riportato a bianco");
#endif

  // 2. Accensione della driver board: senza PWR alto il pannello resta
  //    spento anche con VCC collegato.
  display_power::on();
  Serial.println("PWR: driver board accesa");

  // 3-4. Inizializzazione e orientamento.
  initDisplay();

  // 5-6. Disegno con refresh completo, misurandone la durata. In modalità
  //      pulizia schermo il disegno è solo lo schermo tutto bianco.
  //      display(false) = refresh completo: trasferisce l'immagine e aspetta
  //      che il pannello abbia finito (il pin BUSY torna alto).
  //      display(false) termina spegnendo i generatori di tensione del
  //      pannello: se una qualsiasi di queste attese (accensione, refresh,
  //      spegnimento) va in timeout, il pin BUSY resta basso. Leggiamo BUSY
  //      una volta sola, qui, e usiamo lo stesso valore sia per questa riga
  //      sia per il messaggio di ERRORE più sotto: così non si contraddicono.
#ifdef ALMANACCO_CLEAR_SCREEN
  drawBlank();
#else
  drawHelloWorld();
#endif
  const uint32_t refreshStart = millis();
  display.display(false);
  const uint32_t refreshMs = millis() - refreshStart;
  const bool busyError = displayStillBusy();
  if (busyError) {
    Serial.printf("Refresh NON riuscito: timeout BUSY (durata %lu ms)\n",
                  static_cast<unsigned long>(refreshMs));
  } else {
    Serial.printf("Refresh completo: %lu ms\n", static_cast<unsigned long>(refreshMs));
  }

  // 7. Spegnimento del display, eseguito anche in caso di errore: il
  //    pannello non va mai lasciato alimentato senza un refresh in corso.
  //    hibernate() spegne i generatori di tensione del pannello e mette il
  //    controller in sonno profondo; poi end() mette in alta impedenza i pin
  //    di controllo, così non spingono corrente nella driver board spenta.
  display.hibernate();
  display.end();
  display_power::off();
  Serial.println("PWR: driver board spenta");

  if (busyError) {
    printBusyChecklist();
  }

  // 8. Deep sleep senza sorgenti di risveglio.
  display_power::holdOffDuringSleep();
  Serial.println("Deep sleep: premi RESET per ripetere");
  Serial.flush();  // aspetta che l'ultimo messaggio sia uscito dalla seriale
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  esp_deep_sleep_start();
}

void loop() {
  // Mai raggiunto: setup() termina sempre con il deep sleep.
}
