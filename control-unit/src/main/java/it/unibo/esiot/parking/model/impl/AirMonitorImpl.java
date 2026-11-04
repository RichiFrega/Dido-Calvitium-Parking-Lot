package it.unibo.esiot.parking.model.impl;

import java.util.OptionalDouble;
import java.util.OptionalLong;

import it.unibo.esiot.parking.model.api.AirMonitor;

/**
 * Air quality state machine (doc/interfacce.md, §2 and §6.3).
 *
 * Transitions, evaluated every time a sample arrives:
 * - critical sample (CO > C2 or T > T2): from NORMAL or WARNING go to CRITICAL
 *   and remember when it started; stay in CRITICAL while it lasts and go to
 *   ALARM once it has lasted at least DT without interruption;
 * - warning sample (CO >= C1 or T >= T1, but not critical): go to WARNING;
 * - any other sample: go to NORMAL.
 * ALARM is latched: samples do not change it, only reset() brings it back to NORMAL.
 *
 * The outputs depend only on the level (Moore machine): the sampling period is
 * F1 in NORMAL and F2 in the other levels; the automatic ventilation is 0 in
 * NORMAL, proportional in WARNING and 100 in CRITICAL and ALARM. In ALARM the
 * ventilation is 100 even in manual mode.
 */
public class AirMonitorImpl implements AirMonitor {

    private static final int MAX_VENTILATION = 100;

    /** Who decides the ventilation level: the state machine or the operator. */
    private enum Mode { AUTO, MANUAL }

    private final double c1;
    private final double c2;
    private final double t1;
    private final double t2;
    private final long dtMs;
    private final long f1PeriodMs;
    private final long f2PeriodMs;

    private Level level;
    /** Timestamp of the sample that started the current CRITICAL phase. */
    private long criticalSince;

    private OptionalDouble lastCo;
    private OptionalDouble lastTemperature;
    private OptionalLong lastTimestamp;

    private Mode mode;
    private int manualVentilation;

    /**
     * Creates the state machine in NORMAL, with automatic ventilation and no sample yet.
     *
     * @param c1 CO warning threshold, in ppm
     * @param c2 CO critical threshold, in ppm (greater than c1)
     * @param t1 temperature warning threshold, in °C
     * @param t2 temperature critical threshold, in °C (greater than t1)
     * @param dtMs how long a critical condition must last before the alarm, in milliseconds
     * @param f1PeriodMs sampling period in NORMAL, in milliseconds
     * @param f2PeriodMs sampling period in the other levels, in milliseconds
     * @throws IllegalArgumentException if c1 >= c2, t1 >= t2, or a time is not positive
     */
    public AirMonitorImpl(final double c1, final double c2, final double t1, final double t2,
            final long dtMs, final long f1PeriodMs, final long f2PeriodMs) {
        if (c1 >= c2 || t1 >= t2) {
            throw new IllegalArgumentException("Warning thresholds must be lower than critical thresholds");
        }
        if (dtMs <= 0 || f1PeriodMs <= 0 || f2PeriodMs <= 0) {
            throw new IllegalArgumentException("DT and sampling periods must be positive");
        }
        this.c1 = c1;
        this.c2 = c2;
        this.t1 = t1;
        this.t2 = t2;
        this.dtMs = dtMs;
        this.f1PeriodMs = f1PeriodMs;
        this.f2PeriodMs = f2PeriodMs;

        this.level = Level.NORMAL;
        this.criticalSince = 0;
        this.lastCo = OptionalDouble.empty();
        this.lastTemperature = OptionalDouble.empty();
        this.lastTimestamp = OptionalLong.empty();
        this.mode = Mode.AUTO;
        this.manualVentilation = 0;
    }

    private void setNormal() {
        this.level = Level.NORMAL;
    }

    private void setWarning() {
        this.level = Level.WARNING;
    }

    /** Enters CRITICAL and starts counting DT from the given sample. */
    private void setCritical(final long timestamp) {
        this.level = Level.CRITICAL;
        this.criticalSince = timestamp;
    }

    private void setAlarm() {
        this.level = Level.ALARM;
    }

    @Override
    public boolean analyzeSample(final double co, final double temperature, final long timestamp) {
        final Level before = this.level;

        this.lastCo = OptionalDouble.of(co);
        this.lastTemperature = OptionalDouble.of(temperature);
        this.lastTimestamp = OptionalLong.of(timestamp);

        // The sample is classified once; the transition then depends on the current level.
        final boolean critical = co > this.c2 || temperature > this.t2;
        final boolean warning = co >= this.c1 || temperature >= this.t1;

        switch (this.level) {
            case NORMAL:
            case WARNING:
                if (critical) {
                    setCritical(timestamp);
                } else if (warning) {
                    setWarning();
                } else {
                    setNormal();
                }
                break;
            case CRITICAL:
                if (!critical) {
                    // The condition stopped before DT: no alarm, and DT will restart from zero.
                    if (warning) {
                        setWarning();
                    } else {
                        setNormal();
                    }
                } else if (timestamp - this.criticalSince >= this.dtMs) {
                    setAlarm();
                }
                break;
            case ALARM:
                // Latched: only reset() leaves this level.
                break;
            default:
                break;
        }
        return this.level != before;
    }

    @Override
    public boolean reset() {
        if (this.level != Level.ALARM) {
            return false;
        }
        // If the air is still critical, the next sample goes back to CRITICAL and DT restarts.
        setNormal();
        return true;
    }

    @Override
    public boolean setManualVentilation(final int ventilationLevel) {
        if (ventilationLevel < 0 || ventilationLevel > MAX_VENTILATION) {
            return false;
        }
        if (this.mode == Mode.MANUAL && this.manualVentilation == ventilationLevel) {
            return false;
        }
        this.mode = Mode.MANUAL;
        this.manualVentilation = ventilationLevel;
        return true;
    }

    @Override
    public void setAutoVentilation() {
        this.mode = Mode.AUTO;
    }

    @Override
    public Level getLevel() {
        return this.level;
    }

    @Override
    public long getSamplingPeriodMs() {
        return this.level == Level.NORMAL ? this.f1PeriodMs : this.f2PeriodMs;
    }

    @Override
    public int getVentilationLevel() {
        if (this.level == Level.ALARM) {
            return MAX_VENTILATION;
        }
        if (this.mode == Mode.MANUAL) {
            return this.manualVentilation;
        }
        if (this.level == Level.CRITICAL) {
            return MAX_VENTILATION;
        }
        if (this.level == Level.WARNING) {
            return warningVentilation();
        }
        return 0;
    }

    /**
     * Ventilation in WARNING: proportional to how far the worse of the two
     * values has gone from its warning threshold towards its critical threshold.
     * Only called in WARNING, so a sample has always arrived.
     */
    private int warningVentilation() {
        final double coRatio = ratio(this.lastCo.getAsDouble(), this.c1, this.c2);
        final double temperatureRatio = ratio(this.lastTemperature.getAsDouble(), this.t1, this.t2);
        return (int) Math.round(MAX_VENTILATION * Math.max(coRatio, temperatureRatio));
    }

    /** Position of value between low (0) and high (1), limited to the range [0, 1]. */
    private static double ratio(final double value, final double low, final double high) {
        final double position = (value - low) / (high - low);
        return Math.max(0, Math.min(1, position));
    }

    @Override
    public boolean isManualVentilation() {
        return this.mode == Mode.MANUAL;
    }

    @Override
    public OptionalDouble getLastTemperature() {
        return this.lastTemperature;
    }

    @Override
    public OptionalDouble getLastCO() {
        return this.lastCo;
    }

    @Override
    public OptionalLong getLastSampleTime() {
        return this.lastTimestamp;
    }

}
