package it.unibo.esiot.parking;

/**
 * Main verticle of the Control Unit.
 *
 * It must create the model (ParkingState and AirMonitor) and the three links
 * (GateLink, AirLink, HttpApi), start them and connect them to each other. For
 * example, when the air level enters ALARM it must notify the Gate and set the
 * ventilation to the maximum (doc/interfacce.md, §6.3).
 *
 * All the links should run on the same event loop, so that the state is never
 * modified by two threads at the same time (Reactor pattern, module 2.4).
 */
public class ControlUnitVerticle {
}
