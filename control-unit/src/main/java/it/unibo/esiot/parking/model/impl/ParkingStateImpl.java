package it.unibo.esiot.parking.model.impl;

import it.unibo.esiot.parking.model.api.ParkingState;

/**
 * Parking spots and whether the parking lot is open; the Control Unit is their
 * source of truth (doc/interfacce.md, §1 and §1.1).
 *
 * The counting is the counter of module 2.2: entries and exits are
 * transitions with a guard, so the invariant
 * 0 <= taken spots <= capacity always holds.
 *
 * Closing does not stop the counting. Entries are denied by the Gate, which
 * receives PARK:CLOSED; an entry that arrives anyway (for example a car that
 * was already under the barrier when the parking lot was closed) is a car that
 * really entered, so it is counted.
 */
public class ParkingStateImpl implements ParkingState {

    private static final int MAX_SPOTS = 99;

    private int spots;
    private int takenSpots;
    private boolean closed;

    public ParkingStateImpl(final int spots) {
        checkSpotsRange(spots);
        this.spots = spots;
        this.takenSpots = 0;
        this.closed = false;
    }

    @Override
    public int getSpots() {
        return this.spots;
    }

    @Override
    public int getTakenSpots() {
        return this.takenSpots;
    }

    @Override
    public int getFreeSpots() {
        return this.spots - this.takenSpots;
    }

    @Override
    public boolean isFull() {
        if (getFreeSpots() > 0) {
            return false;
        }
        return true;
    }

    @Override
    public void changeSpots(final int newSpots) {
        checkSpotsRange(newSpots);
        if (newSpots < this.takenSpots) {
            throw new IllegalArgumentException(
                "La capienza non può essere minore dei posti occupati (" + this.takenSpots + "), ricevuto " + newSpots);
        }
        this.spots = newSpots;
    }

    @Override
    public boolean newAccess() {
        if (this.getFreeSpots() > 0) {
            this.takenSpots++;
            return true;
        } else {
            return false;
        }
    }

    @Override
    public boolean exit() {
        if (this.takenSpots > 0) {
            this.takenSpots--;
            return true;
        } else {
            return false;
        }
    }

    @Override
    public boolean isClosed() {
        return this.closed;
    }

    @Override
    public boolean close() {
        if (this.closed) {
            return false;
        }
        this.closed = true;
        return true;
    }

    @Override
    public boolean open() {
        if (!this.closed) {
            return false;
        }
        this.closed = false;
        return true;
    }

    @Override
    public Status getStatus() {
        if (this.closed) {
            return Status.CLOSED;
        }
        return isFull() ? Status.FULL : Status.AVAILABLE;
    }

    private static void checkSpotsRange(final int value) {
        if (value < 1 || value > MAX_SPOTS) {
            throw new IllegalArgumentException(
                "La capienza deve essere tra 1 e " + MAX_SPOTS + ", ricevuto " + value);
        }
    }
}
