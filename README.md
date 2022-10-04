# Particle Filter Example for FPGA

## Basic PlatformIO usage

Build executable:
```
pio run
```

Test software (runs entire application on host CPU):
```
.\program
```

Synthesize hardware design and program FPGA:
```
pio run --target upload
```

Test hardware (runs hardware piece on FPGA and main on host CPU) (**MUST unplug and plug board back in to reset**):
```
.\program <COM port>
```
Use device manager to find your board COM port.
