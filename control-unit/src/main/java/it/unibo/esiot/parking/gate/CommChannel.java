package it.unibo.esiot.parking.gate;

import java.io.IOException;
import java.util.function.Consumer;

/**
 * Line-based communication channel (like CommChannel in module-lab-2.2).
 *
 * Messages are lines of text terminated by '\n'. The channel only splits and
 * joins lines: it knows nothing about the protocol (doc/interfacce.md, §3).
 */
public interface CommChannel {

    /**
     * Opens the channel. From now on, every complete line received is passed
     * to the handler, without the '\n' (and without a trailing '\r').
     *
     * Warning: the handler may be called from a thread of the channel, not
     * from the Vert.x event loop.
     *
     * @param lineHandler function called with each received line
     * @throws IOException if the channel cannot be opened
     */
    void open(Consumer<String> lineHandler) throws IOException;

    /**
     * Sends a line. The '\n' is appended by the channel.
     *
     * @param line the line to send, without '\n'
     * @throws IOException if the channel is not open or the line cannot be sent
     */
    void sendLine(String line) throws IOException;

    /** True if the channel is open. */
    boolean isOpen();

    /** Closes the channel. Calling it when the channel is already closed does nothing. */
    void close();
}