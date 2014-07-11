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
			},	
			
			{
				"type" : "CVSequencer",
				"division" : "eighth",
				"analogOutput" : 16,
				"gateOutput" : 16,
				"gateLength" : 125,
				"min" : 0,
				"max" : 5000
			},		
			
			{
				"type" : "CVSequencer",
				"division" : "whole",
				"analogOutput" : 15,
				"gateOutput" : 15,
				"gateLength" : 125,
				"min" : 0,
				"max" : 5000
			},		

			{
				"type" : "CVSequencer",
				"division" : "quarter",
				"analogOutput" : 14,
				"gateOutput" : 14,
				"gateLength" : 125,
				"min" : 0,
				"max" : 5000
			}		
							
		]
	}
}