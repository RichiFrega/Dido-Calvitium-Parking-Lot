package it.unibo.esiot.parking.gate;

/**
 * Line-based serial channel towards the Arduino, built on JSSC
 * (like SerialCommChannel in lab 2.2).
 *
 * It must:
 * - open the port with the parameters of §3.1 (115200 baud, 8N1);
 * - collect the received bytes and split them into lines terminated by '\n';
 * - deliver each received line to whoever uses the channel;
 * - send a line, appending '\n';
 * - close the port.
 *
 * Warning: JSSC notifies the received data from its own thread, not from the
 * Vert.x event loop.
 */
public class SerialChannel {
}
