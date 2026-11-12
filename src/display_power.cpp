// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Flavio Gambardella

#include "display_power.h"

#include <Arduino.h>
#include <driver/rtc_io.h>

#include "board_pins.h"

namespace display_power {

namespace {
constexpr gpio_num_t kPwrGpio = static_cast<gpio_num_t>(PIN_EPD_PWR);
}

void releaseHold() {
  // Prima di andare in deep sleep "congeliamo" PWR a livello basso (vedi
  // holdOffDuringSleep). Finché il blocco è attivo, il pin ignora qualsiasi
  // scrittura: va rilasciato prima di poterlo riportare alto.
  rtc_gpio_hold_dis(kPwrGpio);
}

void on() {
  // Prima fissiamo il livello basso e solo dopo rendiamo il pin un'uscita:
  // così non c'è nemmeno un istante con PWR alto per caso.
  digitalWrite(PIN_EPD_PWR, LOW);
  pinMode(PIN_EPD_PWR, OUTPUT);
  digitalWrite(PIN_EPD_PWR, HIGH);

  // Il regolatore della driver board ha bisogno di qualche millisecondo per
  // stabilizzarsi prima che il controller del pannello possa ricevere comandi.
  delay(EPD_PWR_SETTLE_MS);
}

void off() {
  digitalWrite(PIN_EPD_PWR, LOW);
}

void holdOffDuringSleep() {
  // In deep sleep l'ESP32 spegne quasi tutto, compresa la logica che pilota
  // i pin: senza blocco, PWR resterebbe "flottante" (né alto né basso) e la
  // driver board potrebbe riaccendersi a metà. Il pin PWR (GPIO 33) è un pin RTC, cioè
  // gestito dalla parte del chip che resta attiva nel sonno: il suo blocco
  // (hold) mantiene il livello basso finché non viene rilasciato.
  digitalWrite(PIN_EPD_PWR, LOW);
  rtc_gpio_hold_en(kPwrGpio);
}

}  // namespace display_power
