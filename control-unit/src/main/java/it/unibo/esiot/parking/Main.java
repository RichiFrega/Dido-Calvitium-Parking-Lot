package it.unibo.esiot.parking;

/**
 * Entry point of the Control Unit.
 *
 * It must:
 * - read the configuration from config.json (or, if missing, from config.example.json);
 * - create Vert.x and deploy ControlUnitVerticle, passing it the configuration.
 *
 * The pom.xml uses it as the main class for "mvn exec:java" and for the jar,
 * so it must contain the main method.
 */
public final class Main {
}
