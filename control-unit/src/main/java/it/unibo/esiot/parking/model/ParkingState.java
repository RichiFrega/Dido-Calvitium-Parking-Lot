package it.unibo.esiot.parking.model;

/**
 * Stato di cui la Control Unit è la fonte di verità (doc/interfacce.md, §1).
 *
 * Contiene:
 * - capienza e posti occupati (sempre tra 0 e la capienza);
 * - livello dell'aria (NORMAL / WARNING / ALARM) e ultimo campione di
 *   temperatura e CO;
 * - periodo di campionamento, modalità e livello della ventilazione;
 * - stato dei collegamenti con il Gate e con l'ESP32.
 *
 * Fornisce anche il JSON di GET /api/status e GET /api/public (§5.2).
 */
public class ParkingState {
}
