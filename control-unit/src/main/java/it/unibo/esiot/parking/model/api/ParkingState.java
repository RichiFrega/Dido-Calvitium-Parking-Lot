package it.unibo.esiot.parking.model.api;

/**
 * Parking spots and whether the parking lot is open (doc/interfacce.md, §1.1).
 *
 * Invariant that always holds: 0 <= taken spots <= capacity.
 */
public interface ParkingState {

    /** Overall state of the parking lot (doc/interfacce.md, §1.1). */
    enum Status { AVAILABLE, FULL, CLOSED }

    /** Total capacity of the parking lot. */
    int getSpots();

    /** Number of taken spots. */
    int getTakenSpots();

    /** Free spots: capacity minus taken spots. */
    int getFreeSpots();

    /** True if there are no free spots, whether the parking lot is open or closed. */
    boolean isFull();

    /**
     * Changes the capacity (PUT /api/config/capacity, doc/interfacce.md §5.2).
     *
     * @param newSpots the new capacity, between 1 and 99 and not lower than the taken spots
     * @throws IllegalArgumentException if the value is not valid
     */
    void changeSpots(int newSpots);

    /**
     * Records an entry that has happened (EV:ENTER).
     *
     * @return true if it was counted, false if there were no free spots
     */
    boolean newAccess();

    /**
     * Records an exit that has happened (EV:EXIT).
     *
     * @return true if it was counted, false if the parking lot was already empty
     */
    boolean exit();

    /** True if the parking lot has been closed by an operator. */
    boolean isClosed();

    /** Closes the parking lot. Returns true if it was open before. */
    boolean close();

    /** Reopens the parking lot. Returns true if it was closed before. */
    boolean open();

    /** Overall state: CLOSED if closed, otherwise FULL or AVAILABLE. */
    Status getStatus();
}
