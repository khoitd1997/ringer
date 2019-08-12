# Bootstrapping Device

When setting up new device, it's possible that there are many clients so it's beneficial to not make the process repetitive. Ideally, users will only need to setup the server and the clients can detect the server out of the box.

## General Direction

User would connect the central server to Wifi using like an app, then the server would handle bootstrapping new devices, when the client device turns on, it would pick the nearest server(aka one with strongest signal). This would require:

- The server having two modes: Normal operation and bootstrap mode
- Bootstrap mode would be running a small webserver that exchanges user wifi network credentials
- Key based authentication to make sure the clients can trust the server and vice versa

## Bootstrapping Server

The user would turn on the server and it would enter boostrap mode by default, user connects to it using a phone app, provision with wifi credentials, the server would then reserve a static IP address and start running a LAN for ringer client devices to connect to.

The server would communicate with the app to list available client devices.

The server would act as router as well as the direct interface to the app for configuration and information.

## Boostrapping User Information

This part will probably change later but a DB is also needed to store information regarding who is who in the house as well as where they currently are(specific room or whether they are in the home at all). This configuration will follow the server configuration, so that the RFID info of each person is ready for the client devices next step.

The speech recognition engine will need to be trained to recognize names that user input. A user might have more than one name.

## Bootstrapping Clients

The client turns on for the first time and would enter boostrap mode, it searches for server with strongest signal and connect to it. The device gets Wifi credentials from the server and reserves a static IP address, let the server know of its address which will be entered into a DB.

The user needs to map which client connects to which room, the user would select a device in the app and the device would flash an LED to indicate it's being configured and the user can start entering metadata about the room.

The data will probably be stored on the server only. After bootstrap, the client enters normal mode and engaches its RFID reader and send information regarding user location to the server.
