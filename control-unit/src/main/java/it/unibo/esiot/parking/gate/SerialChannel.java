package it.unibo.esiot.parking.gate;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.function.Consumer;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import jssc.SerialPort;
import jssc.SerialPortEvent;
import jssc.SerialPortException;

/**
 * Line-based serial channel towards the Arduino, built on JSSC
 * (like SerialCommChannel in module-lab-2.2).
 *
 * The port uses the parameters of doc/interfacce.md, §3.1: 8 data bits, no
 * parity, 1 stop bit (8N1), no flow control. The received bytes are collected
 * in a buffer and split into lines terminated by '\n'; each complete line is
 * passed to the handler given to open().
 *
 * Threads: JSSC notifies the received data from its own thread, so the handler
 * runs on that thread, not on the Vert.x event loop. Whoever opens the channel
 * must move each line to the event loop (for example with vertx.runOnContext).
 *
 * Note: opening the port resets the Arduino UNO (DTR signal). For about a
 * second some garbage characters may arrive; the protocol handles them by
 * waiting for HELLO (doc/interfacce.md, §3.4).
 */
public class SerialChannel implements CommChannel {

    private static final Logger LOG = LoggerFactory.getLogger(SerialChannel.class);

    /**
     * Longest text accepted without a '\n'. The protocol uses lines of at most
     * 32 characters (§3.1); beyond this limit the buffer is discarded.
     */
    private static final int MAX_LINE_LENGTH = 256;

    private final String portName;
    private final int baudRate;

    /** Characters received but not yet terminated by '\n'. Used only by the JSSC thread. */
    private final StringBuilder buffer = new StringBuilder();

    /** The open port, or null when the channel is closed. */
    private volatile SerialPort serialPort;
    private volatile Consumer<String> lineHandler;

    /**
     * Creates the channel without opening it: the port is opened by open(),
     * which can be called again if it fails.
     *
     * @param portName name of the port, for example "COM3" on Windows or "/dev/ttyACM0" on Linux
     * @param baudRate speed in baud, for example 115200
     */
    public SerialChannel(final String portName, final int baudRate) {
        this.portName = portName;
        this.baudRate = baudRate;
    }

    @Override
    public synchronized void open(final Consumer<String> lineHandler) throws IOException {
        if (isOpen()) {
            throw new IOException("Serial port " + this.portName + " is already open");
        }
        this.lineHandler = lineHandler;
        this.buffer.setLength(0);

        final SerialPort port = new SerialPort(this.portName);
        try {
            port.openPort();
            port.setParams(this.baudRate, SerialPort.DATABITS_8, SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);
            // The Arduino UNO has no RTS/CTS lines, so no flow control.
            port.setFlowControlMode(SerialPort.FLOWCONTROL_NONE);
            // Discards anything left in the buffers of the operating system.
            port.purgePort(SerialPort.PURGE_RXCLEAR | SerialPort.PURGE_TXCLEAR);
            // Set before the listener, because the first event may arrive immediately.
            this.serialPort = port;
            port.addEventListener(this::onSerialEvent, SerialPort.MASK_RXCHAR);
        } catch (SerialPortException e) {
            this.serialPort = null;
            closePort(port);
            throw new IOException("Cannot open serial port " + this.portName
                    + " (wrong name, or already in use, for example by the Serial Monitor)", e);
        }
        LOG.info("Serial port {} open at {} baud", this.portName, this.baudRate);
    }

    @Override
    public void sendLine(final String line) throws IOException {
        if (line.indexOf('\n') >= 0 || line.indexOf('\r') >= 0) {
            throw new IllegalArgumentException("A line cannot contain '\\n' or '\\r': " + line);
        }
        final SerialPort port = this.serialPort;
        if (port == null) {
            throw new IOException("Serial port " + this.portName + " is not open");
        }
        try {
            if (!port.writeBytes((line + "\n").getBytes(StandardCharsets.US_ASCII))) {
                throw new IOException("Cannot write to serial port " + this.portName);
            }
        } catch (SerialPortException e) {
            throw new IOException("Cannot write to serial port " + this.portName, e);
        }
    }

    @Override
    public boolean isOpen() {
        final SerialPort port = this.serialPort;
        return port != null && port.isOpened();
    }

    @Override
    public synchronized void close() {
        final SerialPort port = this.serialPort;
        this.serialPort = null;
        if (port != null) {
            closePort(port);
            LOG.info("Serial port {} closed", this.portName);
        }
    }

    /** Called by JSSC, on its own thread, when bytes have been received. */
    private void onSerialEvent(final SerialPortEvent event) {
        final SerialPort port = this.serialPort;
        if (port == null || !event.isRXCHAR() || event.getEventValue() <= 0) {
            return;
        }
        try {
            final byte[] bytes = port.readBytes(event.getEventValue());
            if (bytes != null) {
                onData(new String(bytes, StandardCharsets.US_ASCII));
            }
        } catch (SerialPortException e) {
            LOG.warn("Error while reading from serial port {}", this.portName, e);
        }
    }

    /**
     * Appends the received characters to the buffer and delivers every
     * complete line. JSSC splits the data at random points, so a call may
     * contain half a line, or several lines.
     */
    private void onData(final String chunk) {
        this.buffer.append(chunk);
        int newline = this.buffer.indexOf("\n");
        while (newline >= 0) {
            String line = this.buffer.substring(0, newline);
            this.buffer.delete(0, newline + 1);
            if (line.endsWith("\r")) {
                line = line.substring(0, line.length() - 1);
            }
            if (!line.isEmpty()) {
                deliver(line);
            }
            newline = this.buffer.indexOf("\n");
        }
        if (this.buffer.length() > MAX_LINE_LENGTH) {
            LOG.warn("Discarded {} characters received without a newline", this.buffer.length());
            this.buffer.setLength(0);
        }
    }

    /** Passes a line to the handler; an error in the handler must not stop the reception. */
    private void deliver(final String line) {
        final Consumer<String> handler = this.lineHandler;
        if (handler == null) {
            return;
        }
        try {
            handler.accept(line);
        } catch (RuntimeException e) {
            LOG.error("Error while handling the line \"{}\"", line, e);
        }
    }

    /** Closes the port; closePort() of JSSC also removes the event listener. */
    private void closePort(final SerialPort port) {
        try {
            if (port.isOpened()) {
                port.closePort();
            }
        } catch (SerialPortException e) {
            LOG.warn("Error while closing serial port {}", this.portName, e);
        }
    }
}
