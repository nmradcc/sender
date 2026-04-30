/**********************************************************************
*
* @file   CV.h
* @Author Karl Kobel
* @date   4/2/2000
* @brief  NMRA CV definitions and Manufacturer IDs
*
* NMRA CV definitions and Manufacturer IDs
*
* COPYRIGHT (c) 2000-2017 by K2 Engineering. All Rights Reserved.
*
**********************************************************************/

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

#define cvAddress          				1
#define cvStartVoltage     				2
#define cvAcceleration     				3
#define cvDeceleration     				4
#define cvHighVoltage					5
#define cvMidVoltage					6
#define cvVersion          				7
#define cvManufacturer					8

#define cvPWMPeriod						9
#define cvEMFFeedbackCutout				10
#define cvPacketTimeOut					11
#define cvPowerSourceConversion			12
#define cvAnalogModeFunctionStatus		13
#define cvAnalogModeFunctionStatus2		14

#define cvDecoderLockLow				15
#define cvDecoderLockHigh				16

//#define cvFunctionOutputPulse			16
#define cvExtendedAddressLow			17
#define cvExtendedAddressHigh			18
#define cvConsistAddress				19

#define cvConsistFunctionsActive		21
#define cvConsistFunctionsActive2		22
#define cvAccelerationAdjustment		23
#define cvDecelerationAdjustment		24
#define cvSpeedTable_MidrangeCabSpeedStep 25

#define cvReserved						26

#define cvAutomaticStoppingConfig		27
#define cvBiDirectionalCommConfig		28
#define cvConfigurationData1			29
#define cvDirection        				2900
#define cv28SpeedMode					2901
#define cvDCMode						2902
#define cvAdvancedAck					2903
#define cvSpeedTable					2904
#define cvLongShortAddress				2905
#define cvDecoderType					2907

#define cvErrorInformation				30
#define cvConfigurationData2			31
#define cvConfigurationData3			32

#define cvOutputLocoationFL				33
#define cvOutputLocoationFR				34
#define cvOutputLocoationF1				35
#define cvOutputLocoationF2				36
#define cvOutputLocoationF3				37
#define cvOutputLocoationF4				38
#define cvOutputLocoationF5				39
#define cvOutputLocoationF6				40
#define cvOutputLocoationF7				41
#define cvOutputLocoationF8				42
#define cvOutputLocoationF9				43
#define cvOutputLocoationF10			44
#define cvOutputLocoationF11			45
#define cvOutputLocoationF12			46
#define cvManufacturerUnique47			47
#define cvManufacturerUnique48			48

#define cvManufacturerUnique49			49
#define cvManufacturerUnique50       	50
#define cvManufacturerUnique51       	51
#define cvManufacturerUnique52       	52
#define cvManufacturerUnique53       	53
#define cvManufacturerUnique54       	54
#define cvManufacturerUnique55       	55
#define cvManufacturerUnique56       	56
#define cvManufacturerUnique57       	57
#define cvManufacturerUnique58      	58
#define cvManufacturerUnique59      	59
#define cvManufacturerUnique60      	60
#define cvManufacturerUnique61      	61
#define cvManufacturerUnique62      	62
#define cvManufacturerUnique63      	63
#define cvManufacturerUnique64			64

#define cvKickStart						65
#define cvForwardTrim					66

#define cvSpeedTable1					67
#define cvSpeedTable2					68
#define cvSpeedTable3					69
#define cvSpeedTable4					70
#define cvSpeedTable5					71
#define cvSpeedTable6					72
#define cvSpeedTable7					73
#define cvSpeedTable8					74
#define cvSpeedTable9					75
#define cvSpeedTable10					76
#define cvSpeedTable11					77
#define cvSpeedTable12					78
#define cvSpeedTable13					79
#define cvSpeedTable14              	80
#define cvSpeedTable15              	81
#define cvSpeedTable16              	82
#define cvSpeedTable17              	83
#define cvSpeedTable18              	84
#define cvSpeedTable19              	85
#define cvSpeedTable20              	86
#define cvSpeedTable21              	87
#define cvSpeedTable22              	88
#define cvSpeedTable23              	89
#define cvSpeedTable24              	90
#define cvSpeedTable25              	91
#define cvSpeedTable26              	92
#define cvSpeedTable27              	93
#define cvSpeedTable28					94

#define cvReverseTrim					95

#define cvUserIdentifier1				105
#define cvUserIdentifier2				106

#define cvExcvManufacturerLow			107
#define cvExcvManufacturerHigh			108

#define cvEXVersionLow					109
#define cvEXVersionMid					110
#define cvEXVersionHigh					111

#define cvManufacturerUnique17			112
#define cvManufacturerUnique18			113
#define cvManufacturerUnique19			114
#define cvManufacturerUnique20			115
#define cvManufacturerUnique21			116
#define cvManufacturerUnique22			117
#define cvManufacturerUnique23			118
#define cvManufacturerUnique24			119
#define cvManufacturerUnique25			120
#define cvManufacturerUnique26			121
#define cvManufacturerUnique27			122
#define cvManufacturerUnique28			123
#define cvManufacturerUnique29			124
#define cvManufacturerUnique30			125
#define cvManufacturerUnique31			126
#define cvManufacturerUnique32			127
#define cvManufacturerUnique33			128

//Accessory Decoder CVs
#define cvAccessoryAddressLow			513
#define cvAccessoryActivation			514
#define cvAccessoryTimeOnF1				515
#define cvAccessoryTimeOnF2				516
#define cvAccessoryTimeOnF3				517
#define cvAccessoryTimeOnF4				518
#define cvAccessoryVersion				519
#define cvAccessoryManufacturer			520
#define cvAccessoryAddressHigh			521

#define cvAccessoryConfiguration		541


#ifdef NOT_USE
Reserved by NMRA for future use
CV107,108: expanded  Mfg. ID
CV109-111: expanded CV7
Manufacturer Unique
112-256
O
Reserved for manufacturer use
Indexed area
257-512
Indexed area - see CV# 31,32
Index values of  0-4095  reserved
by NMRA
513-879
-
Reserved by NMRA for future use
880-891
Y
Reserved by NMRA for future use
Decoder Load
892
O
Y
Y
Dynamic Flags
893
O
Y
Y
Fuel/Coal                                                     894                                                     O                                                                                                                                                               Y                                                     Y
Water                                                           895                                                           O                                                                                                                                                                                 Y                                                           Y
SUSI Sound and Function Modules
896-1024
O
Y
See TN-9.2.3
#endif



//#define cvAccessoryManufacturerUnique1	545-593
//#define cvAccessoryManufacturerUnique2	594-640

// Manufacturer IDs
//#define A_Train_Electronics				0x89
#define Advance_IC_Engineering 			0x11
#define AMW								0x13
#define ANE_Model_Co_Ltd				0x2D
#define Aristo_Craft_Trains				0x22
#define Arnold_Rivarossi				0xAD
#define Atlas_Model_Railroad_Products	0x7F
#define AuroTrains                      0xAA
#define Auvidel                         0x4C
#define AXJ_Electronics 				0x6E
#define Bachmann_Trains					0x65
#define Benezan_Electronics				0x72
#define Berros                          0x7A
#define BLOCKsignalling                 0x94
#define Bluecher_Electronic             0x3C
#define Blue_Digital					0xE1
#define Brelec                          0x1F
#define BRAWA_Modellspielwaren_GmbH_Co	0xBA
#define Broadway_Limited_Imports_LLC 	0x26
#define Capecom                         0x2F
#define CML_Electronics_Limited			0x01
#define cmOS_Engineering				0x82
#define Computer_Dialysis_France		0x69
#define Con_Com_GmbH					0xCC
#define csikos_muhely                   0x78
#define cT_Elektronik					0x75
#define CVP_Products					0x87
#define Dapol_Limited					0x9A
#define DCCconcepts                     0x24
#define DCC_Gaspar_Electronic			0x7C
#define DCC_Supplies_Ltd				0x33
#define DCC_Train_Automation			0x90
#define Desktop_Station					0x8C
#define Dietz_Modellbahntechnik			0x73
#define Digi_CZ	                        0x98
#define Digirails                       0x2A
#define Digital_Bahn					0x40
#define Digitools_Elektronika_Kft		0x4B
#define Digitrax						0x81
#define Digsight                        0X1E
#define Doehler_Haas					0x61
#define Educational_Computer_Inc		0x27
#define Electronik_Model_Produktion		0x23
#define Electronic_Solutions_Ulm_GmbH	0x97
#define Electroniscript_Inc				0x5E
#define E_Modell                        0x45
#define Frateschi_Model_Trains			0x80
#define Fucik                           0x9E
#define Gaugemaster                     0x41
#define Gebr_Fleischmann_GmbH_Co		0x9B
#define GFB_Designs						0x2E
#define GooVerModels                    0x51
#define Haber_Koenig_Electronics_GmbH	0x6F
#define HAG_Modelleisenbahn_AG			0x52
#define Harman_DCC						0x62
#define Hattons_Model_Railways			0x4F
#define Heljan_A_S						0x1C
#define Heller_Modenlbahn				0x43
#define HONS_Model						0x58
#define Hornby_Hobbies_Ltd				0x30
#define Integrated_Signal_Systems		0x66
#define Intelligent_Command_Control		0x85
#define Joka_Electronic					0x31
#define JMRI                            0x12
#define JSS_Elektronic                  0x53
#define KAM_Industries					0x16
#define KATO_Precision_Models			0x28
#define Kevtronics_cc					0x5D
#define Kreischer_Datentechnik			0x15
#define KRES_GmbH						0x3A
#define Krois_Modell                    0x34
#define Kuehn_Ing						0x9D
#define LaisDCC                         0x86
#define Lenz_Elektronik_GmbH			0x63
#define LDH                             0x38
#define LGB								0x9F
#define LSdigital                       0x70
#define LS_Models_Sprl					0x4D
#define Massoth_Elektronik_GmbH			0x7B
#define MAWE_Elektronik					0x44
#define MBTronik_PiN_GITmBH				0x1A
#define MD_Electronics					0xA0
#define Mistral_Train_Models			0x1D
#define MoBaTron_de                     0x18
#define Model_Electronic_Railway_Group	0xA5
#define Model_Rectifier_Corp			0x8F
#define Modelleisenbahn_GmbH_Roco		0xA1
#define MollehemGardsproduktion			0x7E
#define MTB Model						0x48
#define MTH Electric Trains, Inc.		0x1B
#define MUT_GmbH						0x76
#define MyLocoSound                     0x74
#define N_Q_Electronics					0x32
#define NAC_Services_Inc				0x25
#define Nagasue_System_Design_Office	0x67
#define Nagoden                         0x6C
#define NCE_Corp						0x0B
#define New_York_Byano_Limited			0x47
#define Ngineering                      0x2B
#define NMRA_Reserved					0xEE
#define Noarail                         0x3F
#define Nucky                           0x9C
#define NYRS                            0x88
#define Opherline1                      0x6A
#define Passmann                        0x29
#define Phoenix_Sound_Systems_Inc		0x6B
#define PpP_Digital						0x4A
#define Pojezdy_EU                      0x59
#define Praecipuus                      0x21
#define PRICOM_Design					0x60
#define ProfiLok_Modellbahntechnik_GmbH	0x7D
#define PSI_Dynatrol					0x0E
#define Public_Domain_Decoders			0x0D
#define QElectronics_GmbH				0x37
#define QS_Industries_QSI				0x71
#define Railflyer_Model_Prototypes_Inc	0x54
#define Railnet_Solutions_LLC			0x42
#define Rails_Europ_Express				0x92
#define Railstars_Limited				0x5B
#define Ramfixx_Technologies			0x0F
#define Rampino_Elektronik				0x39
#define Rautenhaus_Digital_Vertrieb		0x35
#define RealRail_Effects				0x8B
#define Regal_Way_Co_Ltd				0x20
#define Rock_Junction_Controls			0x95
//#define Roco_Modelspielwarren			0x63
#define Rocrail                         0x46
#define RR_Cirkits                      0x57
#define S_Helper_Service				0x17
#define Sanda_Kan_Industrial_Ltd		0x5F
#define Shourt_Line						0x5A
#define SLOMO_Railroad_Models			0x8E
#define Spectrum_Engineering			0x50
#define SPROG_DCC                       0x2C
#define T4T_Technology_for_Trains_GmbH 	0x14
#define Tam_Valley_Depot				0x3B
#define Tams_Elektronik_GmbH			0x3E
#define Tawcrafts                       0x5C
#define TCH_Technology					0x36
#define Team_Digital_LLC				0x19
#define Tehnologistic_train_O_matic		0x4E
#define The_Electric_Railroad_Company	0x49
#define Throttle_Up_Soundtraxx			0x8D
#define Train_Control_Systems			0x99
#define Train_ID_Systems				0x8A
#define TrainModules                    0x3D
#define Train_Technology				0x02
#define TrainTech                       0x68
#define Trenes_Digitales				0x64
#define Trix_Modelleisenbahn			0x83
#define Uhlenbrock_GmbH					0x55
#define Umelec_Ing_Buero				0x93
#define Viessmann_Modellspielwaren_GmbH	0x6D
#define Wm_K_Walthers_Inc				0x96
#define W_S_Ataras_Engineering			0x77
#define Wangrow_Electronics				0x0C
#define Wekomm_Engineering_GmbH			0x56
#define WP_Railshops					0xA3
#define Zimo_Elektronik					0x91
#define ZTC								0x84
