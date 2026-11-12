// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Flavio Gambardella

// Gestione del pin PWR della Waveshare e-Paper Driver HAT.
//
// La driver board ha un interruttore di alimentazione comandato dal pin PWR:
// anche con VCC collegato, finché PWR è basso il pannello resta spento.
// Così l'ESP32 può alimentare il display solo durante un aggiornamento.

#pragma once

namespace display_power {

// Da chiamare per primo all'avvio: sblocca il pin PWR se era rimasto
// "congelato" basso da un deep sleep precedente.
void releaseHold();

// Accende la driver board e attende che l'alimentazione sia stabile.
void on();

// Spegne la driver board (PWR basso).
void off();

// Blocca PWR a livello basso per tutta la durata del deep sleep.
void holdOffDuringSleep();

}  // namespace display_power
