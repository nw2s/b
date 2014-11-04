{
	"program" : 	

	{
		"name" : 			"Game Of Life",
			
		"clock" :

		{
			"type" : "FixedClock",
			"tempo" : 120,
			"beats" : 16			
		},

		"devices" : [
			
			{
				"type" : "GameOfLife",
				"division" : "sixteenth",
				"columns" : 16,
				"rows" : 16,
				"varibright" : true,
				"deviceType" : "grids"
			}					
		]
	}
}
