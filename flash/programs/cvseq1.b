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
				"values" : [
					
					-5000, -4000, -3000, -2000, -1000, 0, 1000, 2000, 3000, 4000, 5000
															
				]
			}				
		]
	}
}