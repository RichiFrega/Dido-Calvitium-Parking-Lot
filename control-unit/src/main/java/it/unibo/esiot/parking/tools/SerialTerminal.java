package it.unibo.esiot.parking.tools;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;

import it.unibo.esiot.parking.gate.CommChannel;
import it.unibo.esiot.parking.gate.SerialChannel;
import jssc.SerialPortList;

/**
 * Test program: a minimal serial terminal built on SerialChannel, to check by
 * hand that the channel receives and sends lines.
 *
 * It prints every line received from the Arduino (prefixed by "<<") and sends
 * every line typed on the keyboard (prefixed by ">>"). Type "quit" to exit.
 * The port name can be passed as the first argument; otherwise the program
 * lists the available ports and asks for one.
 *
 * Run it from the control-unit folder with:
 * mvn -q compile exec:java "-Dexec.mainClass=it.unibo.esiot.parking.tools.SerialTerminal"
 *
 * The Serial Monitor (Arduino IDE or PlatformIO) must be closed first.
 */
public final class SerialTerminal {

    private static final int BAUD_RATE = 115200;

    private SerialTerminal() {
    }

    public static void main(final String[] args) throws IOException {
        final BufferedReader keyboard = new BufferedReader(new InputStreamReader(System.in, StandardCharsets.UTF_8));

        final String portName;
        if (args.length > 0) {
            portName = args[0];
        } else {
            final String[] ports = SerialPortList.getPortNames();
            if (ports.length == 0) {
                System.out.println("No serial port found: is the Arduino connected?");
                return;
            }
            System.out.println("Available ports: " + String.join(", ", ports));
            System.out.print("Port to open: ");
            final String typed = keyboard.readLine();
            if (typed == null || typed.isBlank()) {
                return;
            }
            portName = typed.trim();
        }

        final CommChannel channel = new SerialChannel(portName, BAUD_RATE);
        try {
            // Called from the JSSC thread: printing is enough for a test.
            channel.open(line -> System.out.println("<< " + line));
        } catch (IOException e) {
            System.out.println(e.getMessage());
            return;
        }
        System.out.println("Open " + portName + " at " + BAUD_RATE + " baud. The Arduino restarts:"
                + " wait a couple of seconds. Type a line to send it, \"quit\" to exit.");

        String line = keyboard.readLine();
        while (line != null && !line.equals("quit")) {
            if (!line.isEmpty()) {
                try {
                    channel.sendLine(line);
                    System.out.println(">> " + line);
                } catch (IOException | IllegalArgumentException e) {
                    System.out.println("Not sent: " + e.getMessage());
                }
            }
            line = keyboard.readLine();
        }
        channel.close();
    }
}
