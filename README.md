# pin-checker
4-digit PIN checker built on Arduino Uno with custom LCD1602A driver and 4×4 membrane keypad driver - no external Hardware libraries.
=======
# PIN Checker - Arduino

> **Deutsche Version unten / German version below**

https://github.com/user-attachments/assets/your-video-id.mp4

4-digit PIN entry system built on an Arduino Uno with a custom HD44780 LCD driver and a custom 4x4 membrane keypad driver. No external hardware libraries; both drivers are implemented from scratch directly on top of Arduino's `digitalWrite` / `digitalRead`.

---

## Overview

The system runs a three-state machine: `ENTERING`, `SUCCESS`, and `LOCKED`.

| State | Description |
|---|---|
| `ENTERING` | Accepts 4 digits. Digits are briefly visible then masked with `*` after 500ms. `B` = backspace, `A` = confirm. |
| `SUCCESS` | Correct PIN entered. Displays confirmation and auto-resets to `ENTERING` after 5 seconds. |
| `LOCKED` | Triggered after 3 failed attempts. Requires manual reset by pressing `C`. |

Default PIN: `1234`, configurable in `Pin_checker/Pin_checker.ino`.

---

## Hardware

| Component | Details |
|---|---|
| Microcontroller | Arduino Uno |
| Display | LCD1602A (HD44780 controller) |
| Input | 4x4 membrane keypad |
| Contrast control | 10kOhm potentiometer |

---

## How It Works

**Keypad driver (`Membrane_switch.ino`)**

Row/column matrix scanning: one row is driven `LOW` at a time while all columns are held `HIGH` via `INPUT_PULLUP`. A key press shorts its row to its column, pulling that column `LOW`. A valid press is only registered on a rising edge (current scan pressed, previous scan not pressed) and only after `50ms` has elapsed since the last confirmed press on that key, filtering mechanical contact bounce without using `delay()`.

**LCD driver (`LCD.ino`)**

4-bit HD44780 initialisation sequence implemented from scratch using the LCD-1602A datasheet by SunFounder (see References). The initialisation is asymmetric: three 8-bit reset nibbles must be sent before the 4-bit mode switch command; this is a controller reset protocol specified in the HD44780 datasheet, not a quirk. `RW` is tied to `GND` (write-only); the busy flag is never polled; fixed datasheet-specified delays are used throughout initialisation instead.

**State machine (`Pin_checker.ino`)**

Implemented as an `enum` with three values: `ENTERING`, `SUCCESS`, and `LOCKED`. The main `loop()` branches on the current state and handles input and display logic for each one.

```
                  pinCheck() passes
  ┌─────────────────────────────────────► SUCCESS ──── auto-reset after 5s ────┐
  │                                                                              │
ENTERING ◄────────────────────────────────────────────────────────────────────── ┘
  │                                                                              │
  └──── tries reaches 0 ──────────────► LOCKED ──── 'C' pressed ───────────────┘
```

State transitions:
- `ENTERING -> SUCCESS`: `pinCheck()` returns true on `'A'` confirm. `tries` resets to 3, `buffer` cleared.
- `ENTERING -> LOCKED`: wrong PIN on the third attempt. `tries` resets to 3.
- `SUCCESS -> ENTERING`: automatic after 5 seconds, no input required.
- `LOCKED -> ENTERING`: manual reset by pressing `'C'`.

Key implementation details:
- **`freshState` flag for `ENTERING` **: gates LCD redraws. Set to `true` on every state entry, cleared after the first draw.
- **`digitRevealing` + `millis()` timestamp**: non-blocking digit masking. Each digit stays visible for 500ms then replaced with `'*'`. No `delay()` in the entry flow.
- **Fast input handling**: if a new digit arrives while `digitRevealing` is still active, the previous digit is force-masked immediately.
- **`count` and `buffer[4]`**: `count` tracks digits entered (0-4). `'A'` confirm only processed at `count == 4`. `'B'` backspace decrements `count` and clears the last LCD position. Both reset via `memset` on every state transition.

---

## Wiring

| Signal | Arduino Pin |
|---|---|
| LCD RS | 13 |
| LCD E | 12 |
| LCD DB4 | 11 |
| LCD DB5 | 10 |
| LCD DB6 | 9 |
| LCD DB7 | 8 |
| Keypad ROW1 | 7 |
| Keypad ROW2 | 6 |
| Keypad ROW3 | 5 |
| Keypad ROW4 | 4 |
| Keypad COL1 | 3 |
| Keypad COL2 | 2 |
| Keypad COL3 | A4 |
| Keypad COL4 | A5 |

Full schematic: [`docs/schematic.png`](docs/schematic.png)
Wiring diagram: [`docs/wiring_diagram.png`](docs/wiring_diagram.png)

---

## Project Structure

```
pin-checker/
├── Pin_checker/
│   ├── Pin_checker.ino       # State machine, application logic
│   ├── Membrane_switch.ino   # Keypad driver
│   └── LCD.ino               # HD44780 LCD driver
├── docs/
│   ├── schematic.png
│   └── wiring_diagram.png
├── media/
│   └── demo.mp4
├── README.md
└── LICENSE
```

---

## Possible Improvements

**Hardware output on success**
Currently `SUCCESS` only updates the LCD. A more complete implementation would trigger a physical output, such as driving a servo to an unlocked position or turning on an LED, making the state change unambiguous without reading the display.

**Admin state**
A fourth state `ADMIN`, accessed by pressing `'D'` from `ENTERING`, would accept a separate admin PIN with unlimited attempts. On correct entry it would allow manual override of `SUCCESS` and `LOCKED` back to `ENTERING`, useful for resetting a locked system without waiting for the user-facing flow.

---

## References

- [LCD-1602A Datasheet, SunFounder](https://mm.digikey.com/Volume0/opasdata/d220001/medias/docus/5773/CN0295D%20other%20related%20document.pdf)

---

## License

MIT, see [`LICENSE`](LICENSE).

---
---

# PIN Checker - Arduino (Deutsch)

> **English version above**

https://github.com/user-attachments/assets/your-video-id.mp4

4-stelliges PIN-Eingabesystem auf Basis eines Arduino Uno mit eigenem HD44780-LCD-Treiber und eigenem 4x4-Membrantastatur-Treiber. Keine externen Hardware-Bibliotheken; beide Treiber sind direkt auf `digitalWrite` / `digitalRead` aufgebaut.

---

## Übersicht

Das System verwendet eine Zustandsmaschine mit drei Zuständen: `ENTERING`, `SUCCESS` und `LOCKED`.

| Zustand | Beschreibung |
|---|---|
| `ENTERING` | Nimmt 4 Ziffern entgegen. Ziffern werden kurz angezeigt und nach 500ms durch `*` maskiert. `B` = Löschen, `A` = Bestätigen. |
| `SUCCESS` | Korrekter PIN eingegeben. Bestätigung wird angezeigt, automatischer Reset nach 5 Sekunden. |
| `LOCKED` | Wird nach 3 Fehlversuchen ausgelöst. Manueller Reset durch Drücken von `C` erforderlich. |

Standard-PIN: `1234`, konfigurierbar in `Pin_checker/Pin_checker.ino`.

---

## Hardware

| Komponente | Details |
|---|---|
| Mikrocontroller | Arduino Uno |
| Anzeige | LCD1602A (HD44780-Controller) |
| Eingabe | 4x4-Membrantastatur |
| Kontraststeuerung | 10kOhm-Potentiometer |

---

## Funktionsweise

**Tastatur-Treiber (`Membrane_switch.ino`)**

Zeilen-/Spalten-Matrix-Scanning: eine Zeile wird nacheinander auf `LOW` gezogen, während alle Spalten über `INPUT_PULLUP` auf `HIGH` gehalten werden. Ein Tastendruck verbindet Zeile und Spalte, wodurch die Spalte auf `LOW` gezogen wird. Ein gültiger Tastendruck wird nur bei einer steigenden Flanke registriert (aktuell gedrückt, vorher nicht gedrückt) und erst nach `50ms` seit dem letzten bestätigten Druck, was mechanisches Prellen ohne `delay()` filtert.

**LCD-Treiber (`LCD.ino`)**

4-Bit-HD44780-Initialisierungssequenz von Grund auf implementiert, basierend auf dem LCD-1602A-Datenblatt von SunFounder (siehe Referenzen). Die Initialisierung ist asymmetrisch: Drei 8-Bit-Reset-Nibbles müssen vor dem 4-Bit-Modus-Umschaltbefehl gesendet werden; dies ist ein Controller-Reset-Protokoll gemäß HD44780-Datenblatt. `RW` ist mit `GND` verbunden (nur Schreiben); das Busy-Flag wird nie abgefragt; stattdessen werden feste Verzögerungen aus dem Datenblatt verwendet.

**Zustandsmaschine (`Pin_checker.ino`)**

Als `enum` mit drei Werten implementiert: `ENTERING`, `SUCCESS` und `LOCKED`. Die `loop()`-Funktion verzweigt anhand des aktuellen Zustands und verwaltet Eingabe- und Anzeigelogik fur jeden Zustand.

```
                  pinCheck() erfolgreich
  ┌─────────────────────────────────────► SUCCESS ──── Auto-Reset nach 5s ──────┐
  │                                                                               │
ENTERING ◄─────────────────────────────────────────────────────────────────────── ┘
  │                                                                               │
  └──── tries erreicht 0 ─────────────► LOCKED ──── 'C' gedrückt ───────────────┘
```

Zustandsübergänge:
- `ENTERING -> SUCCESS`: `pinCheck()` gibt bei `'A'`-Bestätigung true zurück. `tries` auf 3 zurückgesetzt, `buffer` geleert.
- `ENTERING -> LOCKED`: Falscher PIN beim dritten Versuch. `tries` auf 3 zurückgesetzt.
- `SUCCESS -> ENTERING`: Automatisch nach 5 Sekunden, keine Eingabe erforderlich.
- `LOCKED -> ENTERING`: Manueller Reset durch Drücken von `'C'`.

Wichtige Implementierungsdetails:
- **`freshState`-Flag für `ENTERING`**: steuert LCD-Neuzeichnungen. Wird bei jedem Zustandseintritt auf `true` gesetzt, nach dem ersten Zeichnen gelöscht.
- **`digitRevealing` + `millis()`-Zeitstempel**: nicht-blockierende Ziffernmaskierung. Jede Ziffer bleibt 500ms sichtbar, dann wird `'*'` geschrieben. Kein `delay()` im Eingabefluss.
- **Schnelle Eingabe**: wenn eine neue Ziffer ankommt, wahrend `digitRevealing` noch aktiv ist, wird die vorherige Ziffer sofort maskiert.
- **`count` und `buffer[4]`**: `count` verfolgt eingegebene Ziffern (0-4). `'A'`-Bestätigung nur bei `count == 4`. `'B'` dekrementiert `count` und leert die letzte LCD-Position. Beide werden bei jedem Zustandswechsel via `memset` zurückgesetzt.

---

## Verdrahtung

| Signal | Arduino-Pin |
|---|---|
| LCD RS | 13 |
| LCD E | 12 |
| LCD DB4 | 11 |
| LCD DB5 | 10 |
| LCD DB6 | 9 |
| LCD DB7 | 8 |
| Tastatur ROW1 | 7 |
| Tastatur ROW2 | 6 |
| Tastatur ROW3 | 5 |
| Tastatur ROW4 | 4 |
| Tastatur COL1 | 3 |
| Tastatur COL2 | 2 |
| Tastatur COL3 | A4 |
| Tastatur COL4 | A5 |

Schaltplan: [`docs/schematic.png`](docs/schematic.png)
Verdrahtungsdiagramm: [`docs/wiring_diagram.png`](docs/wiring_diagram.png)

---

## Projektstruktur

```
pin-checker/
├── Pin_checker/
│   ├── Pin_checker.ino       # Zustandsmaschine, Anwendungslogik
│   ├── Membrane_switch.ino   # Tastatur-Treiber
│   └── LCD.ino               # HD44780-LCD-Treiber
├── docs/
│   ├── schematic.png
│   └── wiring_diagram.png
├── media/
│   └── demo.mp4
├── README.md
└── LICENSE
```

---

## Mögliche Erweiterungen

**Hardware-Ausgabe bei Erfolg**
Aktuell aktualisiert `SUCCESS` nur das LCD. Eine vollständigere Implementierung würde einen physischen Ausgang ansteuern, z.B. einen Servo in eine entsperrte Position fahren oder eine LED einschalten, wodurch der Zustandswechsel ohne Ablesen des Displays eindeutig erkennbar wird.

**Admin-Zustand**
Ein vierter Zustand `ADMIN`, über `'D'` aus `ENTERING` erreichbar, würde einen separaten Admin-PIN mit unbegrenzten Versuchen akzeptieren. Bei korrekter Eingabe ermöglicht er das manuelle Zurücksetzen von `SUCCESS` und `LOCKED` auf `ENTERING`, nützlich zum Entsperren ohne den normalen Benutzerfluss.

---

## Referenzen

- [LCD-1602A Datasheet, SunFounder](https://mm.digikey.com/Volume0/opasdata/d220001/medias/docus/5773/CN0295D%20other%20related%20document.pdf)

---

## Lizenz

MIT, siehe [`LICENSE`](LICENSE).
