{
	"program" : 	

	{
		"name" : 			"CV Sequence Demo 1",
			
		"clock" :

		{
			"type" : "FixedClock",
			"tempo" : 120,
			"beats" : 16			
		},

		"devices" : [
			
			{
				"type" : "CVSequencer",
				"division" : "sixteenth",
				"analogOutput" : 2,
				"randomize" : false,
				"gateOutput" : 1,
				"gateLength" : 20,
				"values" : [
					
					0, 1000, 2000, 3000, 4000, 5000
															
				]
			},

			{
				"type" : "CVSequencer",
				"division" : "triplet eighth",
				"analogOutput" : 3,
				"gateOutput" : 2,
				"gateLength" : 125,
				"min" : 0,
				"max" : 5000
			}		
					
		]
	}
}