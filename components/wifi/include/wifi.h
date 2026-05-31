#ifndef WIFI_H
#define WIFI_H

void wifiSTAConnect(char *ssid, char *password);
void wifiSTADisconnect();
void wifiDestroy();

bool wifiIsRunning();
bool wifiIsSTAConnected();
char* wifiGetSTASsid();
char* wifiGetAvailableNetworks();

#endif //WIFI_H