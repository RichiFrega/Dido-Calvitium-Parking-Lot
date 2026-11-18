package it.unibo.esiot.parking.gate;

public interface GateListener {
    public void onHello(int entered, int exited);
    public void onEnter();
    public void onAbort();
    public void onExit();
    public void onGateOpen();
    public void onGateClosed();
    public void onButtonOpen();
    public void onWake();
    public void onLinkLost();
}
