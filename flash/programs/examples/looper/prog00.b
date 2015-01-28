
{
	"program" : 	

	{
		"name" : 			"Sample Looper Demo 4",
			
		"clock" :

		{
			"type" : "FixedClock",
			"tempo" : 130,
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
				"division" : "whole",
				"triggerOutput" : 3
			},

			{
				"type" : "ProbabilityTriggerSequencer",
				"division" : "quarter",
				"digitalOutput" : 2,
				"probabilityModifier" : 7,
				"triggers" :   [ 20 ]
			},

			{
				"type" : "CVSequencer",
				"division" : "quarter",
				"analogOutput" : 15,
				"min" : 0,
				"max" : 5000
			},

			{
				"type" : "CVSequencer",
				"division" : "quarter triplet",
				"analogOutput" : 16,
				"min" : 0,
				"max" : 5000
			},

			{
				"type" : "Looper",
				"loops" : [
						
					{
						"subfolder" : "metal",
						"filename" : "mtone1.raw"						
					},
					
					{
						"subfolder" : "metal",
						"filename" : "mtone2.raw"						
					},		
					
					{
						"subfolder" : "metal",
						"filename" : "mtone3.raw"						
					},		
					
					{
						"subfolder" : "metal",
						"filename" : "mtone4.raw"						
					},		
					
					{
						"subfolder" : "metal",
						"filename" : "mtone5.raw"						
					},		
					
					{
						"subfolder" : "metal",
						"filename" : "mtone10.raw"						
					},		
					
					{
						"subfolder" : "metal",
						"filename" : "mtone11.raw"						
					},		
					
					{
						"subfolder" : "metal",
						"filename" : "mtone12.raw"						
					},		
					
					{
						"subfolder" : "metal",
						"filename" : "mtone13.raw"						
					}		
											
				], 
				
				"startcontrol" : 1,
				"lengthcontrol" : 2,
				"mixcontrol" : 3,
				"finelengthcontrol" : 4,
				"density" : 5,
				"bitcontrol" : 6,
				"mixmode" : "blend",
				"samplerate" : "24000",
				"glitch" : 1,
				"reverse" : 2,
				"mixtrigger" : 3,
				"resettrigger" : 4,
				"dacOutput" : 1
			}
							
		]
	}
}

