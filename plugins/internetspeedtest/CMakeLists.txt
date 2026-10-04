include(BuildVeyonPlugin)

build_veyon_plugin(internetspeedtest
	NAME InternetSpeedTest
	SOURCES
	InternetSpeedTestFeaturePlugin.cpp
	InternetSpeedTestFeaturePlugin.h
	InternetSpeedTestDialog.cpp
	InternetSpeedTestDialog.h
	internetspeedtest.qrc
	)
