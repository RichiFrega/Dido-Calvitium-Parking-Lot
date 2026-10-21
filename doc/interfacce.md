# Smart Parking: documento delle interfacce

**Versione 1.0 (bozza)** · 21 ottobre 2026 · da approvare tutti e tre entro domenica 25 ottobre

Questo documento è il contratto tra i quattro sottosistemi. Descrive ogni messaggio che passa da un sottosistema all'altro: canale, direzione, formato, significato e reazione attesa. Si cambia solo dopo averne parlato insieme, aggiornando versione e changelog.

| Versione | Data | Modifica |
|---|---|---|
| 1.0 | 21/10/2026 | Prima bozza da discutere |

---

## 1. Panoramica

```
                    ┌──────────────────────┐
                    │      Dashboard       │   browser
                    │ /  (operatore)       │
                    │ /public (tabellone)  │
                    └──────────┬───────────┘
                               │ HTTP / JSON
┌──────────────────┐  seriale  ┌┴─────────────────┐  MQTT   ┌──────────────────┐
│ Gate Controller  │◄─────────►│   Control Unit   │◄───────►│  Air Monitoring  │
│ Arduino UNO      │  USB/UART │ PC · Java+Vert.x │ broker  │  ESP32           │
└──────────────────┘           └──────────────────┘Mosquitto└──────────────────┘
```

### Chi decide cosa (fonte di verità)

Ogni dato ha un solo proprietario. Gli altri sottosistemi ne ricevono una copia e non la modificano per conto proprio.

| Dato | Proprietario | Note |
|---|---|---|
| Posti occupati e liberi, stato AVAILABLE / FULL | Control Unit | il Gate conta in proprio solo in modalità degradata (§3.6) |
| Livello dell'aria NORMAL / WARNING / ALARM | Control Unit | |
| Livello di ventilazione e frequenza di campionamento | Control Unit | l'ESP32 li applica e basta |
| Capienza e soglie | Control Unit | file di configurazione; la capienza si cambia anche dalla dashboard |
| Stato fisico della sbarra e transiti | Gate Controller | la CU lo conosce tramite i messaggi `GATE` ed `EV` |
| Misure di temperatura e CO | Air Monitoring | |

Convenzione: i nomi degli stati sono gli stessi, in maiuscolo, su tutti i canali (`AVAILABLE`, `FULL`, `NORMAL`, `WARNING`, `ALARM`).

---

## 2. Parametri condivisi

Sono valori iniziali, da tarare durante i test. Le unità di misura vanno sempre indicate.

| Parametro | Valore iniziale | Unità | Dove vive | Usato da |
|---|---|---|---|---|
| `CAPACITY` | 5 | posti | config CU, modificabile da dashboard | CU |
| `C1` (soglia WARNING) | 30 | ppm CO | config CU | CU |
| `C2` (soglia ALARM) | 100 | ppm CO | config CU | CU |
| `TMAX` | 40 | °C | config CU | CU |
| `DT` (durata oltre soglia per l'allarme) | 10 | s | config CU | CU |
| `F1` (periodo di campionamento in NORMAL) | 5000 | ms | config CU, inviato all'ESP32 | CU, ESP32 |
| `F2` (periodo di campionamento in WARNING e ALARM) | 1000 | ms | config CU, inviato all'ESP32 | CU, ESP32 |
| `T1` (timeout del transito sotto la sbarra) | 10 | s | firmware Gate | Gate |
| `TSLEEP` (inattività prima dello sleep) | 60 | s | firmware Gate | Gate |
| Periodo del `PING` seriale | 2 | s | CU | CU, Gate |
| Timeout della modalità degradata | 6 | s (3 `PING` persi) | firmware Gate | Gate |
| Periodo di polling della dashboard | 2 (stato), 10 (storico) | s | dashboard | dashboard |
| Campioni tenuti in memoria (`N`) | 100 | campioni | CU | CU |

Le frequenze si trasmettono sempre come **periodo in millisecondi** (`period_ms`), così non ci sono dubbi sull'unità.

---

## 3. Seriale: Gate Controller ↔ Control Unit

### 3.1 Parametri fisici e framing

- 115200 baud, 8N1.
- Testo ASCII, **un messaggio per riga**, terminato da `\n`. La CU accetta anche `\r\n`.
- Lunghezza massima 32 caratteri, terminatore incluso: l'Arduino ha 2 KB di SRAM.
- Formato `TIPO` oppure `TIPO:valore`, in maiuscolo, senza spazi.
- Le righe che iniziano con `#` sono **log di debug** dell'Arduino: la CU le stampa nel proprio log e non le interpreta. È l'unico modo ammesso per fare debug sulla seriale, così i `Serial.print` di prova non rompono il protocollo.
- Un messaggio sconosciuto o malformato si **ignora e si registra nel log**, senza bloccarsi. Vale per entrambi i lati.
- Lato Arduino si usa `MsgService` del lab 2.2, lato Java `SerialCommChannel` (JSSC).

### 3.2 Messaggi Arduino → Control Unit

| Messaggio | Quando | Reazione della CU |
|---|---|---|
| `HELLO:<e>:<u>` | all'avvio e ogni 1 s finché non arriva il primo messaggio dalla CU; `<e>` e `<u>` sono le entrate e le uscite contate in modalità degradata (`HELLO:0:0` dopo un avvio normale) | applica `occupati += e − u` (limitato tra 0 e la capienza), poi invia lo stato completo: `FREE:<n>` e `ALARM:ON` / `ALARM:OFF` |
| `EV:ENTER` | un veicolo ha completato l'ingresso (sequenza A → B) | `occupati += 1` (mai oltre la capienza, altrimenti warning nel log), evento nello storico, risponde `FREE:<n>` |
| `EV:EXIT` | un veicolo ha completato l'uscita (sequenza B → A) | `occupati −= 1` (mai sotto 0), evento nello storico, risponde `FREE:<n>` |
| `EV:ABORT` | un veicolo si è presentato ma non è transitato entro `T1` | solo evento nello storico, il conteggio non cambia |
| `BTN:OPEN` | l'operatore ha premuto il pulsante di apertura manuale | evento nello storico |
| `GATE:OPEN` / `GATE:CLOSED` | la sbarra ha finito di aprirsi o di chiudersi | aggiorna lo stato mostrato in dashboard |
| `SLEEP` | subito prima di entrare in sleep mode | sospende i `PING` e segna il Gate come `SLEEPING` (non come scollegato) |
| `WAKE` | al risveglio dallo sleep | riprende i `PING` e reinvia lo stato completo |
| `PONG` | risposta a ogni `PING` | aggiorna l'ora dell'ultimo contatto con il Gate |

### 3.3 Messaggi Control Unit → Arduino

| Messaggio | Quando | Reazione del Gate |
|---|---|---|
| `FREE:<n>` | in risposta a `HELLO` e `WAKE`, dopo ogni `EV:ENTER` / `EV:EXIT`, quando cambia la capienza | aggiorna l'LCD; con `n = 0` passa a FULL e nega l'ingresso; esce dalla modalità degradata |
| `ALARM:ON` / `ALARM:OFF` | quando il livello dell'aria entra o esce da ALARM, e in risposta a `HELLO` e `WAKE` | ON: buzzer, LCD "EVACUARE", ingresso negato, uscita consentita. OFF: torna al funzionamento normale |
| `CMD:OPEN` | comando di apertura dalla dashboard | apre la sbarra passando dalla stessa FSM del pulsante manuale |
| `PING` | ogni 2 s mentre il Gate è sveglio | risponde `PONG` |

Il Gate non conosce la capienza: gli basta sapere quanti posti sono liberi. Lo stato FULL coincide con `FREE:0`.

I comandi trasportano **valori assoluti**, mai variazioni ("liberi = 2", non "un posto in meno"). Così un messaggio ripetuto non fa danni.

### 3.4 Sequenza di avvio

1. La CU apre la porta seriale. **L'Arduino UNO si resetta** quando la porta viene aperta, per via del segnale DTR.
2. La CU non invia nulla finché non riceve `HELLO`.
3. Dopo il `setup()` l'Arduino invia `HELLO:0:0` ogni secondo.
4. La CU risponde con `FREE:<n>` e `ALARM:OFF` (oppure `ALARM:ON`) e comincia a inviare `PING`.

### 3.5 Sleep mode

- Prima di dormire il Gate invia `SLEEP`. Al risveglio, dal PIR o da un messaggio della CU, invia `WAKE`.
- La modalità di sleep resta da decidere (vedi §8, punto 1). Se è una modalità in cui la USART resta attiva (IDLE), un messaggio della CU come `ALARM:ON` sveglia l'Arduino. Se è POWER-DOWN, i messaggi inviati durante lo sleep si perdono e la CU reinvia lo stato completo al `WAKE`.
- Con un allarme attivo il Gate non va mai in sleep.

### 3.6 Modalità degradata

- Il Gate entra in modalità degradata se, da sveglio, non riceve messaggi dalla CU per 6 s (tre `PING` persi).
- In modalità degradata:
  - continua a gestire la sbarra, contando i posti a partire dall'ultimo `FREE` ricevuto;
  - mostra "OFFLINE" sull'LCD;
  - tiene due contatori, entrate `e` e uscite `u`, dall'inizio della disconnessione;
  - invia `HELLO:<e>:<u>` ogni secondo.
- Quando la CU risponde, il Gate azzera i contatori ed esce dalla modalità degradata appena riceve `FREE:<n>`.

### 3.7 Esempio di sessione

```
A→C  HELLO:0:0
C→A  FREE:5
C→A  ALARM:OFF
C→A  PING
A→C  PONG
A→C  # sensore A: oggetto a 12 cm
A→C  GATE:OPEN
A→C  EV:ENTER
C→A  FREE:4
A→C  GATE:CLOSED
```

---

## 4. MQTT: Air Monitoring ↔ Control Unit

### 4.1 Broker e connessione

- Broker Eclipse Mosquitto sul PC della Control Unit, porta 1883, senza autenticazione per la demo in rete locale. Nel report va discusso come renderlo sicuro: TLS sulla porta 8883, utente e password.
- Client id: `esp32-air` per l'ESP32, `control-unit` per la CU.
- Keep alive: 15 s (default di PubSubClient). Se l'ESP32 cade, il broker se ne accorge entro circa 1,5 volte il keep alive (~22 s) e pubblica il Last Will (§4.2).
- SSID e password del Wi-Fi stanno in un file `secrets.h` escluso dalla repository (`.gitignore`).
- Se per un test si usa il broker pubblico dei lab (`broker.mqtt-dashboard.com`), tutti i topic vanno preceduti da una radice unica del gruppo, ad esempio `smartpark-<sigla>/parking/...`, perché quel broker è condiviso con chiunque.

### 4.2 Topic

| Topic | Pubblica | Si iscrive | QoS | Retained | Payload |
|---|---|---|---|---|---|
| `parking/air/data` | ESP32 | CU | 0 | no | `{"temp": 23.4, "co": 12.0, "seq": 812}` |
| `parking/air/config` | CU | ESP32 | 1 | sì | `{"period_ms": 5000}` |
| `parking/air/ventilation` | CU | ESP32 | 1 | sì | `{"level": 60}` |
| `parking/air/status` | ESP32 (e il broker, come Last Will) | CU | 1 | sì | `online` oppure `offline` (testo semplice) |

**`parking/air/data`**
- `temp`: gradi Celsius, una cifra decimale. `co`: ppm di monossido di carbonio già calibrati, una cifra decimale. `seq`: contatore che parte da 0 all'avvio dell'ESP32 e cresce di 1 a ogni campione; serve alla CU per accorgersi di campioni persi e di riavvii.
- Viene pubblicato ogni `period_ms`.
- QoS 0: un campione perso viene sostituito dal successivo, quindi la conferma di consegna non serve.

**`parking/air/config`**
- `period_ms` è compreso tra 500 e 60000. Un valore fuori intervallo viene ignorato e registrato nel log.
- La CU lo pubblica all'avvio e a ogni cambio di livello dell'aria.

**`parking/air/ventilation`**
- `level` è compreso tra 0 e 100. L'ESP32 lo applica come duty cycle PWM (periferica `ledc`), con `duty = level × 255 / 100`.
- La CU lo pubblica a ogni cambio.

**`parking/air/status`**
- Alla connessione l'ESP32 registra come Last Will `offline` (retained, QoS 1), poi pubblica `online` (retained).

**Perché retained e QoS 1 per i comandi**
- Con *retained*, un ESP32 che si riconnette riceve subito frequenza e ventilazione correnti, senza aspettare il prossimo cambio.
- Con QoS 1 i comandi arrivano almeno una volta. Gli eventuali duplicati sono innocui, perché il payload è un valore assoluto.

### 4.3 Comportamento in caso di problemi

- **ESP32 all'avvio, senza configurazione ricevuta:** usa `period_ms = 5000` e ventilazione 0.
- **ESP32 senza rete o senza broker:**
  - LED rosso acceso, LED verde spento;
  - un nuovo tentativo di connessione ogni 5 s;
  - i campioni misurati durante la disconnessione si scartano, senza buffer (decisione della v1).
- **CU senza dati:** se non arrivano campioni da più di 3 volte `period_ms`, la dashboard mostra "dati non aggiornati". Con `status = offline` mostra "ESP32 non collegato". Per il livello dell'aria vedi §8, punto 3.

---

## 5. HTTP: Dashboard ↔ Control Unit

### 5.1 Convenzioni

- URL di base: `http://localhost:8080`.
- La CU serve anche le pagine della dashboard da una cartella `webroot/` (`StaticHandler` di Vert.x): `/` è la vista operatore, `/public` la vista pubblica. Pagine e API hanno la stessa origine, quindi non ci sono problemi di CORS.
- Corpi delle richieste e delle risposte in JSON (`Content-Type: application/json`).
- Gli istanti sono in millisecondi dal 1/1/1970 (UTC).
- Gli errori rispondono con `{"error": "<messaggio leggibile>"}` e il codice di stato appropriato.

### 5.2 Endpoint

| Metodo e percorso | Corpo | Risposta | Errori |
|---|---|---|---|
| `GET /api/status` | | stato completo (sotto) | |
| `GET /api/history?n=100` | | storico e statistiche (sotto) | 400 se `n` non è tra 1 e 1000 |
| `GET /api/public` | | `{"free": 2, "capacity": 5, "parking": "AVAILABLE", "alarm": false}` | |
| `POST /api/alarm/reset` | nessuno | `{"level": "NORMAL"}` | 409 se il livello non è ALARM |
| `POST /api/gate/open` | nessuno | `{"sent": true}` | 503 se il Gate è `DISCONNECTED` |
| `POST /api/ventilation` | `{"mode": "MANUAL", "level": 70}` oppure `{"mode": "AUTO"}` | `{"mode": "MANUAL", "level": 70}` | 400 se `level` non è tra 0 e 100 |
| `PUT /api/config/capacity` | `{"capacity": 6}` | `{"capacity": 6, "free": 3}` | 400 se minore dei posti occupati, minore di 1 o maggiore di 99 |

**`GET /api/status`**

```json
{
  "capacity": 5,
  "occupied": 3,
  "free": 2,
  "parking": "AVAILABLE",
  "air": {
    "level": "NORMAL",
    "temp": 22.5,
    "co": 8.1,
    "updated": 1763371200000,
    "period_ms": 5000
  },
  "ventilation": { "mode": "AUTO", "level": 0 },
  "gate": { "link": "CONNECTED", "barrier": "CLOSED" },
  "esp":  { "link": "ONLINE" }
}
```

Valori possibili:
- `parking`: `AVAILABLE` oppure `FULL`.
- `air.level`: `NORMAL`, `WARNING` oppure `ALARM`.
- `ventilation.mode`: `AUTO` oppure `MANUAL`.
- `gate.link`: `CONNECTED`, `SLEEPING` oppure `DISCONNECTED`.
- `gate.barrier`: `OPEN`, `CLOSED` oppure `UNKNOWN`.
- `esp.link`: `ONLINE` oppure `OFFLINE`.

**`GET /api/history?n=100`**

Ordinato dal più vecchio al più recente, così la dashboard lo passa direttamente al grafico.

```json
{
  "air": [
    { "ts": 1763371200000, "temp": 22.5, "co": 8.1 }
  ],
  "events": [
    { "ts": 1763371195000, "type": "ENTER" }
  ],
  "stats": {
    "since": 1763337600000,
    "temp": { "min": 19.8, "max": 24.1, "avg": 22.0 },
    "co":   { "min": 2.0,  "max": 35.5, "avg": 9.3 }
  }
}
```

- `events[].type`: `ENTER`, `EXIT`, `ABORT`, `MANUAL_OPEN`, `REMOTE_OPEN`, `ALARM_ON`, `ALARM_OFF`, `ALARM_RESET`.
- `stats` copre il periodo dalla mezzanotte di oggi (`since`).

### 5.3 Regole di comportamento

- **In ALARM** la ventilazione è sempre al 100%, anche se la modalità è MANUAL. La scelta MANUAL torna a valere dopo il reset.
- **Dopo il reset dell'allarme** il livello torna a NORMAL. Se i valori sono ancora sopra le soglie, la FSM li rivaluta al campione successivo e, dopo `DT`, torna in ALARM.
- **La vista pubblica** usa solo `GET /api/public`, che non espone i dati riservati agli operatori.

---

## 6. Scenari end-to-end

### 6.1 Un'auto entra

1. Il sensore A rileva il veicolo. La FSM del Gate apre la sbarra e invia `GATE:OPEN`.
2. Il veicolo passa da A a B. Il Gate invia `EV:ENTER`; quando B si libera chiude la sbarra e invia `GATE:CLOSED`.
3. La CU porta i posti occupati da 3 a 4, registra l'evento e risponde `FREE:1`.
4. Il Gate mostra "Posti liberi: 1".
5. Al polling successivo la dashboard mostra 4 posti occupati su 5.

### 6.2 Parcheggio pieno

1. La CU invia `FREE:0`. Il Gate passa a FULL.
2. Il sensore A rileva un veicolo: la sbarra resta chiusa e l'LCD mostra "PARCHEGGIO PIENO".
3. Un veicolo esce (B → A): `EV:EXIT`, la CU risponde `FREE:1` e il Gate torna AVAILABLE.

### 6.3 Allarme CO

1. L'ESP32 pubblica `{"temp": 24.0, "co": 45.0, ...}`. La CU passa a WARNING: pubblica `config {"period_ms": 1000}` e una ventilazione proporzionale.
2. Il CO supera `C2` e ci resta per `DT` secondi. La CU passa ad ALARM:
   - pubblica `ventilation {"level": 100}`;
   - invia `ALARM:ON` al Gate;
   - registra l'evento `ALARM_ON`.
3. Il Gate attiva il buzzer, mostra "EVACUARE" e nega l'ingresso. L'uscita resta consentita.
4. La dashboard mostra ALARM. L'operatore preme "Reset allarme" (`POST /api/alarm/reset`).
5. La CU torna a NORMAL:
   - invia `ALARM:OFF`;
   - ripristina la ventilazione in AUTO e il periodo `F1`;
   - registra `ALARM_RESET`.

### 6.4 Cavo seriale scollegato

1. Il Gate non riceve `PING` per 6 s ed entra in modalità degradata, con "OFFLINE" sull'LCD.
2. La CU non riceve `PONG` e la dashboard mostra "Gate non collegato".
3. Entrano due auto: il Gate le conta in proprio (`e = 2`).
4. Il cavo viene ricollegato. Il Gate invia `HELLO:2:0`, la CU aggiunge 2 posti occupati e risponde `FREE:<n>`.

---

## 7. Come testare ogni lato da solo

**Seriale.** Nel Serial Monitor dell'Arduino IDE, a 115200 baud con terminatore "Newline", si scrivono a mano i messaggi della CU: `FREE:3`, `ALARM:ON`, `PING`.

**MQTT.**
```bash
# vedere tutto quello che passa
mosquitto_sub -h localhost -t 'parking/#' -v

# simulare l'ESP32 (serve a chi sviluppa la CU)
mosquitto_pub -h localhost -t parking/air/data -m '{"temp":23.4,"co":12.0,"seq":1}'

# simulare la CU (serve a chi sviluppa l'ESP32)
mosquitto_pub -h localhost -t parking/air/config -r -q 1 -m '{"period_ms":1000}'
mosquitto_pub -h localhost -t parking/air/ventilation -r -q 1 -m '{"level":60}'
```

**HTTP.**
```bash
curl http://localhost:8080/api/status
curl -X POST http://localhost:8080/api/alarm/reset
curl -X POST -H 'Content-Type: application/json' \
     -d '{"mode":"MANUAL","level":70}' http://localhost:8080/api/ventilation
```

---

## 8. Punti aperti (da decidere entro domenica 25 ottobre)

1. **Modalità di sleep del Gate.**
   - *IDLE* lascia attiva la USART, quindi un messaggio della CU (ad esempio `ALARM:ON`) sveglia l'Arduino. Però vanno spenti i timer non necessari: il Timer0 di `millis()` lo sveglierebbe ogni millisecondo.
   - *POWER-DOWN* consuma molto meno, ma solo il PIR (interrupt esterno) può svegliarlo, e un allarme che scatta durante lo sleep si sente solo all'arrivo della prossima auto.
   - Da provare con gli esempi `step8_deepsleep` e `step9_lightsleep_with_timer` dei lab.
2. **Dove si calibra il CO.** Proposta: sull'ESP32, che conosce il sensore e la sua curva; la CU riceve già i ppm. In alternativa l'ESP32 manda il valore grezzo dell'ADC e la CU lo converte.
3. **Livello dell'aria con l'ESP32 offline.** Proposta: la CU mantiene l'ultimo livello noto e la dashboard mostra un avviso. Va deciso se, oltre una certa durata, conviene una scelta prudente, come ventilazione al massimo.
4. **Persistenza del conteggio.** Proposta: la CU salva capienza e posti occupati in un file JSON a ogni cambio, così un suo riavvio non azzera il parcheggio.
5. **Broker locale o pubblico per la demo finale.** Proposta: locale, con il broker pubblico solo come piano di riserva e con la radice unica dei topic (§4.1).
