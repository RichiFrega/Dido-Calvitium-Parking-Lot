package it.unibo.esiot.parking.gate;

import java.io.IOException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import io.vertx.core.Vertx;

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

    private static final Logger LOG = LoggerFactory.getLogger(GateLink.class);

    public enum LinkState {WAITING_HELLO, CONNECTED, SLEEPING}

    private final Vertx vertx;
    private final CommChannel channel;
    private final GateListener listener;
    private final long pingPeriodMs;
    private final long linkTimeoutMs;

    private LinkState state = LinkState.WAITING_HELLO;
    private long lastContact;      // time of the last line received, in ms
    private long timerId = -1;     // id of the Vert.x periodic timer

    public GateLink(Vertx vertx, CommChannel channel, GateListener listener, long pingPeriodMs, long linkTimeoutMs){
        this.vertx = vertx;
        this.channel = channel;
        this.listener = listener;
        this.pingPeriodMs = pingPeriodMs;
        this.linkTimeoutMs = linkTimeoutMs;
    }

    public void start() throws IOException{
        channel.open(line -> vertx.runOnContext(v -> onLine(line)));
        timerId = vertx.setPeriodic(pingPeriodMs, id -> onTimer());
    }

    public void stop() {
        vertx.cancelTimer(timerId);
        channel.close();
    }

    private void onLine(final String line){
        lastContact = System.currentTimeMillis();
        if (line.startsWith("#")){
            LOG.info("Gate: {}", line);
            return;
        }
        final String[] parts = line.split(":");   // "HELLO:0:0" -> ["HELLO", "0", "0"]
        switch (parts[0]) {
            case "HELLO" -> onHello(parts);
            case "EV"    -> onEvent(parts);
            case "GATE"  -> onGate(parts);
            case "BTN"   -> listener.onButtonOpen();
            case "SLEEP" -> state = LinkState.SLEEPING;
            case "WAKE"  -> { state = LinkState.CONNECTED; listener.onWake(); }
            case "PONG"  -> { }                   // lastContact is already updated
            default      -> LOG.warn("Unknown message from the Gate: {}", line);
        }
    }

    private void onHello(final String[] parts) {
        if (parts.length != 3) {
            LOG.warn("Malformed HELLO");
            return;
        }
        try {
            final int entered = Integer.parseInt(parts[1]);
            final int exited = Integer.parseInt(parts[2]);
            state = LinkState.CONNECTED;          // first: the listener will send the full state
            listener.onHello(entered, exited);
        } catch (NumberFormatException e) {
            LOG.warn("Malformed HELLO: {}", String.join(":", parts));
        }
    }

    private void onEvent(final String[] parts) {
        if (parts.length != 2) {
            LOG.warn("Malformed GATE");
            return;
        }
        switch (parts[1]) {
            case "ENTER" -> listener.onEnter();
            case "EXIT"  -> listener.onExit();
            case "ABORT" -> listener.onAbort();
            default      -> LOG.warn("Unknown event: {}", parts[1]);
        }
    }

    private void onGate(final String[] parts) {
        if (parts.length != 2) {
            LOG.warn("Malformed EV");
            return;
        }
        switch (parts[1]) {
            case "OPEN"   -> listener.onGateOpen();
            case "CLOSED" -> listener.onGateClosed();
            default       -> LOG.warn("Unknown gate state: {}", parts[1]);
        }
    }

    private void onTimer() {
        if (state != LinkState.CONNECTED) {
            return;                               // waiting for HELLO, or the Gate is asleep
        }
        if (System.currentTimeMillis() - lastContact > linkTimeoutMs) {
            state = LinkState.WAITING_HELLO;
            LOG.warn("The Gate is not responding");
            listener.onLinkLost();
        } else {
            send("PING");
        }
    }

    public void sendFree(final int freeSpots) { sendIfConnected("FREE:" + freeSpots); }
    public void sendAlarm(final boolean on)   { sendIfConnected(on ? "ALARM:ON" : "ALARM:OFF"); }
    public void sendPark(final boolean closed){ sendIfConnected(closed ? "PARK:CLOSED" : "PARK:OPEN"); }
    public void sendOpenCommand()             { sendIfConnected("CMD:OPEN"); }

    public LinkState getLinkState() { return state; }

    private void sendIfConnected(final String message) {
        if (state == LinkState.CONNECTED) {
            send(message);
        }
    }

    private void send(final String message) {
        try {
            channel.sendLine(message);
        } catch (IOException e) {
            LOG.warn("Cannot send {} to the Gate: {}", message, e.getMessage());
        }
    }
}
