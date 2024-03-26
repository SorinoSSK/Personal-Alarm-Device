static void getRSSIDistance(BLEDevice central)
{
    int rssiValue = RSSIMeanFilter(central.rssi());
    int reference = -63;
    float pathLoss = 3.0;
    float distance = pow(10, (reference - rssiValue)/(10*pathLoss));
    BleRSSIValu.setValue(String(distance) + ", " + String(rssiValue));
}