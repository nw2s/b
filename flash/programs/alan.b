{
	"program" : 	

	{
		"name" : 			"Alanesque",
		"description" : 	"blah blah",

		"devices" : [
			
			{
				"type" : "VariableClock",
				"id" : 	"vclock",
				"minTempo" : 20,
				"maxTempo" : 240,
				"analogInput" : 1,
				"ticksPerMeasure" : 16
			},
			
			{
				"type" : "RandomLoopingShiftRegister",
				"id" : "shiftregister",
				"clock" : "vclock",
				
				
			}
			
			
		]

	}
}