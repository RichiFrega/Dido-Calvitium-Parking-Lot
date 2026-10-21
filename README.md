# Dido-Calvitium-Parking-Lot

Smart Parking: elaborato di Embedded Systems and IoT (Ingegneria e Scienze Informatiche, UniBo, a.a. 2026/2027).
Il contratto tra i sottosistemi è in [`doc/interfacce.md`](doc/interfacce.md).

## Struttura

| Cartella | Sottosistema | Tecnologia |
|---|---|---|
| `control-unit/` | Control Unit (backend) | Java 17, Maven, Vert.x 4.5, JSSC |
| `dashboard/` | Dashboard web, servita dalla Control Unit | HTML / JavaScript |
| `gate-controller/` | Gate Controller | Arduino UNO (da creare) |
| `air-monitoring/` | Air Monitoring | ESP32 (da creare) |
| `doc/` | Interfacce, report, schemi, link al video | |

## Control Unit

Prerequisiti: JDK 17 o successivo e Maven 3.9 o successivo. Per la parte MQTT serve un broker
(Eclipse Mosquitto) in ascolto su `localhost:1883`.

### Configurazione

I parametri sono in `control-unit/config.example.json`. Ognuno ne crea una copia personale,
`config.json`, che non va su git perché la porta seriale cambia da un PC all'altro:

```
cd control-unit
copy config.example.json config.json        (Windows)
cp config.example.json config.json          (Linux / macOS)
```

### Comandi

```
mvn compile                                  compila
mvn compile exec:java                        avvia la Control Unit (classe Main)
mvn compile exec:java "-Dexec.mainClass=it.unibo.esiot.parking.tools.ListSerialPorts"
                                             elenca le porte seriali, per trovare quella dell'Arduino
mvn package                                  crea target/control-unit.jar, da avviare con java -jar
```

Il Serial Monitor dell'Arduino IDE va chiuso prima di avviare la Control Unit: la porta seriale
può essere aperta da un solo programma alla volta.

### Classi previste

Le classi sono vuote: ognuna ha un commento con quello che deve fare.

| Classe | Ruolo |
|---|---|
| `Main` | legge la configurazione e avvia Vert.x |
| `ControlUnitVerticle` | verticle principale: crea e collega stato e collegamenti |
| `model/ParkingState` | stato di cui la Control Unit è fonte di verità e JSON per le API |
| `gate/SerialChannel` | seriale a righe di testo con JSSC |
| `gate/GateLink` | protocollo con l'Arduino (interfacce, §3) |
| `air/AirLink` | client MQTT verso l'ESP32 (interfacce, §4) |
| `web/HttpApi` | API REST e pagine della dashboard (interfacce, §5) |
| `tools/ListSerialPorts` | elenca le porte seriali disponibili |
