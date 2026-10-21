package it.unibo.esiot.parking.gate;

/**
 * Protocollo con il Gate Controller (doc/interfacce.md, §3), sopra SerialChannel.
 *
 * Deve:
 * - all'avvio aspettare HELLO, poi inviare lo stato completo (FREE e ALARM);
 * - mandare PING ogni 2 s e accorgersi quando non arrivano più PONG;
 * - gestire SLEEP e WAKE;
 * - interpretare i messaggi dell'Arduino (EV, GATE, BTN, HELLO con i contatori
 *   della modalità degradata) e ignorare le righe di log che iniziano con '#';
 * - permettere alle altre parti della Control Unit di inviare FREE, ALARM e
 *   CMD:OPEN;
 * - riprovare periodicamente se la porta non si apre o il cavo viene scollegato.
 */
public class GateLink {
}
