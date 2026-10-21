package it.unibo.esiot.parking.air;

/**
 * Client MQTT verso l'Air Monitoring (doc/interfacce.md, §4), con vertx-mqtt.
 *
 * Deve:
 * - collegarsi al broker e iscriversi a parking/air/data e parking/air/status;
 * - aggiornare lo stato con i campioni ricevuti e con online / offline;
 * - pubblicare parking/air/config e parking/air/ventilation, retained e QoS 1;
 * - riconnettersi se il collegamento con il broker cade.
 */
public class AirLink {
}
