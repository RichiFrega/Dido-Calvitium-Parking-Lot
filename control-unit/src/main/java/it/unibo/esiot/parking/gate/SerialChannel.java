package it.unibo.esiot.parking.gate;

/**
 * Canale seriale a righe di testo verso l'Arduino, basato su JSSC
 * (come SerialCommChannel del lab 2.2).
 *
 * Deve:
 * - aprire la porta con i parametri del §3.1 (115200 baud, 8N1);
 * - raccogliere i byte ricevuti e dividerli in righe terminate da '\n';
 * - consegnare ogni riga ricevuta a chi usa il canale;
 * - inviare una riga aggiungendo '\n';
 * - chiudere la porta.
 *
 * Attenzione: JSSC notifica i dati ricevuti da un suo thread, non dall'event
 * loop di Vert.x.
 */
public class SerialChannel {
}
