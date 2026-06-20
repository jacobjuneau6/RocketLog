# RocketLog
RocketLog is a rocket altimeter designed to fit in the estes So Long rocket and provide attitude and altitude data on the rocket logging to redundant internal flash.
# Design
[![PCB Preview]()](https://kicanvas.org/?repo=https%3A%2F%2Fgithub.com%2Fjacobjuneau6%2FRocketLog%2Fblob%2Fmain%2Falti%2Falti.kicad_pcb)Click the image to open the interactive KiCanvas viewer.
# Why
I have flown by custom painted and modified So Long rocket many times but have only been able to estimate the appogee and forces experienced. As I want to aluch sime more delicate payloads in the future, I want to see the forces that would be experienced by the payload before I fly some more delicate payloads.
# BOM
| Reference | Value | DigiKey_Part_Number | Link | Cost |
| --- | --- | --- | --- | --- |
| J3 | 10118193-0001LF | 609-4616-1-ND | [Link](https://www.digikey.com/en/products/detail/amphenol-cs-fci/10118193-0001LF/2785388) | $0.58 |
| R5,R6 | RC2512JK-0710KL | YAG3394CT-ND | [Link](https://www.digikey.com/en/products/detail/yageo/RC2512JK-0710KL/5922475) | $0.60 |
| U1 | ESP32-S3-WROOM-1-N8R8 | 5407-ESP32-S3-WROOM-1-N8R8CT-ND | [Link](https://www.digikey.com/en/products/detail/espressif-systems/ESP32-S3-WROOM-1-N8R8/15295891) | $6.73 |
| U2 | TLV75533PDYDR | 296-TLV75533PDYDRCT-ND | [Link](https://www.digikey.com/en/products/detail/texas-instruments/TLV75533PDYDR/22531567) | $0.30 |
| U3 | BMP384 | 828-BMP384CT-ND | [Link](https://www.digikey.com/en/products/detail/bosch-sensortec/BMP384/13681277) | $3.75 |
| U5 | MPU-6500 | 1428-1011-1-ND | [Link](https://www.digikey.com/en/products/detail/tdk-invensense/MPU-6500/4385412) | $8.00 |
| U6 | W25Q128JVSIQ | W25Q128JVSIQ-ND | [Flash](https://www.aliexpress.us/item/3256811354152848.html?utparam-url=scene%3Asearch%7Cquery_from%3A%7Cx_object_id%3A1005011540467600%7C_p_origin_prod%3A) | $7.00 |
| PCB | Custom Boards & Stencils from JLCPCB | | | $27.00 |
| Battery | 300mAh battery | | [Battery](https://www.amazon.com/battery-Rechargeable-Lithium-Polymer-Connector/dp/B07BTTVX4Q?th=1) | $4.00 |
| Digikey Shipping | | | | $8.50 |
