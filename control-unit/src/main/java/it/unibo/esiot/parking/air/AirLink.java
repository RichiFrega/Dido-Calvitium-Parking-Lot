package it.unibo.esiot.parking.air;

/**
 * MQTT client towards the Air Monitoring subsystem (doc/interfacce.md, §4),
 * built with vertx-mqtt.
 *
 * It must:
 * - connect to the broker and subscribe to parking/air/data and parking/air/status;
 * - update the state with the received samples and with online / offline;
 * - publish parking/air/config and parking/air/ventilation, retained, with QoS 1;
 * - reconnect if the connection to the broker drops.
 */
public class AirLink {
}
