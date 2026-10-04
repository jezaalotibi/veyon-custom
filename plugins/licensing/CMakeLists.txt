include(BuildVeyonPlugin)

build_veyon_plugin(licensing
	NAME Licensing
	SOURCES
	LicensingPlugin.cpp
	LicensingPlugin.h
	LicenseManager.cpp
	LicenseManager.h
	LicenseActivationDialog.cpp
	LicenseActivationDialog.h
	licensing.qrc
	)
