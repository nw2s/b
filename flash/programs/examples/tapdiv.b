{
	"program" : 	

	{
		"name" : 			"Tap Divider",
			
		"clock" :

		{
			"type" : "TapTempoClock",
			"tapInput" : 1,
			"resetInput" : 2,
			"beats" : 16			
		},

		"devices" : [
			
			{
				"type" : "Trigger",
				"division" : "whole",
				"triggerOutput" : 1
			},

			{
				"type" : "Trigger",
				"division" : "half",
				"triggerOutput" : 2
			},

			{
				"type" : "Trigger",
				"division" : "quarter",
				"triggerOutput" : 3
			},

			{
				"type" : "Trigger",
				"division" : "quarter triplet",
				"triggerOutput" : 4
			},

			{
				"type" : "Trigger",
				"division" : "dotted eighth",
				"triggerOutput" : 5
			},

			{
				"type" : "Trigger",
				"division" : "eighth",
				"triggerOutput" : 6
			},

			{
				"type" : "Trigger",
				"division" : "eighth triplet",
				"triggerOutput" : 7
			},

			{
				"type" : "Trigger",
				"division" : "sixteenth",
				"triggerOutput" : 8
			}		
		]
	}
}