## Task Board Photos

### Content Disclaimer:
The pictures contained in this directory serve to give a better idea of what the task board will look like in various lighting conditions. The [simulated_lighting](./simulated_lighting/) directory has a distant light for improved visibility. We cannot guarantee the accuracy of this simulation during Test Week, so the worst case scenario of [minimal_lighting](./minimal_lighting/) has also been provided.

Please let us know if you have any questions by emailing `nasa-suits@mail.nasa.gov`

### Organization
The photos are organized by directory, where the directory indicates a different lighting condition. Within these directories, there are multiple angles and scenarios for indicators. You can read the file names as follows:

For file `XX_YY_ZZ.jpeg`:
| Segment |        Description           |              Acronyms                 |
| :-----: | :--------------------------: | :-----------------------------------: |
|  `XX`   | The lighting condition       | `GL` -> good_lighting, `ML` -> minimal_lighting, `SL` -> simulated_lighting |
|  `YY`   | The indicator illumination   | `NI` -> no indicators, -> `SI` -> starting indicators, `MI` -> maximum indicators  |
|  `ZZ`   | The angle of the photo       | `BE` -> bird's eye, `POV` -> point of view, `LPOV` -> low point of view, `RR` -> radial right, `RL` -> radial left |

> Example: `SL_SI_POV.jpeg` would be the photo of the task board using simulated lighting with starting indicators from a traditional point of view

### Close Up Photos
If you are looking for the previous close-up photos, those can be found in the [deprecated](./deprecated/) directory. Close up component photos will not be prepared in multiple lighting conditions.

### Dimensions & Labels
The following table contains the dimensions of each component in both metric and imperial units. The relative location is based on their position in the bird's eye photo. The measurements are of the primary interface of the component, this does not include the base that is slightly larger than the panel portions.

In addition, each item the design evaluator will interact with or need to identify has been labeled visibly to aid in identification. The label code describes the device and its subsystem and has 4 random numbers on the end. The labels will be the primary method of identification in the [ltv-repairs-procedures.pdf](./../../procedures/ltv-repair-procedures.pdf) document.

> Note: The dust sensor does not have a dedicated label, it is identified by being a cylindrical housing smaller than the palm of a hand

|               Name              |    Label Code         |  Color  | Metric (mm) | Imperial (in) |
| ------------------------------- | --------------------- | ------- | ----------- | ------------- |
| Communications                  | ACM4400xxxx           | Silver  | 100 x 100   | 3.94 x 3.94   |
| Navigation                      | NAV5500xxxx           | White   | 150 x 175   | 5.90 x 6.89   |
| Backup Fuse                     | BFH9900xxxx           | Gray    | 135 x 210   | 5.31 x 8.27   |
| Power Distribution & Diagnostic | PDD9900xxxx           | White   | 175 x 315   | 6.89 x 12.4   |
| Software Management & Control   | SMC1100xxxx           | White   | 120 x 145   | 4.72 x 5.71   |
| Dust Sensor                     | N/A (cylindrical item)| White   | 50 x 50     | 1.97 x 1.97   |
| Scientific Power Regulation     | SPR2200xxxx           | Black   | 88 x 85     | 3.46 x 3.35   |
| Scientific Management & Power Distribution| SMPD2200xxxx| Silver  | 55 x 80     | 2.17 x 3.15   |
> Please note the Power Distribution & Diagnostic Component is irregularly shaped. The listed dimension are it's entire rectangular footprint. The diagnostic portion of this component measures `175mm x 155mm (6.89" x 6.10")` and the power distribution portion measures `70mm x 160mm (2.76" x 6.30")`


