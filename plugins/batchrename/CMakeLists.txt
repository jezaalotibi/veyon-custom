include(BuildVeyonPlugin)

build_veyon_plugin(batchrename
	NAME BatchRename
	SOURCES
	BatchRenameFeaturePlugin.cpp
	BatchRenameFeaturePlugin.h
	BatchRenameDialog.cpp
	BatchRenameDialog.h
	batchrename.qrc
	)
