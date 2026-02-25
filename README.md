# ProPrj_PLT251129004125-esp32_WT02
EAP32 + SA818

src
	|-- App
		|-- Common
			|-- DataProc
				DataProc.cpp
				DataProc.h
				DataProc_def.h
				DataPro_Init.cpp
				DP_LIST.inc
				DP_SA818.cpp
			|-- HAL
				CommonMacro.h
				HAL.cpp
				HAL.h
				HAL_ARPS.cpp
				HAL_Button.cpp
				HAL_Clock.cpp
				HAL_Def.h
				HAL_I2C.cpp
				HAL_OTA.cpp
				HAL_Power.cpp
				HAL_SA818.cpp
				HAL_U8g2.cpp
				HAL_Wifi.cpp
				SA818_Channels.h
			|-- Port
				|-- lv_port
					lv_port_disp.cpp
					lv_port_indev.cpp
				Display.cpp
				Display.h
		|-- Configs
			Config.h
			Pinout.h
			Version.h
		|-- Pages
			|-- Radio
				Radio.cpp
				Radio.h
				RadioModel.cpp
				RadioModel.h
				RadioView.cpp
				RadioView.h
			|-- StartUp
				StartUp.cpp
				StartUp.h
				StartUpModel.cpp
				StartUpModel.h
				StartUpView.cpp
				StartUpView.h
			|-- StatusBar
				StatusBar.cpp
				StatusBar.h
			AppFactory.cpp
			AppFactory.h
			Page.h
		|-- Utils
			|-- ButtonEvent
				ButtonEvent.cpp
				ButtonEvent.h
				EventType.inc
			|-- DataCenter
				|-- PingPongBuffer
				Account.cpp
				Account.h
				DataCenter.cpp
				DataCenter.h
				DataCenterLog.h
			|-- OTA
				ota_config.h
				ota_updater.cpp
				ota_updater.h
				wifi_helper.cpp
				wifi_helper.h
			|-- PageManager
				PageBase.h
				PageFactory.h
				PageManager.h
				PM_Anim.cpp
				ResourceManager.cpp
				ResourceManager.h
			|-- WiFiManager
				wifi_manager.cpp
				wifi_manager.h
				wifi_manager_html.cpp
			FontSizeTest.h
			FontTest.h
			OLEDLayout.cpp
			OLEDLayout.h
		App.cpp
		App.h
	|-- test
	main.cpp