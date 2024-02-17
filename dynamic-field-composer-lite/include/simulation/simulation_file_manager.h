#pragma once

#include <iostream>
#include <nlohmann/json.hpp>

#include "./simulation/simulation.h"

#include "./elements/neural_field.h"
#include "elements/gauss_kernel.h"
#include "elements/mexican_hat_kernel.h"
#include "elements/normal_noise.h"
#include "elements/field_coupling.h"
#include "elements/gauss_stimulus.h"
#include "elements/lateral_interactions.h"

namespace dnf_composer
{
	using json = nlohmann::json;

	class SimulationFileManager
	{
	private:
		std::shared_ptr<Simulation> simulation;
		std::string filePath;
	public:
		SimulationFileManager(const std::shared_ptr<Simulation>& simulation);

		void saveElementsToJson() const;
		void loadElementsFromJson() const;

	private:
		static json elementToJson(const std::shared_ptr<element::Element>& element);
		void jsonToElements(const json& jsonElements) const;
	};
}

//          ,----,                                                                                            
//        ,/   .`|                                                                                            
//      ,`   .'  :  ,---,                                  ,-.                                                
//    ;    ;     /,--.' |                              ,--/ /|                                                
//  .'___,/    ,' |  |  :                      ,---, ,--. :/ |                      ,---.           ,--,      
//  |    :     |  :  :  :                  ,-+-. /  |:  : ' /                      '   ,'\        ,'_ /|      
//  ;    |.';  ;  :  |  |,--.  ,--.--.    ,--.'|'   ||  '  /                 .--, /   /   |  .--. |  | :      
//  `----'  |  |  |  :  '   | /       \  |   |  ,"' |'  |  :               /_ ./|.   ; ,. :,'_ /| :  . |      
//      '   :  ;  |  |   /' :.--.  .-. | |   | /  | ||  |   \           , ' , ' :'   | |: :|  ' | |  . .      
//      |   |  '  '  :  | | | \__\/: . . |   | |  | |'  : |. \         /___/ \: |'   | .; :|  | ' |  | |      
//      '   :  |  |  |  ' | : ," .--.; | |   | |  |/ |  | ' \ \         .  \  ' ||   :    |:  | : ;  ; |      
//      ;   |.'   |  :  :_:,'/  /  ,.  | |   | |--'  '  : |--'           \  ;   : \   \  / '  :  `--'   \     
//      '---'     |  | ,'   ;  :   .'   \|   |/      ;  |,'               \  \  ;  `----'  :  ,      .-./     
//                `--''     |  ,     .-./'---'       '--'                  :  \  \          `--`----'         
//                           `--`---'                                       \  ' ;                            
 /*                                                                                                ,---,                                                                               
                                                                                              ,`--.' |  
                 ,--,              ,---,              ____                                    |   :  :  
               ,--.'|            ,--.' |            ,'  , `.                                  '   '  ;  
         ,---, |  | :     ,---.  |  |  :         ,-+-,.' _ |                 ,---,      ,---, |   |  |  
     ,-+-. /  |:  : '    '   ,'\ :  :  :      ,-+-. ;   , ||             ,-+-. /  | ,-+-. /  |'   :  ;  
    ,--.'|'   ||  ' |   /   /   |:  |  |,--. ,--.'|'   |  || ,--.--.    ,--.'|'   |,--.'|'   ||   |  '  
   |   |  ,"' |'  | |  .   ; ,. :|  :  '   ||   |  ,', |  |,/       \  |   |  ,"' |   |  ,"' |'   :  |  
   |   | /  | ||  | :  '   | |: :|  |   /' :|   | /  | |--'.--.  .-. | |   | /  | |   | /  | |;   |  ;  
   |   | |  | |'  : |__'   | .; :'  :  | | ||   : |  | ,    \__\/: . . |   | |  | |   | |  | |`---'. |  
   |   | |  |/ |  | '.'|   :    ||  |  ' | :|   : |  |/     ," .--.; | |   | |  |/|   | |  |/  `--..`;  
   |   | |--'  ;  :    ;\   \  / |  :  :_:,'|   | |`-'     /  /  ,.  | |   | |--' |   | |--'  .--,_     
   |   |/      |  ,   /  `----'  |  | ,'    |   ;/        ;  :   .'   \|   |/     |   |/      |    |`.  
   '---'        ---`-'           `--''      '---'         |  ,     .-./'---'      '---'       `-- -`, ; 
                                                           `--`---'                             '---`"  
                                                                                                        */