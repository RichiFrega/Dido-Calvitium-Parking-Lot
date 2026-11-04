package it.unibo.esiot.parking.gate;

/**
 * Protocol with the Gate Controller (doc/interfacce.md, §3), on top of SerialChannel.
 *
 * It must:
 * - at startup wait for HELLO, then send the full state (FREE, ALARM and PARK);
 * - send PING every 2 s and detect when PONG stops arriving;
 * - handle SLEEP and WAKE;
 * - interpret the Arduino messages (EV, GATE, BTN, HELLO with the degraded-mode
 *   counters) and ignore the log lines starting with '#';
 * - let the other parts of the Control Unit send FREE, ALARM, PARK and CMD:OPEN;
 * - retry periodically if the port does not open or the cable is unplugged.
 */
public class GateLink {
}
