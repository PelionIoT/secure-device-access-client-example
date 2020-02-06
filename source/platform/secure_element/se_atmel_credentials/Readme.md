# Content of the directory
## Default pre-provisioned MCHP credentials:
For more detailed information please download updated FCU, in the extracted directory see docs/ft-demo/generating-keys-and-certificates-for-the-demo.html#using-an-atecc608a-secure-element 
1. cust_def_1_signer.c
1. cust_def_1_signer.h
1. cust_def_2_device.c
1. cust_def_2_device.h


## Default MCHP CA root certificate:
1. default-root-ca.crt

You can replace the default pre-provisioned credentials using Microchip's [Trust Platform Design Suite](https://microchipdeveloper.com/authentication:trust-platform), 
which enables you to generate a certificate authority (CA) and emulate the production flow of signing the pre-provisioned credentials on the secure element with your own CA.
