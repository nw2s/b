{
	"program" : 	

	{
		"name" : 			"Discrete Noise",
		"description" : 	"This simple demo just sets up two independent noise 
							 oscillators on DAC1 and DAC2. The sample rate of the 
							 discrete osc is set using two analog inputs. This is 
							 great for some 8 bit nastiness.",

		"devices" : [
			
			{
				"type" : "DiscreteNoise",
				"dacOutput" : 1,
				"analogInput" : 1,
			},
			
			{
				"type" : "DiscreteNoise",
				"dacOutput" : 2,
				"analogInput" : 2,
			}			
		]
	}
}