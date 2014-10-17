{
	"program" : 	

	{
		"name" : 			"Sample Looper Demo 1",
			
		"clock" :

		{
			"type" : "FixedClock",
			"tempo" : 60,
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
				"type" : "Looper",
				"subfolder" : "melodic",
				"filename" : "musicbox.raw",
				"samplerate" : "24000",
				"dacOutput" : 1
			},

			{
				"type" : "Looper",
				"subfolder" : "noise",
				"filename" : "rain.raw",
				"samplerate" : "24000",
				"dacOutput" : 2
			}
							
		]
	}
}

