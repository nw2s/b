
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
				"type" : "EFLooper",
				"subfolder" : "noise",
				"filename" : "rain.raw",
				"windowsize" : 1,
				"scale" : 3,
				"threshold" : 5,
				"analogOutput" : 16
			},
							
			{
				"type" : "EFLooper",
				"subfolder" : "melodic",
				"filename" : "ebgtr120.raw",
				"windowsize" : 2,
				"scale" : 4,
				"threshold" : 6,
				"analogOutput" : 15
			}
							
			{
				"type" : "EFLooper",
				"subfolder" : "melodic",
				"filename" : "musicbox.raw",
				"windowsize" : 7,
				"scale" : 9,
				"threshold" : 11,
				"analogOutput" : 14
			},
							
			{
				"type" : "EFLooper",
				"subfolder" : "drum",
				"filename" : "bongo120.raw",
				"windowsize" : 8,
				"scale" : 10,
				"threshold" : 12,
				"analogOutput" : 13
			}
							
		]
	}
}