# Telehaptics-Application-over-a-Mobile-Network-Testbed
This repository entails the  source codes for the developed telehaptics system, instruction guides, and sample raw results achieved from the system evaluation over the 4G mobile network testbed.
The developed telehaptics sytem entails the master,network, and slave domains.

**The Master Domain**


This site is where control commands, in form of force and positions measurements are issued. The human operator sits at this site, where the generated data is encoded and transmitted over the network to manipulate the teleopetor at the slave domain.
A 3D Touch Haptic device, Ubuntu OS computer,Raspberry Pi Model B, grove vibration motor were used in this site

**The Network Domain**

This domain provides the network infrastructure that couples the master and the slave domain, allowing the exchange of data and energy. Furthermore, this coupling results in a closed communication loop where data flows between the master and the slave controllers, simultaneously.
Local area network, and a 4G , 5G testbed were configured to constitute the network domain.

**The Slave Domain**

This is the remote teleoperator site that actuates the commands receieved from the master domain. It constitutes a robotic arm and a controller. This site comprised an Adeept 4DoF rasparm, a grove vibration sensor, and a Raspberry Pi controller
