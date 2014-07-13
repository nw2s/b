{
	"program" : 	

	{
		"name" : 			"Sample Looper Demo 3",
			
		"clock" :

		{
			"type" : "FixedClock",
			"tempo" : 120,
			"beats" : 16			
		},

		"devices" : [
											
			{
				"type" : "CVSequencer",
				"division" : "eighth",
				"analogOutput" : 16,
				"min" : 0,
				"max" : 5000
			},		
			
			{
				"type" : "CVSequencer",
				"division" : "quarter",
				"analogOutput" : 14,
				"min" : 0,
				"max" : 5000
			},
					
			{
				"type" : "CVSequencer",
				"division" : "dotted eighth",
				"analogOutput" : 15,
				"min" : 0,
				"max" : 5000
			},
					
			{
				"type" : "CVSequencer",
				"division" : "half",
				"analogOutput" : 13,
				"min" : 0,
				"max" : 5000
			},

			{
				"type" : "ProbabilityTriggerSequencer",
				"division" : "quarter",
				"digitalOutput" : 1,
				"probabilityModifier" : 1,
				"triggers" :   [ 25 ]
			},

			{
				"type" : "ProbabilityTriggerSequencer",
				"division" : "quarter",
				"digitalOutput" : 2,
				"probabilityModifier" : 2,
				"triggers" :   [ 25 ]
			},

			{
				"type" : "Looper",
				"subfolder" : "vocal",
				"filename" : "german.raw",
				"samplerate" : "24000",
				"glitch" : 1,
				"reverse" : 2,
				"dacOutput" : 1
			},
							
			{
				"type" : "Looper",
				"subfolder" : "noise",
				"filename" : "vinyl.raw",
				"samplerate" : "24000",
				"glitch" : 3,
				"reverse" : 4,
				"dacOutput" : 2
			}				
		]
	}
}


