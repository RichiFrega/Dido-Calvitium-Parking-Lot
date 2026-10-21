package it.unibo.esiot.parking;

/**
 * Verticle principale della Control Unit.
 *
 * Deve creare lo stato (ParkingState) e i tre collegamenti (GateLink, AirLink,
 * HttpApi), avviarli e collegarli tra loro. Ad esempio, quando il livello
 * dell'aria entra in ALARM deve avvisare il Gate e portare la ventilazione al
 * massimo (doc/interfacce.md, §6.3).
 *
 * Conviene che tutti i collegamenti girino sullo stesso event loop: così lo
 * stato non viene mai modificato da due thread nello stesso momento (pattern
 * Reactor, modulo 2.4).
 */
public class ControlUnitVerticle {
}
