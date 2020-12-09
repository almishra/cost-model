all: deviceArch deviceCapability

check: 
deviceArch: deviceArch.cu
	nvcc deviceArch.cu -o deviceArch

deviceCapability: deviceCapability.cu
	nvcc deviceCapability.cu -o deviceCapability

clean:
	$(RM) deviceArch deviceCapability
