# Bootstrapping Device

When setting up new device, it's possible that there are many clients so it's beneficial to not make the process repetitive. Ideally, users will only need to setup the server and the clients can detect the server out of the box.

## Current Method

User would connect the central server to Wifi using like an app, then the server would handle bootstrapping new devices, when the client device turns on, it would pick the nearest server(aka one with strongest signal). This would require:

- The server having two modes: Normal operation and bootstrap mode
- Bootstrap mode would be running a small webserver that exchanges user wifi network credentials
- Key based authentication to make sure the clients can trust the server and vice versa
