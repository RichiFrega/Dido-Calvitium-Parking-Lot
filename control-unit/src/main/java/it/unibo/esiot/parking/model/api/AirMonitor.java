package it.unibo.esiot.parking.model.api;

import java.util.OptionalDouble;
import java.util.OptionalLong;

/**
 * Finite state machine of the air quality level (doc/interfacce.md, §2 and §6.3).
 *
 * It is event-triggered: it reacts when a sample arrives and when the operator
 * resets an alarm. The thresholds (C1, C2, T1, T2, DT) and the sampling periods
 * (F1, F2) are passed to the constructor of the implementation.
 */
public interface AirMonitor {

    /**
     * Air quality level. CRITICAL means "above threshold, waiting for DT to
     * elapse before going to ALARM".
     */
    enum Level { NORMAL, WARNING, CRITICAL, ALARM };

    /**
     * Processes a new sample and moves the state machine forward.
     *
     * @param co carbon monoxide concentration, in ppm
     * @param temperature temperature, in °C
     * @param timestamp time the sample was received, in milliseconds since 1/1/1970
     * @return true if the level changed
     */
    public boolean analyzeSample(double co, double temperature, long timestamp);

    /**
     * Operator reset: brings the state machine from ALARM back to NORMAL.
     *
     * @return true if there was an alarm to reset, false otherwise
     */
    public boolean reset();

    /**
     * Switches the ventilation to manual mode with the given level. In ALARM the
     * ventilation stays at 100% anyway (doc/interfacce.md, §5.3).
     *
     * @param level ventilation level, from 0 to 100 (%)
     * @return true if the operation succeeded and the setting changed; false if
     *         the level is out of range or that manual level was already set
     */
    public boolean setManualVentilation( int level );

    /** Switches the ventilation back to automatic mode, computed from the air level. */
    public void setAutoVentilation();

    /** Current air quality level. */
    public Level getLevel();

    /** Sampling period the ESP32 must use in the current level (F1 or F2), in milliseconds. */
    public long getSamplingPeriodMs();

    /** Ventilation level actually applied, from 0 to 100 (%): computed, manual or forced to 100 in ALARM. */
    public int getVentilationLevel();

    /** True if the ventilation is in manual mode, false if it is automatic. */
    public boolean isManualVentilation();

    /**
     * Temperature of the last sample, in °C.
     *
     * The three "last sample" methods return an Optional because before the
     * first sample there is no reading: returning 0 would look like a real
     * measurement, since 0 is a valid value.
     *
     * @return the temperature, or an empty OptionalDouble if no sample has arrived yet
     */
    public OptionalDouble getLastTemperature();

    /** CO concentration of the last sample, in ppm; empty if no sample has arrived yet. */
    public OptionalDouble getLastCO();

    /** Time of the last sample, in milliseconds since 1/1/1970; empty if no sample has arrived yet. */
    public OptionalLong getLastSampleTime();

}
