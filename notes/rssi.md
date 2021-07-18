 RSSI - indicates the energy loss in the process of signal transmission 

 - ranging model - measure RSSI loss values at each node's location and then compare each value with this:
 	- does not cover sufficiently dynamical changes

Loss = 32.44 + 10n log(d) + 1-nlog(f), where
 d is indicated distance
 f  indicated wireless signal frequency 

(the formula for ideal case - vacuum)

path loss model:
 Pl(d) = Pl(d0) + 10n log(d/d0) + Xsigm    (is in dmb - absolute power)

 Pl(d0) - reference signal where distance is d0

 n indicated the path loss index in a specific environment; it indicated the speed of the path loss, which is increased along with increasing distance 

 X sigm also in dB where standart deviation is 4 ~ 10

then

 RSSI = Pt(transmition power) - Pl(d)(path loss when the distance is d)


Signal is affected by:
Absorption (by walls and etc. )

Reflection

Defraction (way splits into several)





