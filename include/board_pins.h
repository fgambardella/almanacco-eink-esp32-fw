// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Flavio Gambardella

// Pin, rotazione e costanti hardware di Almanacco.
//
// Tutto ciò che dipende dal cablaggio o dal pannello sta qui: se cambi un
// collegamento sulla breadboard, cambi una riga sola in questo file.

#pragma once

#include <stdint.h>

// --- Collegamenti tra LOLIN D32 e Waveshare e-Paper Driver HAT -------------
//
// Il display riceve soltanto, quindi MISO non è collegato. Evitiamo GPIO 12:
// è un pin di "strapping" che l'ESP32 legge all'accensione per decidere la
// tensione della flash, e un carico esterno lì può impedire l'avvio.

constexpr int8_t PIN_EPD_SCK = 13;   // CLK della driver board (clock SPI)
constexpr int8_t PIN_EPD_MOSI = 14;  // DIN della driver board (dati SPI)
constexpr int8_t PIN_EPD_MISO = -1;  // non usato: -1 dice alla libreria SPI di non riservare alcun pin
constexpr int8_t PIN_EPD_CS = 15;    // chip select, attivo basso
constexpr int8_t PIN_EPD_DC = 27;    // basso = comando, alto = dati
constexpr int8_t PIN_EPD_RST = 26;   // reset del controller, attivo basso
constexpr int8_t PIN_EPD_BUSY = 25;  // il controller lo tiene basso mentre lavora
constexpr int8_t PIN_EPD_PWR = 33;   // accensione della driver board, attivo alto

// --- Alimentazione della driver board --------------------------------------

// Dopo aver portato PWR alto, il regolatore della driver board impiega un
// attimo a portare stabile la tensione del pannello. Se parlassimo subito al
// controller, potrebbe essere ancora spento e non rispondere al reset.
// 50 ms è un margine ampio rispetto ai tempi tipici di un regolatore: se il
// display non risponde, prova ad aumentarlo (per esempio a 200).
constexpr uint32_t EPD_PWR_SETTLE_MS = 50;

// --- Inizializzazione del controller ---------------------------------------

// Durata dell'impulso di reset (pin RST basso), in millisecondi.
// 2 ms è il valore che GxEPD2 consiglia per le driver board Waveshare più
// recenti, che hanno un circuito di reset "intelligente": con un impulso
// lungo quel circuito può spegnere il pannello invece di resettarlo.
// Se il display non risponde (timeout di BUSY), prova 10 ms, cioè il valore
// predefinito di GxEPD2 per i pannelli senza quel circuito.
constexpr uint16_t EPD_RESET_PULSE_MS = 2;

// Velocità del bus SPI verso il display. 4 MHz è il valore usato negli esempi
// di GxEPD2 e tollera bene i cavetti Dupont lunghi della breadboard.
constexpr uint32_t EPD_SPI_HZ = 4000000;

// --- Orientamento ----------------------------------------------------------

// Il pannello è nativamente orizzontale (800x480). Le rotazioni 1 e 3 lo
// rendono verticale (480x800), una capovolta rispetto all'altra.
// Al banco: se il testo appare capovolto, cambia 1 in 3 (o viceversa).
constexpr uint8_t EPD_ROTATION = 1;

// --- Seriale ---------------------------------------------------------------

constexpr uint32_t SERIAL_BAUD = 115200;

// Attesa massima del monitor seriale all'avvio. Il convertitore USB-seriale
// della D32 non dice all'ESP32 se dall'altra parte c'è un monitor aperto,
// quindi aspettiamo comunque questo tempo per dare al monitor modo di
// collegarsi dopo un reset.
constexpr uint32_t SERIAL_WAIT_MS = 2000;
